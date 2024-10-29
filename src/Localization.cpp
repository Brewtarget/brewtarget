/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * Localization.cpp is part of Brewtarget, and is copyright the following authors 2011-2024:
 *   • Greg Meess <Daedalus12@gmail.com>
 *   • Matt Young <mfsy@yahoo.com>
 *   • Mik Firestone <mikfire@gmail.com>
 *   • Philip Greggory Lee <rocketman768@gmail.com>
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
#include "Localization.h"

#include <memory>

#include <QApplication> // For qApp
#include <QDebug>
#include <QDir>
#include <QLibraryInfo>
#include <QLocale>
#include <QRegularExpression>
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

   QString currentLanguage = "";

   QTranslator btTranslator;

   QLocale initSystemLocale() {
      //
      // At the moment, you need to manually edit the config file to set a forced locale (which is a step up from having
      // to hard-code something and recompile).  Potentially in future we'll allow this to be set via the UI.
      //
      // Per the Qt doco, the meaningful part of the specifier for a locale has the format
      // "language[_script][_country]" or "C", where:
      //
      //    - language is a lowercase, two-letter, ISO 639 language code (also some three-letter codes),
      //    - script is a titlecase, four-letter, ISO 15924 script code (eg Latn, Cyrl, Hebr)
      //    - country is an uppercase, two-letter, ISO 3166 country code (also "419" as defined by United Nations),
      //
      // The separator can be either underscore (the standard) or a minus sign (as used in Java).
      //
      // If the string violates the locale format, or language is not a valid ISO 639 code, the "C" locale is used
      // instead.  If country is not present, or is not a valid ISO 3166 code, the most appropriate country is chosen
      // for the specified language.
      //
      // So, eg, to force French/France locale, add the following line to the [General] section of the
      // xxxPersistentSettings.conf file:
      //    forcedLocale=fr_FR
      //
      if (PersistentSettings::contains(PersistentSettings::Names::forcedLocale)) {
         QLocale forcedLocale =
            QLocale(PersistentSettings::value(PersistentSettings::Names::forcedLocale, "").toString());
         // This probably isn't needed, but should force this locale into places where QLocale::system() is being called
         // instead of Localization::getLocale().  Note that QLocale::setDefault() is not reentrant, but that's OK as
         // we are guaranteed to be single-threaded here.
         QLocale::setDefault(forcedLocale);
         return forcedLocale;
      }
      return QLocale::system();
   }

}


QLocale const & Localization::getLocale() {
   //
   // Note that we can't use std::call_once to initialise systemLocale because it runs too early, ie before
   // PersistentSettings has been initialised.  Using a static local variable is thread-safe and (at the cost of a very
   // small runtime overhead) means that the variable will not be initialised until the first call of this function
   // (which should be after PersistentSettings::initialise() has been called).
   //
   static QLocale systemLocale = initSystemLocale();

   return systemLocale;
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

QString Localization::displayDate(QDate const& date) {
   return date.toString(Localization::getLocale().dateFormat(QLocale::ShortFormat));
}

QString Localization::displayDateUserFormated(QDate const & date) {
   QString format = Localization::numericToStringDateFormat(dateFormat);
   return date.toString(format);
}

void Localization::setLanguage(QString twoLetterLanguage) {
   currentLanguage = twoLetterLanguage;
   QCoreApplication::removeTranslator(&btTranslator);

   QString filename = QString("bt_%1").arg(twoLetterLanguage);
   QDir translations = QDir(Application::getResourceDir().canonicalPath() + "/translations_qm");
   bool const succeeded = btTranslator.load(filename, translations.canonicalPath());
   qDebug() <<
      Q_FUNC_INFO << "Loading" << filename << "from" << translations.canonicalPath() <<
      (succeeded ? "succeeded" : "failed");
   if (succeeded) {
      QCoreApplication::installTranslator(&btTranslator);
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
   return Localization::getLocale().name().split("_")[0];
}

bool Localization::hasUnits(QString qstr) {
   QString decimal = QRegularExpression::escape(Localization::getLocale().decimalPoint());
   QString grouping = QRegularExpression::escape(Localization::getLocale().groupSeparator());

   QRegularExpression amtUnit("((?:\\d+" + grouping + ")?\\d+(?:" + decimal + "\\d+)?|" + decimal + "\\d+)\\s*(\\w+)?");
   QRegularExpressionMatch match = amtUnit.match(qstr);

   // We could use hasCaptured here, but for debugging it's helpful to be able to show what we matched.
   QString const units = match.captured(2);
   bool const result = units.size() > 0;
   qDebug() << Q_FUNC_INFO << qstr << (result ? "has" : "does not have") << "units:" << units;

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
   if (element.property(*propertyName).canConvert<QString>()) {
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
   auto const systemLanguage = getSystemLanguage();
   qDebug() << Q_FUNC_INFO << "Current language:" << currentLanguage << "; System language:" << systemLanguage;
   if (currentLanguage.isEmpty()) {
      Localization::setLanguage(systemLanguage);
   }
   return;
}

void Localization::loadSettings() {
   if (PersistentSettings::contains(PersistentSettings::Names::language)) {
      Localization::setLanguage(PersistentSettings::value(PersistentSettings::Names::language, "").toString());
   }

   dateFormat = static_cast<Localization::NumericDateFormat>(PersistentSettings::value(PersistentSettings::Names::date_format, Localization::YearMonthDay).toInt());
   return;
}

void Localization::saveSettings() {
   PersistentSettings::insert(PersistentSettings::Names::language, currentLanguage);
   PersistentSettings::insert(PersistentSettings::Names::date_format, dateFormat);
   return;
}
