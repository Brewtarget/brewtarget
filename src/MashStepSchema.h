/*
 * MashStepTableSchema.h is part of Brewtarget, and is Copyright the following
 * authors 2019-2024
 * - Mik Firestone <mikfire@fastmail.com>
 *
 * Brewtarget is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Brewtarget is distributed in the mashe that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _MASHSTEPTABLESCHEMA_H
#define _MASHSTEPTABLESCHEMA_H

#include <QString>
// Columns for the mash table
static const QString kcolMashstepType("mstype");
static const QString kcolMashstepInfuseAmt("infuse_amount");
static const QString kcolMashstepStepTemp("step_temp");
static const QString kcolMashstepStepTime("step_time");
static const QString kcolMashstepRampTime("ramp_time");
static const QString kcolMashstepEndTemp("end_temp");
static const QString kcolMashstepInfuseTemp("infuse_temp");
static const QString kcolMashstepDecoctAmt("decoction_amount");
static const QString kcolMashstepStepNumber("step_number");

static const QString kpropInfuseAmt("infuseAmount_l");
static const QString kpropStepTemp("stepTemp_c");
static const QString kpropStepTime("stepTime_min");
static const QString kpropRampTime("rampTime_min");
static const QString kpropEndTemp("endTemp_c");
static const QString kpropInfuseTemp("infuseTemp_c");
static const QString kpropDecoctAmt("decoctionAmount_l");
static const QString kpropStepNumber("stepNumber");

static const QString kxmlPropInfuseAmt("INFUSE_AMOUNT");
static const QString kxmlPropStepTemp("STEP_TEMP");
static const QString kxmlPropStepTime("STEP_TIME");
static const QString kxmlPropRampTime("RAMP_TIME");
static const QString kxmlPropEndTemp("END_TEMP");
static const QString kxmlPropInfuseTemp("INFUSE_TEMP");
static const QString kxmlPropDecoctAmt("DECOCTION_AMOUNT");
static const QString kxmlPropStepType("STEP_TYPE");
#endif // _MASHSTEPTABLESCHEMA_H
