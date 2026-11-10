/*======================================================================================================================
 * widgets/LockAndShareBar.cpp is part of Brewtarget, and is copyright the following authors 2026:
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
#include "widgets/LockAndShareBar.h"

#include <QHBoxLayout>
#include <QPalette>

#ifdef MANUALLY_INCLUDE_MOC
   // Explicitly doing this include reduces potential problems with AUTOMOC when compiling with CMake
   #include "moc_LockAndShareBar.cpp"
#endif

LockAndShareBar::LockAndShareBar(QWidget * parent) :
   QFrame{parent},
   m_text        {new QLabel     {this}},
   m_makeCopy    {new QPushButton{tr("Make a copy for this recipe"), this}},
   m_lockOrUnlock{new QPushButton{this}} {
   this->setFrameShape(QFrame::StyledPanel);
   this->m_text->setWordWrap(true);
   this->m_text->setTextInteractionFlags(Qt::TextSelectableByMouse);
   this->m_makeCopy->setToolTip(
      tr("Make a copy that this recipe will use instead, so you can change it without affecting anything else.  "
         "Any changes you have already made in this editor are kept, and will be saved to the copy when you click "
         "Save.")
   );

   auto * layout = new QHBoxLayout{this};
   layout->addWidget(this->m_text, 1);
   layout->addWidget(this->m_makeCopy);
   layout->addWidget(this->m_lockOrUnlock);

   this->connect(this->m_makeCopy    , &QAbstractButton::clicked, this, &LockAndShareBar::makeCopyClicked);
   this->connect(this->m_lockOrUnlock, &QAbstractButton::clicked, this, [this]() {
      if (this->m_itemLocked) {
         emit this->unlockClicked();
      } else {
         emit this->lockClicked();
      }
      return;
   });

   this->setState("", false, false, false);
   return;
}

LockAndShareBar::~LockAndShareBar() = default;

void LockAndShareBar::setState(QString const & text,
                               bool const needsAttention,
                               bool const offerCopy,
                               bool const itemLocked) {
   this->m_text->setText(text);
   this->m_makeCopy->setVisible(offerCopy);
   this->m_itemLocked = itemLocked;
   if (itemLocked) {
      this->m_lockOrUnlock->setText(tr("Unlock…"));
      this->m_lockOrUnlock->setToolTip(tr("Allow this to be changed"));
   } else {
      this->m_lockOrUnlock->setText(tr("Lock"));
      this->m_lockOrUnlock->setToolTip(
         tr("Prevent this from being changed.  You will still be able to use it in recipes, and to make copies of it.")
      );
   }

   //
   // Use the tool tip background colour (typically pale yellow) to draw attention to the bar when the item is locked or
   // shared.  We use a palette colour rather than a fixed one so that this works with dark themes etc.
   //
   this->setAutoFillBackground(needsAttention);
   this->setBackgroundRole(needsAttention ? QPalette::ToolTipBase : QPalette::Window);
   this->m_text->setForegroundRole(needsAttention ? QPalette::ToolTipText : QPalette::WindowText);
   return;
}
