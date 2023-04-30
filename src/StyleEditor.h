/*
 * StyleEditor.h is part of Brewtarget, and is Copyright the following
 * authors 2009-2023
 * - Jeff Bailey <skydvr38@verizon.net>
 * - Matt Young <mfsy@yahoo.com>
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
#ifndef STYLEEDITOR_H
#define STYLEEDITOR_H
#pragma once

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
 *
 * \brief View/controller dialog to modify styles.
 */
class StyleEditor : public QDialog, public Ui::styleEditor {
   Q_OBJECT

public:
   StyleEditor(QWidget *parent = nullptr, bool singleSyleEditor = false);
   virtual ~StyleEditor();

   void setStyle(Style * s);

public slots:
   void save();
   void newStyle(QString folder = "");
   void removeStyle();
   void clear();
   void clearAndClose();

   void styleSelected(QString const & text);

   /**
    * \brief Receives the \c NamedEntity::changed signal emitted by the \c Style (or its base class)
    */
   void changed(QMetaProperty const property, QVariant const value);

private:
   Style * obsStyle;
   StyleListModel * styleListModel;
   StyleSortFilterProxyModel * styleProxyModel;
   void showChanges(QMetaProperty const * prop = nullptr);
};

#endif
