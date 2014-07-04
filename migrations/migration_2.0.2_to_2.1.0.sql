UPDATE settings set version="2.1.0";
ALTER TABLE equipment ADD COLUMN folder varchar(256) DEFAULT '';
ALTER TABLE fermentable ADD COLUMN folder varchar(256) DEFAULT '';
ALTER TABLE hop ADD COLUMN folder varchar(256) DEFAULT '';
ALTER TABLE misc ADD COLUMN folder varchar(256) DEFAULT '';
ALTER TABLE style ADD COLUMN folder varchar(256) DEFAULT '';
ALTER TABLE yeast ADD COLUMN folder varchar(256) DEFAULT '';
ALTER TABLE water ADD COLUMN folder varchar(256) DEFAULT '';
ALTER TABLE mash ADD COLUMN folder varchar(256) DEFAULT '';
ALTER TABLE brewnote ADD COLUMN folder varchar(256) DEFAULT '';
ALTER TABLE recipe ADD COLUMN folder varchar(256) DEFAULT '';
-- put the default recipes into their own folder, so people can see it
UPDATE recipe SET folder='/brewtarget' WHERE name LIKE 'Bt:%';
-- inventory support stuff
-- used to trigger the code to populate the ingredient inheritance tables 
ALTER TABLE settings ADD COLUMN repopulateChildrenOnNextStart int DEFAULT 0;
UPDATE settings SET repopulateChildrenOnNextStart = 1;
-- Ingredient inheritance tables
-- In order to reliably assertain the parent, children should only be linked to a single parent.
-- Unfortunately I was not able to find an easy way of adding the unique constraint to an already created column.
-- Since the these tables don't appear to be in use in previous versions there should be no problem recreating them.
drop table equipment_children;
drop table fermentable_children;
drop table hop_children;
drop table misc_children;
drop table recipe_children;
drop table style_children;
drop table water_children;
drop table yeast_children;
create table equipment_children( id integer PRIMARY KEY autoincrement, parent_id integer, child_id integer UNIQUE, foreign key(parent_id) references equipment(id), foreign key(child_id)  references equipment(id));
create table fermentable_children( id integer PRIMARY KEY autoincrement, parent_id integer, child_id integer UNIQUE, foreign key(parent_id) references fermentable(id), foreign key(child_id)  references fermentable(id));
create table hop_children( id integer PRIMARY KEY autoincrement, parent_id integer, child_id integer UNIQUE, foreign key(parent_id) references hop(id), foreign key(child_id)  references hop(id));
create table misc_children( id integer PRIMARY KEY autoincrement, parent_id integer, child_id integer UNIQUE, foreign key(parent_id) references misc(id), foreign key(child_id)  references misc(id));
create table recipe_children( id integer PRIMARY KEY autoincrement, parent_id integer, child_id integer UNIQUE, foreign key(parent_id) references recipe(id), foreign key(child_id)  references recipe(id));
create table style_children( id integer PRIMARY KEY autoincrement, parent_id integer, child_id integer UNIQUE, foreign key(parent_id) references style(id), foreign key(child_id)  references style(id));
create table water_children( id integer PRIMARY KEY autoincrement, parent_id integer, child_id integer UNIQUE, foreign key(parent_id) references water(id), foreign key(child_id)  references water(id));
create table yeast_children( id integer PRIMARY KEY autoincrement, parent_id integer, child_id integer UNIQUE, foreign key(parent_id) references yeast(id), foreign key(child_id)  references yeast(id));
-- Inventory tables
drop table fermentable_in_inventory;
drop table hop_in_inventory;
drop table misc_in_inventory;
drop table yeast_in_inventory;
create table fermentable_in_inventory( id integer PRIMARY KEY autoincrement, fermentable_id integer UNIQUE, amount real DEFAULT 0.0, foreign key(fermentable_id) references fermentable(id));
create table hop_in_inventory(id integer PRIMARY KEY autoincrement, hop_id integer UNIQUE, amount real DEFAULT 0.0, foreign key(hop_id) references hop(id));
create table misc_in_inventory(id integer PRIMARY KEY autoincrement, misc_id integer UNIQUE, amount real DEFAULT 0.0, foreign key(misc_id) references misc(id));
create table yeast_in_inventory(id integer PRIMARY KEY autoincrement, yeast_id integer UNIQUE, quanta integer DEFAULT 0, foreign key(yeast_id) references yeast(id));

