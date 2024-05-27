/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * HelpDialog.cpp is part of Brewtarget, and is copyright the following authors 2021-2024:
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
#include "HelpDialog.h"

#include <QEvent>
#include <QLabel>
#include <QTextStream>
#include <QVBoxLayout>
#include <QWidget>

#include "config.h" // For CONFIG_VERSION_STRING
#include "Logging.h"
#include "PersistentSettings.h"

// This private implementation class holds all private non-virtual members of HelpDialog
class HelpDialog::impl {

public:

   /**
    * Constructor
    *
    * It should be safe to pass in a reference to HelpDialog from its constructor because there is nothing else in that
    * class to initialise by the time this pimpl constructor is being called.
    */
   impl(HelpDialog & helpDialog) : label{ new QLabel{} },
                                   layout{ new QVBoxLayout{&helpDialog} } {
      // Create the layout
      this->layout->addWidget(this->label.get());

      // We want they hyperlinks in the text to be clickable (opening in the user's default web browser)
      this->label->setOpenExternalLinks(true);

      this->setText(helpDialog);

      return;
   }

   /**
    * Destructor
    */
   ~impl() = default;

   /**
    * Set the text.  This is a separate function because we want to be able to redisplay in a different language.
    */
   void setText(HelpDialog & helpDialog) {
      static QString const wikiUrl   = QString{"%1/wiki"  }.arg(CONFIG_HOMEPAGE_URL);
      static QString const issuesUrl = QString{"%1/issues"}.arg(CONFIG_HOMEPAGE_URL);
      QString mainText;
      QTextStream mainTextAsStream{&mainText};
      mainTextAsStream <<
         "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">"
         "<html>"
         "<head>"
         "<style type=\"text/css\">"
         "</style>"
         "</head>"
         ""
         "<h1>" << CONFIG_APPLICATION_NAME_UC << "</h1>"
         "version " << CONFIG_VERSION_STRING << " " << HelpDialog::tr("for") << " " << QSysInfo::prettyProductName() <<
         "<h2>" << HelpDialog::tr("Online Help") << "</h2>"
         "<p>" <<
         HelpDialog::tr("<p>The %1 wiki is at "
                        "<a href=\"%2\">%2</a>.</p>").arg(CONFIG_APPLICATION_NAME_UC, wikiUrl) <<
         "<p>" <<
         HelpDialog::tr("If you find a bug, or have an idea for an enhancement, please raise an issue at <br/>"
                        "<a href=\"%1\">%1</a>.").arg(issuesUrl) <<
         "</p>"
         "<h2>" << HelpDialog::tr("Your Data") << "</h2>"
         "<p>" <<
         HelpDialog::tr("Recipes, ingredients and other important data are stored in one or more files in the "
                        "following folder (which is configurable via the 'Tools > Options' menu):") <<
         "</p>"
         "<ul>"
         "<li><pre>" << this->makeClickableDirLink(PersistentSettings::getUserDataDir().absolutePath()) << "</pre></li>"
         "</ul>"
         "<p>" << HelpDialog::tr("It is a good idea to take regular backups of this folder.") << "</p>"
         "<h2>" << HelpDialog::tr("Settings and Log files") << "</h2>"
         "<p>" <<
         HelpDialog::tr("The contents of the following folder(s) can be helpful for diagnosing problems:") <<
         "<ul>"
         "<li>" << HelpDialog::tr("Configuration:") << "<pre>" << this->makeClickableDirLink(PersistentSettings::getConfigDir().absolutePath()) << "</pre></li>"
         "<li>" << HelpDialog::tr("Logs:") << "<pre>" << this->makeClickableDirLink(Logging::getDirectory().absolutePath()) << "</pre></li>"
         "</ul>" <<
         HelpDialog::tr("The location of the log files can be configured via the 'Tools > Options' menu.") <<
         "</p>"
         "</html>";
      this->label->setText(mainText);

      helpDialog.setWindowTitle(HelpDialog::tr("Help"));
      return;
   }

   /**
    * Given a path to a directory, make a link that will allow the the user to open that directory in
    * Explorer/Finder/Dolphin/etc
    */
   QString makeClickableDirLink(QString const & directoryPath) {
      return QString{"<a href=\"file:///%1\">%1</a>"}.arg(directoryPath);
   }

   std::unique_ptr<QLabel> label;
   std::unique_ptr<QVBoxLayout> layout;

};


HelpDialog::HelpDialog(QWidget * parent) : QDialog(parent),
                                           pimpl{std::make_unique<impl>(*this)} {
   this->setObjectName("helpDialog");
   this->pimpl->setText(*this);
   return;
}

// See https://herbsutter.com/gotw/_100/ for why we need to explicitly define the destructor here (and not in the
// header file)
HelpDialog::~HelpDialog() = default;


void HelpDialog::changeEvent(QEvent* event) {
   if (event->type() == QEvent::LanguageChange) {
      this->pimpl->setText(*this);
   }
   // Pass the event down to the base class
   QDialog::changeEvent(event);
   return;
}
