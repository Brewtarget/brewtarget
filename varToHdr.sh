#!/bin/bash

HDR="$1.h"
CPP="$1.cpp"
ECHO='/bin/echo'

#---------------Do the header file-----------------
$ECHO -e "#include <string>\n#include <exception>\n#include \"xmlnode.h\"" > $HDR
$ECHO -e "\nclass $1;\nclass $1Exception;\n" >> $HDR
$ECHO -e "class $1\n{" >> $HDR
$ECHO -e "public:\n" >> $HDR
$ECHO -e "   $1();\n   $1(const XmlNode *node);\n" >> $HDR

sed 's#\(\S\+\)\s\(\S\+\)#void set\2( \1 var );#' < $1 >> $HDR
$ECHO "" >> $HDR
sed 's#\(\S\+\)\s\(\S\+\)#\1 get\2() const;#' < $1 >> $HDR
$ECHO -e "\nprivate:\n" >> $HDR
sed 's#^\(.*\)$#\1;#' < $1 >> $HDR
$ECHO -e '   void setDefaults();' >> $HDR
$ECHO -e '\n};\n' >> $HDR

$ECHO -e "class $1Exception : public std::exception\n{\npublic:\n" >> $HDR
$ECHO -e '   virtual const char* what() const throw()\n   {' >> $HDR
$ECHO '      return std::string("BeerXML ... error: " + _err + "\n").c_str();' >> $HDR
$ECHO -e "   }\n\n   $1Exception( std::string message )\n   {" >> $HDR
$ECHO -e '      _err = message;\n   }\n' >> $HDR
$ECHO -e "   ~$1Exception() throw() {}\n\nprivate:\n" >> $HDR
$ECHO -e '   std::string _err;\n};\n' >> $HDR

#-------------Do the cpp file----------------
$ECHO -e '#include <string>\n#include <vector>' > $CPP
$ECHO -e '#include "xmlnode.h"\n#include "stringparsing.h"' >> $CPP
$ECHO -e "#include \"$HDR\"\n" >> $CPP
$ECHO -e "void $1::setDefaults()\n{" >> $CPP
sed "s#\(\S\+\)\s\(\S\+\)#   \2 = 0.0;#" < $1 >> $CPP
$ECHO -e "}\n" >> $CPP
$ECHO -e "$1()\n{\n   setDefaults();\n}\n\n$1(const XmlNode *node)\n{\n}\n" >> $CPP

sed "s#\(\S\+\)\s\(\S\+\)#void $1::set\2( \1 var )\n{\n}\n#" < $1 >> $CPP
$ECHO -e '\n// "GET" METHODS' >> $CPP
sed "s#\(\S\+\)\s\(\S\+\)#\1 $1::get\2() const\n{\n   return \2;\n}\n#" < $1 >> $CPP

