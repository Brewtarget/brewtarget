# Recipe versions -- an illustrated guide
Well. There won't be any pictures, but I will try to describe how they work and why.

## Motivation
I write and tweak a lot of recipes. I desperately want some way of being able to view that history and maybe say "Yeah, that one wasn't good. Let's go back two versions and try this instead".

My database is already littered with "Beer v1", "Beer v2", and so on. I find it to be ugly, and I am not very good at remembering to do it. And that is exactly the thing computers are supposed to do -- handle routine, mundane tasks that humans are not good at.

## Background
The very simple idea is that a copy of a recipe is made before you modify it, and any modifications are made to the copy -- in other words, copy on write.  The tricky parts were how to implement and how to represent it.

With the addition of folders somewhere around v2.1, how to represent it became easier. The HEAD of a recipe would appear in the tree, and the previous versions would be displayed underneath it, as if the HEAD were a folder.

The other main question was what defines a "write" and when do we want to make the copy. If we literally copied on every write, the database would soon be littered with copies -- remember that we set og/fg on every load of the recipe.

# Design Choices
There are any number of design choices that have to be made when doing something like this.

## Ancestors or Descendants?
I played with this for a bit and decided that a given recipe should know its nearest ancestor instead of knowing its nearest descendant. A root node will only know of itself. Every ancestor will have a display of false, with only the HEAD node having a display of true.

This had a few really easy wins for me. The BtTreeSortFilterProxy algorithm already shows only those recipes with display set to true. This allowed me to create a version and do nothing more to get the display behaving. It did mean I had to figure out some clever ways to override it when I wanted.

This also introduces some basic terminology. I started thinking this in terms of fork, HEAD, etc. but somewhere transitioned to thinking in terms of ancestors and descendants. I will still sometimes discuss HEAD or leaf recipe, but I will mostly discuss things in terms of descendants and ancestors.

## Branches?
I am pretty much declaring there will be no branches. Branches imply merges and I do not feel smart enough to handle that sort of nonsense.

This has some consequences:
- Once a recipe is forked, the ancestor recipe becomes readonly;
- You cannot delete a recipe after it has been forked, without first deleting all of the descendants of the recipe;
- you will need to orphan each descendant if you want to reorder the relationship;
- you can only really operate on the HEAD recipe

## What "write" means.
Anything that changes the recipe after it has been brewed should fork the recipe. But what constitutes a "change"? For example, if I change the assistant brewer on the Extras tab should that fork the recipe?

I have basically come to this rule set:
   1. If a change is made to a Recipe object, the recipe does not fork;
   2. If a change is made to an Instruction object, the recipe does not fork;
   3. If a change is made to a Mash object, the recipe does not fork;
   4. If a change is made to a BrewNote object, the recipe does not fork;
   5. If a change is made to the first mash step that only changes the infusion temperature, the recipe does not fork;
   6. All other changes fork the recipe.

I am very conflicted about the instructions. I can make an argument for forking the recipe if a step is added, removed or moved -- it is an important change to the recipe, and people would want that history as well. I can also make an argument that says modifying the instructions doesn't change the recipe, only the process and it shouldn't fork. Right now, I am saying modifying the instructions won't fork.

The fifth rule is a bit weird. The problem is that users are supposed to be recalculating a mash each time they brew in order to adjust the starting temperature. Doing that would adjust the starting temperature, but not the target temp of the initial infusion. This should not fork a recipe. Any other change to a mash -- adding more steps, changing the volumes in a step, changing the target temperature, etc. -- should.

## Manual Controls
Another important series of considerations were just what manual controls would be allowed. This is important particularly when considering the first time this feature is exposed, it would be nice to allow people to say "this recipe is an ancestor of that" and have the right thing happen.

### Defining ancestors
The user will be able to manually mark a recipe as an ancestor of another, by simply dragging the ancestor to the descendant.

Dragging recipes around was very confusing and not productive. I ended up having to create a small dialog that allows you to assign an ancestor to a descendant. I still do not think this is the best solution, but it made more sense than drag/drop.

### Forking
Users will be able to manually fork a recipe. This will override the usual checks and will be done even if the ancestor didn't get brewed.

### Locking
Users will be able to hard lock the recipe. If I am going to make ancestors read only, we should expose that ability to the users.

If a recipe is locked, it means no changes will be permitted to the recipe except brewnotes. This actually makes other aspects of the code easier -- when a recipe is forked, it just sets the lock flag and the same code that prevents an ancestor from being modified works.

### Disabling the versions
The final user control is that the option dialog allows the user to turn the versioning on or off. It was suggested by some people that they would not like the versioning happening automatically. By default, we will version. The value of this option will not impact the ability of the user to either manually lock or version a recipe.

## Locking Recipes
I had considered simply overloading the display variable to control if the recipe is locked; display == false would imply the recipe is locked.

If I want the users to be able to hard lock a recipe, setting display to false will make the recipe disappear from the tree. Mucking with the sort/filter/proxy simply isn't easy. And there may well come a time when we need to distinguish between the two states.

Therefore, I will need a another column on the recipe table.

This is going to be somewhat tricky. You should be able to unlock a leaf recipe, but you cannot unlock an ancestor (remember, no branches). In short, I will need to find some way to lock the lock flag.

## Deleting Recipes
This is a hard one. Assuming that ancestoral recipes are locked and cannot be deleted, what happens when the descendant is deleted? Do we delete the entire history, or do we just delete the descendant and make the most recent ancestor active again?

I don't think there's a sane way to prompt the user, based on the prompts we already have.

The default behavior will be to only delete the HEAD recipe. The behavior can be changed via the options screen to delete the entire chain. There is no per-recipe prompt, as I would have wanted. There is just no sane way I could see to do that.

## Open design questions
1. Am I right about what "write" means?
2. Currently, every recipe (HEAD or not) carries a list of each of its ancestors. Is this a good idea, or should only the HEAD know its ancestors?
3. I really do not like the drag/drop thing. I cannot see a different way to do it that doesn't become modal.
4. I fear what this feature will do to the inventory magics.

# Implementation
This has been an interesting exercise. The majority of the changes are in the database -- both tables and code -- and the BtTree[View,Model,Item]s. It has required me to understand much better when we change a recipe.

## Signals
I needed the signals for new items to stop being named newEquipmentSignal, newHopSignal, etc. The problem is there is no easy way to use a template method to call those. I decided to change each of the methods to be newSignal(Equipment\*), newSignal(Hop\*), etc.

That gave me one easy signal to call that could easily be templated.  Based on how Qt does it signals, we are still being selective in which signals are being trapped -- that is, the hops tables still only get notified when a new hop is added.

I also changed the name of the signal from newSignal to createdSignal, just to better mirror the deletedSignal.

### spawned(Recipe \*ancestor, Recipe \*descendant)
I added a new signal to the Database class to indicate when a recipe is spawned.

## Database changes
As indicated earlier, we have some basic changes to make to the Recipe table in the database and a number of changes to make to the Database class.

### Table Changes
I had to make two changes to the Recipe table.

#### ancestor\_id
This column stores the id for the recipe's ancestor, or to itself if it has none. As part of the upgrade to the database, each existing recipe will have it's ancestor\_id set to its own ID. This will make each recipe a root node.

#### locked
This is a boolean column indicating if the recipe is hardlocked or not. The initial database upgrade will set this attribute to false for every recipe.

### New class methods
The changes to the class were a bit more extensive, as you may imagine. I had to write a number of new methods, along with some modifications to existing methods.

#### breed(Recipe \*parent)
This is a convenience function to determine if the parent isn't locked and if it wants to be versioned. If it is unlocked and does want a version, newRecipe is called and the new recipe is returned. Otherwise the recipe is returned unmolested.

#### clone()
This is a little messy. With all the different new methods like newEquipment, newHop, etc. there is no easy way to template this. Each NamedEntity type is handled, the proper new* method is invoked. If required, the new thing is added to the recipe.

#### getParentRecipe(MashStep const \*step)
I had to make a new getParentRecipe() method to find the recipe to which a mashstep belongs. The weird part is the query doesn't touch the mash table. You can go straight from the recipe table to the mashstep table.

Because that query is different, I needed the different method.

#### modifyIngredient(NamedEntity \** object, QString propName, QVariant value, bool notify)
This is the work horse. When an ingredient in a recipe is modified, this is the method that handles the logic. Given how important this method is, it is pretty short.

getParentRecipe is called to determine which recipe, if any, the NamedEntity is in.

If the parent recipe is found, and that recipe wants to be versioned, we:
- call spawnWithExclusion() to clone the recipe, less the ingredient being changed;
- the ingredient being changed is cloned;

If there is no parent recipe, or the recipe doesn't want to be versioned, we ignore all of that logic and just operate on the original NamedEntity.

Once that we've done all that, we call updateEntry to make the actually modification we wanted to in the first place.

A tricky bit here are the signals. I was originally signalling from spawnWithExclusion(), but that caused a very fun infinite loop. I had to move the signaling into modifyIngredient, but only if we spawned a new recipe.

#### wantsVersion(Recipe \*thing)
Determines if the recipe wants to be versioned or not.

If the user has disabled versioning via the options panel, this method always returns false.

Otherwise, the method searches for any brewnote associated with the recipe. If at least one brewnote is found, the method returns true. Otherwise, it returns false.

#### setAncestor(Recipe \*descendant, Recipe \*ancestor, bool transact)
This method handles the hard work of making one recipe an ancestor of another.

If the ancestor and the descendant point to the same recipe, then this method will orphan a recipe. It is probably a bit of overloading that I will regret later, but it makes twisted sense to me.

This requires three different steps:
    1. Set the ancestor\_id of the descendant to the ID of the anscestor;
    2. If we are orphaning a recipe, set the display flag to true. Otherwise, set the display flag to false;
    3. If we are orphaning a recipe, set the locked flag to false. Otherwise, set the locked flag to true.

#### ancestoralIds(Recipe const \*descendant)
This is the evil that started it all. It runs a single query that finds all of the descendant's ancestoral recipes. I had started out using a QList of keys. This method still returns the QList\<int\>, but those get translated into Recipe\* upstream.

Why is this evil? It uses a recursive SQL query.

An interesting side effect of the query is that the HEAD recipe is also in the list of ancestors. I've considered "fixing" that, but haven't to date. It does require some special casing later.

#### numberOfRecipes()
This is a simple method that returns the number of recipes in the database.  This information is used in several places in the code, and each time we were querying the DB instead of using the allRecipes list. So I fixed it.

### Modified class methods
This is a smaller list than you may anticipate, since much of the hard work is done in the tree classes.

#### addToRecipe() methods.
I added a new set of bulk addToRecipe() methods that allow that the calling method to say "Add all of these, except this one". It makes sense in context of how I handle the cloning.

All of the addToRecipe() methods were modified to:
- Respect the recipe locked flag. If you try to add anything to a locked recipe, the method simply returns
- Before any changes are made, we call breed() with the original recipe.
- This required some small changes to reference the spawn instead of the original
- Finally, they emit the spawned signal if the recipe spawned.

## Recipe Class changes
Most of this work focuses on the recipes, so the Recipe class had a few changes.

### New attributes
I had to add two new attributes to the recipe class:
   - m\_locked: determines if the recipe is locked or not
   - m\_ancestors: this is a QList of Recipe\* that stores the recipe's ancestors

### New methods
As I've said before, where there are new attributes there must be setters and getters.

#### locked()
Returns true if the recipe is locked, and false if not.

#### setLocked(bool var)
Sets the locked attribute to var

#### ancestors()
Returns the contents of the m\_ancestors attribute. If the m\_ancestors attribute hasn't been initialized (will this happen?), loadAncestors() is called.

#### loadAncestors()
It takes the results from Database::ancestoralIds() and translates them into actual Recipes. The resultant list is stored in the m\_ancestors attribute.

As a side note, I had thought to do this only for leaf nodes but some of the display code requires every recipe, ancestor or not, to have this attribute set.

#### hasAncestors()
Returns true if the recipe has ancestors (ie, m\_ancestors.size() > 1).

#### setAncestor(Recipe \*ancestor)
Sets the provided recipe as the current recipe's ancestor. It calls Database::setAncestor and then loadAncestor()

### Modified methods
Other than those new six methods, I didn't have to do a lot to recipe.

## BtTree changes
A lot of the hard work is actually being done by the various BtTree classes, in conjunction with the filter proxies. The basic approach I took was to modify the filter proxies so that they would or would not display the ancestors, and then modify the model to show different information to the view.

When discussing these changes, you should likely keep in mind that versioning only applies to recipes.

###BtTreeView
Of the four classes I will discuss in this section, BtTreeView had the fewest changes. I basically had to pass the requests to view or hide ancestors through to the model, and handle the doing some clever work on the context menus.

The class itself got a new menu and five QActions defined. These are used to hold the menu actions and easily enable/disable them as I want.

We also emit a new signal called recipeSpawn. This signal is trapped by MainWindow to force an update of the display.

#### showAncestors()
This method makes sure a recipe tree has made the request, gets the selected rows and then calls the model's showAncestors() method for each row.

#### hideAncestors()
This method makes sure a recipe tree has made the request, gets the selected rows and then calls the model's hideAncestors() method for each row.

#### orphanRecipe()
This method makes sure a recipe tree has made the request, gets the selected rows and then calls the model's orphanRecipe() method for each row.

#### spawnRecipe()
This method makes sure a recipe tree has made the request, gets the selected rows and then calls the model's spawnRecipe() method for each row.

#### enableDelete(bool enabled)
#### enableShowAncestor(bool enabled)
#### enableHideAncestor(bool enabled)
#### enableOrphan(bool enabled)
#### enableSpawn(bool enabled)
These five methods enable and disable context menu options.

#### setupContextMenu(QWidget \*top, QWidget \*editor)
This method was changed to add a new submenu named "Ancestors" to the Recipe tree. This submenu contains four options: Show, Hide, Orphan and Version, which calls showAncestors, hideAncestors, orphanRecipe and spawnRecipe respectively.

#### contextMenu(QModelIndex selected)
When the context menu is popped, this method will now dynamically enable and disable the ancestor options. The decision path is something like:
- If the recipe is locked, disable the delete action
- If the recipe is showing ancestors and is the leaf descendant, then enable the hideAncestors action
- If the recipe has ancestors but we are not showing them, enable the showAncestors action
- If the recipe has ancestors and we are the leaf node, then enable the orphanRecipe action
- If the recipe is a leaf node, then enable the spawnAncestor action.

These choices are made independently of each other. I am somewhat confident they behave properly.

### BtTreeItem changes
This class was the least modified in all of this. After a lot of messing around in the filter, it occurred to me that I needed the individual BtTreeItem to know if it was to be displayed or not.

So the class got a new boolean attribute called m\_showMe. If m\_showMe is true, the display attribute will be over-ridden. If it is false, the display attribute will be used. The easiest way to think of this is simple boolean or
logic -- m\_showMe or display. If m\_showMe is true, display is ignored. If m\_showMe is false, display controls what gets shown.

This required adding the getter/setter methods.

### BtTreeModel changes
This class of all received the most significant changes. If you've figured out the tricks I use to display the folders, this shouldn't surprise you.

I redesigned the class a little to avoid many switch statements. The individual trees now set the column count during initialization.

The methods are in someways quite similar: a item pointed to by the index is removed from the tree, things are done and then it is put back in the tree.  I've tried a number of ways around this, but it seems this is a required series of steps to make things work.

#### showChild(QModelIndex child)
A small convenience method that finds the BtTreeItem at the provided index and returns the current value of m\_showMe. This is used by the filters to determine if a node will override the display.

#### setShowChild(QModelIndex child, bool val)
For every getter, there will be a setter and this is it. It sets the m\_showMe attribute of the `child` BtTreeItem to `val`.

#### makeAncestors(NamedEntity \*anc, NamedEntity \*dec)
This method allows a user to drop one recipe on another to create an ancestory. The NamedEntity pointers are an odd side effect of the drag/drop mechanisms.

If the two recipes are the same thing, the method returns.

Otherwise, the two NamedEntity are translated into Recipe objects. The method finds the QModelIndex of the ancestor in the tree and then removes it from the display. The method then sets up the relationship between the two recipes in the database using the Recipe's setAncestor method. That handles all the underlying complexity in the database.

The method then finds the descendant in the tree. It is important that this happens after we have removed the ancestor; otherwise, the indexes will be incorrect. We then remove all of the descendant's brewnotes, and add everything back in. It is a little odd, but we will show duplicate brewnotes otherwise.

#### showAncestors(QModelIndex ndx)
When called, this builds the necessary subtree to display a recipe's ancestors. This method is made a little more complex by the idea that all brewnotes are shown on the leaf recipe until we show ancestors. Then the brewnotes are shown associated with the version of the recipe they were brewed with.

If the ndx is invalid, the method simply return.

The first task is to remove all of the children from the recipe. This basically removes any brewnotes from the display.

Once that is done, the method gets the recipe from the index and gets the list of ancestors from that recipe. The brewnotes associated with the recipe are
added back to the display.

To make the context menu generation work the way I want, the m\_showMe attribute on the leaf node is set to true.

The method then loops through all the ancestors, and adds each ancestor as a subtree to the leaf node. The showMe attribute for each BtTreeItem is set to true, which will cause the filter to do the right thing. A dataChanged() signal is emitted, which will cause the filter to do its thing and show the ancestor. The final step is to add the brewnotes back to the recipe.

As a happy side effect of the recursive query, we don't have to work to sort anything -- the list of ancestors will always be youngest to oldest.

#### hideAncestors(QModelIndex ndx)
This is the logical inverse of showAncestors(), and it works in a very similar fashion.

The rows under the leaf recipe are removed. This removes everything from the display.

Once that is done, the method gets the recipe from the index and gets the list of ancestors from that recipe. All brewnotes, both the leaf node's and the ancestors', are added back to the leaf recipe.

To make the context menu generation work the way I want, the m\_showMe attribute on the leaf node is set to false.

The method loops through the ancestors and sets the m\_showMe back to false. dataChanged() is emitted to force the filter to re-evaluate the list.

#### orphanRecipe(QModelIndex ndx)
This method does the heavy work of removing the HEAD recipe from a chain. The method first finds the recipe to orphaned from the provided ndx, and then finds the recipe's ancestor.

Once we have that, we start the work. The orphan is removed from the tree, and we use setAncestor() to make the orphan its own parent. The brewnotes are added back to the now orphaned recipe.

The ancestor is unlocked and the display is set to true. The ancestor is added to the tree and then the ancestor's brewnotes are added back to it.

#### spawnRecipe(QModelIndex ndx)
When a recipe is spawned, the immediate ancestor is removed from the tree. We then emit the dataChanged() signal to make the filter redraw, and then a recipeSpawn to let the MainWindow know it has to do a few things.

## MainWindow/UI changes

### New methods
Displaying an ancestor or an otherwise locked recipe raised some interesting problems. Simply disabling the recipe tab and the ingredient tabs sort of worked, but not quite.

Disabling the recipe tab would also disable the lock button. A recipe, therefore, could only ever be locked and never unlocked. Disabling the tab also made the text very hard to read.

#### lockRecipe(int state)
My solution then was to write this method. It handles enabling and disabling all of the fields when a recipe is locked, or when a locked recipe (aka, ancestor) is loaded.

If `state` is Qt::Checked, then we are locking the recipe. If it is Qt::Unchecked, we are unlocking the recipe. A few booleans are set so we can lock/unlock with the same commands.

The recipe is first locked/unlocked. The style and equipment boxes are disabled/enabled, and then the input fields in the upper left of the main panel.

The delete buttons are disabled/enabled, and drag/drop is disabled/enabled on the recipe tree. The fermentable, hop, misc and yeast tables are disabled/enalbed along with the add, remove and edit buttons.

#### versionedRecipe(Recipe \*descendant)
This is a slot method that traps the recipeSpawn signal from the recipe tree.  When it is invoked, we make sure the descendant is displayed and selected in the recipe tree.

### Changed methods
Oddly, only a few things needed to be modified.

#### MainWindow()
MainWindow needed to trap two new signals:
- recipeSpawn as explained previously
- stateChanged from the new locked checkbox so the right things would occur

Additionally, I solved the unreadable text problem by creating a new style sheet that made the text for a disabled field a darkish grey. This style sheet is then attached to the recipe tab and the ingredients widget. I will likely come to regret my color choices, but we does the best we can.

#### setRecipe(Recipe \*recipe)
A few changes were required here. When a recipe is loaded, the locked checkbox is set appropriately. Oddly, calling setCheckState doesn't actually invoke the signal, so I also manually call lockRecipe() to get the proper thing done.

The last chunk is to make sure the "locked" button is enabled if this is a leaf recipe or disabled if it is an ancestor.

