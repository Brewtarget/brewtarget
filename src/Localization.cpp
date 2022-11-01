/*
 * Localization.cpp is part of Brewtarget, and is copyright the following
 * authors 2011-2021:
 *   • Greg Meess <Daedalus12@gmail.com>
 *   • Matt Young <mfsy@yahoo.com>
 *   • Mik Firestone <mikfire@gmail.com>
 *   • Philip Greggory Lee <rocketman768@gmail.com>
 *
 * Brewtarget is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Brewtarget is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "Localization.h"

#include <memory>

#include <QApplication> // For qApp
#include <QDebug>
#include <QDir>
#include <QLibraryInfo>
#include <QLocale>
#include <QTranslator>

#include "Application.h"
#include "model/NamedEntity.h"
#include "PersistentSettings.h"
#include "utils/BtStringConst.h"

//
// Anonymous namespace for constants, global variables and functions used only in this file
//
namespace {
   Localization::NumericDateFormat dateFormat = Localization::YearMonthDay;

   QString currentLanguage = "en";

   QTranslator defaultTrans;
   QTranslator btTrans;

}

void Localization::setDateFormat(NumericDateFormat newDateFormat) {
   dateFormat = newDateFormat;
   return;
}

Localization::NumericDateFormat Localization::getDateFormat() {
   return dateFormat;
}

QString Localization::numericToStringDateFormat(NumericDateFormat numericDateFormat) {
   switch (numericDateFormat) {
      case Localization::MonthDayYear:
         return "MM-dd-yyyy";
         break;
      case Localization::DayMonthYear:
         return "dd-MM-yyyy";
         break;
      default:
      case Localization::YearMonthDay:
         return "yyyy-MM-dd";
   }
}

QString Localization::displayDate(QDate const& date )
{
   QLocale loc(QLocale::system().name());
   return date.toString(loc.dateFormat(QLocale::ShortFormat));
}

QString Localization::displayDateUserFormated(QDate const & date) {
   QString format = Localization::numericToStringDateFormat(dateFormat);
   return date.toString(format);
}

void Localization::setLanguage(QString twoLetterLanguage) {
   currentLanguage = twoLetterLanguage;
   qApp->removeTranslator(&btTrans);

   QString filename = QString("bt_%1").arg(twoLetterLanguage);
   QDir translations = QDir(Application::getResourceDir().canonicalPath() + "/translations_qm");

   if (btTrans.load(filename, translations.canonicalPath())) {
      qApp->installTranslator(&btTrans);
   }
   return;
}

QString const & Localization::getCurrentLanguage() {
   return currentLanguage;
}

QString const & Localization::getSystemLanguage() {
   // QLocale::name() is of the form language_country,
   // where 'language' is a lowercase 2-letter ISO 639-1 language code,
   // and 'country' is an uppercase 2-letter ISO 3166 country code.
   return QLocale::system().name().split("_")[0];
}

bool Localization::hasUnits(QString qstr) {
   QString decimal = QRegExp::escape(QLocale::system().decimalPoint());
   QString grouping = QRegExp::escape(QLocale::system().groupSeparator());

   QRegExp amtUnit("((?:\\d+" + grouping + ")?\\d+(?:" + decimal + "\\d+)?|" + decimal + "\\d+)\\s*(\\w+)?");
   amtUnit.indexIn(qstr);

   bool result = amtUnit.cap(2).size() > 0;

   qDebug() << Q_FUNC_INFO << qstr << (result ? "has" : "does not have") << "units";

   return result;
}

double Localization::toDouble(QString text, bool* ok) {
   // Try system locale first
   bool success = false;
   QLocale sysDefault = QLocale();
   double ret = sysDefault.toDouble(text, &success);

   // If we failed, try C locale (ie what QString now does by default)
   if (!success) {
      ret = text.toDouble(&success);
   }

   // If we were asked to return the success, return it here.
   if (ok != nullptr) {
      *ok = success;
   }

   // Whatever we got, we return it
   return ret;
}

double Localization::toDouble(NamedEntity const & element,
                              BtStringConst const & propertyName,
                              char const * const caller) {
   if (element.property(*propertyName).canConvert(QVariant::String) ) {
      // Get the amount
      QString value = element.property(*propertyName).toString();
      bool ok = false;
      double amount = Localization::toDouble(value, &ok);
      if (!ok) {
         qWarning() << Q_FUNC_INFO << "(Called from" << caller << "): could not convert" << value << "to double";
      }
      return amount;
   }

   // If the supplied property couldn't be converted to a string then we assume it also could not be converted to a
   // number and return 0.
   return 0.0;
}

double Localization::toDouble(QString text, char const * const caller) {
   bool success = false;
   double ret = Localization::toDouble(text, &success);

   if (!success) {
      qWarning() << Q_FUNC_INFO << "(Called from" << caller << "): could not convert" << text << "to double";
   }

   return ret;
}


void Localization::loadTranslations() {
   if (qApp == nullptr) {
      return;
   }

   // Load translators.
   defaultTrans.load("qt_" + QLocale::system().name(), QLibraryInfo::location(QLibraryInfo::TranslationsPath));
   if (getCurrentLanguage().isEmpty()) {
      setLanguage(getSystemLanguage());
   }
   //btTrans.load("bt_" + getSystemLanguage());

   // Install translators
   qApp->installTranslator(&defaultTrans);
   //qApp->installTranslator(btTrans);

   return;
}

void Localization::loadSettings() {
   if (PersistentSettings::contains(PersistentSettings::Names::language)) {
      Localization::setLanguage(PersistentSettings::value(PersistentSettings::Names::language,"").toString());
   }

   dateFormat = static_cast<Localization::NumericDateFormat>(PersistentSettings::value(PersistentSettings::Names::date_format, Localization::YearMonthDay).toInt());
   return;
}

void Localization::saveSettings() {
   PersistentSettings::insert(PersistentSettings::Names::language, currentLanguage);
   PersistentSettings::insert(PersistentSettings::Names::date_format, dateFormat);
   return;
}
