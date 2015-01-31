How It Works
============

The Short Form
--------------

If you don't want to read or don't care about the magic, follow these basic
steps in the Qt Designer to make the units & scales stuff work.

1. Create your label and line edit field. I am asking that people follow the
   basic naming convention of:
   label\_fieldname for labels
   lineEdit\_fieldname for line Edits
2. Make sure the nearest parent that can have a dynamic property has one named
   "configSection" defined. Things like groupBoxes can, things like
   verticalLayouts cannot. Most existing UI elements should already have this
   set. If not, create it. I have been using the name of the form for the
   value.
3. Right click on the new label and find the Promote To list. Note that you
   want the drop down list, not the dialog. If that list isn't available, open
   another form like mainWindow.ui or brewNote.ui first. This will prepopulate
   a lot of things and save you some typing later.
 * BtColorLabel for color
 * BtDateLabel for dates like 2014-12-13 -- there is no BtDateEdit
 * BtDensityLabel for specific gravity/plato
 * BtMassLabel for weights
 * BtMixedLabel for "mixed" fields like Amount on the misc editor.
 * BtTemperatureLabel for temperatures
 * BtTimeLabel for time
 * BtVolumeLabel for volumes
4. Once you have promoted the label, change the "contextMenuPolicy" to
   "CustomContextMenu" in the property editor.
5. Edit the "buddy" attribute to the name of the associated lineEdit.
6. On some very rare occassions, you may need to add a dynamic attribute
   called "editField" on a Label. For example, BtDateLabels have no
   BtDateEdit, so would need the editField attribute defined. For more
   examples, you will need the gory details.
7. Right click on the associated LineEdit and promote it to the equivalent
   BtLineEdit type.
 * BtColorEdit for color
 * BtDensityEdit for specific gravity/plato
 * BtMassEdit for weights
 * BtMixedEdit for mixed fields like Amount in the misc editor
 * BtTemperatureEdit for temperatures
 * BtTimeEdit for time
 * BtVolumeEdit for volumes
8. Add a custom string property named "editField". This should normally be the
   name of the property defined in the associated BeerXML object (e.g.,
   "carbMax\_vol" for the miscEditor's lineEdit\_carbMax ). There are
   exceptions to this rule, but this is the tl;dr version.
9. Still using the Designer, add a Signal/Slot
10. Change the sender to the label's name, set the signal to
   "labelChanged(unitDisplay)", set the receiver to the lineEdit and the slot
   to "lineChanged(unitDisplay)"
11. Save your changes and compile.

The Magic Explained
-------------------

The magic relies on a two Classes and a signal.

The BtLabel class knows what kind of label it is and what menu to pop. When
somebody right clicks on a BtLabel, a menu is found and displayed showing
units and scales, as appropriate. When the user selects a unit or scale, the
BtLabel accesses the configSection and the editField dynamic property to
determine the proper section and attribute name, and then stores that choice
in the configuration file.

Once the property is stored, a signal is generated using the previous value of
that property.

The BtLineEdit also knows what kind of field it is and the two dynamic
properties. When the BtLabel emits its signal, the BtLineEdit redisplays its
contents. The signal carries the old unit/scale with it, so BtLineEdit first
converts that to SI and then from SI to the new unit/scale as read from the
config file. This works because setText does not emit a signal and we can muck
with the contents without having to recalculate anything.

When a BtLineEdit fields interfaces with a BeerXMLElement, it is
really important that the value of the editField attribute be the same as the
attribute being edited. For example, the BatchSize field on the main window
has an editField value of batchSize\_l. This allows some of the very nice
setText() syntax shown later.

### BtLabels

The first part are the labels. The labels are responsible for popping the
necessary context menu, setting the appropriate attribute in the config file
and signalling that something has changed.

BtLabels know five things about themselves:
 * what kind of label they are
 * who their parent object is
 * the section name in the configuration file
 * the attribute name in the configuration file. This should also be the name of the attribute in the BeerXMLElement
 * what menu it should pop

#### Constructor
The first two items are set in the constructor, along with connecting the
customContextMenuRequested() signal to the proper slot. Unfortunately, the
dynamic properties are not available at this point, so the other three pieces of
information have to wait. NOTE: I need to check this against Qt5. It may have
changed.

#### initializeSection
This method tries to find the name of the section in the configuration file.
If the work has already been done, it just returns the cached value.

Otherwise, it uses a multi-step logic tree to find this name. If a name is
found at any time, evaluation stops and that name is used. The order is:

1. A dynamic property called configSection set on the label itself.
2. The configured buddy has a dynamic property called configSection
3. The parent object has a dynamic property called configSection
4. The name of the parent object.

In the course of development, I started with labels and lineEdits defining the
configSection every time. I really cannot express how repetitive that was. I
then found the buddy attribute, which reduced the typing by half. I then
realized I could use the parent name, but sometimes the names were very
unhelpful, like "groupBox" which would have lead to all sorts of
name collisions. So I finally decided to put the dynamic property on the
parent and then leave the decision tree in place just in case you need
to override it.

#### initializeProperty
This method tries to figure out the property name to use. It works like
initializeSection, but uses a shorter decision tree:

1. A dynamic property called editField is set on the label itself
2. A dynamic property called editField is set on the buddy lineEdit
3. If neither is found, nothing is done. I am not sure if this is a good idea
or not

This allows the label to override what is on the lineEdit, and I do make use of
this in a few places like the style editor. I need to think of a better null
behavior than a qDebug() line, but that is what I have at the moment.

#### initializeMenu
The final initialize method gets the menu. This has to be called late, since
we need to know the property and section. Based on the type of the label, the
proper Brewtarget::setupMenu() method is called. The Mixed labels use the
volume menu, which is weird but it mostly works.

#### popContextMenu
This does all the hard work.

The property, configSection and menu are initialized if required. The menu is
then executed.

If the unit menu is what returns, the requested unit is set using the property
name and config section to generate the name. Otherwise, the scale is set.

Some special handling is done when the property name is og, fg or color\_srm.
For these three property names, we also set a min and max attribute. This is
required to make the sliders work properly, and unexpectedly made the Style
editor actually work.

The final bit of processing is to switch the text of the label on
BtColorLabels so that they show the unit being used.

When all of this is done, a labelChanged() signal is emitted, using the
unit and scale of the field before it was changed. This basically signals the
associated BtLineEdit that the unit or scale has changed and the LineEdit
needs to redisplay.

#### Everything else
Everything past that is to simply initialize each label as the proper type.

### BtLineEdit
This class is a bit trickier than the last. This class extends the QLineEdit
class to handle a bunch of different things. The ultimate goal of this class
is to change *everyplace* in brewtarget that says
setText(Brewtarget::displayAmount()) to simply say setText(QString). And I am
very, very close.

Please note that I have over loaded setText() four times.

#### Constructor
BtLineEdit knows five things:

* Its parent
* What kind of BtLineEdit it is
* What the default Unit is (e.g., Units::kilograms)
* The configSection that holds its unit and scale
* The property name that defines it unit and scale

Again, my reliance on dynamic properties means it knows the first three during
construction and we have to figure the last two out later.

The only other thing the constructor does is to connect the editingFinished()
signal to the lineChanged() slot.

#### lineChanged()
This method simply calls the more complex lineChanged(unitDisplay,unitScale) with
noUnit and noScale. It has to be there so that the signature of this slot
matches the signature of the signal.

#### initializeProperty()
This is very similar to the same method in BtLabel. If the value is not known,
it looks for the dynamic property called "editField". Note, there is no
complex series of guesses like there is in BtLabel. "editField" on a
BtLineEdit is required for the magic to work.

#### initializeSection()
Again, similar to the same method in BtLabel. The hierarchy looks like:

1. A dynamic property called "configSection" on the BtLineEdit itself
2. A dynamic property called "configSection" on its parent object
3. The name of its parent

All of the caveats from BtLabel apply. I strong recommend setting this on the
nearest parent that it can be set on. It saves a lot of typing.

#### lineChanged(unitDisplay,unitScale)
One of the things I noticed while working on this code is how frequently
finishedEditing() fires. If focus leaves the window, leaves the field, etc.
this signal is sent. So the first check looks to see who signaled and return
if the BtLineEdit signaled and nothing had actually changed.

If something else signaled (that is, a BtLabel) a boolean is set to ensure we
treat the units correctly. It gets weird a bit further in.

The configSection and property name are discovered, and then we get the unit
and scale as written in the config file. It is somewhat important to
understand that the BtLabel has already written the new values to the config
file at this point. So the unit and scale we get is the *new* unit and scale.

Given a BtLineEdit of mass, volume, temperature or time, we convert the
current value to SI using the previous unit. We then setText to the new unit
and scale via a call to displayAmount.

Color and density work the same way but use a different default precision.

Finally, generic types and the default just go to double and display it.

If force wasn't set, it means we have modified the value not just the display.
Under those circumstances, we emit a textModified signal that is used by
upstream processes to redo their caclulations.

#### toSI(unitDisplay,unitScale,boolean)
Given a unitDisplay and a unitScale, this method finds the appropriate unit
system and calls its qstringToSI() method.

If the boolean is true, we will override the provided unit and scale and get
the current values out of the config file. This changes depending on who
calls lineChanged() -- input in the field will use the config file, a
lineChanged emitted by a BtLabel will not.

Once we figure out which units and scale to use, we use
Brewtarget::findUnitSystem to find the proper UnitSystem for the new unit. If
we find a UnitSystem, we then find the proper Unit for the provided scale. If
we cannot find the scale, we use the default scale for that UnitSystem (eg,
Unit::kilogram for siWeightUnitSystem).

Assuming that all works, we invoke the proper toSI() method for that
UnitSystem.

If we cannot find an approproate UnitSystem and the BtLineEdit is a STRING
type, we just return 0.

If all else fails, we just return the value of the text() in the BtLineEdit
converted to a double.

#### displayAmount(double,double)
I got tired of having to find the unitDisplay and unitScale each time I called
Brewtarget::displayAmount. So this method simply isolates all that work for
me.

#### setText(double,double)
This is the base. Given two doubles (amount and precision), the text is set to
the displayAmount() using QLineEdit::setText().

#### setText(BeerXMLElement\*,double)
Given a BeerXMLElement\* and an optional double, this version will use the
editField attribute on the BtLineEdit to get the value from the
BeerXMLElement.  displayAmount(double,double) is then called using the found
value and the provided precision. Finally, QLineEdit::setText() is called to
display the results.

#### setText(QString,double)
Given a QString and a double, the QString is converted to a double,  and
displayAmount(double,double) is then called using the converted
value and the provided precision. The results are finally displayed with a
call to QLineEdit::setText().

#### setText(QVariant,double)
Given a QVariant and a precision, the QVariant is converted either to a string
if the BtLineEdit is a STRING or a double otherwise. Once the conversion is
done, we call displayAmount() and QLineEdit::setText()

### Everything else
After that, it is all configuration work, except for....

### BtMixedEdit
The problem was how to handle fields, like the amounts on miscellaneous items,
that can represent either masses or volumes. After a lot of thought, it
occurred to me that all I had to do was fudge the class a little.

As the comments suggest, this class is kind of evil. Well, actually it is
pretty much all evil and I strongly suspect I will come to regret this
decision.

#### constructor()
So the constructor lies. It sets itself as a VOLUME and a default unit of
Units::liters.

#### setIsWeight(boolean)
And this is the evil. All of the check boxes that mark if something is a mass
or volume trigger this slot.

If the checkbox is marked, it means the associated field is now a Mass. This
will cause the BtMixedEdit to change its type to Mass and its default unit to
Units::kilograms. If the box is unchecked, it change its type back to Volume
and the default to Units::liters. We then call lineChanged() to do its magic.

It works astonishingly well, but you do get some odd results if you change the
field from one to the other with a value already in the line edit. 2 lb will
suddenly become 2 gallons.
