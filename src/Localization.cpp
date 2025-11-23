/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * Localization.cpp is part of Brewtarget, and is copyright the following authors 2011-2025:
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

#include <algorithm>
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

   QString currentTwoLetterLanguageCode{""};
   QLocale currentLanguageLocale{};

   //
   // It's not super-well explained in the Qt documentation (or the wiki article at
   // https://wiki.qt.io/How_to_create_a_multi_language_application), but there are, typically, two sets of translation
   // files:
   //    - The application's own *.qm translation files (in our case bt-*.qm) that are generated from its *.ts files and
   //      (should ideally) contain translations of all the application-specific text.
   //    - Qt's own qt_*.qm translation files that ship with the Qt framework and contain translations for Qt's buttons,
   //      dialog boxes, error messages and so on.
   //
   // So, we need one translator for each of these two sets of files.  When we change languages, we have to reload each
   // one.
   //
   // (I wish I could say huge thought and cleverness went into making these two variable names the same length, but it
   // was more luck than judgement!)
   //
   QTranslator qtFrameworkTranslator;
   QTranslator applicationTranslator;

   /**
    * \brief This is basically a wrapper around (one of the overloads of) \c QTranslator::load.
    *
    *        As explained at https://doc.qt.io/qt-6/qtranslator.html#load-1, the \c QTranslator::load function does a
    *        fair bit of logic to try to find the "best match" translation file.  Eg, in a locale with UI languages
    *        "fr_CA" and "en", then, given prefix "qt_" and suffix ".qm", it will look for:
    *           qt_fr_CA.qm
    *           qt_fr_ca.qm
    *           qt_fr_CA
    *           qt_fr_ca
    *           qt_fr.qm
    *           qt_fr
    *           qt_en.qm
    *           qt_en
    */
   void loadTranslator(QTranslator   & translator,
                       QLocale const & locale,
                       QString const & filenamePrefix,
                       QDir    const & directory) {
      // We have to remove the old translator before we load the new one
      if (!translator.isEmpty()) {
         qApp->removeTranslator(&translator);
      }

      // Qt logging output stream pads everything with spaces by default, so we do our own concatenation here where we
      // don't want the extra spaces.
      QString message{};
      QTextStream messageAsStream{&message};
      messageAsStream <<
         filenamePrefix << "*.qm translations for locale " << locale.name() << " with language list " <<
         locale.uiLanguages().join(", ") << " from " << directory.canonicalPath();

      qInfo() << Q_FUNC_INFO << "Loading" << message;
      if (translator.load(locale, filenamePrefix, "", directory.canonicalPath(), ".qm")) {
         qInfo() << Q_FUNC_INFO << "Loaded" << translator.language() << "translations from" << translator.filePath();
         qApp->installTranslator(&translator);
      } else {
         qWarning() << Q_FUNC_INFO << "Unable to load" << message;
      }

      return;
   }

   QLocale initSystemLocale() {
      auto systemLocale = QLocale::system();

      QStringList uiLanguages = systemLocale.uiLanguages();
      qInfo() << Q_FUNC_INFO << "System locale" << systemLocale << "with UI languages" << uiLanguages;

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
      if (PersistentSettings::contains_ck(PersistentSettings::Names::forcedLocale)) {
         QString forcedLocaleName = PersistentSettings::value_ck(PersistentSettings::Names::forcedLocale, "").toString();
         QLocale forcedLocale{forcedLocaleName};
         qInfo() <<
            Q_FUNC_INFO << "Read config setting" << PersistentSettings::Names::forcedLocale << "=" <<
            forcedLocaleName << "so overriding system locale" << systemLocale << "with" << forcedLocale <<
            "with UI languages" << forcedLocale.uiLanguages();
         // This probably isn't needed, but should force this locale into places where QLocale::system() is being called
         // instead of Localization::getLocale().  Note that QLocale::setDefault() is not reentrant, but that's OK as
         // we are guaranteed to be single-threaded here.
         QLocale::setDefault(forcedLocale);
         return forcedLocale;
      }
      return systemLocale;
   }

}

//
// NOTE that when we add a new language, we need to:
//    - Update the list below
//    - Create a new bt_*.ts file in the ../translations directory
//    - Create a new flag svg in the ../images directory
//    - Add a file alias for the new flag in ../resources.qrc
//    - Add the name of the new bt_*.ts file to both ../CMakeLists.txt and ../meson.build
//
QVector<Localization::LanguageInfo> const & Localization::languageInfo() {
   //
   // We can't safely construct this object until the Qt resource system has started up, so we use a Meyers Singleton
   // rather than just have clients directly reference statically initialised object.
   //
   static const QVector<Localization::LanguageInfo> languageInfos {
      //
      // The order here is alphabetical by language name in English (eg "German" rather then "Deutsch").  This is the same
      // as the order of the QLocale::Language enum values (see https://doc.qt.io/qt-6/qlocale.html#Language-enum).
      //
      // TODO: one day it would be nice to sort this at run-time by the name in whatever the currently-set language is.
      //
      // For languages spoken in more than one country, we usually choose the flag for the country where that language has
      // been spoken the longest (eg France for French)
      //
      {QLocale::Basque         , "eu", QIcon{":/images/flagBasque.svg"     }, "Basque"          , QObject::tr("Basque"          )},
      {QLocale::Catalan        , "ca", QIcon{":/images/flagCatalonia.svg"  }, "Catalan"         , QObject::tr("Catalan"         )},
      {QLocale::Chinese        , "zh", QIcon{":/images/flagChina.svg"      }, "Chinese"         , QObject::tr("Chinese"         )},
      {QLocale::Czech          , "cs", QIcon{":/images/flagCzech.svg"      }, "Czech"           , QObject::tr("Czech"           )},
      {QLocale::Danish         , "da", QIcon{":/images/flagDenmark.svg"    }, "Danish"          , QObject::tr("Danish"          )},
      {QLocale::Dutch          , "nl", QIcon{":/images/flagNetherlands.svg"}, "Dutch"           , QObject::tr("Dutch"           )},
      {QLocale::English        , "en", QIcon{":/images/flagUK.svg"         }, "English"         , QObject::tr("English"         )},
      {QLocale::Estonian       , "et", QIcon{":/images/flagEstonia.svg"    }, "Estonian"        , QObject::tr("Estonian"        )},
      {QLocale::French         , "fr", QIcon{":/images/flagFrance.svg"     }, "French"          , QObject::tr("French"          )},
      {QLocale::Galician       , "gl", QIcon{":/images/flagGalicia.svg"    }, "Galician"        , QObject::tr("Galician"        )},
      {QLocale::German         , "de", QIcon{":/images/flagGermany.svg"    }, "German"          , QObject::tr("German"          )},
      {QLocale::Greek          , "el", QIcon{":/images/flagGreece.svg"     }, "Greek"           , QObject::tr("Greek"           )},
      {QLocale::Hungarian      , "hu", QIcon{":/images/flagHungary.svg"    }, "Hungarian"       , QObject::tr("Hungarian"       )},
      {QLocale::Italian        , "it", QIcon{":/images/flagItaly.svg"      }, "Italian"         , QObject::tr("Italian"         )},
      {QLocale::Latvian        , "lv", QIcon{":/images/flagLatvia.svg"     }, "Latvian"         , QObject::tr("Latvian"         )},
      {QLocale::NorwegianBokmal, "nb", QIcon{":/images/flagNorway.svg"     }, "Norwegian Bokmål", QObject::tr("Norwegian Bokmål")},
      {QLocale::Polish         , "pl", QIcon{":/images/flagPoland.svg"     }, "Polish"          , QObject::tr("Polish"          )},
      {QLocale::Portuguese     , "pt", QIcon{":/images/flagBrazil.svg"     }, "Portuguese"      , QObject::tr("Portuguese"      )},
      {QLocale::Russian        , "ru", QIcon{":/images/flagRussia.svg"     }, "Russian"         , QObject::tr("Russian"         )},
      {QLocale::Serbian        , "sr", QIcon{":/images/flagSerbia.svg"     }, "Serbian"         , QObject::tr("Serbian"         )},
      {QLocale::Spanish        , "es", QIcon{":/images/flagSpain.svg"      }, "Spanish"         , QObject::tr("Spanish"         )},
      {QLocale::Swedish        , "sv", QIcon{":/images/flagSweden.svg"     }, "Swedish"         , QObject::tr("Swedish"         )},
      {QLocale::Turkish        , "tr", QIcon{":/images/flagTurkey.svg"     }, "Turkish"         , QObject::tr("Turkish"         )},
   };
   return languageInfos;
};

QLocale const & Localization::getLocale() {
   //
   // Note that we can't use std::call_once to initialise systemLocale because it runs too early, ie before
   // PersistentSettings has been initialised.  Using a static local variable is thread-safe and (at the cost of a very
   // small runtime overhead) means that the variable will not be initialised until the first call of this function
   // (which should be after PersistentSettings::initialise() has been called).
   //
   Q_ASSERT(PersistentSettings::isInitialised());
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

QString Localization::displayDateUserFormated(std::optional<QDate> const & date) {
   if (!date) {
      return "-";
   }
   return Localization::displayDateUserFormated(*date);
}


[[nodiscard]] bool Localization::isSupportedLanguage(QString const & twoLetterLanguage) {
   QVector<Localization::LanguageInfo> const & languageInfos {Localization::languageInfo()};
   auto const match = std::find_if(
      languageInfos.begin(),
      languageInfos.end(),
      [& twoLetterLanguage](auto const & record) { return twoLetterLanguage == record.iso639_1Code; }
   );
   return match != languageInfos.end();
}


void Localization::setLanguage(QLocale const & newLanguageLocale) {
   //
   // The wording here is a bit stilted, but it's because a locale can have more than one UI language
   //
   qInfo() <<
      Q_FUNC_INFO << "Changing language from that for locale" << currentLanguageLocale << "to that for locale" <<
      newLanguageLocale;
   //
   // On Linux, because there is a built-in package manager, we don't ship the Qt framework translation files.  Provided
   // qt6-translations-l10n is installed, the Qt translation files will be in a standard location (eg
   // /usr/share/qt6/translations).
   //
   // On Windows and Mac, we have to package the Qt translation files with our application.  However, this is done
   // automatically for us by windeployqt and macdeployqt.  Eg the former puts Qt's translations in the application's
   // \bin\translations subdirectory.
   //
   // In all cases, our own translations live in a different folder.
   //
   static const QDir qtFrameworkTranslationsDir{QLibraryInfo::path(QLibraryInfo::TranslationsPath)};
   static const QDir applicationTranslationsDir{Application::getResourceDir().canonicalPath() + "/translations_qm"};
   loadTranslator(qtFrameworkTranslator, newLanguageLocale, "qt_", qtFrameworkTranslationsDir);
   loadTranslator(applicationTranslator, newLanguageLocale, "bt_", applicationTranslationsDir);
   currentLanguageLocale = newLanguageLocale;
   //
   // QTranslator::language() returns "the target language as stored in the translation file", so this _should_ give us
   // back the value of language in the opening TS tag of the XML in our own bt_*.ts files, and this _should_ already be
   // a two-letter language code.
   //
   // However, for reasons I didn't get to the bottom of, it can be that we actually get back empty string from
   // QTranslator::language().  In which case, our best guess at the language that has just been set comes from the
   // name of the locale.
   //
   currentTwoLetterLanguageCode = applicationTranslator.language();
   qDebug() << Q_FUNC_INFO << "QTranslator::language() returned" << currentTwoLetterLanguageCode;
   if (currentTwoLetterLanguageCode.isEmpty()) {
      auto const qtLanguageCode = newLanguageLocale.language();
      qDebug() << Q_FUNC_INFO << "QLocale::language() returned" << qtLanguageCode;
      QVector<Localization::LanguageInfo> const & languageInfos {Localization::languageInfo()};
      auto const match = std::find_if(
         languageInfos.begin(),
         languageInfos.end(),
         [& qtLanguageCode](auto const & record) { return qtLanguageCode == record.qtLanguageCode; }
      );
      if (match != languageInfos.end()) {
         currentTwoLetterLanguageCode = match->iso639_1Code;
      } else {
         //
         // If we get here, we're out of ideas for how to deduce the language that was set
         //
         qWarning() <<
            Q_FUNC_INFO << "Could not deduce language from locale" << newLanguageLocale << ".  This may be a bug!";
         qDebug().noquote() << Q_FUNC_INFO << Logging::getStackTrace();
      }
   } else {
      // Doesn't hurt to force the length to be <=2
      currentTwoLetterLanguageCode.truncate(2);
   }
   qDebug() << Q_FUNC_INFO << "currentTwoLetterLanguageCode" << currentTwoLetterLanguageCode;
   return;
}

void Localization::setLanguage(QString const & twoLetterLanguage) {
   if (!Localization::isSupportedLanguage(twoLetterLanguage)) {
      //
      // This could be a coding error or could be bad data in a config file.  Either way, we can safely continue.
      // If it's a coding error then it's useful to have the stack trace.
      //
      qWarning() <<
         Q_FUNC_INFO << "Ignoring request to set language to" << twoLetterLanguage << "as not in translations list";
      qDebug().noquote() << Q_FUNC_INFO << Logging::getStackTrace();
      return;
   }

   qInfo() << Q_FUNC_INFO << "Changing language from" << currentTwoLetterLanguageCode << "to" << twoLetterLanguage;
   Localization::setLanguage(QLocale{twoLetterLanguage});
   // Normally the QLocale overload of Localization::setLanguage should be able to work out what language was set, but,
   // if not, we can say at least what we are trying to set!
   if (currentTwoLetterLanguageCode.isEmpty()) {
      currentTwoLetterLanguageCode = twoLetterLanguage;
   }

   if (currentTwoLetterLanguageCode != twoLetterLanguage) {
      qWarning() <<
         Q_FUNC_INFO << "Attempt to set language to" << twoLetterLanguage << "actually resulted in setting it to" <<
         currentTwoLetterLanguageCode;
   }

   return;
}

QString Localization::getCurrentLanguage() {
   return currentTwoLetterLanguageCode;
}

QString Localization::getSystemLanguage() {
   // QLocale::name() is of the form language_country,
   // where 'language' is a lowercase 2-letter ISO 639-1 language code,
   // and 'country' is an uppercase 2-letter ISO 3166 country code.
   QString localeName {Localization::getLocale().name()};
   qDebug() << Q_FUNC_INFO << "Locale name:" << localeName;
   return localeName.split("_")[0];
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

double Localization::toDouble(QString text, bool * ok) {
   // Try system locale first
   bool success = false;
   double ret = 0.0;

   try {
      QLocale sysDefault = QLocale();
      ret = sysDefault.toDouble(text, &success);

      // If we failed, try C locale (ie what QString now does by default)
      if (!success) {
         ret = text.toDouble(&success);
      }
   } catch (std::invalid_argument const & ex) {
      qWarning() << Q_FUNC_INFO << "Could not parse" << text << "as number:" << ex.what();
   } catch(std::out_of_range const & ex) {
      qWarning() << Q_FUNC_INFO << "Out of range parsing" << text << "as number:" << ex.what();
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

double Localization::toDouble(QString text, char const * const caller, bool * ok) {
   bool success = false;
   double ret = Localization::toDouble(text, &success);

   if (!success) {
      qWarning() << Q_FUNC_INFO << "(Called from" << caller << "): could not convert" << text << "to double";
   }

   if (ok) {
      *ok = success;
   }

   return ret;
}


void Localization::loadTranslations() {
   // TBD: Not sure if we really need this check here, but it's not hurting anything.
   if (!qApp) {
      return;
   }

   // If Localization::loadSettings() has already set the language, then we don't need to do anything here.  Otherwise,
   // by default, we'll try to use the language of the system locale.
   if (currentTwoLetterLanguageCode.isEmpty()) {
      QLocale systemLocale = Localization::getLocale();
      qInfo() << Q_FUNC_INFO << "Setting language based on system locale:" << systemLocale;
      Localization::setLanguage(systemLocale);
      // If that didn't work then we'll try getting the system language directly.
      if (currentTwoLetterLanguageCode.isEmpty()) {
         Localization::setLanguage(Localization::getSystemLanguage());
      }
   }

   return;
}

void Localization::loadSettings() {

   if (PersistentSettings::contains_ck(PersistentSettings::Names::language)) {
      // It can be that the config file contains an empty setting for language ("language="), in which case we should
      // ignore it
      QString savedLanguage = PersistentSettings::value_ck(PersistentSettings::Names::language, "").toString();
      if (!savedLanguage.isEmpty()) {
         qInfo() << Q_FUNC_INFO << "Config file requests language as" << savedLanguage;
         Localization::setLanguage(savedLanguage);
      }
   }

   dateFormat = static_cast<Localization::NumericDateFormat>(
      PersistentSettings::value_ck(PersistentSettings::Names::date_format, Localization::YearMonthDay).toInt()
   );
   return;
}

void Localization::saveSettings() {
   PersistentSettings::insert_ck(PersistentSettings::Names::language   , currentTwoLetterLanguageCode);
   PersistentSettings::insert_ck(PersistentSettings::Names::date_format, dateFormat);
   return;
}
