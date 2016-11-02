# Brewtarget Changelog

This changelog is for high-level user-visible changes to Brewtarget, intended
for consumption by the typical end-user.

## v2.5.0

### New Features

* It is now possible to print and export your inventory.

## v2.4.0

### New Features

* PostgreSQL 9.5 is now a supported database.
* SQLite database is automatically backed up
* Temporary database has been removed, in favor of the automated backups
* All writes are now live -- no need to save your work.
* Three new mash types have been added: Batch Sparge, Fly Sparge and No
  Sparge. The maths should work.
* Units&scale now work for input as well as output
* UI state persists
* Improved equipment editor.

### Bug Fixes

* When you change the scale of the Boil Time in the equipment dialog the prg crash [#137](https://github.com/Brewtarget/brewtarget/issues/137)
* Can't delete a style from the tree [#155](https://github.com/Brewtarget/brewtarget/issues/155)
* OG P wrongly calculated for OG sg in refractometer tool [#159](https://github.com/Brewtarget/brewtarget/issues/159)
* We are not properly deleting things [#164](https://github.com/Brewtarget/brewtarget/issues/164)
* Not linking with QtSvg: macdeployqt misses the svg plugins [#169](https://github.com/Brewtarget/brewtarget/issues/169)
* First wort hop adjustment too high by a factor of 100 [#177](https://github.com/Brewtarget/brewtarget/issues/177)
* Program crashes after moving up or down a mash step for an empty mash profile [#180](https://github.com/Brewtarget/brewtarget/issues/180)
* HopSortFilterProxy isn't quite behaving properly on TIMECOL [#182](https://github.com/Brewtarget/brewtarget/issues/182)
* Display amount always in volumeunit in yeast editor [#183](https://github.com/Brewtarget/brewtarget/issues/183)
* Half the recipes in a folder are moved when dragged-and-dropped into another folder [#195](https://github.com/Brewtarget/brewtarget/issues/195)
* Brewtarget::Initialize fails on a blank system [#210](https://github.com/Brewtarget/brewtarget/issues/210)
* Bug: Incorrect mash temperature is shown in editor and grid [#220](https://github.com/Brewtarget/brewtarget/issues/220)
* Changing IBU formula or mash/fwh percentages, nothing happens [#223](https://github.com/Brewtarget/brewtarget/issues/223)
* fromXML methods are not fault tolerant [#239](https://github.com/Brewtarget/brewtarget/issues/239)
* Database VANISHED!!! [#247](https://github.com/Brewtarget/brewtarget/issues/247)
* Instant crash when export to print (on Ubuntu) [#250](https://github.com/Brewtarget/brewtarget/issues/250)
* Bad css on recipe output [#251](https://github.com/Brewtarget/brewtarget/issues/251)
* Printed pages do not fill page width on high-dpi displays [#88](https://github.com/Brewtarget/brewtarget/issues/88)
* New ingredient can't be created into a folder [#117](https://github.com/Brewtarget/brewtarget/issues/117)
* ibu/color formulas do not persist [#133](https://github.com/Brewtarget/brewtarget/issues/133)
* Two running Brewtargets Resets Database [#73](https://github.com/Brewtarget/brewtarget/issues/73)
* Keep inventory when copying a recipe [#72] (https://github.com/Brewtarget/brewtarget/issues/72)

## v2.3.1

### New Features

* None

### Bug Fixes

* Bad amount/weight behavior in yeast editor [#183](https://github.com/Brewtarget/brewtarget/issues/183)
* Bad time sorting in hop table [#182](https://github.com/Brewtarget/brewtarget/issues/182)
* First wort hop adjustment 100x too high [#177](https://github.com/Brewtarget/brewtarget/issues/177)
* OG in Plato wrongly displayed in refracto dialog [#159](https://github.com/Brewtarget/brewtarget/issues/159)

## v2.3.0

### New Features

* Clarify UI design in ingredient tables [#81](https://github.com/Brewtarget/brewtarget/issues/81)
* Recipe scaling is a wizard and unifies batch/efficiency scaling [#108](https://github.com/Brewtarget/brewtarget/pull/108)
* Ingredient searching [#6](https://github.com/Brewtarget/brewtarget/issues/6)
* More accurate ABV calcs for high-gravity recipes [#48](https://github.com/Brewtarget/brewtarget/issues/48)
* BBCode export [#66](https://github.com/Brewtarget/brewtarget/pull/66)

### Bug Fixes

* User date formate not used in recipe tree [#123](https://github.com/Brewtarget/brewtarget/pull/123)
* Oddities in brewday step numbering [#97](https://github.com/Brewtarget/brewtarget/pull/97)
* Crash in instruction editor [#86](https://github.com/Brewtarget/brewtarget/issues/86)

### Incompatibilities

## v2.2.0

### New Features

* Scale recipe tool removes equipment [#91](https://github.com/Brewtarget/brewtarget/issues/91)
* Noonan IBU calculation [#7](https://github.com/Brewtarget/brewtarget/issues/7)
* Print output uses units and scales options
* Delete button deletes all selected ingredients
* Add sorting for inventory columns
* Calories per 330mL for SI units
* Upgrade to Qt5 from Qt4

### Bug Fixes

* Crash when creating new recipe folder [#98](https://github.com/Brewtarget/brewtarget/issues/98)
* Bad localization behavior for specific heat input field [#77](https://github.com/Brewtarget/brewtarget/issues/77)
* Bad ingredient amount behavior in non-US locales [#65](https://github.com/Brewtarget/brewtarget/issues/65)
* Leaf/plug utilization adjustment is backwards [#64](https://github.com/Brewtarget/brewtarget/issues/64)
* Scale by efficiency is incorrect with sugars in recipe [#29](https://github.com/Brewtarget/brewtarget/issues/29)
* Cannot export recipe [#39](https://github.com/Brewtarget/brewtarget/issues/39)
* Recipe does not update after yeast changes [#30](https://github.com/Brewtarget/brewtarget/issues/30)
* Multiple dialogs when cancelling multiple actions [#25](https://github.com/Brewtarget/brewtarget/issues/25)
* Bad color range after style change using EBC [#2](https://github.com/Brewtarget/brewtarget/issues/2)
* Crash on yeast import
* Crash when double-clicking recipes on brewdate
* Crash on removing hop from hop dialog
* Late sugar additions affect boil gravity
* Updated product id for Ringwood Ale yeast
* Fixes import/export of brewnotes
* Bug 1374421 -- cannot delete brewnotes

### Incompatibilities

## v2.1.0

### New Features

* Folders for organizing recipes [#1109740](https://bugs.launchpad.net/brewtarget/+bug/1109740).
* Recipe parameter [sliders](https://blueprints.launchpad.net/brewtarget/+spec/recipe-sliders)
  to make it easier to visualize the value and range of IBUs, color, etc.

### Bug Fixes

* Boil SG was wrong if kettle losses were not zero [#1328761](https://bugs.launchpad.net/brewtarget/+bug/1328761).
* Extract recipes crash brew-it [#1340484](https://bugs.launchpad.net/brewtarget/+bug/1340484)
* Incorrect abbreviation in manual [#1224236](https://bugs.launchpad.net/bugs/1224236).
* Bad IBUs for extract recipes [#1286655](https://bugs.launchpad.net/bugs/1286655).
* "Brew It" fails for extract recipes [#1340484](https://bugs.launchpad.net/bugs/1340484).
* Missing icons for some distributions [#1346342](https://bugs.launchpad.net/bugs/1346342).
* Failed to launch on OSX with case-sensitive filesystems [#1259374](https://bugs.launchpad.net/bugs/1259374).

### Incompatibilities

## v2.0.3

Minor bugfix release.

### New Features

### Bug Fixes

* Manual button failed to display the manual [#1282618](https://bugs.launchpad.net/brewtarget/+bug/1282618).
* Selecting FG units did not change displayed units [#128751](https://bugs.launchpad.net/brewtarget/+bug/1287511).
* Windows builds now properly find phonon library [#1226862](https://bugs.launchpad.net/brewtarget/+bug/1226862).
* Mash wizard does not overshoot target boil size when recipe includes extract or sugar.[#1233744](https://bugs.launchpad.net/brewtarget/+bug/1233744)

### Incompatibilities

## v2.0.2

This is a minor bugfix release.

### New Features

* Windows installer now does automatic upgrade from previous versions.
* Replaced language icons with a combobox for selecting language.
* Added Greek and Chinese translations.

### Bug Fixes

* Fixed Slackware build error [#1109493](https://bugs.launchpad.net/bugs/1109493)
* Installs in Fedora 17 [#1109534](https://bugs.launchpad.net/bugs/1109534)
* Wrong ingredients being added to recipe fixed [#1158620](https://bugs.launchpad.net/bugs/1158620)
* Fixed compile error on FreeBSD 9.0 64-bit [#1131231](https://bugs.launchpad.net/bugs/1131231)
* Late-added sugars now show up in recipe instructions [#1155816](https://bugs.launchpad.net/bugs/1155816)
* Fixed misc. ingredient amounts being improperly interpreted [#1160610](https://bugs.launchpad.net/bugs/1160610)
* Rpm package no longer provides /usr and subdirectories [#1164045](https://bugs.launchpad.net/bugs/1164045)
* Fixed issue causing Fermentable EBC values to be constantly divided by 2 [#1170088](https://bugs.launchpad.net/bugs/1170088)
* Fixed labeling of EBC values when adding new styles [#1173774](https://bugs.launchpad.net/bugs/1173774)
* Fixed inaccurate color preview [#1177546](https://bugs.launchpad.net/bugs/1177546)
* Fixed crashing when importing recipes from Brewmate [#1192269](https://bugs.launchpad.net/bugs/1192269).
* Building with `-no-phonon` flag works correctly [#1212921](https://bugs.launchpad.net/bugs/1212921)
* Equipment editor should no longer show up empty. [#1227787](https://bugs.launchpad.net/brewtarget/+bug/1227787)
* Closing the equipment editor now always reverts all changes.
* Update mash tun mass and specific heat when equipment is dropped on recipe. [#1233754](https://bugs.launchpad.net/brewtarget/+bug/1233754)
* No longer crashes when copying recipe that has no style selected. [#1233745](https://bugs.launchpad.net/brewtarget/+bug/1233745)
* Made the manual open in a browser. [#1224584](https://bugs.launchpad.net/brewtarget/+bug/1224584).

### Incompatibilities

None

## v2.0.1

This is a minor bugfix release.

### New Features

* Added Russian translation.
* Significant update to Spanish translation.

### Bug Fixes

* Fixed bug preventing new equipments from being properly saved [#1132311](https://bugs.launchpad.net/bugs/1132311)
* Fixed crash when editing recipe or taste notes [#1134983](https://bugs.launchpad.net/bugs/1134983)
* Fixed bug preventing efficiency changes from immediately updating the recipe [#1129201](https://bugs.launchpad.net/bugs/1129201)
* Fixed Windows issue causing changes not to be saved [#1133821](https://bugs.launchpad.net/bugs/1133821)
* Fixed strange boil kettle efficiency calculations in brewnotes [#1121200](https://bugs.launchpad.net/bugs/1121200)

### Incompatibilities

None

## v2.0.0

This is a major overhaul of the Brewtarget backend.

### New Features

* Moved XML database to SQLite.
* Customizable equipment-specific hop utilization.
* Ability to select a "default" equipment to use in new recipes.
* Customizable units in individual display fields.
* Drag'n'drop ingredient lists.

### Bug Fixes

Numerous

### Incompatibilities

* Can no longer directly read v2.0.0 database from earlier versions, though you
  can still export recipes and ingredients that can be read by earlier
  versions.

