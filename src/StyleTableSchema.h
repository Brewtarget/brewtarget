/*
 * StyleTableSchema.h is part of Brewtarget, and is Copyright the following
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

#ifndef _STYLETABLESCHEMA_H
#define _STYLETABLESCHEMA_H

// Columns for the style table
// These are defined in the main constants file
// const QString kcolStyleName("name");
// const QString kcolStyleNotes("notes");
static QString ktableStyle("style");
static QString kcolStyleType("s_type");
static QString kcolStyleCategory("category");
static QString kcolStyleCategoryNumber("category_number");
static QString kcolStyleStyleLetter("style_letter");
static QString kcolStyleStyleGuide("style_guide");
static QString kcolStyleOGMin("og_min");
static QString kcolStyleOGMax("og_max");
static QString kcolStyleFGMin("fg_min");
static QString kcolStyleFGMax("fg_max");
static QString kcolStyleIBUMin("ibu_min");
static QString kcolStyleIBUMax("ibu_max");
static QString kcolStyleColorMin("color_min");
static QString kcolStyleColorMax("color_max");
static QString kcolStyleABVMin("abv_min");
static QString kcolStyleABVMax("abv_max");
static QString kcolStyleCarbMin("carb_min");
static QString kcolStyleCarbMax("carb_max");
static QString kcolStyleProfile("profile");
static QString kcolStyleIngredients("ingredients");
static QString kcolStyleExamples("examples");

// properties for objects
// These are defined in the main constants file
// const QString kpropName("name");
// const QString kpropNotes("notes");
// const QString kpropType("type");
const QString kpropCategory("category");
const QString kpropCategoryNumber("categoryNumber");
const QString kpropStyleLetter("styleLetter");
const QString kpropStyleGuide("styleGuide");
const QString kpropOGMin("ogMin");
const QString kpropOGMax("ogMax");
const QString kpropFGMin("fgMin");
const QString kpropFGMax("fgMax");
const QString kpropIBUMin("ibuMin");
const QString kpropIBUMax("ibuMax");
const QString kpropColorMin("colorMin_srm");
const QString kpropColorMax("colorMax_srm");
const QString kpropCarbMin("carbMin_vol");
const QString kpropCarbMax("carbMax_vol");
const QString kpropABVMin("abvMin_pct");
const QString kpropABVMax("abvMax_pct");
const QString kpropProfile("profile");
const QString kpropIngredients("ingredients");
const QString kpropExamples("examples");

// these are also in the main constants file
// const QString kxmlNameProp("NAME");
// const QString kxmlNotesProp("NOTES");
const QString kxmlPropCategory("CATEGORY");
const QString kxmlPropCategoryNumber("CATEGORY_NUMBER");
const QString kxmlPropStyleLetter("STYLE_LETTER");
const QString kxmlPropStyleGuide("STYLE_GUIDE");
const QString kxmlPropOGMin("OG_MIN");
const QString kxmlPropOGMax("OG_MAX");
const QString kxmlPropFGMin("FG_MIN");
const QString kxmlPropFGMax("FG_MAX");
const QString kxmlPropIBUMin("IBU_MIN");
const QString kxmlPropIBUMax("IBU_MAX");
const QString kxmlPropColorMin("COLOR_MIN");
const QString kxmlPropColorMax("COLOR_MAX");
const QString kxmlPropCarbMin("CARB_MIN");
const QString kxmlPropCarbMax("CARB_MAX");
const QString kxmlPropABVMin("ABV_MIN");
const QString kxmlPropABVMax("ABV_MAX");
const QString kxmlPropProfile("PROFILE");
const QString kxmlPropIngredients("INGREDIENTS");
const QString kxmlPropExamples("EXAMPLES");

#endif // _STYLETABLESCHEMA_H
