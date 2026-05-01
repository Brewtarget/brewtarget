/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * tools/PrimingDialog.h is part of Brewtarget, and is copyright the following authors 2009-2026:
 *   • Matt Young <mfsy@yahoo.com>
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
#ifndef TOOLS_PRIMINGDIALOG_H
#define TOOLS_PRIMINGDIALOG_H
#pragma once

#include <QDialog>
#include "ui_primingDialog.h"

class QButtonGroup;
class QWidget;

/*!
 * \class PrimingDialog
 *
 * \brief Dialog to calculate priming sugar amounts
 */
class PrimingDialog : public QDialog, public Ui::primingDialog {
   Q_OBJECT
public:
   explicit PrimingDialog(QWidget * parent = nullptr);
   ~PrimingDialog() override;

public slots:
   void calculate();

private:
   QButtonGroup * sugarGroup;
};

#endif
