# Brewtarget Changelog

This changelog is for high-level user-visible changes to Brewtarget, intended
for consumption by the typical end-user.

## v2.0.3

### New Features

### Bug Fixes

### Incompatibilities

## v2.0.2

This is a minor bugfix release.

### New Features

None

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
* Building with `-no-phonon` flag works correctly [#1212921](https://bugs.launchpad.net/bugs/1212921)
* Equipment editor should no longer show up empty. [#1227787](https://bugs.launchpad.net/brewtarget/+bug/1227787)
* Closing the equipment editor now always reverts all changes.
* The mash wizard used to overshot target boil size when ingredients included extract or sugar.[#1233744](https://bugs.launchpad.net/brewtarget/+bug/1233744)

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

