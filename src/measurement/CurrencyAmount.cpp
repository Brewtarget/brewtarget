/*======================================================================================================================
 * measurement/CurrencyAmount.cpp is part of Brewtarget, and is copyright the following authors 2025:
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
 =====================================================================================================================*/
#include "measurement/CurrencyAmount.h"

#include <algorithm>

#include <QDebug>
#include <QLocale>
#include <QRegularExpression>
#include <QRegularExpressionMatch>

#include "Localization.h"

namespace {
   /**
    * \brief Returns the default currency to use based on current locale
    */
   CurrencyInfo const * currencyForCurrentLocale() {
      return CurrencyInfo::getFromIsoAlphabeticCode(
         Localization::getLocale().currencySymbol(QLocale::CurrencySymbolFormat::CurrencyIsoCode)
      );
   }

   CurrencyInfo const * getCurrencyFromSymbolOrCode(QString const & symbolOrCode) {
      //
      // Now the tricky bit.  We need to work out what currency the user meant from the currency symbol.
      //
      // If there's no symbol or code, we'll just assume local currency at the bottom of the function
      //
      if (!symbolOrCode.isEmpty()) {
         //
         // If it's 3 characters long, let's see if it's an ISO code
         //
         if (symbolOrCode.length() == 3) {
            CurrencyInfo const * returnValue = CurrencyInfo::getFromIsoAlphabeticCode(symbolOrCode);
            if (returnValue->m_isoAlphabeticCode == symbolOrCode) {
               // We have a match, so we're done.
               return returnValue;
            }
         }

         //
         // It wasn't an ISO code that we recognise.  Let's see if it's the same as the currency symbol for the current
         // locale.
         //
         QString const localCurrencySymbol{
            Localization::getLocale().currencySymbol(QLocale::CurrencySymbolFormat::CurrencySymbol)
         };
         if (symbolOrCode == localCurrencySymbol) {
            //
            // Symbols match, so assume currencies are the same.  Eg, if your locale is Canada, we'll assume "$" means CAD,
            // rather than USD, AUD, NZD, etc.
            //
            return CurrencyInfo::getFromIsoAlphabeticCode(
               Localization::getLocale().currencySymbol(QLocale::CurrencySymbolFormat::CurrencyIsoCode)
            );
         }

         //
         // Symbol is not the local currency, so we'll try to find it in the list of currencies we know about.
         //
         std::vector<CurrencyInfo> const & currencyInfos = CurrencyInfo::getAll();
         auto found = std::find_if(currencyInfos.cbegin(),
                                 currencyInfos.cend(),
                                 [&symbolOrCode](CurrencyInfo const & ci){ return ci.m_unitSymbol == symbolOrCode; });
         if (found != currencyInfos.cend()) {
            return &(*found);
         }

         //
         // Couldn't understand symbol so use local currency
         //
         qInfo() << Q_FUNC_INFO << "Could not understand currency symbol" << symbolOrCode;
      }

      return CurrencyInfo::getFromIsoAlphabeticCode(
         Localization::getLocale().currencySymbol(QLocale::CurrencySymbolFormat::CurrencyIsoCode)
      );

   }
}

CurrencyInfo::CurrencyInfo(QString const & isoAlphabeticCode    ,
                           QString const & isoNumericCode       ,
                           QString const & fullName             ,
                           QString const & shortName            ,
                           QString const & unitSymbol           ,
                           QString const & centsName            ,
                           int     const   numDigitsAfterDecimal,
                           int     const   centsPerUnit         ) :
   m_isoAlphabeticCode    {isoAlphabeticCode    },
   m_isoNumericCode       {isoNumericCode       },
   m_fullName             {fullName             },
   m_shortName            {shortName            },
   m_unitSymbol           {unitSymbol           },
   m_centsName            {centsName            },
   m_numDigitsAfterDecimal{numDigitsAfterDecimal},
   m_centsPerUnit         {centsPerUnit         } {
   return;
}

std::vector<CurrencyInfo> const & CurrencyInfo::getAll() {
   // Meyers singleton
   static std::vector<CurrencyInfo> const currencyInfos{
      //
      // NOTE: Some of the rows in this table are a bit fun to edit because they contain right-to-left characters (eg
      // from Arabic or Hebrew).  Others don't exactly line up because some characters are not the same width as others
      // even in a fixed-width font.
      //
      // NOTE: Rows below should be sorted by first column.
      //
      /*:
       *
       */
      {"AED", "784", tr("United Arab Emirates Dirham"        ), tr("Dirham"   ),    "د.إ.", tr("Fils"        ), 2,  100},
      {"AFN", "971", tr("Afghan Afghani"                     ), tr("Afghani"  ),     "Af", tr("Pul"         ), 2,  100},
      {"ALL", "008", tr("Albanian Lek"                       ), tr("Lek"      ),      "L", tr("Qindarka"    ), 2,  100},
      {"AMD", "051", tr("Armenian Dram"                      ), tr("Dram"     ),      "֏", tr("Luma"        ), 2,  100},
      {"AOA", "973", tr("Angolan Kwanza"                     ), tr("Kwanza"   ),     "Kz", tr("Centimo"     ), 2,  100},
      {"ARS", "032", tr("Argentine Peso"                     ), tr("Peso"     ),    "AR$", tr("Centavo"     ), 2,  100},
      {"AUD", "036", tr("Australian Dollar"                  ), tr("Dollar"   ),    "AU$", tr("Cent"        ), 2,  100},
      {"AWG", "533", tr("Aruban Florin"                      ), tr("Florin"   ),      "ƒ", tr("Cent"        ), 2,  100},
      {"AZN", "944", tr("Azerbaijani Manat"                  ), tr("Manat"    ),    "ман", tr("Qapik"       ), 2,  100},
      {"BAM", "977", tr("Bosnia-Herzegovina Convertible Mark"), tr("Mark"     ),     "KM", tr("Fening"      ), 2,  100},
      {"BBD", "052", tr("Barbados Dollar"                    ), tr("Dollar"   ),   "BBD$", tr("Cent"        ), 2,  100},
      {"BDT", "050", tr("Bangladeshi Taka"                   ), tr("Taka"     ),      "৳", tr("Poisha"      ), 2,  100},
      {"BGN", "975", tr("Bulgarian Lev"                      ), tr("Lev"      ),    "лв.", tr("Stotinka"    ), 2,  100},
      {"BHD", "048", tr("Bahraini Dinar"                     ), tr("Dinar"    ),     "BD", tr("Fils"        ), 3, 1000},
      {"BIF", "108", tr("Burundian Franc"                    ), tr("Franc"    ),    "FBu", tr("Centime"     ), 0,  100},
      {"BMD", "060", tr("Bermudian Dollar"                   ), tr("Dollar"   ),      "$", tr("Cent"        ), 2,  100},
      {"BND", "096", tr("Brunei Dollar"                      ), tr("Dollar"   ),     "B$", tr("Cent"        ), 2,  100},
      {"BOB", "068", tr("Bolivian Boliviano"                 ), tr("Boliviano"),    "Bs.", tr("Centavo"     ), 2,  100},
      {"BRL", "986", tr("Brazilian Real"                     ), tr("Real"     ),     "R$", tr("Centavo"     ), 2,  100},
      {"BSD", "044", tr("Bahamian Dollar"                    ), tr("Dollar"   ),      "$", tr("Cent"        ), 2,  100},
      {"BTN", "064", tr("Bhutanese Ngultrum"                 ), tr("Ngultrum" ),    "Nu.", tr("Chetrum"     ), 2,  100},
      {"BWP", "072", tr("Botswana Pula"                      ), tr("Pula"     ),      "P", tr("Thebe"       ), 2,  100},
      {"BYN", "933", tr("Belarusian Ruble"                   ), tr("Ruble"    ),     "Br", tr("Kapiejka"    ), 2,  100},
      {"BZD", "084", tr("Belize Dollar"                      ), tr("Dollar"   ),    "BZ$", tr("Cent"        ), 2,  100},
      {"CAD", "124", tr("Canadian Dollar"                    ), tr("Dollar"   ),    "CA$", tr("Cent"        ), 2,  100},
      {"CDF", "976", tr("Congolese Franc"                    ), tr("Franc"    ),     "FC", tr("Centime"     ), 2,  100},
      {"CHF", "756", tr("Swiss Franc"                        ), tr("Franc"    ),    "Fr.", tr("Centime"     ), 2,  100},
      {"CLP", "152", tr("Chilean Peso"                       ), tr("Peso"     ),    "CL$", tr("Centavo"     ), 0,    0},
      {"CNY", "156", tr("Chinese Yuan"                       ), tr("Yuan"     ),    "CN¥", tr("Fen"         ), 2,  100},
      {"COP", "170", tr("Colombian Peso"                     ), tr("Peso"     ),    "CO$", tr("Centavo"     ), 2,  100},
      {"CRC", "188", tr("Costa Rican Colón"                  ), tr("Colón"    ),      "₡", tr("Centimo"     ), 2,  100},
      {"CUP", "192", tr("Cuban Peso"                         ), tr("Peso"     ),    "$MN", tr("Centavo"     ), 2,  100},
      {"CVE", "132", tr("Cabo Verdean Escudo"                ), tr("Escudo"   ),    "CV$", tr("Centavo"     ), 2,  100},
      {"CZK", "203", tr("Czech Koruna"                       ), tr("Koruna"   ),     "Kč", tr("Haléř"       ), 2,  100},
      {"DJF", "262", tr("Djiboutian Franc"                   ), tr("Franc"    ),    "Fdj", tr("Centime"     ), 0,  100},
      {"DKK", "208", tr("Danish Krone"                       ), tr("Krone"    ),    "kr.", tr("Øre"         ), 2,  100},
      {"DOP", "214", tr("Dominican Peso"                     ), tr("Peso"     ),    "RD$", tr("Centavo"     ), 2,  100},
      {"DZD", "012", tr("Algerian Dinar"                     ), tr("Dinar"    ),     "DA", tr("Santeem"     ), 2,  100},
      {"EGP", "818", tr("Egyptian Pound"                     ), tr("Pound"    ),     "E£", tr("Qirsh"       ), 2,  100},
      {"ERN", "232", tr("Eritrean Nakfa"                     ), tr("Nakfa"    ),    "Nkf", tr("Cent"        ), 2,  100},
      {"ETB", "230", tr("Ethiopian Birr"                     ), tr("Birr"     ),     "Br", tr("Santim"      ), 2,  100},
      {"EUR", "978", tr("Euro"                               ), tr("Euro"     ),      "€", tr("Cent"        ), 2,  100},
      {"FJD", "242", tr("Fiji Dollar"                        ), tr("Dollar"   ),    "FJ$", tr("Cent"        ), 2,  100},
      {"FKP", "238", tr("Falkland Islands Pound"             ), tr("Pound"    ),    "FK£", tr("Penny"       ), 2,  100},
      {"GBP", "826", tr("Pound Sterling"                     ), tr("Pound"    ),      "£", tr("Penny"       ), 2,  100},
      {"GEL", "981", tr("Georgian Lari"                      ), tr("Lari"     ),      "₾", tr("Tetri"       ), 2,  100},
      {"GHS", "936", tr("Ghanaian Cedi"                      ), tr("Cedi"     ),    "GH₵", tr("Pesewa"      ), 2,  100},
      {"GIP", "292", tr("Gibraltar Pound"                    ), tr("Pound"    ),      "£", tr("Penny"       ), 2,  100},
      {"GMD", "270", tr("Gambian Dalasi"                     ), tr("Dalasi"   ),      "D", tr("Butut"       ), 2,  100},
      {"GNF", "324", tr("Guinean Franc"                      ), tr("Franc"    ),     "FG", tr("Centime"     ), 0,  100},
      {"GTQ", "320", tr("Guatemalan Quetzal"                 ), tr("Quetzal"  ),      "Q", tr("Centavo"     ), 2,  100},
      {"GYD", "328", tr("Guyanese Dollar"                    ), tr("Dollar"   ),     "G$", tr("Cent"        ), 2,  100},
      {"HKD", "344", tr("Hong Kong Dollar"                   ), tr("Dollar"   ),    "HK$", tr("Cent"        ), 2,  100},
      {"HNL", "340", tr("Honduran Lempira"                   ), tr("Lempira"  ),      "L", tr("Centavo"     ), 2,  100},
      {"HTG", "332", tr("Haitian Gourde"                     ), tr("Gourde"   ),      "G", tr("Centime"     ), 2,  100},
      {"HUF", "348", tr("Hungarian Forint"                   ), tr("Forint"   ),     "Ft", tr("fillér"      ), 2,  100},
      {"IDR", "360", tr("Indonesian Rupiah"                  ), tr("Rupiah"   ),     "Rp", tr("Sen"         ), 2,  100},
      {"ILS", "376", tr("Israeli Shekel"                     ), tr("Shekel"   ),      "₪", tr("Agora"       ), 2,  100},
      {"INR", "356", tr("Indian Rupee"                       ), tr("Rupee"    ),    "Rs.", tr("Paisa"       ), 2,  100},
      {"IQD", "368", tr("Iraqi Dinar"                        ), tr("Dinar"    ),    "د.ع‎", tr("Fils"        ), 3,  100},
      {"IRR", "364", tr("Iranian Rial"                       ), tr("Rial"     ),     "﷼", tr("Dinar"       ), 2,  100},
      {"ISK", "352", tr("Icelandic Krona"                    ), tr("Krona"    ),     "kr", tr("Aurar"       ), 0,  100},
      {"JMD", "388", tr("Jamaican Dollar"                    ), tr("Dollar"   ),     "J$", tr("Cent"        ), 2,  100},
      {"JOD", "400", tr("Jordanian Dinar"                    ), tr("Dinar"    ),     "JD", tr("Fils"        ), 3, 1000},
      {"JPY", "392", tr("Japanese Yen"                       ), tr("Yen"      ),      "¥", tr(""            ), 0,    0},
      {"KES", "404", tr("Kenyan Shilling"                    ), tr("Shilling" ),    "KSh", tr("Cent"        ), 2,  100},
      {"KGS", "417", tr("Kyrgyzstani Som"                    ), tr("Som"      ),      "с", tr("Tyiyn"       ), 2,  100},
      {"KHR", "116", tr("Cambodian Riel"                     ), tr("Riel"     ),      "៛", tr( "Sen"        ), 2,  100},
      {"KMF", "174", tr("Comorian Franc"                     ), tr("Franc"    ),     "CF", tr("Centime"     ), 0,  100},
      {"KPW", "408", tr("North Korean Won"                   ), tr("Won"      ),      "₩", tr("Chon"        ), 2,  100},
      {"KRW", "410", tr("South Korean Won"                   ), tr("Won"      ),      "₩", tr("Jeon"        ), 0,  100},
      {"KWD", "414", tr("Kuwaiti Dinar"                      ), tr("Dinar"    ),     "KD", tr("Fils"        ), 3, 1000},
      {"KYD", "136", tr("Cayman Islands Dollar"              ), tr("Dollar"   ),    "CI$", tr("Cent"        ), 2,  100},
      {"KZT", "398", tr("Kazakhstani Tenge"                  ), tr("Tenge"    ),      "₸", tr("Tıyn"        ), 2,  100},
      {"LAK", "418", tr("Lao Kip"                            ), tr("Kip"      ),     "₭N", tr("Att"         ), 2,  100},
      {"LBP", "422", tr("Lebanese Pound"                     ), tr("Pound"    ),    "LL.", tr("Qirsh"       ), 2,  100},
      {"LKR", "144", tr("Sri Lankan Rupee"                   ), tr("Rupee"    ),    "Rs.", tr("Cent"        ), 2,  100},
      {"LRD", "430", tr("Liberian Dollar"                    ), tr("Dollar"   ),     "L$", tr("Cent"        ), 2,  100},
      {"LSL", "426", tr("Lesotho Loti"                       ), tr("Loti"     ),      "L", tr("Sente"       ), 2,  100},
      {"LYD", "434", tr("Libyan Dinar"                       ), tr("Dinar"    ),     "LD", tr("Dirham"      ), 3, 1000},
      {"MAD", "504", tr("Moroccan Dirham"                    ), tr("Dirham"   ),     "DH", tr("Centime"     ), 2,  100},
      {"MDL", "498", tr("Moldovan Leu"                       ), tr("Leu"      ),      "L", tr("Ban"         ), 2,  100},
      {"MGA", "969", tr("Malagasy Ariary"                    ), tr("Ariary"   ),     "Ar", tr("Iraimbilanja"), 2,    5},
      {"MKD", "807", tr("Macedonian Denar"                   ), tr("Denar"    ),    "den", tr("Deni"        ), 2,  100},
      {"MMK", "104", tr("Myanmar Kyat"                       ), tr("Kyat"     ),     "Ks", tr("Pya"         ), 2,  100},
      {"MNT", "496", tr("Mongolian Tögrög"                   ), tr("Tögrög"   ),      "₮", tr("möngö"       ), 2,  100},
      {"MOP", "446", tr("Macanese Pataca"                    ), tr("Pataca"   ),   "MOP$", tr("Avo"         ), 2,  100},
      {"MRU", "929", tr("Mauritanian Ouguiya"                ), tr("Ouguiya"  ),     "UM", tr("Khoums"      ), 2,    5},
      {"MUR", "480", tr("Mauritian Rupee"                    ), tr("Rupee"    ),    "Rs.", tr("Cent"        ), 2,  100},
      {"MVR", "462", tr("Maldivian Rufiyaa"                  ), tr("Rufiyaa"  ),    "MRf", tr("laari"       ), 2,  100},
      {"MWK", "454", tr("Malawian Kwacha"                    ), tr("Kwacha"   ),     "MK", tr("Tambala"     ), 2,  100},
      {"MXN", "484", tr("Mexican Peso"                       ), tr("Peso"     ),    "MX$", tr("Centavo"     ), 2,  100},
      {"MYR", "458", tr("Malaysian Ringgit"                  ), tr("Ringgit"  ),     "RM", tr("Sen"         ), 2,  100},
      {"MZN", "943", tr("Mozambican Metical"                 ), tr("Metical"  ),    "MTn", tr("Centavo"     ), 2,  100},
      {"NAD", "516", tr("Namibian Dollar"                    ), tr("Dollar"   ),     "N$", tr("Cent"        ), 2,  100},
      {"NGN", "566", tr("Nigerian Naira"                     ), tr("Naira"    ),      "₦", tr("Kobo"        ), 2,  100},
      {"NIO", "558", tr("Nicaraguan Córdoba"                 ), tr("Córdoba"  ),     "C$", tr("Centavo"     ), 2,  100},
      {"NOK", "578", tr("Norwegian Krone"                    ), tr("Krone"    ),     "kr", tr("øre"         ), 2,  100},
      {"NPR", "524", tr("Nepalese Rupee"                     ), tr("Rupee"    ),    "Rs.", tr("Paisa"       ), 2,  100},
      {"NZD", "554", tr("New Zealand Dollar"                 ), tr("Dollar"   ),    "NZ$", tr("Cent"        ), 2,  100},
      {"OMR", "512", tr("Omani Rial"                         ), tr("Rial"     ),     "OR", tr("Baisa"       ), 3, 1000},
      {"PAB", "590", tr("Panamanian Balboa"                  ), tr("Balboa"   ),    "B/.", tr("Centésimo"   ), 2,  100},
      {"PEN", "604", tr("Peruvian Sol"                       ), tr("Sol"      ),    "S/.", tr("Céntimo"     ), 2,  100},
      {"PGK", "598", tr("Papua New Guinean Kina"             ), tr("Kina"     ),      "K", tr("Toea"        ), 2,  100},
      {"PHP", "608", tr("Philippine Peso"                    ), tr("Peso"     ),      "₱", tr("Sentimo"     ), 2,  100},
      {"PKR", "586", tr("Pakistani Rupee"                    ), tr("Rupee"    ),    "Rs.", tr("Paisa"       ), 2,  100},
      {"PLN", "985", tr("Polish Zloty"                       ), tr("Zloty"    ),     "zł", tr("Grosz"       ), 2,  100},
      {"PYG", "600", tr("Paraguayan Guaraní"                 ), tr("Guaraní"  ),      "₲", tr("Centimo"     ), 0,  100},
      {"QAR", "634", tr("Qatari Riyal"                       ), tr("Riyal"    ),     "QR", tr("Dirham"      ), 2,  100},
      {"RON", "946", tr("Romanian Leu"                       ), tr("Leu"      ),      "L", tr("Ban"         ), 2,  100},
      {"RSD", "941", tr("Serbian Dinar"                      ), tr("Dinar"    ),    "din", tr("Para"        ), 2,  100},
      {"RUB", "643", tr("Russian Ruble"                      ), tr("Ruble"    ),      "₽", tr("Kopek"       ), 2,  100},
      {"RWF", "646", tr("Rwandan Franc"                      ), tr("Franc"    ),    "FRw", tr("Centime"     ), 0,  100},
      {"SAR", "682", tr("Saudi Riyal"                        ), tr("Riyal"    ),     "SR", tr("Halalah"     ), 2,  100},
      {"SBD", "090", tr("Solomon Islands Dollar"             ), tr("Dollar"   ),    "SI$", tr("Cent"        ), 2,  100},
      {"SCR", "690", tr("Seychelles Rupee"                   ), tr("Rupee"    ),    "Rs.", tr("Cent"        ), 2,  100},
      {"SDG", "938", tr("Sudanese Pound"                     ), tr("Pound"    ),    "£SD", tr("Qirsh"       ), 2,  100},
      {"SEK", "752", tr("Swedish Krona"                      ), tr("Krona"    ),     "kr", tr("Öre"         ), 2,  100},
      {"SGD", "702", tr("Singapore Dollar"                   ), tr("Dollar"   ),     "S$", tr("Cent"        ), 2,  100},
      {"SHP", "654", tr("Saint Helena Pound"                 ), tr("Pound"    ),      "£", tr("Penny"       ), 2,  100},
      {"SLL", "925", tr("Sierra Leonean New Leone"           ), tr("New Leone"),     "Le", tr("Cent"        ), 2,  100},
      {"SOS", "706", tr("Somali Shilling"                    ), tr("Shilling" ), "Sh.So.", tr("Senti"       ), 2,  100},
      {"SRD", "968", tr("Surinamese Dollar"                  ), tr("Dollar"   ),    "Sr$", tr("Cent"        ), 2,  100},
      {"SSP", "728", tr("South Sudanese Pound"               ), tr("Pound"    ),    "SS£", tr("Qirsh"       ), 2,  100},
      {"STN", "930", tr("Sao Tome Dobra"                     ), tr("Dobra"    ),     "Db", tr("Centimo"     ), 2,  100},
      {"SVC", "222", tr("Salvadoran Colón"                   ), tr("Colón"    ),      "₡", tr("Centavo"     ), 2,  100},
      {"SYP", "760", tr("Syrian Pound"                       ), tr("Pound"    ),     "LS", tr("Qirsh"       ), 2,  100},
      {"SZL", "748", tr("Swazi Lilangeni"                    ), tr("Lilangeni"),      "L", tr("Cent"        ), 2,  100},
      {"THB", "764", tr("Thai Baht"                          ), tr("Baht"     ),      "฿", tr("Satang"      ), 2,  100},
      {"TJS", "972", tr("Tajikistani Somoni"                 ), tr("Somoni"   ),     "SM", tr("Diram"       ), 2,  100},
      {"TMT", "934", tr("Turkmenistan Manat"                 ), tr("Manat"    ),     "m.", tr("Tenge"       ), 2,  100},
      {"TND", "788", tr("Tunisian Dinar"                     ), tr("Dinar"    ),     "DT", tr("Millime"     ), 3, 1000},
      {"TOP", "776", tr("Tongan Pa'anga"                     ), tr("Pa'anga"  ),     "T$", tr("Seniti"      ), 2,  100},
      {"TRY", "949", tr("Turkish Lira"                       ), tr("Lira"     ),     "TL", tr("Kuruş"       ), 2,  100},
      {"TTD", "780", tr("Trinidad and Tobago Dollar"         ), tr("Dollar"   ),    "TT$", tr("Cent"        ), 2,  100},
      {"TWD", "901", tr("New Taiwan Dollar"                  ), tr("Dollar"   ),    "NT$", tr("Cent"        ), 2,  100},
      {"TZS", "834", tr("Tanzanian Shilling"                 ), tr("Shilling" ),    "TSh", tr("Senti"       ), 2,  100},
      {"UAH", "980", tr("Ukrainian Hryvnia"                  ), tr("Hryvnia"  ),      "₴", tr("Kopiyka"     ), 2,  100},
      {"UGX", "800", tr("Ugandan Shilling"                   ), tr("Shilling" ),    "USh", tr("Cent"        ), 0,  100},
      {"USD", "840", tr("US Dollar"                          ), tr("Dollar"   ),      "$", tr("Cent"        ), 2,  100},
      {"UYU", "858", tr("Uruguayan Peso"                     ), tr("Peso"     ),     "$U", tr("Centésimo"   ), 2,  100},
      {"UZS", "860", tr("Uzbekistani Som"                    ), tr("Som"      ),    "сум", tr("Tiyin"       ), 2,  100},
      {"VES", "928", tr("Venezuelan Bolívar"                 ), tr("Bolívar"  ),   "Bs.F", tr("Centimo"     ), 2,  100},
      {"VND", "704", tr("Vietnamese Dong"                    ), tr("Dong"     ),      "₫", tr("Hào"         ), 0,   10},
      {"VUV", "548", tr("Vanuatu Vatu"                       ), tr("Vatu"     ),     "VT", tr(""            ), 0,    0},
      {"WST", "882", tr("Samoan Tala"                        ), tr("Tala"     ),      "T", tr("Sene"        ), 2,  100},
      {"XAF", "950", tr("Central African CFA Franc"          ), tr("Franc"    ),     "Fr", tr("Centime"     ), 0,  100},
      {"XCD", "951", tr("East Caribbean Dollar"              ), tr("Dollar"   ),      "$", tr("Cent"        ), 2,  100},
      {"XCG", "532", tr("Caribbean Guilder"                  ), tr("Guilder"  ),     "Cg", tr("Cent"        ), 2,  100},
      {"XOF", "952", tr("West African CFA Franc"             ), tr("Franc"    ),      "₣", tr("Centime"     ), 0,  100},
      {"XPF", "953", tr("CFP Franc"                          ), tr("Franc"    ),      "₣", tr("Centime"     ), 0,  100},
      {"XXX", "999", tr("No Currency"                        ), tr("None"     ),      "×", tr("None"        ), 0,    0}, // Must be present for getFromIsoAlphabeticCode to work
      {"YER", "886", tr("Yemeni Rial"                        ), tr("Rial"     ),     "YR", tr("Fils"        ), 2,  100},
      {"ZAR", "710", tr("South African Rand"                 ), tr("Rand"     ),      "R", tr("Cent"        ), 2,  100},
      {"ZMW", "967", tr("Zambian Kwacha"                     ), tr("Kwacha"   ),     "ZK", tr("Ngwee"       ), 2,  100},
      {"ZWL", "924", tr("Zimbabwe Gold"                      ), tr("Gold"     ),    "ZiG", tr("Cent"        ), 2,  100},

   };
   return currencyInfos;
}

CurrencyInfo const * CurrencyInfo::getFromIsoAlphabeticCode(QString const & isoAlphabeticCode) {
   std::vector<CurrencyInfo> const & currencyInfos = CurrencyInfo::getAll();
   //
   // We don't want to reinvent the wheel of binary search, so we use the STL one.  This means we need a CurrencyInfo
   // object to give the STL algorithm.  We only care about the first field, so the other ones can be dummy values.
   //
   CurrencyInfo const searchItem{isoAlphabeticCode, "", "", "", "", "", 0, 0};
   auto lowerBound = std::lower_bound(
      currencyInfos.cbegin(),
      currencyInfos.cend(),
      searchItem,
      [](CurrencyInfo const & lhs, CurrencyInfo const & rhs) {
         return lhs.m_isoAlphabeticCode < rhs.m_isoAlphabeticCode;
      }
   );

   if (isoAlphabeticCode == lowerBound->m_isoAlphabeticCode) {
      // Funny syntax here is because we have to convert an iterator to an object and then return its address
      return &(*lowerBound);
   }

   //
   // If we didn't find anything, we'll return the XXX "no currency" object, which means it's a coding error if there
   // isn't an "XXX" entry in the list in getAll().  In other words, if we get this far searching for "XXX", something
   // is wrong in getAll().
   //
   static QString const noCurrencyCode{"XXX"};
   Q_ASSERT(isoAlphabeticCode != noCurrencyCode);
   return CurrencyInfo::getFromIsoAlphabeticCode(noCurrencyCode);
}

CurrencyAmount::CurrencyAmount(QString const & inputString) {
   //
   // Sadly Qt does not offer us the inverse of QLocale::toCurrencyString, so we have to roll it ourselves
   //
   // Approach here is similar to that in Measurement::Unit::splitAmountString, and some of the comments there also
   // apply here.  The extra complication is that the currency symbol or code could be at the start (eg "€ 1,23" or
   // "EUR 1,23") or the end (eg "1,23 €" or "1,23 EUR").
   //
   // NOTE: For the moment, we only initialise this regexp once, which means that, if the user changes locale, the new
   //       settings for thousands ("group") separator and decimal point will not take effect until the program is
   //       restarted.
   //
   static const QString groupSeparator {Localization::getLocale().groupSeparator()};
   static const QString symbolCaptureRegexp {
      "(" // Capture group start
         "[^" // Characters that are not...
            "\\s"   // whitespace -- which may include group separator if that is " "
            "\\d" + // digits
            (groupSeparator == " " ? QString{} : QRegularExpression::escape(groupSeparator)) + // group separator if not " "
            QRegularExpression::escape(Localization::getLocale().decimalPoint()) + // decimal separator
         "]*" // Zero or more of them
      ")" // Capture group end
   };
   static QRegularExpression const moneyRegexp {
      symbolCaptureRegexp + // Capture group 1
      "\\s*" // Any spaces etc before the amount
      "("    // Start of capture group 2 = amount
         "["   // Characters that are...
            "\\d" + // digits
            QRegularExpression::escape(Localization::getLocale().groupSeparator()) +
            QRegularExpression::escape(Localization::getLocale().decimalPoint()) +
         "]+" // One ore more of them
      ")" // End of capture group 2 = Any combination of digits, decimal points etc = the amount
          // (NB: We leave it to Localization::toDouble() below to do full parsing.)
      "\\s*" +  // Any spaces etc after the amount
      symbolCaptureRegexp, // Capture group 3
      QRegularExpression::CaseInsensitiveOption
   };

   QRegularExpressionMatch match = moneyRegexp.match(inputString);
   if (!match.hasMatch()) {
      qDebug() << Q_FUNC_INFO << "Unable to parse" << inputString << "so treating as 0.00";
      this->m_currencyInfo = currencyForCurrentLocale();
      this->m_totalAsCents = 0;
      return;
   }

   //
   // Currency symbol (eg $, €, £) or code (eg USD, EUR, GBP) can be at beginning or end.  (If there is one at the start
   // and the end, we just take the one at the start!)
   //
   // In theory the call to QString::trimmed() here isn't necessary if we got the regexp correct above.  In practice, in
   // some locales I'm getting a preceding space on group 3 when I match, say, "1,23 €".
   //
   QString currencySymbol = match.captured(1).trimmed();
   if (currencySymbol.isEmpty()) {
      currencySymbol = match.captured(3).trimmed();
   }

   QString numericPartOfInput{match.captured(2)};
   double const amount = Localization::toDouble(numericPartOfInput, Q_FUNC_INFO, nullptr);
   this->m_totalAsCents = static_cast<int>(amount * 100.0);

   this->setCurrencyFromSymbolOrCode(currencySymbol);

   return;
}

CurrencyAmount::CurrencyAmount(QString const symbolOrCode, double const amount) :
   m_currencyInfo{getCurrencyFromSymbolOrCode(symbolOrCode)},
   m_totalAsCents{static_cast<int>(amount * 100.0)} {
   return;
}

CurrencyAmount::CurrencyAmount(QString const symbolOrCode, int const amountAsCents) :
   m_currencyInfo{getCurrencyFromSymbolOrCode(symbolOrCode)},
   m_totalAsCents{amountAsCents} {
   return;
}

CurrencyAmount::CurrencyAmount() :
   m_currencyInfo{currencyForCurrentLocale()},
   m_totalAsCents{0} {
   return;
}

CurrencyAmount::~CurrencyAmount() = default;

// Copy constructor
CurrencyAmount::CurrencyAmount(CurrencyAmount const & other) :
   m_currencyInfo{other.m_currencyInfo},
   m_totalAsCents{other.m_totalAsCents} {
   return;
}

// Copy assignment operator
CurrencyAmount & CurrencyAmount::operator=(CurrencyAmount const & other) {
   *this = CurrencyAmount{other};
   return *this;
}

// Move constructor
CurrencyAmount::CurrencyAmount(CurrencyAmount && other) noexcept :
   m_currencyInfo{other.m_currencyInfo},
   m_totalAsCents{other.m_totalAsCents} {
   return;
}

// Move assignment operator
CurrencyAmount & CurrencyAmount::operator=(CurrencyAmount && other) noexcept {
   std::swap(this->m_currencyInfo, other.m_currencyInfo);
   std::swap(this->m_totalAsCents, other.m_totalAsCents);
   return *this;
}

void CurrencyAmount::setCurrencyFromSymbolOrCode(QString const & symbolOrCode) {
   this->m_currencyInfo = getCurrencyFromSymbolOrCode(symbolOrCode);
   return;
}


bool CurrencyAmount::operator==(CurrencyAmount const & other) const {
   //
   // CurrencyInfo objects are const singletons, so it's valid to compare pointers for equality
   //
   return (this->m_currencyInfo == other.m_currencyInfo &&
           this->m_totalAsCents == other.m_totalAsCents);

}

bool CurrencyAmount::operator!=(CurrencyAmount const & other) const {
   // Don't reinvent the wheel '!=' should just be the opposite of '=='
   return !(*this == other);
}

std::partial_ordering CurrencyAmount::operator<=>(CurrencyAmount const & other) const {
   //
   // We're not realistically going to know how to compare amounts in different currencies.  For the moment,
   // we'll group different currencies together.
   //
   if (this->m_currencyInfo != other.m_currencyInfo) {
      return std::partial_ordering::unordered;
   }
   if (this->m_totalAsCents == other.m_totalAsCents) {
      return std::partial_ordering::equivalent;
   }
   return (this->m_totalAsCents < other.m_totalAsCents) ? std::partial_ordering::less :
                                            std::partial_ordering::greater;
}

double CurrencyAmount::asUnits() const {
   return this->m_totalAsCents / 100.0;
}

QString CurrencyAmount::asDisplayable() const {
   //
   // QLocale::toCurrencyString does the heavy lifting for us, including working out how many decimal places to display
   //
   return Localization::getLocale().toCurrencyString(this->asUnits(), this->m_currencyInfo->m_unitSymbol);
}

int CurrencyAmount::centsPart() const { return this->m_totalAsCents % 100; }
int CurrencyAmount::unitsPart() const { return this->m_totalAsCents / 100; }
