/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * serialization/json/JsonUtils.cpp is part of Brewtarget, and is copyright the following authors 2021-2024:
 *   • Matt Young <mfsy@yahoo.com>
 *
 * Brewtarget is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * Brewtarget is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with this program.  If not, see
 * <http://www.gnu.org/licenses/>.
 ╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌*/
#include "serialization/json/JsonUtils.h"

#include <iostream>
#include <sstream>

// We could just include <boost/json.hpp> which pulls all the Boost.JSON headers in, but that seems overkill
#include <boost/json/parse_options.hpp>
#include <boost/json/parse.hpp>
#include <boost/json/string.hpp>
#include <boost/json/serialize.hpp>
#include <boost/json/stream_parser.hpp>

#include <QDebug>
#include <QFile>
#include <QString>

#include "utils/BtException.h"
#include "utils/BtStringStream.h"
#include "utils/ErrorCodeToStream.h"

[[nodiscard]] boost::json::value JsonUtils::loadJsonDocument(QString const & fileName, bool allowComments) {

   QFile inputFile(fileName);

   if (!inputFile.open(QIODevice::ReadOnly)) {
      // Some slight duplication here but there's value in having the log messages in English and the on-screen display
      // message in the user's preferred language
      qWarning() <<
         Q_FUNC_INFO << "Could not open " << fileName << " for reading (error #" << inputFile.error() << ":" <<
         inputFile.errorString() << ")";
      QString errorMessage{
         QObject::tr("Could not open %1 for reading (error # %2)").arg(fileName).arg(inputFile.error())
      };
      throw BtException(errorMessage);
   }

   qint64 fileSize = inputFile.size();
   if (fileSize <= 0) {
      BtStringStream errorMessage;
      errorMessage << "File " << fileName << " has no data (length is " << fileSize << " bytes)";
      qWarning() << Q_FUNC_INFO << errorMessage.asString();
      throw BtException(errorMessage.asString());
   }

   //
   // A few notes on how we do the parsing:
   //
   // Using a streaming parser
   // ------------------------
   // The simplest way to have Boost.JSON parse a document is to call boost::json::parse().  However, this doesn't
   // give you the best error handling.  In particular if there is a problem with the json input, you'll just get
   // a std::error_code that says, eg, "syntax error" without giving you any clue where in the input the problem is.
   //
   // So, instead, we need to create a streaming parser and give it the source one line at a time.  That way, if we
   // hit an error we can get the line number that first caused it.
   //
   // String encodings
   // ----------------
   // JSON files are UTF-8 encoded as required by RFC8259 (see 8.1 of https://datatracker.ietf.org/doc/html/rfc8259,
   // which says "JSON text exchanged between systems that are not part of a closed ecosystem MUST be encoded using
   // UTF-8").
   //
   // Internally we have a few options for storing strings:
   //    - QString is UTF-16 encoded but provides mechanisms for converting to and from UTF-8
   //    - std::string is 8-bit chars so it can be used to store UTF-8 but it doesn't understand that encoding (so, for
   //      example, std::string::length() will give incorrect answers on UTF-8 strings with multi-byte characters
   //      because std::string itself doesn't understand multi-byte characters).  Usually reading and writing UTF-8
   //      from/to std::string "just works" on Linux/Mac because the OS "does the right thing" for you.
   //    - In C++20 there is std::u8string, which is proper UTF-8 but there is not yet as much supporting infrastructure
   //      around this (eg for string processing and manipulation) -- see
   //      https://stackoverflow.com/questions/55556200/convert-between-stdu8string-and-stdstring
   //    - Boost.JSON has its own boost::json::string class that natively stores data in UTF-8 encoding.  However, it is
   //      not a fully-fledged UTF-8 string container, rather it has just enough features for the needs of JSON
   //      processing.
   //
   // We use boost::json::string and boost::json::string_view for dealing with actual JSON (reading or writing from a
   // file) and then convert to/from QString for bits of text that we store internally.
   //
   // The boost::json::string class has a similar interface and functionality to std::basic_string, with a few
   // differences, including that access to characters in the range (size(), capacity()) is permitted, including write
   // access via the data() member function.  However, we can't use this as an efficient way to read data directly into
   // the string, because modifying the character array returned by data() does not tell the string it has changed in
   // size, and increasing the size of the string results in data being overwritten.
   //
   // Instead, we use boost::json::string_view (which is an alias either for boost::string_view or std::string_view
   // depending on how the code is compiled) to put a lightweight wrapper around a buffer of char * that Qt can provide.
   //
   // References:
   //    - https://doc.qt.io/qt-6/qstring.html says
   //         "QString stores a string of 16-bit QChars, where each QChar corresponds to one UTF-16 code unit. (Unicode
   //          characters with code values above 65535 are stored using surrogate pairs, i.e., two consecutive QChars.)
   //          ...
   //          Behind the scenes, QString uses implicit sharing (copy-on-write) to reduce memory usage and to avoid the
   //          needless copying of data. This also helps reduce the inherent overhead of storing 16-bit characters
   //          instead of 8-bit characters.
   //          ...
   //          In addition to QString, Qt also provides the QByteArray class to store raw bytes and traditional 8-bit
   //          '\0'-terminated strings. For most purposes, QString is the class you want to use. It is used throughout
   //          the Qt API"
   //    - https://doc.qt.io/qt-5/qfile.html says
   //         "TextStream takes care of converting the 8-bit data stored on disk into a 16-bit Unicode QString. By
   //          default, it assumes that the user system's local 8-bit encoding is used (e.g., UTF-8 on most unix based
   //          operating systems; see QTextCodec::codecForLocale() for details). This can be changed using
   //          QTextStream::setCodec()".  HOWEVER, NOTE THAT, in Qt6, QTextCodec and QTextStream::setCodec have been
   //          removed and are replaced by QTextStream::setEncoding and QStringConverter (which are new in Qt6).
   //    - https://www.boost.org/doc/libs/1_80_0/libs/json/doc/html/json/ref/boost__json__string.html says
   //         "Instances of boost::json::string store and manipulate sequences of char using the UTF-8 encoding. The
   //          elements of a string are stored contiguously. A pointer to any character in a string may be passed to
   //          functions that expect a pointer to the first element of a null-terminated char array. String iterators
   //          are regular char pointers.
   //          ...
   //          boost::json::string member functions do not validate any UTF-8 byte sequences passed to them."
   //
   // Exceptions
   // ----------
   // If you give it an error code parameter, then Boost.JSON will report its errors via that rather than by throwing
   // std::system_error exception (or boost::system::system_error if linking with Boost).  However, even when using
   // error codes, std::bad_alloc exceptions thrown from the underlying memory_resource are still possible, so it's
   // good practice to catch and recast these.
   //
   // Character escaping
   // ------------------
   // Similarly, per https://www.boost.org/doc/libs/1_77_0/libs/json/doc/html/json/dom/string.html, when a
   // boost::json::string is formatted to a std::ostream, the result is a valid JSON. That is, the result will be double
   // quoted and the contents properly escaped per the JSON specification.
   //
   try {
      std::error_code errorCode;

      boost::json::parse_options parseOptions;
      parseOptions.allow_comments = allowComments;
      boost::json::stream_parser streamParser{
         boost::json::storage_ptr{}, // Default memory resource
         parseOptions,               // Default parse options (strict parsing)
      };
      QByteArray rawInputLine{};
      for (auto [lineNumber, bytesLeftToRead] = std::tuple{1, fileSize};
         bytesLeftToRead > 0;
         ++lineNumber, bytesLeftToRead -= rawInputLine.size()) {
         // Because of the way UTF-8 is encoded (see eg https://www.johndcook.com/blog/2019/09/09/how-utf-8-works/), it
         // is entirely valid to treat it as an ASCII file for many purposes, including "give me a line of text".
         rawInputLine = inputFile.readLine();
         boost::json::string_view inputStringView{rawInputLine.data()};
         streamParser.write(inputStringView, errorCode);
         if (errorCode) {
            BtStringStream errorMessage{};
            errorMessage << "Parsing failed at line " << lineNumber << ": " << errorCode;
            qWarning() << Q_FUNC_INFO << errorMessage.asString();
            throw BtException(errorMessage.asString());
         }
      }

      streamParser.finish(errorCode);
      if (errorCode) {
         BtStringStream errorMessage;
         errorMessage << "Parsing failed after reading last line: " << errorCode;
         qWarning() << Q_FUNC_INFO << errorMessage.asString();
         throw BtException(errorMessage.asString());
      }
      boost::json::value parsedDocument = streamParser.release();

      return parsedDocument;
   } catch (std::bad_alloc const & exception) {
      // Not sure that there's a concise and user-friendly way to describe a memory allocation exception, but at
      // least we can give the user something semi-meaningful to report to a maintainer.
      BtStringStream errorMessage;
      errorMessage << "Memory allocation error (" << exception.what() << ") while parsing " << fileName;
      qWarning() << Q_FUNC_INFO << errorMessage.asString();
      throw BtException(errorMessage.asString());
   }
}

void JsonUtils::serialize(std::ostream & stream,
                          boost::json::value const & val,
                          std::string_view const tabString,
                          std::string* currentIndent) {
   // If indentation is 0 then we just want Boost.JSON native serialisation, ie without any additional spaces or
   // newlines
   if (tabString.length() == 0) {
      stream << val;
      return;
   }

   //
   // The rest of this function is heavily inspired by
   // https://www.boost.org/doc/libs/1_80_0/libs/json/doc/html/json/examples.html#json.examples.pretty
   //
   std::string initialIndent;
   if (!currentIndent) {
      currentIndent = &initialIndent;
   }

   //
   // We only need special processing for objects and arrays.  We could just call boost::json::serialize or operator<<
   // for other sorts of values, but mostly it's slightly more efficient to serialise them directly.
   //
   // Note, per the comment in JsonRecord::toJson, that we want to skip over the output of any key:value pair where the
   // value is an empty object, as it would likely cause parse errors when the document is validated.
   //
   switch(val.kind()) {
      case boost::json::kind::object:
      {
         stream << "{\n";
         currentIndent->append(tabString);
         auto const & obj = val.get_object();
         if (!obj.empty()) {
            bool firstWritten = false;
            for (auto ii = obj.begin(); ii != obj.end(); ++ii) {
               //
               // Skip over key:value output when value is empty object
               //
               if (ii->value().kind() == boost::json::kind::object &&
                   ii->value().get_object().size() == 0) {
                  qDebug() << Q_FUNC_INFO << "Skipping output of empty object for" << ii->key();
                  continue;
               }

               if (firstWritten) {
                  stream << ",\n";
               }
               stream << *currentIndent;
               // Per https://www.boost.org/doc/libs/1_80_0/libs/json/doc/html/json/dom/object.html, an object's key is
               // a boost::json::string_view and its value is a boost::json::value
               //
               // Almost all the time, it would be absolutely fine to just write out the key (inside quotes) directly
               // (because boost::json::string_view type "has API equivalent to ...  std::string_view").  However, it is
               // technically legal (albeit usually inadvisable) for a JSON key to include special characters (", \, \n,
               // \t, etc) which need to be escaped, and we don't want to reinvent the wheel for such escaping.
               stream << boost::json::serialize(ii->key());
               // Some people like a space before the : and some don't.  Both are valid.  The examples at
               // http://json-schema.org/understanding-json-schema/reference/object.html omit them, so we go with that.
               stream << ": ";
               JsonUtils::serialize(stream, ii->value(), tabString, currentIndent);
               firstWritten = true;
            }
         }
         stream << "\n";
         currentIndent->resize(currentIndent->size() - tabString.length());
         stream << *currentIndent << "}";
         break;
      }

      case boost::json::kind::array:
      {
         stream << "[\n";
         currentIndent->append(tabString);
         auto const & arr = val.get_array();
         if (!arr.empty()) {
            for (auto ii = arr.begin(); ii != arr.end(); ++ii) {
               if (ii != arr.begin()) {
                  stream << ",\n";
               }
               stream << *currentIndent;
               JsonUtils::serialize(stream, *ii, tabString, currentIndent);
            }
         }
         stream << "\n";
         currentIndent->resize(currentIndent->size() - tabString.length());
         stream << *currentIndent << "]";
         break;
      }

      case boost::json::kind::string:
         // This is a case where we do want to use the Boost.JSON operator<< rather than, say val.get_string().c_str(),
         // because any newlines or other special characters in the string need to be escaped.
         stream << val;
         break;

      case boost::json::kind::uint64:
         stream << val.get_uint64();
         break;

      case boost::json::kind::int64:
         stream << val.get_int64();
         break;

      case boost::json::kind::double_:
         stream << val.get_double();
         break;

      case boost::json::kind::bool_:
         stream << (val.get_bool() ? "true" : "false");
         break;

      case boost::json::kind::null:
         stream << "null";
         break;

      // NB: Deliberately no default case.  boost::json::kind is a strongly-typed enum, so we'll get a compiler warning
      // if we haven't explicitly handled every possible value above.
   }

   if (currentIndent->empty()) {
      stream << "\n";
   }

   return;
}

template<class S>
S & operator<<(S & stream, boost::json::kind const knd) {
   std::ostringstream output;
   output << "boost::json::kind::" << knd;
   stream << output.str().c_str();
   return stream;
}

//
// Instantiate the above template function for the types that are going to use it
// (This is all just a trick to allow the template definition to be here in the .cpp file and not in the header, so we
// don't have to pull in std::ostringstream headers in other parts of the code.)
//
template QDebug & operator<<(QDebug & stream, boost::json::kind const knd);
template QTextStream & operator<<(QTextStream & stream, boost::json::kind const knd);

template<class S,
         std::enable_if_t<(std::is_same<QDebug, S>::value || std::is_same<QTextStream, S>::value), bool> >
S & operator<<(S & stream, boost::json::value const & val) {
   // Boost.JSON already handles output to standard library output streams, so we are just piggy-backing on this to
   // provide the same output in the Qt output streams we care about.
   std::ostringstream output;

   // Following line can sometimes be helpful to uncomment for debugging but NB it will break things that are relying on
   // serialization producing valid JSON
//   output << "(" << val.kind() << "): ";

   JsonUtils::serialize(output, val, "  ");
   stream << output.str().c_str();
   return stream;
}

template QDebug & operator<<(QDebug & stream, boost::json::value const & val);
template QTextStream & operator<<(QTextStream & stream, boost::json::value const & val);
