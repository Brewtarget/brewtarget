/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * ScaleRecipeTool.h is part of Brewtarget, and is copyright the following authors 2009-2025:
 *   • Matt Young <mfsy@yahoo.com>
 *   • Mik Firestone <mikfire@gmail.com>
 *   • Philip Greggory Lee <rocketman768@gmail.com>
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
#ifndef SCALE_RECIPE_TOOL_H
#define SCALE_RECIPE_TOOL_H
#pragma once

#include <QAbstractItemModel>
//#include <QAbstractButton>
//#include <QAbstractListModel>
#include <QButtonGroup>
#include <QComboBox>
//#include <QDialog>
#include <QEvent>
#include <QFormLayout>
#include <QLabel>
//#include <QLayout>
#include <QLineEdit>
#include <QSortFilterProxyModel>
#include <QWidget>
#include <QWizardPage>

// Forward declarations
class Equipment;
class EquipmentListModel;
class Recipe;

/*!
 * \brief Wizard to scale a recipe's ingredients to match a new \c Equipment
 */
class ScaleRecipeTool : public QWizard {
   Q_OBJECT

public:
   ScaleRecipeTool(QWidget* parent = nullptr);
   //! \brief Set the observed \c Recipe
   void setRecipe(Recipe* rec);

private slots:
   void accept() Q_DECL_OVERRIDE;

private:
   //! \brief Scale the observed recipe for the new \c equip
   void scale(Equipment* equip, double newEff);

   Recipe * m_recObs;
///   QButtonGroup m_scaleGroup;
   EquipmentListModel    * m_equipListModel;
   QSortFilterProxyModel * m_equipSortProxyModel;
};

//================================================ ScaleRecipeIntroPage ================================================

/**
 * \brief
 */
class ScaleRecipeIntroPage : public QWizardPage {

   Q_OBJECT

public:
   ScaleRecipeIntroPage(QWidget* parent = nullptr);

public slots:
   void doLayout();
   void retranslateUi();

protected:
   virtual void changeEvent(QEvent* event);

private:
   QVBoxLayout* layout;
   QLabel* label;
};

//============================================== ScaleRecipeEquipmentPage ==============================================

/**
 * \brief
 */
class ScaleRecipeEquipmentPage : public QWizardPage {

   Q_OBJECT

public:
   ScaleRecipeEquipmentPage(QAbstractItemModel* listModel, QWidget* parent = 0);

public slots:
   void doLayout();
   void retranslateUi();

protected:

   virtual void changeEvent(QEvent* event);

private:

   QFormLayout *        layout;
   QLabel *             m_equipLabel;
   QComboBox *          m_equipComboBox;
   QAbstractItemModel * m_equipListModel;
   QLabel *             m_efficiencyLabel;
   QLineEdit *          m_efficiencyLineEdit;
};

#endif
