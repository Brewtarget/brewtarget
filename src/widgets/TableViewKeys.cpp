/*======================================================================================================================
 * widgets/TableViewKeys.cpp is part of Brewtarget, and is copyright the following authors 2026:
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
#include "widgets/TableViewKeys.h"

#include <QAbstractItemView>
#include <QEvent>
#include <QKeyEvent>
#include <QObject>

namespace {
   /**
    * \brief Event filter that intercepts the Delete/Backspace key on the view it is installed on.
    *
    *        We don't need the Q_OBJECT macro as we have no signals or slots.
    */
   class DeleteKeyFilter : public QObject {
   public:
      DeleteKeyFilter(QAbstractItemView & view, std::function<void()> onDelete) :
         QObject{&view},
         m_view{view},
         m_onDelete{std::move(onDelete)} {
         return;
      }

      bool eventFilter(QObject * watched, QEvent * event) override {
         if (watched == &this->m_view) {
            if (event->type() == QEvent::ShortcutOverride || event->type() == QEvent::KeyPress) {
               auto const * keyEvent = static_cast<QKeyEvent const *>(event);
               if (isDeleteKey(*keyEvent)) {
                  if (event->type() == QEvent::ShortcutOverride) {
                     // Accepting the ShortcutOverride event means the key gets delivered to the view as a normal key
                     // press, rather than triggering a shortcut such as MainWindow::actionDeleteSelected
                     event->accept();
                  } else {
                     this->m_onDelete();
                  }
                  return true;
               }
            }
         }
         return this->QObject::eventFilter(watched, event);
      }

   private:
      static bool isDeleteKey(QKeyEvent const & keyEvent) {
         // We only want the plain key, not, eg, Ctrl+Backspace
         if (keyEvent.modifiers() & ~Qt::KeypadModifier) {
            return false;
         }
         return keyEvent.key() == Qt::Key_Delete || keyEvent.key() == Qt::Key_Backspace;
      }

      QAbstractItemView & m_view;
      std::function<void()> m_onDelete;
   };
}

void TableViewKeys::handleDeleteKey(QAbstractItemView & view, std::function<void()> onDelete) {
   // The filter is a child of the view, so it will get deleted when the view does
   view.installEventFilter(new DeleteKeyFilter{view, std::move(onDelete)});
   return;
}
