/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * serialization/json/JsonXPath.h is part of Brewtarget, and is copyright the following authors 2022-2023:
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
#ifndef SERIALIZATION_JSON_JSONXPATH_H
#define SERIALIZATION_JSON_JSONXPATH_H
#pragma once

#include <string>
#include <string_view>
#include <variant>
#include <vector>

#include <boost/json/value.hpp>

/**
 * \brief \c JsonXPath is, essentially, almost the same as a JSON Pointer (see
 *        https://datatracker.ietf.org/doc/html/rfc6901) with the exception that the leading '/' character is optional.
 *
 *        Essentially, this gives us something very akin to the simpler parts of XML's XPath.  We add a few other bits
 *        (see below) but we do not try to replicate the full XPath functionality -- just the small subset that we need.
 *
 *        We have a couple of motivations for omitting the leading '/' character and one motivation for using a rather
 *        different name (XPath rather than Pointer).
 *
 *        JSON Pointers are defined in terms of a JSON document, but any non-leaf node in a JSON document tree can be
 *        treated as a JSON document (at least for the purposes of navigation) and, eg, Boost.JSON supports using a JSON
 *        Pointer as a relative reference from any node.  So, when we are dealing with relative paths in the document
 *        tree, the leading '/' on a JSON Pointer can sometimes be a bit confusing.
 *
 *        Just as importantly, where a relative JSON Pointer refers to a key directly inside the current object, we'd
 *        like it to have the same syntax as accessing that key directly.  Eg, suppose the node we are looking at in a
 *        JSON document includes the following:
 *           "name": "Super Hops",
 *           "origin": "Planet Krypton",
 *           "alpha_acid": {
 *             "unit": "%",
 *             "value": 4.5
 *           }
 *        We would like to be able to refer to "name", "origin", "alpha_acid", "alpha_acid/unit" and "alpha_acid/value".
 *        We don't want to have to distinguish between "/name" and "name" depending on whether we are accessing that
 *        property via key:value pair or JSON Pointer.  (Of course, we could just put a '/' at the front of everything,
 *        but it seems redundant, especially as, in reality, >90% of the references we make are to direct children of
 *        the current node.
 *
 *        We prefer XPath over Pointer because the former is unambiguous (and has a valid strong analogy with a file
 *        system path).
 *
 *        NOTE: Somewhat after writing this I discovered JSONPath at https://goessner.net/articles/JsonPath/index.html,
 *              which has a lot richer functionality than JSON Pointers but is only one of many competing proposals for
 *              querying JSON (including the jq tool at https://jqlang.github.io/jq/).  These are interesting (eg
 *              `equipment_items[?(@.form=="Mash Tun")]` gives the element(s) of the equipment_items array that have a
 *              form field equal to "Mash Tun"), but they are not currently supported by Boost.JSON.  So, for now, we
 *              we continue with our wrapper around JSON Pointer.
 *
 *        \b Additions: There are certain places where we need to be able to pick out a named item in an array, eg
 *        because of the way BeerJSON stores equipment.  The XPath format for, eg, accessing the entry in an
 *        "equipment_items" array whose "form" field is "Mash Tun", is `equipment_items[form="Mash Tun"]`.  We implement
 *        this part of XPath too, but in the simplest way possible.
 */
class JsonXPath {
public:
   /**
    * \brief Constructor
    */
   JsonXPath(char const * const xPath);
   ~JsonXPath();

   /**
    * \brief This is equivalent to `strlen(asXPath_c_str()) == 0`
    *
    *        Per the comment for JsonRecordDefinition::FieldType::Record, there is one circumstance where we need an
    *        "empty" XPath (ie one constructed from "").
    */
   bool isEmpty() const;

   /**
    * \brief Use this instead of \c boost::json::value::find_pointer to traverse a \c JsonXPath that might contain named
    *        array item identifiers.
    *
    *        Note that we do not offer an equivalent to \c boost::json::value::at_pointer (which does the same as
    *        \c boost::json::value::find_pointer but uses exceptions rather than an error code to report problems).
    *
    * \param startingValue Since this is a pointer, we don't have the same worries as in the \c JsonRecord constructor
    * \param errorCode If we couldn't follow the path, we (or Boost.JSON) will write an error code into this variable.
    *                  It usually won't be a lot of help in telling you what the error was. :-/
    * \return \c nullptr if there was an error (in which case \c errorCode will be non-zero) or else the value that you
    *         reach by following this path from \c startingValue
    */
//! @{
   boost::json::value const * followPathFrom(boost::json::value const * startingValue,
                                             std::error_code & errorCode) const;
   boost::json::value * followPathFrom(boost::json::value * startingValue,
                                       std::error_code & errorCode) const;
//! @}

   /**
    * \brief Instead of calling \c boost::json::value::set_at_pointer, use this function to ensure that any "parent"
    *        objects for a non-trivial key exist, and then call \c boost::json::object::emplace.  Eg, if JsonXPath is
    *        foo/bar/fruit[type="Banana"]/dog/cat, then this will ensure foo/bar/fruit[type="Banana"]/dog exists, move
    *        \c objectPointer to this location and return cat.
    * \param objectPointer Caller passes this in as the starting point and function returns the ending point (which will
    *                      be the same as the starting point for a trivial XPath).
    * \return the trivial XPath that can now be used on objectPointer
    */
   std::string makePointerToLeaf(boost::json::value ** valuePointer) const;

   /**
    * \brief For a trivial path, return it without the leading slash (as a \c string_view because that's what we're
    *        going to pass to Boost.JSON).  Caller's responsibility to ensure this is indeed a trivial path.
    */
   std::string_view asKey() const;

   /**
    * \brief This returns a C-style string as that's most universally usable for logging
    */
   char const * asXPath_c_str() const;

   /**
    * \brief If we have a JsonXPath of the form foo/bar/fruit[type="Banana"]/hum/bug, then we split it up into bits we
    *        can represent as JSON Pointers (\c /foo/bar/fruit and \c /hum/bug) and named array item identifiers
    *        (\c [type="Banana"]).
    *
    *        Collectively, we call these "path parts" (see \c PathPart below).
    *
    *        JSON Pointers are stored in \c std::string, because that's easier to pass to Boost.JSON than \c QString.
    *        A relative JSON Pointer is the same as its corresponding JsonXPath fragment, except that it has a '/' at
    *        the start.
    *
    *        Named array item identifiers need a struct (because Boost.JSON doesn't know about them and we have to do
    *        the parsing ourselves).
    *
    *        NB: These aliases and structs are public to make logging easier, but they are only really used by code in
    *            json/JsonXPath.cpp.
    */
//! @{
   using JsonPointer = std::string;
   struct NamedArrayItemId {
      std::string key;
      std::string value;
   };
//! @}

   /**
    * \brief There are other times when we want to break things down to a finer granularity.
    *
    *        If we have a JsonXPath of the form foo/bar/fruit[type="Banana"]/hum/bug, then we split it up into bits we
    *        can represent as JSON keys (\c foo, \c bar, \c fruit, \c hum, \c bug) and named array item identifiers
    *        (\c [type="Banana"]).
    *
    *        Collectively, we call these "path nodes" (see \c PathNode below).
    */
   using JsonKey = std::string;
   using PathPart = std::variant<JsonPointer, NamedArrayItemId>;
   // Note that, in JsonXPath::moveObjectPointerToLeaf it is very helpful to have a null state for this variant.  We do
   // not have the same requirement for PathPart.
   using PathNode = std::variant<std::monostate, JsonKey, NamedArrayItemId>;

private:
   // NOTE: We don't make any of our member variables const as we want to store \c JsonXPath objects inside (structs
   //       inside) a vector, and anything you put in a vector needs to be CopyConstructible and Assignable.

   //
   // All three of the member variables below store the complete JsonXpath, just in 3 different ways:
   //    - m_rawXPath is best for logging
   //    - m_pathParts is best for reading from a document
   //    - m_pathNodes is best for writing to a document
   // We accept the storage inefficiency for the benefit of simplifying our algorithms.
   //

   /**
    * \brief This is only used for logging (see operator<< below)
    */
   char const * m_rawXPath;
   std::vector<PathPart> m_pathParts;
   std::vector<PathNode> m_pathNodes;
};

/**
 * \brief Convenience functions for logging
 */
//! @{
template<class S>
S & operator<<(S & stream, JsonXPath const & jsonXPath) {
   stream << jsonXPath.asXPath_c_str();
   return stream;
}
template<class S>
S & operator<<(S & stream, JsonXPath::NamedArrayItemId const & namedArrayItemId) {
   stream << "[" << namedArrayItemId.key.c_str() << "=\"" << namedArrayItemId.value.c_str() << "\"]";
   return stream;
}
template<class S>
S & operator<<(S & stream, JsonXPath::PathPart const & pathPart) {
   if (std::holds_alternative<JsonXPath::JsonPointer>(pathPart)) {
      stream << std::get<JsonXPath::JsonPointer>(pathPart).c_str();
   } else {
      stream << std::get<JsonXPath::NamedArrayItemId>(pathPart);
   }
   return stream;
}
template<class S>
S & operator<<(S & stream, JsonXPath::PathNode const & pathNode) {
   if (std::holds_alternative<std::monostate>(pathNode)) {
      stream << "null";
   } else if (std::holds_alternative<JsonXPath::JsonKey>(pathNode)) {
      stream << std::get<JsonXPath::JsonKey>(pathNode).c_str();
   } else {
      stream << std::get<JsonXPath::NamedArrayItemId>(pathNode);
   }
   return stream;
}
//! @}

#endif
