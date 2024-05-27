/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * widgets/InfoText.cpp is part of Brewtarget, and is copyright the following authors 2023:
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
#include "widgets/InfoText.h"

#include <QDebug>

#include "widgets/InfoButton.h"

InfoText::InfoText(QWidget * parent, Qt::WindowFlags flags) : QLabel(parent, flags), m_infoButton{nullptr} {
   this->setWordWrap(true);
   //
   // Note that ideally we would specify spacing etc here in points rather than pixels to be independent of display
   // resolution.  HOWEVER, per https://doc.qt.io/qt-5/stylesheet-reference.html#length, Qt only supports points for
   // font sizes and other lengths must be in px (pixels), em (width of 'M' in current font) or ex (height of 'x' in
   // current font).
   //
   // Ideally we would automatically keep the colors here in sync with those in InfoButton, but currently waiting for
   // the Qt API to settle down in this area so we can specify colors in the same way in both places.  (Setting a QColor
   // object using color codes in "#RRGGBB" format is achieved by calling QColor::setNamedColor post-construction until
   // Qt 6.6 when you are supposed to switch to using the QColor::fromString factory function.)
   //
   this->setStyleSheet(
      "QLabel {"
         " background-color : #148263;"
         " color : #f8e027;"
         " padding : 0.5ex;"
         " spacing : 1ex;"
         " border-radius : 0.5ex;"
      "}"
   );

   //
   // You might think that, at this point, we ought to be able to discover the parent dialog (eg EquipmentEditor) from
   // which we are being created -- eg by following a chain of QWidget * parent pointers and calling the
   // QWidget::parentWidget() member function on each one, or simply by invoking the QWidget::window() member function.
   // However, this is unlikely to work in practice.  The code generated from the .ui file does not always set the
   // parent parameter when it constructs a widget.  Eg, in the code generated for equipmentEditor.ui, we might see
   // something along the following lines:
   //
   //    tabWidget_editor = new QTabWidget(equipmentEditor);
   //    ...
   //    tab_general = new QWidget();
   //    ...
   //    infoButton_name = new InfoButton(tab_general);
   //    ...
   //    infoText_name = new InfoText(tab_general);
   //    ...
   //    tabWidget_editor->addTab(tab_general, QString());
   //    ...
   //
   // This means that, in the InfoText and InfoButton constructors, we can only get our direct parent, but cannot access
   // the "ultimate" parent (EquipmentEditor, which inherits from Ui_equipmentEditor).
   //
   // So, my original plan to subclass QDialog and add a signal that EquipmentEditor etc could send after executing
   // setupUi didn't fly.  Fortunately, the alternative of piggy-backing on setText seems to work just fine.
   //
   return;
}

InfoText::~InfoText() = default;

void InfoText::setText(const QString & text) {
   // We don't need to alter the base class functionality, we are just piggy-backing on it
   this->QLabel::setText(text);

   // Now is a chance to initialise our buddy, if we didn't already.
   if (!m_infoButton) {
      QWidget * buddy = this->buddy();
      if (buddy) {
         m_infoButton = qobject_cast<InfoButton *>(buddy);
         // It's a coding error if we don't have a buddy at this point or it's not an InfoButton
         Q_ASSERT(m_infoButton);
         m_infoButton->linkWith(this);
      }
   }
   return;
}


void InfoText::mouseReleaseEvent(QMouseEvent * event) {
   // Not sure the base class actually does anything, but good form to call it
   this->QLabel::mouseReleaseEvent(event);
///   qDebug() << Q_FUNC_INFO << "Buddy" << this->buddy();
   // Simplest way to achieve what we want and keep the button and info text in sync is to tell the button it was
   // clicked.
   if (m_infoButton && this->isVisible()) {
      m_infoButton->doClick();
   }
   return;
}



///void InfoText::connectWithBuddy() {
///   QWidget * buddy = this->buddy();
///   InfoButton * buddyButton = qobject_cast<InfoButton *>(buddy);
///   // It's a coding error if we don't have a buddy at this point or it's not an InfoButton
///   Q_ASSERT(buddyButton);
///   buddyButton->linkWith(this);
///   return;
///}

void InfoText::show() {
   this->QLabel::show();
   this->resizeParent();
   return;
}

void InfoText::hide() {
   this->QLabel::hide();
   this->resizeParent();
   return;
}

void InfoText::resizeParent() {
   this->adjustSize();
   QWidget * parentWidget = this->parentWidget();
   if (parentWidget) {
      parentWidget->adjustSize();
   }
   return;
}
