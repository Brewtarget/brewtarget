/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * serialization/json/JsonUtils.h is part of Brewtarget, and is copyright the following authors 2021-2022:
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
#ifndef SERIALIZATION_JSON_JSONUTILS_H
#define SERIALIZATION_JSON_JSONUTILS_H
#pragma once

#include <boost/json/value.hpp>

class QDebug;
class QString;
class QTextStream;

namespace JsonUtils {
   /**
    * \brief Loads a JSON document from the supplied file path and parses it into a tree of Boost.JSON objects, the root
    *        of which can be accessed via the \c getParsedDocument() member function
    *
    * \param fileName is either the absolute path to a file on local storage, or the path (or alias) of a resource
    *                 packaged with the program
    *
    * \param allowComments Strictly, JSON documents are not allowed to contain comments.  In reality, it is sometimes
    *                      useful to have them.  Turning this option on will cause C/C++-style comments in the document
    *                      being opened to be ignored (rather than generate an error).
    *
    * \throw BtException containing text that can be displayed to the user
    */
   [[nodiscard]] boost::json::value loadJsonDocument(QString const & fileName, bool allowComments = false);

   /**
    * \brief Output a \c boost::json::value to a stream as nicely formatted valid JSON.  Essentially adds nice
    *        formatting (aka pretty printing) to \c boost::json::serialize
    *
    *        By default, Boost.JSON outputs JSON that takes minimal space but isn't very readable.  JSON isn't a
    *        complicated standard, so making it readable is a small algorithm.
    *
    * \param stream The stream to output to
    * \param val    The value to serialise
    * \param tabString What indentation to use.  Typically we would pass in two or three spaces ("  " or "   "), but
    *                  should work even for heresies such as "        " and "\t".
    *                  Set to "" to turn off all indentation and line breaks (ie Boost.JSON native serialisation).
    * \param currentIndent Allows the function to call itself recursively, keeping track of the current indent level.
    *                      Normally the initial call should omit this parameter so it defaults to \c nullptr.
    */
   void serialize(std::ostream & stream,
                  boost::json::value const & val,
                  std::string_view const tabString = "",
                  std::string * currentIndent = nullptr);
}

/**
 * \brief Convenience function for logging.  (Boost JSON includes output to std::ostream, but we need to be able to
 *        output to \c QDebug and \c QTextStream
 */
template<class S>
S & operator<<(S & stream, boost::json::kind const k);

/**
 * \brief Convenience function for logging & serialisation.  (Boost JSON includes output to std::ostream, but we need to
 *        be able to output to \c QDebug and \c QTextStream.)
 *
 *        Note lots of things can be implicitly converted to boost::json::value, which can give a lot of false matches
 *        to this function if we're not careful.
 */
template<class S,
         std::enable_if_t<(std::is_same<QDebug, S>::value || std::is_same<QTextStream, S>::value), bool> = true>
S & operator<<(S & stream, boost::json::value const & val);



#endif
