/*======================================================================================================================
 * widgets/BtComboBoxObject.h is part of Brewtarget, and is copyright the following authors 2024-2025:
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
#ifndef WIDGETS_BTCOMBOBOXOBJECT_H
#define WIDGETS_BTCOMBOBOXOBJECT_H
#pragma once

#include <QComboBox>
#include <QDebug>
#include <QString>
#include <QTextStream>
#include <QVariant>

#include "utils/BtStringConst.h"

//======================================================================================================================
//========================================== Start of property name constants ==========================================
// See comment in model/NamedEntity.h
#define AddPropertyName(property) namespace PropertyNames::BtComboBoxObject { inline BtStringConst const property{#property}; }
AddPropertyName(currentId)
#undef AddPropertyName
//=========================================== End of property name constants ===========================================
//======================================================================================================================

/**
 * \brief Extends \c QComboBox to show a list of a particular \c NamedEntity - eg \c Style for a \c Recipe.  Used in
 *        conjunction with \c BtComboBoxObjectBase to make \c BtComboBoxStyle etc, because classes inheriting
 *        from QObject can't be templated, as the Qt MOC won't be able to process them.  (See comment in
 *        qtModels/tableModels/BtTableModel.h for more info.)
 */
class BtComboBoxObject : public QComboBox {
   Q_OBJECT

protected:
   BtComboBoxObject(char const * const name, QWidget * parent);
   virtual ~BtComboBoxObject();

public:
   //=================================================== PROPERTIES ====================================================
   /**
    * \brief The ID of the selected \c NamedEntity.
    *        This read-only property is only used by \c ScaleRecipeTool so we can use \c QWizardPage::registerField
    */
   Q_PROPERTY(int currentId    READ getCurrentId)

   [[nodiscard]] int getCurrentId() const;

   /**
    * \brief Similar to \c SmartField::getAsVariant.  Wrapper around \c BtComboBoxObjectBase::getItemRaw.
    */
   [[nodiscard]] virtual QVariant getAsVariant() const = 0;

   /**
    * \brief Set the ID of the selected \c NamedEntity.  This should only be called via
    *        \c BtComboBoxObjectBase::setItem() (which does additional checks).
    *
    * \param value -1 means nothing selected
    */
   void setCurrentId(int value);

   void setDefault();

   /**
    * \brief Similar to \c SmartField::setFromVariant.  Wrapper around \c BtComboBoxObjectBase::setItemRaw.
    */
   virtual void setFromVariant(QVariant const & value) = 0;

   /**
    * \brief Post-construction initialisation.  See \c BtComboBoxObjectBase for overrides.
    *
    *        (This needs to be virtual so we can use \c BtComboBoxObject in \c EditorBaseField.)
    */
   virtual void init() = 0;

private:
   QString const m_name;
};

#endif
