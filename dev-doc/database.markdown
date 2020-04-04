# Introduction
This is intended to document what changes I've made to how we interact with the database and why I made them. Additionally, I hope to explain how to add a new column to an existing table and how to add a new table. While the first part is important, I suspect most people will care about the second part.

## History
In the beginning, there were XML flat files. These files were strictly BeerXML compliant and took a long time to load. I tend to have a lot of recipes with numerous brewnotes on each one. Loading the flat files took in excess of 30 seconds from the time Brewtarget started before it was usable. And I had the brilliant idea "Hey! Let's put this all into an RDBMS!".

I mocked up a sample schema (which can still be seen in ideas/parse\_database.pl) and put the idea forward. Rocketman agreed with my idea and we set about it. It was hard and, frankly, I think burned both me and Rocketman out. Rocketman did the initial heavy lift of figuring out how to do it in Qt, as it was beyond my ability. I chased the bugs and wrote the code to make it complete. This work was released as Brewtarget v2.0

In the course of making these changes, we made a few decisions that would have an impact and are relevant to this document. Our first decision was to use SQLite as the RDBMS, with an intent that we would expand support later. This was done to make the transition for users easy, and to reduce the amount of code we needed to write. The second decision was that we would cache nothing -- all reads would read from the database and all writes would immediately write to the database.

After the release of version 2, I decided to expand the database to support PostgreSQL. I wanted to an RDBMS with a network interface, so that we could consider a mobile version and not have to worry about synchronizing data between multiple sources. My choice to use PostgreSQL was one mostly of personal preference, but it turns out I made a lucky choice. We had used 'id' as the primary key on all the tables, and it seems 'id' is a reserved word for MySQL. One of the important drivers of this series of changes has been how to address that problem in as transparent a manner as possible.

In the process of going to PostgreSQL, it became apparent that we were suffering significant performance issues due to our decision to never cache. I did some investigation and decided that we needed to cache at least three properties: name, deleted and display. I modified the BeerXMLElement object and wrote some primitive caching mechanisms that resulted in a significant performance increase. Some of this change can be considered as the logical successor to that.

## Other issues
Over time, we have grown an uncomfortable number of hashes, arrays, etc. to define the tables. In the current code base, if you want to add a column to a table you need to edit 7 (maybe 8) files in multiple places and make sure that your edits were consistent. This makes me very unhappy and does not spark joy. So I wanted to find a better way.

I also have a dislike for DatabaseSchemaHelper. Between the code and the headers, it consists of 2000 lines of code that is hard to read, hard to parse and very difficult to maintain. It also duplicates almost all of the information we are stashing in the previously mentioned hashes, arrays and maps. Any solution needed to reduce that class to the task of updating schema.

# Overview
This change is massive. It touches literally every BeerXML object and changes how we interact with every table. I will attempt to document what the changes are and why they are. I promise it is worth it; my start up time has gone from 8 seconds to 2 seconds and a remote database almost becomes usable.

## Caches
Every table primitive (eg, hops, equipment, etc.) now caches the attributes.

### Reading
Every read operation now simply returns the cached value. All by itself, this change results in a massive performance increase. The main performance problem we were experiencing was caused by the number of queries we ran, not the queries themselves. Reducing the number of reads we made was huge.

This resulted in each BeerXMLElement getting a long list of attributes. I tried to follow the basic naming convention of preceding the name of the attribute with m\_. For example, Recipe has an attribute called m\_type. I would have normally just done the \_ prefix, but this has the potential to cause problems and the m\_ is recommended.

Every get method now just returns the cached values. It is up to the setters and the loaders to make sure the cache is correct.

### Writing
When writing a new value, we now have to update the cached value. This isn't hard, but it does raise a question. Do we update the cached value and then write to the database, or do we write to the database and then update the cache? I decided to update the cache and then write the value. This does raise a remote possibility that we could have a value in cache that isn't in the database. Doing it the other way (write first, update cache second) would ensure that the database was always correct. 

This also massively increased the performance. Every write we used to make actually created two round trips to the database -- one to set the attribute, and the next to get the value we just set. 

### Loading
The initial database load requires a suprisingly large number of queries. This is an unexpected and really unfortunate side effect of our original decision to cache nothing. As quickly as I can describe it, we would first query the database for (say) all the recipes, then we would query the database for each recipe's name (needed for the trees), then query the database for the brewdate, then query the database for the style. Each time we had to display these again, it would be the same pattern. 

This resulted in thousands of queries to the database on my data. As soon as I had the caches in place, it quickly became apparent I could fix that issue. 

I modified the initial query in `Database::populateElements` to return all of the fields from the database. I created a new initializer for the BeerXMLElements that expected a `QSqlRecord` and it just copies the values from the `QSqlRecord` directly into the newly created object. This replaces four (or more) roundtrips to the database with one, and preloads the cache for us.

### Creating
This was the fourth major problem I was trying to tackle, and lead to all the rest of what I've done.

When we are creating a new element, like a Hop, the basic pattern was to:
  1. Create the element in the database;
  2. The user fills out the dialog and presses "Save";
  3. Each attribute is individually written to the database, resulting in 10+ round trips on each dialog

It also resulted in the Cancel button not deleting the element we had just created, which I found really annoying.

A previous attempt had been made at addressing this issue, and it was that attempt that really started this whole change. The previous attempt was only done for one BeerXMLElement and went, in my opinion, too far. Under almost every circumstance, we are actually writing very infrequently to the database. The previous solution basically cached every write, regardless of when it was made, in order to optimize this one experience.

My solution goes the other way. Under almost every circumstance, the write process works as it has -- writes are automatically made to the database as soon as the field is completed. I have made a second path that does something different, and it is up to each component to decide which path to take.

#### m\_cacheOnly
The first step was to define a new attribute on every BeerXMLElement called `m_cacheOnly`. When this attribute is `true`, we only update the cached value. No writes are made to the database. I went through a few different iterations of this, and settled on this approach. It is actually quite clean and reasonable hides all the implementation details.

This means that I also had to write something that could flush the entire BeerXMLElement cache to the database. I played at this bit, and realize the best solution was to have something that could do one massive INSERT into the database at the right place.

#### Database::insertElement
I introduced a new method that uses the TableSchema class to make the big insert string. It then does the work in the database. If the insert is successful, the new key is returned. For all of the prep work I had to do, the code itself is pretty compact and elegant.

#### Database::insert[whatever]
Of course, it wasn't quite that easy. There are a number of signals that need to be emitted, and some house work to be done. Based on how our signals work, I had to create a separate insert method for each class. It calls insertElement, sets the cacheOnly flag to false, emits the necessary signals and returns the key.

The mashsteps and the brewnotes were a little harder, mostly because I had to link them into their parent objects.

#### New constuctors
This required me to define a new constructor for each BeerXMLElement. The new contructor takes the name of the element (or date, in the case of a brewnote), eg `new Recipe(name)`, sets all the fields to their default levels and, very importantly, sets cacheOnly to true. It is the responsibility of the calling method to call the insert method to actually write the element to the database.

## Schemas
The second part of my solution was to create a method that could do one insert and write every column to the database. This was at first looking like a lot of unpleasant code, because I would have to basically write one method for each primitive, teach it about every column in the table, etc. I started looking at what we already had, and that is when I noticed the profusion of arrays, hashs, maps, etc. that we had written to solve this kind of problem. I take a lot of blame for that as I am probably the one who wrote the majority of them.

This required a rethink, and I think I've come up with an elegant solution. Instead of having a bunch of maps, lists and vectors running around, I decided to create proper objects that we could initialize and have methods on those objects to generate the metadata we needed. This introduced three new classes: PropertySchema, TableSchema and DatabaseSchema. 

### PropertySchema
This class defines a specific property, for example, `brewer`. It defines the property name (aka, the name of the property on the Recipe object), the database column name (see later explanations, because this got complex), the BeerXML property name, the type (eg, `string`), the default value and the size. The intent of this class is to define the mapping between a single property, its database column and the BeerXML property. 

This class has grown complex. To support multiple databases with different column names, or different type names, etc. I had to rethink and rework this approach. Now what happens is that I store all of the information for each database in the PropertySchema object. You can request the property definition for a specific database by using the `Brewtarget::DBType` enum type when calling the various methods on PropertySchema. The default is to the use the what ever `Brewtarget::dbType()` returns.

Based on what I had to code later, I defined two initializers. One is for normal properties like `name` or `brewDate`. The other is intended for foreign keys, like `recipe_id` and `equipment_id`. I consider them both to be properties, but they have different uses and need different information.

There are the standard setter and getter methods and not much else. I expect most of the properties to be set when the object is created.

### TableSchema
This class collects all the properties for a single table into one place, and provides some useful methods for querying information. It differentiates between properties and foreign keys. Generally speaking, we do not set the foreign keys when inserting the record into the database -- those relationships are typically handled in the higher level code.

Each table in the database is represented by a TableSchema object. 

A TableSchema object contains two QMap objects: `m_properties` and `m_foreignKeys`. The QMap objects map from the object property to the appropriate PropertySchema object for that property. I am overloading a little on the properties and foreign keys and this may change.

The hard work in TableSchema is actually defining the table. It is up to the developer (me so far, and you if you want to modify tables) to create that QMap entry. This has resulted in a TableSchema.cpp already being quite long, and the associated header file isn't much better.

#### StyleSchema, HopSchema, etc.
In order to keep TableSchema from getting worse, I decided I would break the constant definitions (eg, `const static QString kpropName("name")`) into separate files, one for each BeerXMLElement. Each of the header files needs to be included where those constants are needed.

I decided on the following prefixes:
  * `kprop` indicates an object property name;
  * `kcol` indicates a database column name;
  * `kxmlProp` indicates an XML property name;

This caused a few issues that I had to resolve.

Almost every object has a property called `notes`. I could have done some fun `#ifdef` work, but decided instead that I would create TableSchemaConst.h and any constant that was used by more than one object would be defined in there. I applied the same rule to shared columns and XML properties.

Every object has a property called `id`, but the name of the column changes in the database. I had to modify the second convention to include the table name, eg, kcolMashType. This has left me with some things that break the conventions like `kcolName`. There is no easy solution, and so far this one has worked.

I tried to take some short cuts with things like foreign keys. Technically, there is no property on an individual Recipe object that stores the `equipment_id`. I had originally tried to just use the kcol for all things, like `kcolRecipeId` and `kcolEquipmentId`. Over time, I find it increasingly annoying to remember when I could use something like `kpropMashType` or and I had to use `kcolRecipeId`. So I got rid of the confusion. Everything has a kcol and a kprop constant, even if they happen to be the same value. Please follow this convention, should you be adding a new table.

### DatabaseSchema
This class combines all the TableSchema into one place. I am expecting there to be one defined when the database itself is initialized. The main intent of this class is to allow me to remove all the bloody hashes, maps, arrays, QLists, QStringLists, etc. that we have developed over time. Instead of maintaining those, we will be able to simply ask the DatabaseSchema object for them.

Additionally, this class will allow me to reduce DatabaseSchemaHelper down to handling upgrades.

## Removing the hashes, maps and TagToProps
One side effect of all this work is I was able to remove all of the constant declarations (well, most of them, anyway) from the beginning of each BeerXMLElement. All of that information is now available through the proper TableSchema object. I removed the tabToPropHash() methods for the same reasons. 

Instead, each object simply includes two schema files: `TableSchemaConst.h` and it's specific file, eg, `HopSchema.h`. All of the necessary constants are defined in one place.

## Simplifying `set()`
I changed the naming conventions for all of the constants. This meant I had to modify every call to use the new names for both the properties and the columns to be set. This got me to wondering why each object had to know the name of the database column being set. I realized that the Database object already knew everything and all the object had to do was say "Update this property". The Database object could get everything it needed to know from the TableSchema.

So `set(const QString &prop_name, const QString &col_name, const QVariant &value, bool notify)` became `setEasy(QString prop_name, QVariant value, bool notify)`. I will probably rename it to `set()` as soon as I am confident everything everywhere  is using the new signature. It has does a nice job of simplyfying the code, and it opens a new possibility.

### Different column names for different databases
This section is still under development. The current intent is that you would define the property for each database, and then everything just works. This is untested, and may change.

## `Database::fromXML`
Removing the tagToProps calls resulted in a problem when importing the element from an XML file. The original fromXML method expects that hash to be available. Since I just deleted it, it wasn't. It took some slight reworking, but all of the fromXml methods had to be changed to fit the new way.

This led, as it always does, to me reading the code and thinking "We did what?!" The import from XML code was a mess. In particular, there are a number of enumerated types (like Hop::USE) that are a little suspect in XML. In the original code, if we couldn't translate the received USE into one of our enumerated USES, we would find the most similar hop and just use that. I found that really bad. So I fixed all that code. Now, we import it anyway but warn the user to check what we imported.

## Inventory
I have also modified how the inventory system works. The previous design was (imho) overly complex. In essence, we have a many-to-one relationship between ingredients and inventory. The initial implementation required 3 distinct queries to get the inventory amount for any ingredient, and it was called A LOT. In order to reduce the number of calls, I reworked this. Now, each ingredient which can have inventory has an `inventory_id` field that points to the correct row in the appropriate inventory table. The migration code is awful and I would recommend not looking at it too closely.

This helped reduce the number of round trips but I still had to query the database for the inventory amounts on every display.

### Caching and signals
Caching the inventory amount presented an unusual challenge. Editing the inventory amount only happens on the parent item, but each child has to display the new amount. To get this done, I added a new signal to Database -- `changeInventory`. When the inventory is set, this signal is raised using the `Database::DBTable` type, the inventory id of the row modified and the new amount.

Every table model catches the signal, makes sure the DBTable type is of interest, and then determines if that inventory id is being used by anything in its list. If it is, `cacheOnly` is set true, the cached amount in inventory is updated, `cacheOnly` is set back to false and the model generates the necessary signals to get the tables redrawn. It is, perhaps, a little complex but works.

I am considered redoing the signals a little. I can probably reduce the signalling noise if I use a different signal per ingredient -- eg, `hopInventoryChanged`. That would be a lot of work, and I haven't quite talked myself into it yet.

# How TO
This is more interesting to more people. This is my basic idea on how to add either a new column to a table or a new table to the database.

## New Row
To add a new row will consist of these steps. For the sake of examples, I will assume we are adding a new column to the Hop table called `terpines`.

### Define the constants
In HopSchema.h, you would add three consants:
```c++
const static QString kcolHopTerpines("terpines");
const static QString kpropTerpines("terpines");
const static QString kxmlPropTerpine("terpines");
```
It would be nicest if you would add them in the proper section of the file -- keep the props together, the columns together, etc.

### Add the row to the TableSchema
In TableSchema.cpp, modify the `defineHopTable()` method with these lines:
```c++
tmp[kpropTerpines] = new PropertySchema( kpropTerpines, kcolHopTerpines, kxmlPropTerpine, QString("real"), QVariant(0.0));
```

### Update the existing tables
You will still be responsible for handling the migration code in `DatabaseSchemaHelper`, as well as updating the Hop object itself to do the new work. This is work you would have had to do anyway, but all the rest (writing to the table, creating the table from scratch, etc) is now done. Instead of editing in 7 (or 8) places just to add the new column, you have to edit three.

### A note on `Q_PROPERTY`
For all of the abstraction to work, it is really important that the `Q_PROPERTY` for any new parameter is defined properly. There are several places in the new system where I use Qt's metaobject system to access values. If the new property is not defined via `Q_PROPERTY`, the right thing will not happen.

## New Tables
This would be harder, but the general idea would be along these lines.

### Define new schema header file
You would need to define all the kprop and kcol constants. A new table is unlikely to be defined in BeerXML, but you should probably make sure your table can be exported to XML if needed (like `brewnotes` does, but `hop_in_inventory` doesn't).

### Include the new header file in TableSchema
You will want to add the new header file to TableSchema.cpp.

### Create the new define method
You will need to create the new define method like defineHopTable. Make sure you set the key, the properties and the foreign keys separately. Include any of the other object properties (like `m_childTable`) as needed. If they are not used (like equipment has no in recipe table), simply do nothing. They will default to the proper values.

It is really important that your new class properly invoke the `Q_PROPERTY` macros correctly. There are some loops that expect to use the Qt Meta system to invoke getters and setters. If you do not set up the `Q_PROPERTY` correctly, bad things will happen.

### Invoke the new define method
In the DatbaseSchema class, make sure the new table is properly initialized in `loadTables`.

### Do the rest of the work.
You will need to create the necessary work as a migration, but most of the tasks can be handled easily enough -- using `generateCreateTable()` and `generateInsertRow()` will give you the strings required to create the new table in the database and to insert the new information.

