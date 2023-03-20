/*
 * AboutDialog.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2023
 * - Matt Young <mfsy@yahoo.com>
 * - Philip Greggory Lee <rocketman768@gmail.com>
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
#include "AboutDialog.h"

#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QScrollArea>
#include <QSpacerItem>
#include "config.h"

AboutDialog::AboutDialog(QWidget * parent) :
   QDialog(parent),
   label(0) {
   setObjectName("aboutDialog");
   doLayout();

   //
   // Do not translate this. It is important that the copyright/license text is not altered.
   //
   // This is the master list of copyrights from which we construct the one in the COPYRIGHT file in the root directory.
   // (We do it that way around because it's easier to add comments here!)  This is our best effort at constructing the
   // full list of contributors but there are omissions where we don't have sufficient information to attribute someone.
   // IF YOUR NAME SHOULD BE ON THIS LIST AND ISN'T OR IF YOU KNOW OF ANY OTHER CORRECTIONS, PLEASE CONTACT US (VIA
   // https://github.com/Brewtarget/brewtarget/issues) SO WE CAN RECTIFY IT.
   //
   // We need the years in which people made their contributions so that we can abide by the expected copyright format
   // in deb package files on Linux.
   //
   // Constructing this list was a bit fun.  The primary sources of information were:
   //   - Git commits to the Brewtarget project - which need a bit of manual wrangling because different commits by the
   //                                             same person sometimes have different variations on their name or
   //                                             email.
   //   - Git commits to the Brewken project
   //   - Comment headers in the source code - which are sometimes missing info (eg because someone omitted to update
   //                                          the comment when they made a change) but also contain authors not present
   //                                          in the Git commits, such as when other open-source code is copied-and-
   //                                          pasted into our code base.  (We do _not_ include all the copyrights from
   //                                          libraries and frameworks because that would be unmanageable.)
   //
   // For the git commits, we ran this command:
   //
   //    git log --format='%aN <%aE> ÷÷%ai' | sed 's/÷÷\([0-9]*\)-.*$/\1/' | sort -u
   //
   // and then did some manual de-duplication and tidying up.  In particular, note that we have dropped entries where we
   // don't have both real name and a valid email address and have not been able to determine them with a few searches.
   // These are marked in comments below.
   //
   // For the comments, we ran this command from the source directory (this gets most C++ header comments, but not from
   // make files and shell scripts etc):
   //
   //    find . -type f -name "*.cpp" -o -type f -name "*.h" | while read ii;
   //    do awk '/@/{print} /\*\//{exit}' $ii
   //    done | sort -u | sed 's/^[^A-Za-z]*//' | uniq
   //
   // Process for annual updates is simpler.  Eg, if you want committers from 2022:
   //
   //    git log --format='%aN <%aE> ÷÷%ai' | grep ÷÷2022 | sed 's/÷÷\([0-9]*\)-.*$/\1/' | sort -u
   //
   // Finally, to process this source file(!) into the format we use in the COPYRIGHT file, it's:
   //
   //    cat src/AboutDialog.cpp | awk '/"  <li>.*@/{print}' | sed 's/.*<li>/   /; s+</li>.*$++; s/&lt;/</; s/&gt;/>/'
   //
   // .:TODO:. We should probably get Meson or the `bt` build script to do some of the processing
   //
   label->setText(
      QString::fromUtf8(
         "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">"
         "<html>"
         " <head>"
         "  <style type=\"text/css\">"
         "  li {font-family: \"Lucida Console\", \"Courier New\", monospace;}"
         "  </style>"
         " </head>"
         ""
         " <h1>Brewtarget %1</h1>"
         " <p>"
         "  Brewtarget, free software for developing beer recipes, is copyright:"
         " </p>"
         " <ul>"
         "  <li>2018      Adam Hawes &lt;ach@hawes.net.au&gt;</li>"
         "  <li>2015-2016 Aidan Roberts &lt;aidanr67@gmail.com&gt;</li>"
         "  <li>2012      A.J. Drobnich &lt;aj.drobnich@gmail.com&gt;</li>"
         //     2017      André Rodrigues <andre@sabayon.local>              // Invalid email address
         "  <li>2021      Artem Martynov &lt;martynov-a@polyplastic.by&gt;</li>"
         //     2021      Artsiom <xzfantom@gmail.com>                       // No second name - probably https://github.com/xzfantom
         "  <li>2016-2018 Blair Bonnett &lt;blair.bonnett@gmail.com&gt;</li>"
         "  <li>2017      Brian Rower &lt;brian.rower@gmail.com&gt;</li>"
         "  <li>2016      Carles Muñoz Gorriz &lt;carlesmu@internautas.org&gt;</li>"
         //     2011      Charles Fourneau [plut0nium]</li>"                 // Invalid email address - probably https://github.com/plut0nium
         "  <li>2012      Christopher Hamilton &lt;marker5a@gmail.com&gt;</li>" // Commit is "marker5a <unsure>", but email matches posts from Christopher Hamilton at https://zfsonlinux.topicbox.com/groups/zfs-discuss/T8d6d9f2d30940caa-Mb986306739b7cd8cca97865e!
         "  <li>2015      Chris Pavetto &lt;chrispavetto@gmail.com&gt;</li>"
         "  <li>2019-2021 Chris Speck &lt;cgspeck@gmail.com&gt;</li>"
         "  <li>2010-2013 Dan Cavanagh &lt;dan@dancavanagh.com&gt;</li>"
         //     2016      Daniel Moreno <danielm5@users.noreply.github.com>  // Invalid email address - probably https://github.com/danielm5
         "  <li>2015-2020 Daniel Pettersson &lt;pettson81@gmail.com&gt;</li>"
         "  <li>2013      David Grundberg &lt;individ@acc.umu.se&gt;</li>"
         //     2016-2017 eltomek <tlorek@gmail.com>                         // No second name - probably https://github.com/eltomek
         "  <li>2010      Eric Tamme &lt;etamme@gmail.com&gt;</li>"
         //     2015      f1oki <f1oki@gmx.com>                              // No name - probably https://github.com/f1oki
         "  <li>2016      Greg Greenaae &lt;ggreenaae@gmail.com&gt;</li>"
         "  <li>2015-2017 Greg Meess &lt;Daedalus12@gmail.com&gt;</li>"
         "  <li>2019      Idar Lund &lt;idarlund@gmail.com&gt;</li>"
         "  <li>2016-2020 Iman Ahmadvand &lf;iman72411@gmail.com&gt;</li>"   // Code by https://stackoverflow.com/users/5446734/iman4k lifted
                                                                             // from https://stackoverflow.com/questions/14780517/toggle-switch-in-qt
                                                                             // and further modified for use in widgets/ToggleSwitch.*,
                                                                             // widgets/Animator.*, widgets/SelectionControl.*
         "  <li>2017-2019 Jamie Daws &lt;jdelectronics1@gmail.com&gt;</li>"
         "  <li>2019      Jean-Baptiste Wons &lt;wonsjb@gmail.com&gt;</li>"
         "  <li>2011      Jeff Bailey &lt;skydvr38@verizon.net&gt;</li>"
         "  <li>2015      Jerry Jacobs &lt;jerry@xor-gate.org&gt;</li>"
         "  <li>2019      Joe Aczel &lt;jaczel@fastmail.com.au&gt;</li>"     // Commit is "Jaczel <jaczel@fastmail.com.au>".  Name from https://github.com/jaczel
         "  <li>2017-2018 Jonatan Pålsson &lt;jonatan.p@gmail.com&gt;</li>"
         "  <li>2017-2018 Jonathon Harding &lt;github@jrhardin.net&gt;</li>" // See also https://github.com/kapinga
         //     2011      Julein <j2bweb@gmail.com>                          // No second name
         "  <li>2015      Julian Volodia &lt;julianvolodia@gmail.com&gt;</li>"
         "  <li>2012-2015 Kregg Kemper &lt;gigatropolis@yahoo.com&gt;</li>"
         "  <li>2012      Luke Vincent &lt;luke.r.vincent@gmail.com&gt;</li>"
         "  <li>2018      Marcel Koek &lt;koek.marcel@gmail.com&gt;</li>"
         "  <li>2016      Mark de Wever &lt;koraq@xs4all.nl&gt;</li>"
         "  <li>2015      Markus Mårtensson &lt;mackan.90@gmail.com&gt;</li>"
         "  <li>2017      Matt Anderson &lt;matt.anderson@is4s.com&gt;</li>" // Commit is "andersonm <matt.anderson@is4s.com>", but second name clear from email
         "  <li>2020-2022 Mattias Måhl &lt;mattias@kejsarsten.com&gt;</li>"
         "  <li>2020-2023 Matt Young &lt;mfsy@yahoo.com&gt;</li>"
         "  <li>2014-2017 Maxime Lavigne &lt;duguigne@gmail.com&gt;</li>"
         "  <li>2018      Medic Momcilo &lt;medicmomcilo@gmail.com&gt;</li>"
         "  <li>2016      Mike Evans &lt;mikee@saxicola.co.uk&gt;</li>"
         "  <li>2010-2023 Mik Firestone &lt;mikfire@gmail.com&gt;</li>"
         "  <li>2016      Mikhail Gorbunov &lt;mikhail@sirena2000.ru&gt;</li>"
         //     2016      mik <mik@suse.ztech.us>                            // Incomplete name
         "  <li>2016      Mitch Lillie &lt;mitch@mitchlillie.com&gt;</li>"
         "  <li>2017      Padraic Stack &lt;padraic.stack@gmail.com>&gt;</li>"
         "  <li>2013      Peter Buelow &lt;goballstate@gmail.com&gt;</li>"
         "  <li>2018-2020 Peter Urbanec &lt;git.user@urbanec.net>&gt;</li>"
         "  <li>2009-2018 Philip Greggory Lee &lt;rocketman768@gmail.com&gt;</li>"
         //     2011      przybysh                                           // Commit is "przybysh <unsure>"
         //     2018      Priceless Brewing <shadowchao99@gmail.com>         // Probably https://pricelessbrewing.github.io/ = Mark, but don't have second name
         "  <li>2009-2010 Rob Taylor &lt;robtaylor@floopily.org&gt;</li>"
         "  <li>2016-2018 Ryan Hoobler &lt;rhoob@yahoo.com&gt;</li>"         // Probably https://github.com/rhoob
         "  <li>2014-2015 Samuel Östling &lt;MrOstling@gmail.com&gt;</li>"
         "  <li>2016      Scott Peshak &lt;scott@peshak.net&gt;</li>"
         "  <li>2009      Ted Wright &lt;tedwright@users.sourceforge.net&gt;</li>"
         "  <li>2015-2016 Théophane Martin &lt;theophane.m@gmail.com&gt;</li>"
         "  <li>2013      Tim Payne &lt;swstim@gmail.com&gt;</li>"           // Probably https://github.com/swstim
         "  <li>2016      Tyler Cipriani &lt;tcipriani@wikimedia.org&gt;</li>"
         //     2013      U-CHIMCHIM\mik <mik@chimchim.(none)>               // Incomplete name and email
         " </ul>"
         ""
         // **********************************************************************************************************
         // * Note that the HTML source indentation here is different than above so that we don't pick up testers as *
         // * copyright holders in the awk command above!                                                            *
         // **********************************************************************************************************
         " <p>The following people have made notable contributions with testing and bug reports:</p>"
         " <ul>"
         "  <li>Mik Firestone &lt;mikfire@gmail.com&gt;</li>"
         "  <li>Nikolas &quot;Jazzbeerman&quot; </li>"
         " </ul>"
         ""
         " <h2>License (GPLv3)</h2>"
         " <p>"
         "  Brewtarget is free software: you can redistribute it and/or modify<br/>"
         "  it under the terms of the GNU General Public License as published by<br/>"
         "  the Free Software Foundation, either version 3 of the License, or<br/>"
         "  (at your option) any later version.<br/>"
         "  <br/>"
         "  Brewtarget is distributed in the hope that it will be useful,<br/>"
         "  but WITHOUT ANY WARRANTY; without even the implied warranty of<br/>"
         "  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the<br/>"
         "  GNU General Public License for more details.<br/>"
         "  <br/>"
         "  You should have received a copy of the GNU General Public License<br/>"
         "  along with Brewtarget.  If not, see &lt;<a href=\"http://www.gnu.org/licenses/\">http://www.gnu.org/licenses/</a>&gt;"
         " </p>"
         ""
         " <h2>Source Code</h2>"
         " <p>"
         "  Brewtarget's source code is available at <a href=\"https://github.com/Brewtarget/brewtarget\">github.com/Brewtarget/brewtarget</a>"
         " </p>"
         "</html>"
      )
      .arg(CONFIG_VERSION_STRING)
   );
   return;
}


void AboutDialog::changeEvent(QEvent* event) {
   if (event->type() == QEvent::LanguageChange) {
      retranslateUi();
   }
   QDialog::changeEvent(event);
   return;
}


void AboutDialog::doLayout()  {
   QVBoxLayout* verticalLayout = new QVBoxLayout(this);
      QScrollArea* scrollArea = new QScrollArea(this);
         label = new QLabel(scrollArea);
         scrollArea->setWidgetResizable(true);
         scrollArea->setWidget(label);
      QHBoxLayout* horizontalLayout = new QHBoxLayout;
         QSpacerItem* horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
         horizontalLayout->addItem(horizontalSpacer);
      verticalLayout->addWidget(scrollArea);
      verticalLayout->addLayout(horizontalLayout);
   this->retranslateUi();
   return;
}

void AboutDialog::retranslateUi() {
   setWindowTitle(tr("About Brewtarget"));
   return;
}
