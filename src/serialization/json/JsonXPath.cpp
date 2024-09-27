/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * serialization/json/JsonXPath.cpp is part of Brewtarget, and is copyright the following authors 2022-2024:
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
#include "serialization/json/JsonXPath.h"

// Uncomment this to use old-school debugging "logs" in constructor!  See NOTE below in constructor comments.
//#include <iostream>

#include <regex>

#include <boost/algorithm/string/classification.hpp> // For boost::algorithm::is_any_of
#include <boost/algorithm/string/split.hpp>
#include <boost/json/array.hpp>
#include <boost/json/object.hpp>

#include <QDebug>
#include <QtGlobal> // For Q_ASSERT

#include "serialization/json/JsonUtils.h"

JsonXPath::JsonXPath(char const * const xPath) :
   m_rawXPath{xPath},
   m_pathParts{},
   m_pathNodes{} {

   // Because we're using std::string (because it's easier to pass in and out of Boost.JSON), we use the C++ standard
   // regular expressions library rather than QRegExp here.  (The latter is obviously a better choice when we're
   // manipulating QString objects.)

   //
   // We could, in principle, use std::cregex_iterator on the raw char * string, but it's a bit clunky because you need
   // a pointer to the end of the string in order to construct the iterator.  It's simpler just to convert to
   // std::string and follow the example at https://en.cppreference.com/w/cpp/regex/regex_iterator
   //
   // Moreover, we also want to prepend the initial '/' that JSON Pointers need.  (I assert that the first bit of a
   // JsonXPath is always going to be a JSON Pointer and not a named array item identifier.)
   //
   std::string const xPathAsString{std::string{"/"}.append(xPath)};

   //
   // If we're constructed with an empty XPath, we need to bail out here.
   //
   if (xPathAsString.length() == 1) {
      return;
   }

   //
   // This regexp should match either of the following:
   //    - JSON Pointer: '/' followed by any number of characters that are not '['
   //    - Named array item identifier: '[' followed by any number of characters that are not ']' followed by ']'
   //
   // So, eg, if xPath is "foo/bar/fruit[type=\"Banana\"]/hum/bug", then xPathAsString will be
   // "/foo/bar/fruit[type=\"Banana\"]/hum/bug" and we should get three matches:
   //    - "/foo/bar/fruit"
   //    - "[type=\"Banana\"]"
   //    - "/hum/bug"
   //
   // Note that we rely on the '+' regexp operator being available and greedy (ie the match is as long as possible) aka
   // "leftmost longest".  (I _think_ it is always greedy, but I know it is not always available.)  You can find out
   // which regular expression standards (of course there are loads of different ones!) support greedy '+' at
   // https://www.regular-expressions.info/refrepeat.html.  TLDR: "POSIX extended" regular expressions are one of the
   // types that work the way we want, so that's what we specify.
   //
   static std::regex const pathParts_regex{"/[^[]+|[[][^]]+[]]", std::regex_constants::extended};

   // The start and end points for a regex iterator
   auto const xPath_begin = std::sregex_iterator(xPathAsString.begin(), xPathAsString.end(), pathParts_regex);
   auto const xPath_end   = std::sregex_iterator();

   // NOTE: This constructor is typically called early in program start-up, long before logging is initialised.   So, if
   //       you want to log out diagnostic messages for debugging, you have to log to std::cout.  Uncomment
   //       `#include <iostream>` above and uncomment the `std::cout << ...` statements.
//   std::cout <<
//      "***** Parsed " << xPathAsString << " and found " << std::distance(xPath_begin, xPath_end) << " path part(s)\n";

   for (std::sregex_iterator ii = xPath_begin; ii != xPath_end; ++ii) {
      std::string pathPart = ii->str();
      // Normally leave this logging statement commented out as otherwise it's fills up too much of the log files
//      qDebug() << Q_FUNC_INFO << "Matched" << pathPart.c_str();
      if (pathPart[0] == '/') {
         // This is the easy case
         this->m_pathParts.push_back(pathPart);

         //
         // Now we split the JSON Pointer into its constituent keys
         //
         // If we were using QString then QString::split would help us here, but we need std::string_view for
         // Boost.JSON so std::string is a more helpful starting point.
         //
         // In C++20, it should also be possible to do this with std::ranges::split_view, but I haven't yet managed to
         // get gcc to compile even example code.  So, for now at least, we use Boost.
         //
         // Don't think we need the optional boost::algorithm::token_compress_on final parameter as we never have "//"
         // in the paths.
         //
         // Note that splitting on '/' for a "/dog/cat" string is going to give us "", "dog", "cat", so we just discard
         // any empty string.  This is OK as there is never a case where two successive slashes are valid in an XPath,
         // ie we never see anything along the lines of "/dog//cat".
         //
         std::vector<std::string> jpKeys{};
         boost::algorithm::split(jpKeys, pathPart, boost::algorithm::is_any_of("/"));
         for (auto const & jpKey : jpKeys) {
            if (!jpKey.empty()) {
               this->m_pathNodes.push_back(jpKey);
            }
         }

      } else {
         Q_ASSERT(pathPart[0] == '['); // Coding error if this is not true

         // Now we have to split the named array item identifier (eg "[type=\"Banana\"]") into its constituent parts.
         // Since we're having so much fun with regular expressions, it's time for another one.

         //
         // This regex has two sub-expressions:
         //    1) "Key": matches the text between '[' and '='
         //    2) "Value": matches the text between the quotes (or actually between "=\"" and "\"]")
         //
         // Note that std::regex_constants::extended means we use "(" and ")" (rather than "\(" and "\)") for the
         // capturing groups.  See https://www.regular-expressions.info/refcapture.html for more.
         //
         static std::regex const namedArrayItemId_regex{"[[]([^=]+)=\"(.+)\"[]]", std::regex_constants::extended};
         std::smatch namedArrayItemId_match;
         bool const matched = std::regex_match(pathPart, namedArrayItemId_match, namedArrayItemId_regex);
         // It's a coding error if we didn't get a match
         Q_ASSERT(matched);
         // It's a coding error if we didn't match exactly two sub-expressions.  Note that size() gives us "number of
         // marked subexpressions plus 1".
         Q_ASSERT(namedArrayItemId_match.size() == 3);
         std::string key   = namedArrayItemId_match[1].str();
         std::string value = namedArrayItemId_match[2].str();
         // See comment above about logging!
//         std::cout << "***** Key: " << key << ", value: " << value << "\n";
         JsonXPath::NamedArrayItemId namedArrayItemId{key, value};
         this->m_pathParts.push_back(namedArrayItemId);
         // A named array item identifier doesn't need any further splitting, so it goes as-is onto the list of path
         // nodes.
         this->m_pathNodes.push_back(namedArrayItemId);
      }
   }

   // It's a coding error if there were no path parts!
   Q_ASSERT(this->m_pathParts.size() > 0);

   // It's a coding error if the first path part is not a JSON Pointer.  This is because a named array item identifier
   // is meaningless without a preceding array name.
   Q_ASSERT(std::holds_alternative<JsonXPath::JsonPointer>(this->m_pathParts.front()));

   // Belive it or not, it's also a coding error if the last path part is not a JSON Pointer.  This is because a named
   // array item identifier gets us to a specific object in an array but it doesn't identify any field of the object to
   // read or write, so it's always followed by something else, viz JSON Pointer.
   Q_ASSERT(std::holds_alternative<JsonXPath::JsonPointer>(this->m_pathParts.back()));

   // Similar assertions apply for path nodes
   Q_ASSERT(this->m_pathNodes.size() > 0);
   Q_ASSERT(std::holds_alternative<JsonXPath::JsonKey>(this->m_pathNodes.front()));
   Q_ASSERT(std::holds_alternative<JsonXPath::JsonKey>(this->m_pathNodes.back()));

   return;
}

JsonXPath::~JsonXPath() = default;

bool JsonXPath::isEmpty() const {
   return 0 == this->m_pathParts.size();
}

boost::json::value const * JsonXPath::followPathFrom(boost::json::value const * startingValue,
                                                     std::error_code & errorCode) const {
   // It's a coding error to call with a null pointer
   Q_ASSERT(startingValue);
   // Note that if this->isEmpty(), the outer for loop does not execute (because this->m_pathParts.size() == 0) and we
   // return startingValue, which is the desired behaviour.
   boost::json::value const * destinationValue = startingValue;
   for (auto const & pathPart : this->m_pathParts) {
      if (std::holds_alternative<JsonXPath::JsonPointer>(pathPart)) {
         // For a JSON Pointer, Boost.JSON does all the work
         auto jsonPointer{std::get<JsonXPath::JsonPointer>(pathPart)};
         // Normally have this commented out as it generates lots of logging
//         qDebug() <<
//            Q_FUNC_INFO << "Following path part" << jsonPointer.c_str() << "from" << *destinationValue << "in" <<
//            this->m_rawXPath;
         destinationValue = destinationValue->find_pointer(std::string_view{jsonPointer}, errorCode);
         // If we already know there's no result, stop looping through the path parts
         // This is not an error per se, just that nothing was found
         if (!destinationValue) {
            qDebug() <<
               Q_FUNC_INFO << "No result from" << jsonPointer.c_str() << "while following" << this->m_rawXPath;
            // std::error_code usually holds an implementation-defined value, but we can use POSIX error codes.
            // Here I'm taking a liberal interpretation of the Posix "bad address" (EFAULT) code.  It was a toss up
            // between that and "invalid argument" (EINVAL).
            errorCode = std::make_error_code(std::errc::bad_address);
            return nullptr;
         }
      } else {
         Q_ASSERT(std::holds_alternative<JsonXPath::NamedArrayItemId>(pathPart));

         // For a named array item identifier, we have to do things by hand
         auto const & namedArrayItemId = std::get<JsonXPath::NamedArrayItemId>(pathPart);

         // Firstly, the current value has better be an array
         if (!destinationValue->is_array()) {
            qWarning() <<
               Q_FUNC_INFO << "Following" << this->m_rawXPath << "resulted in trying to apply" << namedArrayItemId <<
               "to" << destinationValue->kind();
            errorCode = std::make_error_code(std::errc::bad_address);
            return nullptr;
         }

         // Now we loop through the array, examining elements until we find a match.  (We only care about the first
         // match as, in our use cases, we are not expecting multiple matches and cannot usefully interpret them.)
         bool foundInArray = false;
         boost::json::array const & destinationValueAsArray = destinationValue->get_array();
         qDebug() <<
            Q_FUNC_INFO << "Searching through" << destinationValueAsArray.size() << "array items for" <<
            namedArrayItemId;
         //
         // Note that we do not use the range for (eg `for (auto arrayEntry : destinationValueAsArray)`) because we want
         // arrayEntry to be a pointer to constant values and we don't want any copying going on.
         //
         for (boost::json::value const * arrayEntry  = destinationValueAsArray.cbegin();
                                         arrayEntry != destinationValueAsArray.cend(); ++arrayEntry) {
            // The elements of the array had better be objects (ie associative containers holding key and value pairs)
            // Technically, according to https://json-schema.org/, in a JSON array "each element in an array may be of a
            // different type".  So we could just skip over the current element if it's not an object.  However, in our
            // use cases, we do not have heterogeneous arrays, so it's better to barf up an error straight away.
            auto arrayEntryAsObject = arrayEntry->if_object();
            if (!arrayEntryAsObject) {
               qWarning() <<
                  Q_FUNC_INFO << "While following" << this->m_rawXPath << "found" << arrayEntry->kind() <<
                  "when applying" << namedArrayItemId;
               errorCode = std::make_error_code(std::errc::bad_address);
               return nullptr;
            }

            // Again, in theory, we could just skip over any object that doesn't have the key we want to look up, but
            // our data doesn't have such cases, so we just error straight away.
            auto value = arrayEntryAsObject->if_contains(std::string_view{namedArrayItemId.key});
            if (!value) {
               qWarning() <<
                  Q_FUNC_INFO << "While following" << this->m_rawXPath << "found array entry without correct key "
                  "when applying" << namedArrayItemId;
               errorCode = std::make_error_code(std::errc::bad_address);
               return nullptr;
            }

            // You get the idea by now... :-)
            auto valueAsString = value->if_string();
            if (!valueAsString) {
               qWarning() <<
                  Q_FUNC_INFO << "While following" << this->m_rawXPath << "found array entry without non-string value "
                  "for key (" << value->kind() << ") when applying" << namedArrayItemId;
               errorCode = std::make_error_code(std::errc::bad_address);
               return nullptr;
            }

            if (*valueAsString == namedArrayItemId.value) {
               // It isn't normally necessary to enable the next log statement
//               qDebug() << Q_FUNC_INFO << "Found" << valueAsString->c_str();
               destinationValue = arrayEntry;
               foundInArray = true;
               break;
            }

            qDebug() <<
               Q_FUNC_INFO << "Skipping" << valueAsString->c_str() << "while searching for" << namedArrayItemId <<
               "as part of" << this->m_rawXPath;
         }

         if (!foundInArray) {
            // It's not necessarily an error if we didn't find the thing we were looking for.  It might be optional.
            qDebug() << Q_FUNC_INFO << "No match found for" << namedArrayItemId << "when following" << this->m_rawXPath;
            errorCode = std::make_error_code(std::errc::bad_address);
            return nullptr;
         }
      }
   }
   return destinationValue;
}

boost::json::value * JsonXPath::followPathFrom(boost::json::value * startingValue,
                                               std::error_code & errorCode) const {
   return const_cast<boost::json::value *>(
      this->followPathFrom(const_cast<boost::json::value const*>(startingValue),
                           errorCode)
   );
}

std::string JsonXPath::makePointerToLeaf(boost::json::value ** valuePointer) const {
   //
   // Start with the special case of the empty XPath, in which case we want valuePointer to be unchanged
   //
   if (this->isEmpty()) {
      qDebug() << Q_FUNC_INFO << "Empty XPath";
      return "";
   }

   //
   // What we're trying to do is create all the "nodes" except the last one in the path.  Eg, if we have a path
   // "/foo/bar/fruit[type=\"Banana\"]/fish/face", we want to ensure the following structure exists:
   //
   //    "foo": {
   //       "bar": {
   //          "fruit": [
   //             { ... },
   //             ...
   //             {
   //                 "type": "Banana"
   //                 "fish": {
   //                 }
   //             },
   //             ...
   //             { ... }
   //          ]
   //       }
   //    }
   // Then we set the object pointer to the "fish" object and return "face" as the key for the caller to use.  (Leaving
   // this last bit for the caller saves us from having to deal with all the possibilities of what value might be.  I
   // think this class is already complicated enough!)
   //
   // Looping over everything in the path is straightforward, as the list of "path nodes" gives us the right
   // granularity:
   //    { "foo", "bar", "fruit", "[type=\"Banana\"]", "fish", "face" }
   //
   // What we need to be careful about is ensuring that the work of creating any required empty objects/arrays happens
   // at the right time.  Essentially the right time is mostly just as we're about to process the _next_ item in the
   // loop.  There are actually 3 cases:
   //
   //    1) Node follows Node.  Eg when we get to "bar", we know we should create object "foo" if it doesn't yet exist.
   //       At this point:
   //          - currentNode   = "bar"
   //          - priorNode     = "foo"
   //          - previousValue = the JSON value in which "foo" should exist (ie the one passed in as valuePointer)
   //       Similarly, when we get to "face", we know we should create object "fish" if it doesn't yet exist.
   //       At this point:
   //          - currentNode   = "face"
   //          - priorNode     = "fish"
   //          - previousValue = the JSON value in which "fish" should exist, which is the object in the "fruit" array
   //                            in which type=Banana
   //
   //    2) Named Array Item Id follows Node.  Eg when we get to "[type=\"Banana\"]" we know we need to create array
   //       "fruit" if it doesn't exist yet.
   //       At this point:
   //          - currentNode   = "[type=\"Banana\"]"
   //          - priorNode     = "fruit"
   //          - previousValue = the JSON value for "bar" (in which "foo" should exist)
   //
   //    3) Node follows Named Array Item Id.  Eg when we get to "fish" we know we need to create a new element of
   //       "fruit" with type=Banana.
   //          - currentNode   = "fish"
   //          - priorNode     = "[type=\"Banana\"]"
   //          - previousValue = the JSON value for "fruit" (in which the array with an entry of "type" : "Bananan"
   //                            should exist)
   //
   // Note that:
   //    a) We never have Named Array Item Id follows Named Array Item Id;
   //    b) A JsonXPath can never end in a named array item identifier;
   // because neither of these would make a lot of sense.
   //
   // As we loop round performing the above algorithm, we're actually keeping track of three "levels" of the JSON tree.
   // Broadly speaking we can think of it that previousValue contains priorNode contains currentNode.  It's just that
   // previousValue points to the actual JSON tree node, where as priorNode and currentNode are the names or identifiers
   // to get to subsequent nodes in the tree.
   //
   // Note that previousValue starts out as the containing JSON value.  In the example above, when we get to "bar",
   // we'll check "foo" exists in this JSON value (creating it if necessary) and then move previousValue to point to
   // "foo".  So, the first time through the loop (when priorNode is null), previousValue doesn't change.
   //
   //
   // We are probably doing more string copying here than we need to, but the strings are short and I am not (yet)
   // worried about a need to optimise this.
   //
   // TBD: We might be able to do something clever with boost::json::value::set_at_pointer, but I didn't work out how
   // (or rather I couldn't see a way that the net result would be simpler than our approach below).
   //
   boost::json::value * previousValue = *valuePointer;
   JsonXPath::PathNode priorNode{};
   for (auto const & currentNode : this->m_pathNodes) {
      // It's a coding error for any node other than the initial "previous node" to be null
      Q_ASSERT(!std::holds_alternative<std::monostate>(currentNode));

      //
      // First we look at the previous path node, if there was one, to see if a JSON value needs creating for it
      //
      if (!std::holds_alternative<std::monostate>(priorNode)) {
         if (std::holds_alternative<JsonXPath::JsonKey>(priorNode)) {
            auto const & previousKey{std::get<JsonXPath::JsonKey>(priorNode)};
            qDebug() << Q_FUNC_INFO << "previousKey:" << previousKey.c_str();
            Q_ASSERT(previousValue->is_object());
            boost::json::value * currentValue = previousValue->get_object().if_contains(previousKey);
            if (!currentValue) {
               // Previous key does not exist, so we need to create an entry for it.  The type depends on the _current_
               // key (see examples in comment above)
               if (std::holds_alternative<JsonXPath::JsonKey>(currentNode)) {
                  // This is case (1) Node follows Node
                  qDebug() << Q_FUNC_INFO << "Making sub-object for" << previousKey.c_str();
                  previousValue->get_object()[previousKey].emplace_object();
               } else {
                  // This is case (2) Named Array Item Id follows Node
                  Q_ASSERT(std::holds_alternative<JsonXPath::NamedArrayItemId>(currentNode));
                  qDebug() << Q_FUNC_INFO << "Making sub-array for" << previousKey.c_str();
                  previousValue->get_object()[previousKey].emplace_array();
               }
               // The previous key should now exist!
               currentValue = previousValue->get_object().if_contains(previousKey);
               Q_ASSERT(currentValue);
            }
            previousValue = currentValue;
         } else {
            Q_ASSERT(std::holds_alternative<JsonXPath::NamedArrayItemId>(priorNode));
            auto const & namedArrayItemId{std::get<JsonXPath::NamedArrayItemId>(priorNode)};
            qDebug() <<
               Q_FUNC_INFO << "namedArrayItemId.key:" << namedArrayItemId.key.c_str() << ", namedArrayItemId.value:" <<
               namedArrayItemId.value.c_str();
            // As noted above, we cannot have Named Array Item Id follows Named Array Item Id, so we assert that here
            Q_ASSERT(std::holds_alternative<JsonXPath::JsonKey>(currentNode));
            //
            // In this case, previousValue points to the array holding priorNode.  We need to search through the
            // array to find an entry holding the right key:value entry
            //
            Q_ASSERT(previousValue->is_array());
            bool found = false;
            for (auto & item : previousValue->get_array()) {
               // For our purposes, we are only expecting array entries to be JSON objects (aka associative arrays).
               // Because this is data we are creating/writing, we can confidently say it's a coding error if we find
               // something else.
               Q_ASSERT(item.is_object());

               // Similarly, we are not expecting to find any entries in the array that do not have the key of the
               // key:value pair for which we are searching.
               boost::json::value * itemIdAsValue = item.get_object().if_contains(namedArrayItemId.key);
               Q_ASSERT(itemIdAsValue);

               // And the value of the key:value pair must be a string, for similar reasons (viz that we know we do not
               // create any other type of data in this circumstance.
               auto const * itemIdAsString = itemIdAsValue->if_string();
               Q_ASSERT(itemIdAsString);

               if (*itemIdAsString == namedArrayItemId.value) {
                  //
                  // Now that we found the array entry, we need to move the "previous item" pointer to it
                  //
                  previousValue = &item;
                  found = true;
                  break;
               }

               qDebug() <<
                  Q_FUNC_INFO << "Skipping array entry with" << namedArrayItemId.key.c_str() << "=" <<
                  itemIdAsString->c_str() << "while searching for" << namedArrayItemId.value.c_str();
            }

            if (!found) {
               // This is case (3) Node follows Named Array Item Id
               qDebug() <<
                  Q_FUNC_INFO << "Creating new array element with" << namedArrayItemId.key.c_str() << "=" <<
                  namedArrayItemId.value.c_str();
               // We're letting BoostJSON do all the constructor calling as we want it to own the objects being made.
               boost::json::value & insertedObjectAsValue =
                  previousValue->get_array().emplace_back(boost::json::object_kind);
               boost::json::object * insertedObject = insertedObjectAsValue.if_object();
               Q_ASSERT(insertedObject);
               insertedObject->emplace(namedArrayItemId.key, namedArrayItemId.value);
               // Now that we created the array entry, we need to move the "previous item" pointer to it
               previousValue = &insertedObjectAsValue;
            }
         }
      }

      //
      // Now we need to move the pointers from the previous to the current node, ready either for the next cycle round
      // the loop or the end of the loop.  We will already have handled previousValue above (because it doesn't change
      // on the first time through the loop).  We just need to handle priorNode.
      //
      priorNode = currentNode;
   }

   // Pass back where we ended up
   *valuePointer = previousValue;

   // As discussed above, it's a coding error if we didn't end up in a JSON Pointer which must, by definition, have at
   // least one key.
   Q_ASSERT(std::holds_alternative<JsonXPath::JsonKey>(priorNode));
   return std::get<JsonXPath::JsonKey>(priorNode);
}

std::string JsonXPath::asKey() const {
   // It's a coding error if there is more than one path part
   Q_ASSERT(this->m_pathParts.size() == 1);

   // It's also a coding error if there is more than one path node
   Q_ASSERT(this->m_pathNodes.size() == 1);

   // We need to get the first path part (m_pathParts[0]) and then strip the beginning '/' character from it
   // (.substr(1)) before we pass it to the string_view constructor.  (Maybe there is a slick way to skip over the '/'
   // in the std::string constructor, but I didn't yet find it.)
   return std::get<JsonXPath::JsonKey>(this->m_pathParts[0]).substr(1);
}

char const * JsonXPath::asXPath_c_str() const {
   return this->m_rawXPath;
}
