# Brewtarget Changelog

This change log is for high-level user-visible changes to Brewtarget, intended for consumption by the typical end-user.
Note however that we also process it into a Debian-compliant text change log, so we need to keep the format consistent.
In particular, the Release Timestamp section is needed as part of this (and you need to be meticulous about the date
format therein).

## Forthcoming in v3.1.0

### New Features

* It is now possible to print and export your inventory.

## v3.0.0
New features, rewrites of several low-level interfaces, changes to the basic data model, lots of bug fixes.

### New Features

* PostgreSQL 9.5 is now a supported database
* SQLite database is automatically backed up
* Temporary database has been removed, in favor of the automated backups
* All writes are now live -- no need to save your work.
* Three new mash types have been added: Batch Sparge, Fly Sparge and No Sparge. The maths should work.
* Units and scale now work for input as well as output
* Recipe versioning
* UI state persists
* Improved equipment editor
* Undo/Redo [#370](https://github.com/Brewtarget/brewtarget/issues/370)
* XML import is more robust
* Improved display on HDPI displays


### Bug Fixes

* 2.3.0 usability enhancement [#179](https://github.com/Brewtarget/brewtarget/issues/179)
* 2.4 creating recipe crashes brewtarget [#419](https://github.com/Brewtarget/brewtarget/issues/419)
* ABV Calculation Error [#398](https://github.com/Brewtarget/brewtarget/issues/398)
* Adjusting volume on preboil tab doesn't change bk efficiency [#255](https://github.com/Brewtarget/brewtarget/issues/255)
* A new dir misbehave? [#643](https://github.com/Brewtarget/brewtarget/issues/643)
* Avoid recipe recalc on load enhancement [#270](https://github.com/Brewtarget/brewtarget/issues/270)
* A zombie recipe ;), just cosmetic thing. [#645](https://github.com/Brewtarget/brewtarget/issues/645)
* Backup Database - can't choose filename, and clicking Cancel gives error [#497](https://github.com/Brewtarget/brewtarget/issues/497)
* Bad css on recipe output [#251](https://github.com/Brewtarget/brewtarget/issues/251)
* Beer XML imports don't seem to be working. [#576](https://github.com/Brewtarget/brewtarget/issues/576)
* beerxml import seems broken [#500](https://github.com/Brewtarget/brewtarget/issues/500)
* Boil time for hops is shown in whole hours (90 min -> 2h) [#560](https://github.com/Brewtarget/brewtarget/issues/560)
* Brew it / generate brew notes [#403](https://github.com/Brewtarget/brewtarget/issues/403)
* Brewnotes don't work any more [#417](https://github.com/Brewtarget/brewtarget/issues/417)
* BrewNote should contain a copy of a the recipe on the day it was brewed. enhancement normal priority [#106](https://github.com/Brewtarget/brewtarget/issues/106)
* Brewtarget (2.4.0) crashes when copying recipe [#589](https://github.com/Brewtarget/brewtarget/issues/589)
* brewtarget crashed, now I am warned there are two instances running [#533](https://github.com/Brewtarget/brewtarget/issues/533)
* Brewtarget crashes when exporting a recipe with a Sparge mash [#626](https://github.com/Brewtarget/brewtarget/issues/626)
* Brewtarget doesn't stop you running multiple instances [#526](https://github.com/Brewtarget/brewtarget/issues/526)
* Brewtarget::initialize() fails on a blank system [#210](https://github.com/Brewtarget/brewtarget/issues/210)
* Brewtarget misinterprets the mash temperatures: 65,000 C as 65.000,000 C [#569](https://github.com/Brewtarget/brewtarget/issues/569)
* Brewtarget should log its version number [#495](https://github.com/Brewtarget/brewtarget/issues/495)
* Broken unit tests after #453 [#455](https://github.com/Brewtarget/brewtarget/issues/455)
* BT 2.3.1: BeerXML output does not contain estimated IBU value [#452](https://github.com/Brewtarget/brewtarget/issues/452)
* BT DEV 2.4.0 - dependency libgcc-s1 on ubuntu 18.04 [#623](https://github.com/Brewtarget/brewtarget/issues/623)
* btOS - BrewTarget OS GNU Linux Embedded - RaspberryPI 3 [#355](https://github.com/Brewtarget/brewtarget/issues/355)
* BtTreeView selected items take precedence over locally selected entities [#340](https://github.com/Brewtarget/brewtarget/issues/340)
* Buggy refactor [#230](https://github.com/Brewtarget/brewtarget/issues/230)
* Bug: Incorrect mash temperature is shown in editor and grid [#220](https://github.com/Brewtarget/brewtarget/issues/220)
* Bug With database not creating in windows on fresh install [#486](https://github.com/Brewtarget/brewtarget/issues/486)
* Building on windows and getting started on Windows? [#397](https://github.com/Brewtarget/brewtarget/issues/397)
* Cannot delete style [#371](https://github.com/Brewtarget/brewtarget/issues/371)
* Can't add mash step in dev [#221](https://github.com/Brewtarget/brewtarget/issues/221)
* Can't delete a style from the tree [#155](https://github.com/Brewtarget/brewtarget/issues/155)
* Can't enter 90 minutes in hops schedule duplicate [#243](https://github.com/Brewtarget/brewtarget/issues/243)
* Chaging unit or scale on a table column does not automatically refresh [#43](https://github.com/Brewtarget/brewtarget/issues/43)
* Changing IBU formula or mash/fwh percentages, nothing happens [#223](https://github.com/Brewtarget/brewtarget/issues/223)
* CI/CD automated builds [#456](https://github.com/Brewtarget/brewtarget/issues/456)
* Color of the malt as displayed in the treeView_ferm is expressed in SRM regardless of unit settings [#345](https://github.com/Brewtarget/brewtarget/issues/345)
* Compile error in MashWizard.cpp [#422](https://github.com/Brewtarget/brewtarget/issues/422)
* Copy database doesn't copy the settings table properly [#306](https://github.com/Brewtarget/brewtarget/issues/306)
* Copying database fails [#303](https://github.com/Brewtarget/brewtarget/issues/303)
* "Copy Recipe" doesn't input inventory quantities [#72](https://github.com/Brewtarget/brewtarget/issues/72)
* core dump on creating new database [#142](https://github.com/Brewtarget/brewtarget/issues/142)
* Core dump when closing brewtarget [#311](https://github.com/Brewtarget/brewtarget/issues/311)
* Crash at closing after deleting a hop [#116](https://github.com/Brewtarget/brewtarget/issues/116)
* Crash when creating new Equipment [#634](https://github.com/Brewtarget/brewtarget/issues/634)
* Crash when importing .xml-file [#280](https://github.com/Brewtarget/brewtarget/issues/280)
* Creating a new recipe makes the program crash on start-up [#518](https://github.com/Brewtarget/brewtarget/issues/518)
* Database performance [#309](https://github.com/Brewtarget/brewtarget/issues/309)
* DatabaseSchemaHelper::create doesn't define mashstep table properly [#145](https://github.com/Brewtarget/brewtarget/issues/145)
* DatabaseSchemaHelper() defines hop table incorrectly [#150](https://github.com/Brewtarget/brewtarget/issues/150)
* DatabaseSchemaHelper defines mash.name as not null [#149](https://github.com/Brewtarget/brewtarget/issues/149)
* Database VANISHED!!! [#247](https://github.com/Brewtarget/brewtarget/issues/247)
* Database wiped on Mac OS X 10.12 due to logout [#333](https://github.com/Brewtarget/brewtarget/issues/333)
* "Date First Brewed" should default to date recipe was created [#301](https://github.com/Brewtarget/brewtarget/issues/301)
* DB edits in Options dialog [#570](https://github.com/Brewtarget/brewtarget/issues/570)
* Default database directory location [#272](https://github.com/Brewtarget/brewtarget/issues/272)
* default_database.sqlite is a little broken [#476](https://github.com/Brewtarget/brewtarget/issues/476)
* Deleting equipment, style or named mash dumps core [#237](https://github.com/Brewtarget/brewtarget/issues/237)
* Deleting fermentable from recipe causes a crash. [#436](https://github.com/Brewtarget/brewtarget/issues/436)
* develop branch announcement [#168](https://github.com/Brewtarget/brewtarget/issues/168)
* develop crashing on adding equipment to a new recipe [#507](https://github.com/Brewtarget/brewtarget/issues/507)
* Disconnected signal/slot pair when printing [#276](https://github.com/Brewtarget/brewtarget/issues/276)
* Display amount always in volumeunit in yeast editor [#183](https://github.com/Brewtarget/brewtarget/issues/183)
* Doesn't build using cmake 3.x [#409](https://github.com/Brewtarget/brewtarget/issues/409)
* Editing mash crashes the program [#606](https://github.com/Brewtarget/brewtarget/issues/606)
* Enable GitHub Discussions? [#528](https://github.com/Brewtarget/brewtarget/issues/528)
* Error in equipment editor definitions [#214](https://github.com/Brewtarget/brewtarget/issues/214)
* Feature request: Add logging options to GUI [#474](https://github.com/Brewtarget/brewtarget/issues/474)
* Feature request - Alpha % in Inventory Report [#466](https://github.com/Brewtarget/brewtarget/issues/466)
* Fermentable added to recipe has inventory amount set to zero. [#396](https://github.com/Brewtarget/brewtarget/issues/396)
* Feature proposal: Duplicate button in inventory [#332](https://github.com/Brewtarget/brewtarget/issues/332)
* FG calculation does not take into account unfermentable ingredients (example: Lactose) [#358](https://github.com/Brewtarget/brewtarget/issues/358)
* Final Batch Sparge always assuming 15min in Mash Wizard enhancement normal priority [#63](https://github.com/Brewtarget/brewtarget/issues/63)
* First wort hop adjustment too high by a factor of 100 [#177](https://github.com/Brewtarget/brewtarget/issues/177)
* First wort hop adjustment too high by a factor of 100 [#177](https://github.com/Brewtarget/brewtarget/issues/177)
* Fixed efficiency [#641](https://github.com/Brewtarget/brewtarget/issues/641)
* Foreign keys seem to need to be last [#144](https://github.com/Brewtarget/brewtarget/issues/144)
* fromXML methods are not fault tolerant [#239](https://github.com/Brewtarget/brewtarget/issues/239)
* Generate instructions on a new recipe dumps core [#513](https://github.com/Brewtarget/brewtarget/issues/513)
* Graphical/text representation of gravity,color, IBUs, etc has mis-sized and truncated text [#484](https://github.com/Brewtarget/brewtarget/issues/484)
* Graphics on new install glitched [#537](https://github.com/Brewtarget/brewtarget/issues/537)
* Half the recipes in a folder are moved when dragged-and-dropped into another folder [#195](https://github.com/Brewtarget/brewtarget/issues/195)
* High DPI Display problems [#434](https://github.com/Brewtarget/brewtarget/issues/434)
* Hops inventory does not update duplicate [#324](https://github.com/Brewtarget/brewtarget/issues/324)
* HopSortFilterProxy isn't quite behaving properly on TIMECOL [#182](https://github.com/Brewtarget/brewtarget/issues/182)
* Hydrometer 60F calibration [#330](https://github.com/Brewtarget/brewtarget/issues/330)
* I broke database conversions again [#580](https://github.com/Brewtarget/brewtarget/issues/580)
* IBU/color formulas do not persist [#133](https://github.com/Brewtarget/brewtarget/issues/133)
* Importing an XML file from our export throws a lot of warnings [#475](https://github.com/Brewtarget/brewtarget/issues/475)
* Importing a recipe is broken [#444](https://github.com/Brewtarget/brewtarget/issues/444)
* Improve XML import/export error handling. enhancement normal priority [#421](https://github.com/Brewtarget/brewtarget/issues/421)
* Incorrect balloon comments for Strike Temp and Final Temp [#54](https://github.com/Brewtarget/brewtarget/issues/54)
* Incorrect mash temperature is shown in editor and grid [#220](https://github.com/Brewtarget/brewtarget/issues/220)
* Ingredients - add Levteck yeast enhancement [#284](https://github.com/Brewtarget/brewtarget/issues/284)
* Initialising a new database [#319](https://github.com/Brewtarget/brewtarget/issues/319)
* Initial value of IBU Adjustments erroneous on systems that use coma as decimal separator [#158](https://github.com/Brewtarget/brewtarget/issues/158)
* Installer: skip terminates [#11](https://github.com/Brewtarget/brewtarget/issues/11)
* Instant crash when export to print (on Ubuntu) [#250](https://github.com/Brewtarget/brewtarget/issues/250)
* Instruction increment trigger may be broken [#141](https://github.com/Brewtarget/brewtarget/issues/141)
* Inventory error?! Unknown signal. Trying to add inventory to yeasts. [#601](https://github.com/Brewtarget/brewtarget/issues/601)
* Is there anyone building this on macOS? [#375](https://github.com/Brewtarget/brewtarget/issues/375)
* I was thinking about windows installer build [#512](https://github.com/Brewtarget/brewtarget/issues/512)
* Keep inventory when copying a recipe [#72] (https://github.com/Brewtarget/brewtarget/issues/72)
* Kettle volume loss due to trub [#385](https://github.com/Brewtarget/brewtarget/issues/385)
* Macbook pro retina display fonts are pixelated (HiDPI) enhancement normal priority [#259](https://github.com/Brewtarget/brewtarget/issues/259)
* Make XML import more robust [#504](https://github.com/Brewtarget/brewtarget/issues/504)
* Mash Designer - batch sparge type has no affect on water slider amount range [#282](https://github.com/Brewtarget/brewtarget/issues/282)
* Mash Designer lets you over-shoot target collected wort volume [#339](https://github.com/Brewtarget/brewtarget/issues/339)
* Mash designer - lower step temperature causes negative infusion [#94](https://github.com/Brewtarget/brewtarget/issues/94)
* Mash designer produces inconsistent results [#279](https://github.com/Brewtarget/brewtarget/issues/279)
* Mash Designer Temperature Bugs [#412](https://github.com/Brewtarget/brewtarget/issues/412)
* Mash Designer - temperature slider reversal [#283](https://github.com/Brewtarget/brewtarget/issues/283)
* Mash Profile deletion causes fatal error [#342](https://github.com/Brewtarget/brewtarget/issues/342)
* Mash wizard creates fly sparge step even when "No sparge" radio button checked [#351](https://github.com/Brewtarget/brewtarget/issues/351)
* Mash wizard does not adjust sparge water temperature for changes in mash temperature [#357](https://github.com/Brewtarget/brewtarget/issues/357)
* Minimum and maximum recommended IBU values do not change when switching between Tinseth's and Rager's [#630](https://github.com/Brewtarget/brewtarget/issues/630)
* Moving mash steps causes crash. [#265](https://github.com/Brewtarget/brewtarget/issues/265)
* Moving mash steps doesn't update form. [#267](https://github.com/Brewtarget/brewtarget/issues/267)
* Nested html documents in recipe printout [#277](https://github.com/Brewtarget/brewtarget/issues/277)
* New ingredient can't be created into a folder [#117](https://github.com/Brewtarget/brewtarget/issues/117)
* New ingredient can't be created into a folder [#117](https://github.com/Brewtarget/brewtarget/issues/117)
* Newly created folders are not stored in the db [#346](https://github.com/Brewtarget/brewtarget/issues/346)
* New MashStep doesn't get saved properly [#628](https://github.com/Brewtarget/brewtarget/issues/628)
* New mash type are behaving poorly [#244](https://github.com/Brewtarget/brewtarget/issues/244)
* No menu question [#136](https://github.com/Brewtarget/brewtarget/issues/136)
* Notepad style recipe option. [#394](https://github.com/Brewtarget/brewtarget/issues/394)
* Not linking with QtSvg: macdeployqt misses the svg plugins [#169](https://github.com/Brewtarget/brewtarget/issues/169)
* Number of decimals for the color in the fermentable editor [#314](https://github.com/Brewtarget/brewtarget/issues/314)
* OG P wrongly calculated for OG sg in refractometer tool [#159](https://github.com/Brewtarget/brewtarget/issues/159)
* Old recipe crashes Brewtarget [#420](https://github.com/Brewtarget/brewtarget/issues/420)
* OSX - Crashes when run directly from DMG (read-only installer image) [#269](https://github.com/Brewtarget/brewtarget/issues/269)
* Pedantic compiler warnings [#233](https://github.com/Brewtarget/brewtarget/issues/233)
* Persistent backups and temporary backups enhancement [#261](https://github.com/Brewtarget/brewtarget/issues/261)
* PPA is out of date [#381](https://github.com/Brewtarget/brewtarget/issues/381)
* Printed pages do not fill page width on high-dpi displays [#88](https://github.com/Brewtarget/brewtarget/issues/88)
* Printing not working in develop [#263](https://github.com/Brewtarget/brewtarget/issues/263)
* Print preview is no longer WYSIWYG [#258](https://github.com/Brewtarget/brewtarget/issues/258)
* Problems with OGs and FGs for each style (2.3.0) [#166](https://github.com/Brewtarget/brewtarget/issues/166)
* Problem with amounts in inventory [#532](https://github.com/Brewtarget/brewtarget/issues/532)
* Program crashes after moving up or down a mash step for an empty mash profile [#180](https://github.com/Brewtarget/brewtarget/issues/180)
* Put string constants for property names in namespaces and match names to values [#520](https://github.com/Brewtarget/brewtarget/issues/520)
* QCoreApplication::applicationDirPath: Please instantiate the QApplication object first bug needs info [#115](https://github.com/Brewtarget/brewtarget/issues/115)
* qt 5.11 breaks fermentables [#416](https://github.com/Brewtarget/brewtarget/issues/416)
* QT5 minimal version [#521](https://github.com/Brewtarget/brewtarget/issues/521)
* QtMultimedia gstreamer link issue [#9](https://github.com/Brewtarget/brewtarget/issues/9)
* Recalc button enhancement [#135](https://github.com/Brewtarget/brewtarget/issues/135)
* Recipe Extras Tab >> Notes & Taste Notes should be saved? [#365](https://github.com/Brewtarget/brewtarget/issues/365)
* Recipe versioning [#327](https://github.com/Brewtarget/brewtarget/issues/327)
* Removing ingredient from the recipe does not update calculated beer parameters [#341](https://github.com/Brewtarget/brewtarget/issues/341)
* Request: Double click ingredient opens ingredient editor [#350](https://github.com/Brewtarget/brewtarget/issues/350)
* Require specific minimum Qt version in CMakeLists.txt [#152](https://github.com/Brewtarget/brewtarget/issues/152)
* Right-click on top level Recipes in treeview Segfaults with null pointer [#609](https://github.com/Brewtarget/brewtarget/issues/609)
* Saving a "note" to a the dated tab causes application to crash [#483](https://github.com/Brewtarget/brewtarget/issues/483)
* Saving error [#373](https://github.com/Brewtarget/brewtarget/issues/373)
* Scottland -> Scotland [#529](https://github.com/Brewtarget/brewtarget/issues/529)
* Segfault merging database [#291](https://github.com/Brewtarget/brewtarget/issues/291)
* Signals, signals everywhere enhancement [#274](https://github.com/Brewtarget/brewtarget/issues/274)
* Slots: signals in BeerXMLElement subclasses? [#473](https://github.com/Brewtarget/brewtarget/issues/473)
* Specific heat label fix (cal/(g*K)) to (Cal/(g*K)) [#404](https://github.com/Brewtarget/brewtarget/issues/404)
* SQL Database Error in mashstep [#461](https://github.com/Brewtarget/brewtarget/issues/461)
* SQLite to postgres is broken. Again. [#505](https://github.com/Brewtarget/brewtarget/issues/505)
* Support for Brew-in-a-bag enhancement [#41](https://github.com/Brewtarget/brewtarget/issues/41)
* TESTs failing when Testing::initTestCase() is creating new Hop or Fermentable [#604](https://github.com/Brewtarget/brewtarget/issues/604)
* Text Notes on Brew Days gone and do not work anymore [#457](https://github.com/Brewtarget/brewtarget/issues/457)
* Thank you very much for the free open source program! [#494](https://github.com/Brewtarget/brewtarget/issues/494)
* The feature of calculating ingredients enhancement [#188](https://github.com/Brewtarget/brewtarget/issues/188)
* There is no Q_PROPERTY line in brewnotes for the new attenuation field [#440](https://github.com/Brewtarget/brewtarget/issues/440)
* Time automatically displayed in hours if the value in minute exceeds 60 min enhancement [#46](https://github.com/Brewtarget/brewtarget/issues/46)
* TravisCI can't find webkit because it's gone forever [#217](https://github.com/Brewtarget/brewtarget/issues/217)
* Trying to delete "brewtarget" folder in the Recipes list causes segfault [#338](https://github.com/Brewtarget/brewtarget/issues/338)
* Tun Volume Entered is 0 [#298](https://github.com/Brewtarget/brewtarget/issues/298)
* Two running Brewtargets Resets Database [#73](https://github.com/Brewtarget/brewtarget/issues/73)
* Two Running Brewtargets Resets Database [#73](https://github.com/Brewtarget/brewtarget/issues/73)
* Typo in Yeast editor [#384](https://github.com/Brewtarget/brewtarget/issues/384)
* UI -2.3.0 usability enhancement [#179](https://github.com/Brewtarget/brewtarget/issues/179)
* UI compiler complains about missing widget [#209](https://github.com/Brewtarget/brewtarget/issues/209)
* UI - DB edits in Options dialog [#570](https://github.com/Brewtarget/brewtarget/issues/570)
* Unable to build on Linux Mint 18.1 [#337](https://github.com/Brewtarget/brewtarget/issues/337)
* Units on input are not working [#238](https://github.com/Brewtarget/brewtarget/issues/238)
* updating the database is misbehaving [#564](https://github.com/Brewtarget/brewtarget/issues/564)
* Updating the inventory seems broken [#315](https://github.com/Brewtarget/brewtarget/issues/315)
* use bool type in schema enhancement normal priority [#148](https://github.com/Brewtarget/brewtarget/issues/148)
* UTF-8 compatible export for BeerXML [#624](https://github.com/Brewtarget/brewtarget/issues/624)
* We are not properly deleting things [#164](https://github.com/Brewtarget/brewtarget/issues/164)
* We are not properly deleting things [#164](https://github.com/Brewtarget/brewtarget/issues/164)
* When removing a style through Style Editor and clicking on cancel the change is not undone enhancement [#198](https://github.com/Brewtarget/brewtarget/issues/198)
* When you change the scale of the Boil Time in the equipment dialog the prg crash [#137](https://github.com/Brewtarget/brewtarget/issues/137)
* Where is database stored? [#441](https://github.com/Brewtarget/brewtarget/issues/441)
* Why does MainWindow create a recipe by poking directly into the Database()? [#446](https://github.com/Brewtarget/brewtarget/issues/446)
* Why Xerces & Xalan? [#625](https://github.com/Brewtarget/brewtarget/issues/625)
* Will there be version 2.3.1 for Windows? [#464](https://github.com/Brewtarget/brewtarget/issues/464)
* Windows build failing with error: 'M_PI' was not declared in this scope [#538](https://github.com/Brewtarget/brewtarget/issues/538)
* Windows build fails [#292](https://github.com/Brewtarget/brewtarget/issues/292)
* Xerces-C and Xalan-C is not included in NSIS Installer [#567](https://github.com/Brewtarget/brewtarget/issues/567)
* XML recipe import not setting amounts [#588](https://github.com/Brewtarget/brewtarget/issues/588)
* Yeast attenuation min-max [#410](https://github.com/Brewtarget/brewtarget/issues/410)

### Release Timestamp
Thu, 18 Aug 2022 08:46:19 +0100

## v2.3.1

### New Features

* None

### Bug Fixes

* Bad amount/weight behavior in yeast editor [#183](https://github.com/Brewtarget/brewtarget/issues/183)
* Bad time sorting in hop table [#182](https://github.com/Brewtarget/brewtarget/issues/182)
* First wort hop adjustment 100x too high [#177](https://github.com/Brewtarget/brewtarget/issues/177)
* OG in Plato wrongly displayed in refracto dialog [#159](https://github.com/Brewtarget/brewtarget/issues/159)

### Release Timestamp
Sat, 19 Mar 2016 14:27:30 -0700

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

### Release Timestamp
Sun, 10 Jan 2016 13:38:30 -0800

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

### Release Timestamp
Sun, 09 Nov 2014 13:14:30 -0600

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

### Release Timestamp
Sun, 14 Sep 2014 13:41:30 -0500

## v2.0.3

Minor bugfix release.

### New Features

### Bug Fixes

* Manual button failed to display the manual [#1282618](https://bugs.launchpad.net/brewtarget/+bug/1282618).
* Selecting FG units did not change displayed units [#128751](https://bugs.launchpad.net/brewtarget/+bug/1287511).
* Windows builds now properly find phonon library [#1226862](https://bugs.launchpad.net/brewtarget/+bug/1226862).
* Mash wizard does not overshoot target boil size when recipe includes extract or sugar.[#1233744](https://bugs.launchpad.net/brewtarget/+bug/1233744)

### Release Timestamp
Sat, 03 May 2014 09:38:30 -0500

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

### Release Timestamp
Mon, 27 Jan 2014 08:38:30 -0600

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

### Release Timestamp
Mon, 17 Mar 2013 20:41:21 -0600

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

### Release Timestamp
Sat, 29 Dec 2012 09:02:42 -0500


## v1.2.4

New upstream release

### Release Timestamp
Sun, 02 Oct 2011 12:23:44 -0500

## v1.2.3

New upstream release 1.2.3
* Initial debian release (Closes: #538751)

### Release Timestamp
Sun, 13 Mar 2011 09:38:00 -0600
