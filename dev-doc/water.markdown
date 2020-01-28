# Overview
I am tired of having to use external tools to adjust my water chemistry, so now I don't.

It has taken some wedging to make this all work right, as there is a significant disconnect between what the Beer XML spec does for water and what I want to do. The code to bridge that gap is still outstanding.

## Design goals
I wanted to have a screen where I can configure the salt additions to my brewing recipes, including estimating the pH. 

I wanted to keep the disruption to the main screen as minimal as possible. Not all people care about water chemistry and I did not want to make them have to deal with it.

I wanted to be able to save and reload the water modifications for a specific recipe.

## Limitations
I have currently limited water modifications to just "sparge" and "mash". I considered trying to allow you to associate a specific salt addition to a specific mash step, which is what seems to be implied by the waters BeerXML. It got really confusing, really fast. It also isn't how I do water chemistry. A quick survey of several spreadsheets (ez_water, Br'un water and Kai's) as well as an on line tools (Brewers Friend) suggested nobody else was supporting this either.

There is no way I can see to automatically link a salt, eg CaCo3, to it's equivalent item in the misc table. The problem is basically that the only way to link them is to search the misc table by name, eg, "Calcium Carbonate". "Calcium Carbonate" might be spelled "KalziumKarbonate" if you are using an older German spelling. There simply is no way I found to get around this problem.

In the current Water table, there is no mention of which salt is used just the ppm of the ion. Unfortunately, a Ca ion can be added by three different salts: CaCO3, CaCl2 or CaSO4. This makes using the existing water table for my purposes awkward.

The current list of salts is hard coded. If we want to add a new one, it would require editing the code and updating things. I am not overly concerned by this, as I do not believe we will discover a new brewing salt tomorrow. This choice may come to haunt me later. 

I have avoided the spelling problem by referring to every salt by it's chemical formula, eg. CaSO4 instead of `gypsum`. I am hoping anybody who wants to use this tool/feature will be able to read the chemistry and translate it mentally as needed.

Once a water is associated with a recipe, there is no way to remove it. You can change it, you can update it but it can't be removed.

# Solution
My solution required two minor changes to the mainWindow -- a new Tool called "Water Chemistry" and a new BtTreeView to display the water profiles. Almost all of the work is done in the new "waterDialog".

## Water table changes
To support the new dialog, I needed to modify the existing water table by adding several columns:
    * `wtype`      -- indicates if a water is a base or a target when added to a recipe;
    * `ph`         -- remembers the calculated pH of the recipe;
    * `alkalinity` -- remembers the alkalinity of the base water;
    * `as_hco3`    -- is the alkalinity measured in HCO3 ppm or CO3 ppm?
    * `sparge_ro`  -- the percent of the sparge water that is RO
    * `mash_ro`    -- the percent of the mash water that is RO

## Salt table
I considered getting clever with the water table. For example, if a water had a Ca ppm and a Cl ppm, then it is CaCl2. I dislike that kind of clever, because I prefer to declare a thing instead of inferring its thingness. So I added a new table called salt. A salt table has these columns:
    * `id`    -- the standard autoincrementing ID everything uses;
    * `addTo` -- an enumerated type indicating if the salt is being added to the mash, the sparge, both in equal amounts or both in proportion;
    * `amount` -- the amount being added
    * `amount_is_weight` -- I cheated a little and store both acid and salts in this table. Since lactic acid is almost always a liquid, I need to support proper units for both types;
    * `is_acid` -- false if it is a salt, true if it is an acid;
    * `name`    -- a printable name for the salt/acid
    * `stype`   -- an enumerated type indicating what salt/acid it is (eg CaCl2 or H3PO4);
    * `misc_id` -- an idea in progress

Unlike all the other tables, the enumerated types are stored as their values and not as strings. The type is translated to something displayable only when required, which is almost never.

The addition of the new table required an new Schema header file and some modifications to the TableSchema class to support it. They are pretty standard changes and I will not document them closely. I will, however, state that I think the TableSchema idea works really well. Adding a new table turned out to be pretty straight forward.

### salt\_in\_recipe
A simple linking table so I can quickly find all of the salts in a recipe. It follows the standard format of our \_in\_recipe tables.

## Salt
I created a new class called `Salt` which supplies the interface between the code and the database object. It does the usual setters, getters and initializers. It defines two new enum types (`WhenToAdd` and `Types`) as described above. It also includes 7 methods to calculate the molality of a given ion. 

I cheated slightly and made `Salt` handle both salts like NaCl and acids like H3PO4. While I really wanted say things like ` drop table acid`, I couldn't quite bring myself to do all the work required. I also considered renaming the `Salt` class and tables, but could never find anything that actually worked.

## GUI
I added two new interfaces, `WaterDialog` and `SaltTableModel`, and rehabilitated the existing `WaterEditor` for modifying profiles. While it would be a long and boring bit of writing, I am not going to describe how the screens are laid out. Run it and you will see it.

Modifying a salt is done via the `saltTableModel`. Changing a salt will generate a `newTotals` signal. This signal is caught by the `waterDialog` and causes the calculations to be redone. I could have probably done that differently, but this does what I need and it seems more idiomatic.
