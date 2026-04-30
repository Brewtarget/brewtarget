/*======================================================================================================================
 * BrewLogWidget.h is part of Brewtarget, and is copyright the following authors 2009-2026:
 *   • Jeff Bailey <skydvr38@verizon.net>
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
 =====================================================================================================================*/
#ifndef BREWLOGWIDGET_H
#define BREWLOGWIDGET_H
#pragma once

#include <QDate>
#include <QFocusEvent>
#include <QString>
#include <QMetaProperty>
#include <QVariant>
#include <QWidget>
#include "ui_brewLogWidget.h"

// Forward declarations.
class BrewLog;

/*!
 * \class BrewLogWidget
 *
 * \brief View/controller widget that edits BrewLogs.
 */
class BrewLogWidget : public QWidget, public Ui::brewLogWidget {
    Q_OBJECT

public:
    explicit BrewLogWidget(QWidget *parent = nullptr);
    ~BrewLogWidget() override;

   void setBrewLog(BrewLog * bNote);
   BrewLog * brewLog() const;

protected:
   void focusOutEvent(QFocusEvent *e) override;

public slots:
   void updateBrewDate(QDate const & datetime);
   void updateName();
   void updateSG();
   void updateVolumeIntoBK_l();
   void updateStrikeTemp_c();
   void updateMashFinTemp_c();

   void updateOG();
   void updatePostBoilVolume_l();
   void updateVolumeIntoFerm_l();
   void updatePitchTemp_c();

   void updateFG();
   void updateFinalVolume_l();
   void updateFermentDate(QDate const & datetime);
   void updateDateFormat();

   void updateNotes();
//   void saveAll();

   void changed(QMetaProperty,QVariant);
   void showChanges(QString field = "");

   /**
    * The signal coming into this slot has two parameters:
    *   • Measurement::SystemOfMeasurement oldSystemOfMeasurement,
    *   • std::optional<Measurement::UnitSystem::RelativeScale> oldForcedScale
    * However, because we have access to the underlying "standard units" value, we don't need to be told the old unit or
    * scale.  Qt allows slots to ignore parameters - eg it is happy to deliver a two-parameter signal to a
    * zero-parameter slot.  So that is what we do here.
    */
   void updateProjOg();

private:
   BrewLog * m_brewLog = nullptr;
};

#endif
