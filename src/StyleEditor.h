/*
 * StyleEditor.h is part of Brewtarget, and is Copyright the following
 * authors 2009-2014
 * - Jeff Bailey <skydvr38@verizon.net>
 * - Mik Firestone <mikfire@gmail.com>
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

#ifndef _STYLEEDITOR_H
#define   _STYLEEDITOR_H

class StyleEditor;

#include <QDialog>
#include <QMetaProperty>
#include <QVariant>
#include "ui_styleEditor.h"

// Forward declarations.
class Style;
class StyleListModel;
class StyleSortFilterProxyModel;

/*!
 * \class StyleEditor
 * \author Philip G. Lee
 *
 * \brief View/controller dialog to modify styles.
 */
class StyleEditor : public QDialog, public Ui::styleEditor
{
   Q_OBJECT

public:
   StyleEditor( QWidget *parent=0, bool singleSyleEditor = false );
   virtual ~StyleEditor() {}
   void setStyle( Style* s );

   void newStyle(QString folder);

public slots:
   void save();
   void newStyle();
   void removeStyle();
   void clear();
   void clearAndClose();

   void styleSelected( const QString& text );
   void changed(QMetaProperty,QVariant);

private:
   Style* obsStyle;
   StyleListModel* styleListModel;
   StyleSortFilterProxyModel* styleProxyModel;
   void showChanges(QMetaProperty* prop = 0);
};

#endif   /* _STYLEEDITOR_H */

