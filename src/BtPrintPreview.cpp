/*
 * BtPrintPreview.cpp is part of Brewtarget, and is copyright the following
 * authors 2021:
 *   • Matt Young <mfsy@yahoo.com>
 *
 * Brewtarget is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * Brewtarget is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "BtPrintPreview.h"

#include <QEvent>
#include <QFile>
#include <QLabel>
#include <QPrinter>
#include <QScrollArea>
#include <QTextDocument>
#include <QTextStream>
#include <QVBoxLayout>
#include <QWidget>


// This private implementation class holds all private non-virtual members of BtPrintPreview
class BtPrintPreview::impl {
public:

   /**
    * Constructor
    */
   impl(BtPrintPreview * btPrintPreview) : verticalLayout{new QVBoxLayout{btPrintPreview}},
                                           scrollArea{new QScrollArea{btPrintPreview}},
                                           label{new QLabel{btPrintPreview}},
                                           document{new QTextDocument{btPrintPreview}} {
      //
      // Qt documentation says "QLabel is well-suited to display small rich text documents, such as small documents
      // that get their document specific settings (font, text color, link color) from the label's palette and font
      // properties. For large documents, use QTextEdit in read-only mode instead. QTextEdit can also provide a
      // scroll bar when necessary."
      //
      // We use QLabel here because we don't have to size it.  Qt will set it to a sensible size for the content.
      // (AFAICT there isn't a way to do this for QTextBrowser or QTextEdit.)
      //
      // We do have to do some small extra work to enable scroll bars (for if the user resizes window, or there's a
      // lot of content) and to fix the background color (default grey) and vertical alignment (default centered).
      // But, other than that, this seems to work.
      //
      // A more significant problem is that QLabel doesn't have a print function (whereas QTextBrowser and QTextEdit
      // do).  From Qt 6.1, QLabel can display the contents of a linked QTextDocument via
      // QLabel::setResourceProvider(), but we're not yet using Qt 6.1.  So, for the moment, we just put the same
      // content in a non-displayed QTextDocument.
      //
      this->label->setStyleSheet("QLabel { background-color : white; color : black; }");
      // Note, per Qt documentation, that Qt::AlignLeft means "logical left" and will actually align right if we're
      // displaying a right-to-left language such as Hebrew.  In other words, this alignment setting works for both
      // left-to-right and right-to-left languages.
      this->label->setAlignment(Qt::AlignLeft | Qt::AlignTop);

      this->scrollArea->setWidgetResizable(true);
      this->scrollArea->setWidget(label);
      this->verticalLayout->addWidget(scrollArea);

      return;
   }

   /**
    * Destructor
    *
    * Note that we don't have to destroy any of the objects we created because Qt will handle that for us.  (Per
    * https://doc.qt.io/qt-5/objecttrees.html, "When you create a QObject with another object as parent, it's added to
    * the parent's children() list, and is deleted when the parent is.")
    */
   ~impl() = default;


   void retranslateUi() {
      return;
   }


   QVBoxLayout * verticalLayout;
   QScrollArea * scrollArea;
   QLabel * label;
   QTextDocument * document;

};


BtPrintPreview::BtPrintPreview(QWidget * parent) :
   QDialog(parent),
   pimpl{ new impl{this} } {
   return;
}

// See https://herbsutter.com/gotw/_100/ for why we need to explicitly define the destructor here (and not in the
// header file)
BtPrintPreview::~BtPrintPreview() = default;

void BtPrintPreview::setContent(QString const & content) {
   this->pimpl->label->setText(content);
   this->pimpl->document->setHtml(content);
   return;
}

void BtPrintPreview::print(QPrinter* printer) {
   this->pimpl->document->print(printer);
   return;
}

void BtPrintPreview::exportHtml(QFile* file) {
   QTextStream(file) << this->pimpl->document;
   return;
}

void BtPrintPreview::changeEvent(QEvent * event) {
   if (event->type() == QEvent::LanguageChange) {
      this->pimpl->retranslateUi();
   }
   QDialog::changeEvent(event);
   return;
}
