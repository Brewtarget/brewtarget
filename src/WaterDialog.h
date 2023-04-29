/*
 * WaterDialog.h is part of Brewtarget, and is Copyright the following
 * authors 2009-2023
 * - Matt Young <mfsy@yahoo.com>
 * - Maxime Lavigne <duguigne@gmail.com>
 * - Philip G. Lee <rocketman768@gmail.com>
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
#ifndef WATERDIALOG_H
#define WATERDIALOG_H
#pragma once

#include <memory>

#include <QButtonGroup>
#include <QDialog>
#include <QVector>
#include <QWidget>

#include "ui_waterDialog.h"

#include "measurement/Unit.h"
#include "model/Water.h"

class WaterListModel;
class WaterSortFilterProxyModel;
class WaterEditor;
class SaltTableModel;
class SaltItemDelegate;
class Salt;

/*!
 * \class WaterDialog
 *
 * \brief Trying my hand at making water chemistry work
 *
 * .:TBD:. This class (and associated UI files etc) might better be called Water Chemistry Dialog
 */
class WaterDialog : public QDialog, public Ui::waterDialog {
   Q_OBJECT

public:
   WaterDialog(QWidget* parent = nullptr);
   void setRecipe(Recipe* rec);

   ~WaterDialog();

public slots:
   void update_baseProfile(int selected);
   void update_targetProfile(int selected);
   void newTotals();
   void removeSalts();
   void setMashRO(int val);
   void setSpargeRO(int val);
   void saveAndClose();
   void clearAndClose();

signals:
   void newSalt(Salt* drop);
   void newSalts(QList<Salt*> drops);

private:

   void setDigits();
   void calculateGrainEquivalent();

   double calculateRA() const;
   double calculateGristpH();
   double calculateMashpH();
   double calculateSaltpH();
   double calculateAddedSaltpH();
   double calculateAcidpH();

   QVector<SmartDigitWidget *>    m_ppm_digits;
   QVector<SmartDigitWidget *>    m_total_digits;
   WaterListModel *            m_base_combo_list;
   WaterListModel *            m_target_combo_list;
   SaltTableModel *            m_salt_table_model;
   SaltItemDelegate *          m_salt_delegate;
   WaterEditor *               m_base_editor;
   WaterEditor *               m_target_editor;
   Recipe *                    m_rec;
   std::shared_ptr<Water>      m_base;
   std::shared_ptr<Water>      m_target;
   double                      m_mashRO;
   double                      m_spargeRO;
   double                      m_total_grains;
   double                      m_thickness;
   double                      m_weighted_colors;
   WaterSortFilterProxyModel * m_base_filter;
   WaterSortFilterProxyModel * m_target_filter;
};

#endif
