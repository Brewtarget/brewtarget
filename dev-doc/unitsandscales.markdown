Units, UnitSystem and UnitSystems
---------------------------------
The intent of this documentation is to make some sense of all the different
classes with the word "unit" in their name. This should not be a line-by-line
analysis; I will try to keep it at a higher level and describe
the function of each of the major classes and each of the important methods.

This should help the next time somebody wants to dive into this part of the
code base.

UnitSystems
===========
I will start with the easiest of the three. UnitSystems is simply a
convenience class. Each of the methods, e.g. usWeightSystem(),
instantiates a static UnitSystem of the same type. This makes sure we only
ever have one, but don't have any more than we need.

To some extent, we could probably do a trick like what is done in Unit and
just instantiate one of everything in UnitSystem.

The only time you should need to modify this class is if you are adding a
completely new unit system.

UnitSystem
====

UnitSystem is next. This one is more complex, but still not that hard to
understand. It consists of the base class, UnitSystem, and then 12 subclasses
to define each unique unit system, like USVolumeUnitSystem or
SIWeightUnitSystem.

The UnitSystem classes are the ones that know all of the units of a specific
system, e.g., SIWeightUnitSystem knows Units::kilograms, Units::grams and
Units::milligrams are all part of it.

The UnitSystem classes understand the relative sizing between the individual
units. To continue the previous example, SIWeightUnitSystem understands that
millgrams are smaller than grams are smaller than kilograms.

You should avoid using Units directly, but work through one of the
methods documented below from the appropriate UnitSystem.

### UnitSystem
The UnitSystem class does most of the work. The subclasses set the
important variables to control the ouput.

#### The Constructor
The constructor is there to create the regex required to parse input strings.
Previously, brewtarget required a space between the number and the unit, e.g.
"12 L". At some point, mikfire decided this was too annoying and wanted to be
able to say "12L" as well.

The final regex is somewhat complex, because we need to handle "1,200.5L" and
we need to handle "1.200,5L". We know which one to use based on the locale and
construct the regex. This still may not be one of my best ideas....

#### qstringToSI()
This methods takes a qstring, breaks the string into an amount and a unit and
then translates the amount into the SI equivalent. The calling code has the
option to send a default unit and to force qstringToSI() to use that default
unit.

We use the regex to split the string, but we use a Locale-aware conversion to
turn the string into a double. This should let us parse 1,250 and 1.250
properly depending on locale.

The tricky part was to figure out what "3 qt" means. It could mean 3 imperial
quarts or 3 US quarts. The code jumps through a number of hoops to make a
smart guess. The logic currently does something like this:

1. If the field is set as US or Imperial, use what the field was set to.
2. If the field is set to SI and the system default is US or Imperial, use
   the system default
3. If all else fails, we will assume you meant US Customary.

#### displayAmount()
This method is sort of the reverse of qstringToSI() -- given a double, a unit
system and a scale, it will generate a string suitable for display. If the
scale is provided, the returned string will be in that scale. If no scale is
provided, we will do the logic to find the largest scale.

The refactor required some changes in this part of the code. In order for the
parent class to be able to do the work, all of the unit classes (not unit
system classes, but unit classes) had to define a boundary amount. Setting
this allowed us to craft a nice for loop to find the largest scale.

Other units have no scale, like temperature. A special scale of "without" is
defined to allow the code to short circuit most of the hard work.

#### amountDisplay()
The purpose of this method is to stop displayAmount().toDouble(). The problem
with that approach was that "1.056 sg" will be returned by displayAmount().
"1.056 sg".toDouble() will return 0. This method will return 1.056.

Other than just returning the translated amount, amountDisplay() works exactly
like displayAmount.

#### scaleUnit()
Find the appropriate unit in a given UnitSystem that matches the provided
unit. A QMap look up is used to translate from the scales (extrasmall through
huge) to the correct unit (e.g., teaspoon to barrels for US Volumes)

### The others
All of the other UnitSystem files are subclasses of UnitSystem, and they exist
mostly to populate two maps and set some variables.

The first map, scaleToUnit, is used in the parent's scaleUnit() method to
provide the translation from scale (like extrasmall) to a unit (like tsp).
This is a QMap, which means we will iterate through the list in the order
created. It is very, very important that the list be created from smallest to
largest. Otherwise, displayAmount() will break in interesting and bad ways.

The second map, qstringToUnit, translates from a unit name like "mL" to the
appropriate unit class like Units::milliliters within the UnitSystem.

##unit.cpp and unit.h

The Unit class and all of its subclasses are defined between these two files.
There is still some refactoring to do here, as there is code in the .h file
and I think it could be tightened up further.

The base class provides seven methods used by all of the subclasses, and each
subclass defines a few constants and also knows how to convert itself to and
from SI.

###Unit
This is the base class. It provides most of the interesting methods, plus a
number of enumerated types.

I need to make note that a QMultiMap is used to map from strings like "qt" to
an actual Unit like Unit::us\_quarts. This may seem like a duplication of some
of the work done by UnitSystem and it sort of is. It is used by getUnit when
the calling method doesn't know how to handle a unit (e.g., you have SI set as
the display but enter "12 qt").

#### unitFromString and valueFromString
These are private methods used to extract the unit or the value from a string.
Assuming an input of "12 qt", valueFromString() will return 12 and
unitFromString() will return "qt".

####convert()
Converts a given QString value like "12L" into another unit. I don't think
any class uses this method but I don't know why. As it isn't used, I won't go
in depth on it.

####getUnit()
This was a joyous refactoring. After puzzling over this method for months, it
finally occurred to me there were two very simple use cases to cover.

Under almost every circumstance, there is a one-to-one relation between unit
names and Units. 'C' will always map to Unit::celsius, for example. The snotty
bits are the volumes, since we have to deal with the difference between
USCustomary and Imperial.

So the first use case is there is only one match. If we find just one, the one
we found is returned.

The other use case is the volumes, which can return 2 Units for a given name.
We just iterate through those two items to see if one of them matches the
system default. If it does, it gets returned.

If no match was found, we default to USCustomary and return the appropriate
Unit.

####setupMap()
This just sets up the QMultiMap used by getUnit().

###All the Rest
All of the other methods in this file exist simply to configure a specific
unit. To do that, the constructor sets four values:

* unitName -- the name for the unit. This is what the user would enter into a
field, like "kg" or "L".
* SIUnitName -- the name of the SI unit for this unit. For example, if
unitName is "lb", SIUnitName is "kg". I think there is an opportunity to
refactor this sometime later.
* \_type -- defines if this unit is a Mass, Volume, Temp, etc.
* \_unitSystem -- can be SI, Imperial or USCustomary.

The two methods defined for each Unit is toSI and fromSI. They convert the
provided value as indicated.
