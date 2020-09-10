/*
 * HopTableSchema.h is part of Brewtarget, and is Copyright the following
 * authors 2019-2024
 * - Mik Firestone <mikfire@fastmail.com>
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

#ifndef _HOPTABLESCHEMA_H
#define _HOPTABLESCHEMA_H

#include <QString>

// Columns for the hop table
static const QString kcolHopForm("form");
static const QString kcolHopType("htype");
static const QString kcolHopOrigin("origin");
static const QString kcolHopAlpha("alpha");
static const QString kcolHopAmount("amount");
static const QString kcolHopBeta("beta");
static const QString kcolHopHSI("hsi");
static const QString kcolHopHumulene("humulene");
static const QString kcolHopCaryophyllene("caryophyllene");
static const QString kcolHopCohumulone("cohumulone");
static const QString kcolHopMyrcene("myrcene");

// properties for objects
static const QString kpropAlpha("alpha_pct");
static const QString kpropBeta("beta_pct");
static const QString kpropHSI("hsi_pct");
static const QString kpropSubstitutes("substitutes");
static const QString kpropHumulene("humulene_pct");
static const QString kpropCaryophyllene("caryophyllene_pct");
static const QString kpropCohumulone("cohumulone_pct");
static const QString kpropMyrcene("myrcene_pct");

static const QString kpropFormString("formString");

// these are also in the main constants file
static const QString kxmlPropAlpha("ALPHA");
static const QString kxmlPropBeta("BETA");
static const QString kxmlPropHSI("HSI");
static const QString kxmlPropHumulene("HUMULENE");
static const QString kxmlPropCaryophyllene("CARYOPHYLLENE");
static const QString kxmlPropCohumulone("COHUMULONE");
static const QString kxmlPropMyrcene("MYRCENE");

// These are not in BeerXML
static const QString kxmlPropUse("USE");
static const QString kxmlPropForm("FORM");
//
#endif // _HOPTABLESCHEMA_H
