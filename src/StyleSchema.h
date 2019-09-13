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
static const QString kcolStyleType("s_type");
static const QString kcolStyleCategory("category");
static const QString kcolStyleCategoryNumber("category_number");
static const QString kcolStyleStyleLetter("style_letter");
static const QString kcolStyleStyleGuide("style_guide");
static const QString kcolStyleOGMin("og_min");
static const QString kcolStyleOGMax("og_max");
static const QString kcolStyleFGMin("fg_min");
static const QString kcolStyleFGMax("fg_max");
static const QString kcolStyleIBUMin("ibu_min");
static const QString kcolStyleIBUMax("ibu_max");
static const QString kcolStyleColorMin("color_min");
static const QString kcolStyleColorMax("color_max");
static const QString kcolStyleABVMin("abv_min");
static const QString kcolStyleABVMax("abv_max");
static const QString kcolStyleCarbMin("carb_min");
static const QString kcolStyleCarbMax("carb_max");
static const QString kcolStyleProfile("profile");
static const QString kcolStyleIngredients("ingredients");
static const QString kcolStyleExamples("examples");

// properties for objects
static const QString kpropCategory("category");
static const QString kpropCategoryNumber("categoryNumber");
static const QString kpropStyleLetter("styleLetter");
static const QString kpropStyleGuide("styleGuide");
static const QString kpropOGMin("ogMin");
static const QString kpropOGMax("ogMax");
static const QString kpropFGMin("fgMin");
static const QString kpropFGMax("fgMax");
static const QString kpropIBUMin("ibuMin");
static const QString kpropIBUMax("ibuMax");
static const QString kpropColorMin("colorMin_srm");
static const QString kpropColorMax("colorMax_srm");
static const QString kpropCarbMin("carbMin_vol");
static const QString kpropCarbMax("carbMax_vol");
static const QString kpropABVMin("abvMin_pct");
static const QString kpropABVMax("abvMax_pct");
static const QString kpropProfile("profile");
static const QString kpropIngredients("ingredients");
static const QString kpropExamples("examples");

// these are also in the main constants file
// const QString kxmlNameProp("NAME");
// const QString kxmlNotesProp("NOTES");
static const QString kxmlPropCategory("CATEGORY");
static const QString kxmlPropCategoryNumber("CATEGORY_NUMBER");
static const QString kxmlPropStyleLetter("STYLE_LETTER");
static const QString kxmlPropStyleGuide("STYLE_GUIDE");
static const QString kxmlPropOGMin("OG_MIN");
static const QString kxmlPropOGMax("OG_MAX");
static const QString kxmlPropFGMin("FG_MIN");
static const QString kxmlPropFGMax("FG_MAX");
static const QString kxmlPropIBUMin("IBU_MIN");
static const QString kxmlPropIBUMax("IBU_MAX");
static const QString kxmlPropColorMin("COLOR_MIN");
static const QString kxmlPropColorMax("COLOR_MAX");
static const QString kxmlPropCarbMin("CARB_MIN");
static const QString kxmlPropCarbMax("CARB_MAX");
static const QString kxmlPropABVMin("ABV_MIN");
static const QString kxmlPropABVMax("ABV_MAX");
static const QString kxmlPropProfile("PROFILE");
static const QString kxmlPropIngredients("INGREDIENTS");
static const QString kxmlPropExamples("EXAMPLES");

#endif // _STYLETABLESCHEMA_H
