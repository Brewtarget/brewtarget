/*
 * AboutDialog.h is part of Brewtarget, and is Copyright the following
 * authors 2009-2015
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

#ifndef _ABOUTDIALOG_H
#define _ABOUTDIALOG_H

#include <QDialog>
#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QSpacerItem>
#include "config.h"

/*!
 * \class AboutDialog
 * \author Philip G. Lee
 *
 * \brief Simple "about" dialog for Brewtarget.
 */
class AboutDialog : public QDialog
{
public:
   AboutDialog(QWidget* parent=0)
           : QDialog(parent),
             label(0)
   {
      setObjectName("aboutDialog");
      doLayout();

      // Do not translate this. It is important that the copyright/license
      // text is not altered.
      label->setText(
         QString::fromUtf8(
            "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">"
            "<html>"
            " <head>"
            "  <style type=\"text/css\">"
            "  </style>"
            " </head>"
            ""
            " <h1>Brewtarget %1</h1>"
            " <p>"
            "  Brewtarget, for developing beer recipes, is Copyright 2009-2015"
            "  by the following developers."
            " </p>"
            " <h2>Developers</h2>"
            " <ul>"
            "  <li>Philip G. Lee &lt;rocketman768@gmail.com&gt; -- Lead Developer</li>"
            "  <li>Mik Firestone &lt;mikfire@gmail.com&gt;</li>"
            "  <li>Maxime Lavigne &lt;duguigne@gmail.com&gt;</li>"
            "  <li>Theophane Martin &lt;theophane.m@gmail.com&gt;</li>"
            "  <li>Dan Cavanagh &lt;dan@dancavanagh.com&gt;</li>"
            "  <li>Rob Taylor &lt;robtaylor@floopily.org&gt;</li>"
            "  <li>Kregg K &lt;gigatropolis@yahoo.com&gt;</li>"
            "  <li>A.J. Drobnich &lt;aj.drobnich@gmail.com&gt;</li>"
            "  <li>Ted Wright &lt;tedwright@users.sourceforge.net&gt;</li>"
            "  <li>Charles Fourneau (plut0nium) &lt;charles.fourneau@gmail.com&gt;</li>"
            "  <li>Samuel Östling &lt;MrOstling@gmail.com&gt;</li>"
            "  <li>Peter Buelow &lt;goballstate@gmail.com&gt;</li>"
            "  <li>David Grundberg &lt;individ@acc.umu.se&gt;</li>"
            "  <li>Daniel Pettersson &lt;pettson81@gmail.com&gt;</li>"
            "  <li>Tim Payne &lt;swstim@gmail.com&gt;</li>"
            "  <li>Luke Vincent &lt;luke.r.vincent@gmail.com&gt;</li>"
            "  <li>Eric Tamme &lt;etamme@gmail.com&gt;</li>"
            "  <li>Chris Pavetto &lt;chrispavetto@gmail.com&gt;</li>"
            "  <li>Markus Mårtensson &lt;mackan.90@gmail.com&gt;</li>"
            "  <li>Julein &lt;j2bweb@gmail.com&gt;</li>"
            "  <li>Jeff Bailey &lt;skydvr38@verizon.net&gt;</li>"
            "  <li>Piotr Przybyla (przybysh) &lt;przybysh@gmail.com&gt;</li>"
            "  <li>Chris Hamilton &lt;marker5a@gmail.com&gt;</li>"
            "  <li>Julian Volodia &lt;julianvolodia@gmail.com&gt;/li>"
            "  <li>Jerry Jacobs &lt;jerry@xor-gate.org&gt;</li>"
            "  <li>Gregg Meess &lt;Daedalus12@gmail.com&gt;</li>"
            " </ul>"
            ""
            " <h2>License (GPLv3)</h2>"
            " <p>"
            "  <pre>"
            "  Brewtarget is free software: you can redistribute it and/or modify"
            "  it under the terms of the GNU General Public License as published by"
            "  the Free Software Foundation, either version 3 of the License, or"
            "  (at your option) any later version."
            "  <br/><br/>"
            "  Brewtarget is distributed in the hope that it will be useful,"
            "  but WITHOUT ANY WARRANTY; without even the implied warranty of"
            "  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the"
            "  GNU General Public License for more details."
            "  <br/><br/>"
            "  You should have received a copy of the GNU General Public License"
            "  along with Brewtarget.  If not, see &lt;<a href=\"http://www.gnu.org/licenses/\">http://www.gnu.org/licenses/</a>&gt;"
            "  </pre>"
            " </p>"
            ""
            " <h2>Source Code</h2>"
            " <p>"
            "  Brewtarget's source code is located at <a href=\"https://github.com/Brewtarget/brewtarget\">github.com/Brewtarget/brewtarget</a>"
            " </p>"
            "</html>"
         )
         .arg(VERSIONSTRING)
      );
   }

   void changeEvent(QEvent* event)
   {
      if(event->type() == QEvent::LanguageChange)
         retranslateUi();
      QDialog::changeEvent(event);
   }

   //! \name Public UI Variables
   //! @{
   QLabel* label;
   //! @}

private:

   void doLayout()
   {
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
      retranslateUi();
   }

   void retranslateUi()
   {
      setWindowTitle(tr("About Brewtarget"));
   }
};

#endif   /* _ABOUTDIALOG_H */
