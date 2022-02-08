/*
 * AlcoholTool.h is part of Brewtarget, and is Copyright the following
 * authors 2009-2021
 * - Matt Young <mfsy@yahoo.com>
 * - Ryan Hoobler <rhoob@yahoo.com>
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
#ifndef ALCOHOLTOOL_H
#define ALCOHOLTOOL_H
#pragma once

#include <memory> // For PImpl

#include <QDialog>

class QWidget;
class QEvent;

/*!
 * \brief Dialog to calculate ABV from OG and FG readings - optionally with temperature correction.
 */
class AlcoholTool : public QDialog {
   Q_OBJECT

public:
   AlcoholTool(QWidget* parent = nullptr);
   virtual ~AlcoholTool();

public slots:
   void calculate();

   /**
    * \brief Turn the advanced controls (temperature correction) on or off
    */
   void toggleAdvancedControls();

protected:
   virtual void changeEvent(QEvent* event);
   //! Called when the user closes the tool
   virtual void done(int r);

private:
   // Private implementation details - see https://herbsutter.com/gotw/_100/
   class impl;
   std::unique_ptr<impl> pimpl;
};

#endif
