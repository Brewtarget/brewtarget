/*
 * SettingsTableSchema.h is part of Brewtarget, and is Copyright the following
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

#ifndef _SETTINGSTABLESCHEMA_H
#define _SETTINGSTABLESCHEMA_H

// this is kind of weird, as this table has no properties. But I've decided
// that all of the propertyTo* calls should only take kprops.
static const QString kpropSettingsVersion("version");
static const QString kpropSettingsRepopulate("repopulatechildrenonnextstart");
// Columns for the settings table
static const QString kcolSettingsVersion("version");
static const QString kcolSettingsRepopulate("repopulatechildrenonnextstart");

// this table is a meta table. It has no object and no XML schemas. Weird,
// innit?
//
#endif // _SETTINGSTABLESCHEMA_H
