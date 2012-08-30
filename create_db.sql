-- NOTE: none of the BeerXML property names should EVER change. This is to
--       ensure backwards compatability when rolling out ingredient updates to
--       old versions.

-- NOTE: deleted=1 means the ingredient is "deleted" and should not be shown in
--                 any list.
--       deleted=0 means it isn't deleted and may or may not be shown.
--       display=1 means the ingredient should be shown in a list, available to
--                 be put into a recipe.
--       display=0 means the ingredient is in a recipe already and should not
--                 be shown in a list, available to be put into a recipe.

create table equipment(
   id integer PRIMARY KEY autoincrement,
   -- BeerXML properties
   name varchar(256) not null DEFAULT '',
   boil_size real DEFAULT 0.0,
   batch_size real DEFAULT 0.0,
   tun_volume real DEFAULT 0.0,
   tun_weight real DEFAULT 0.0,
   tun_specific_heat real DEFAULT 0.0,
   top_up_water real DEFAULT 0.0,
   trub_chiller_loss real DEFAULT 0.0,
   evap_rate real DEFAULT 0.0,
   boil_time real DEFAULT 0.0,
   calc_boil_volume boolean DEFAULT 0,
   lauter_deadspace real DEFAULT 0.0,
   top_up_kettle real DEFAULT 0.0,
   hop_utilization real DEFAULT 0.0,
   notes text DEFAULT '',
   -- Out BeerXML extensions
   real_evap_rate real DEFAULT 0.0,
   boiling_point real DEFAULT 100.0,
   absorption real DEFAULT 1.085,
   -- Metadata
   deleted boolean DEFAULT 0,
   display boolean DEFAULT 1
);

create table fermentable(
   id integer PRIMARY KEY autoincrement,
   -- BeerXML properties
   name varchar(256) not null DEFAULT '',
   ftype varchar(32) DEFAULT 'Grain',
   amount real DEFAULT 0.0,
   yield real DEFAULT 0.0,
   color real DEFAULT 0.0,
   add_after_boil boolean DEFAULT 0,
   origin varchar(32) DEFAULT '',
   supplier varchar(256) DEFAULT '',
   notes text DEFAULT '',
   coarse_fine_diff real DEFAULT 0.0,
   moisture real DEFAULT 0.0,
   diastatic_power real DEFAULT 0.0,
   protein real DEFAULT 0.0,
   max_in_batch real DEFAULT 100.0,
   recommend_mash boolean DEFAULT 0,
   is_mashed boolean DEFAULT 0,
   ibu_gal_per_lb real DEFAULT 0.0,
   -- Display stuff
   display_unit integer DEFAULT -1,
   display_scale integer DEFAULT -1,
   -- meta data
   deleted boolean DEFAULT 0,
   display boolean DEFAULT 1
);

create table hop(
   id integer PRIMARY KEY autoincrement,
   -- BeerXML properties
   name varchar(256) not null DEFAULT '',
   alpha real DEFAULT 0.0,
   amount real DEFAULT 0.0,
   use varchar(32) DEFAULT 'Boil',
   time real DEFAULT 0.0,
   notes text DEFAULT '',
   htype varchar(32) DEFAULT 'Both',
   form  varchar(32) DEFAULT 'Pellet',
   beta real DEFAULT 0.0,
   hsi real DEFAULT 0.0,
   origin varchar(32),
   substitutes text DEFAULT '',
   humulene real DEFAULT 0.0,
   caryophyllene real DEFAULT 0.0,
   cohumulone real DEFAULT 0.0,
   myrcene real DEFAULT 0.0,
   -- Display stuff
   display_unit integer DEFAULT -1,
   display_scale integer DEFAULT -1,
   -- meta data
   deleted boolean DEFAULT 0,
   display boolean DEFAULT 1
);

create table misc(
   id integer PRIMARY KEY autoincrement,
   -- BeerXML properties
   name varchar(256) not null DEFAULT '',
   mtype varchar(32) DEFAULT 'Other',
   use varchar(32) DEFAULT 'Boil',
   time real DEFAULT 0.0,
   amount real DEFAULT 0.0,
   amount_is_weight boolean DEFAULT 1,
   use_for text DEFAULT '',
   notes text DEFAULT '',
   -- Display stuff.
   -- Be careful: this will change meaning based on amount_is_weight
   display_unit integer DEFAULT -1,
   display_scale integer DEFAULT -1,
   -- meta data
   deleted boolean DEFAULT 0,
   display boolean DEFAULT 1
);

create table style(
   id integer PRIMARY KEY autoincrement,
   -- BeerXML properties
   name varchar(256) not null DEFAULT '',
   s_type varchar(64) DEFAULT 'Ale',
   category varchar(256) DEFAULT '',
   category_number varchar(16) DEFAULT '',
   style_letter varchar(1) DEFAULT '',
   style_guide varchar(1024) DEFAULT '',
   og_min real DEFAULT 1.0,
   og_max real DEFAULT 1.100,
   fg_min real DEFAULT 1.0,
   fg_max real DEFAULT 1.100,
   ibu_min real DEFAULT 0.0,
   ibu_max real DEFAULT 100.0,
   color_min real DEFAULT 0.0,
   color_max real DEFAULT 100.0,
   abv_min real DEFAULT 0.0,
   abv_max real DEFAULT 100.0,
   carb_min real DEFAULT 0.0,
   carb_max real DEFAULT 100.0,
   notes text DEFAULT '',
   profile text DEFAULT '',
   ingredients text DEFAULT '',
   examples text DEFAULT '',
   -- meta data
   deleted boolean DEFAULT 0,
   display boolean  DEFAULT 1
);

create table yeast(
   id integer PRIMARY KEY autoincrement,
   -- BeerXML properties
   name varchar(256) not null DEFAULT '',
   ytype varchar(32) DEFAULT 'Ale',
   form varchar(32) DEFAULT 'Liquid',
   amount real DEFAULT 0.0,
   amount_is_weight boolean DEFAULT 0,
   laboratory varchar(32) DEFAULT '',
   product_id varchar(32) DEFAULT '',
   min_temperature real DEFAULT 0.0,
   max_temperature real DEFAULT 32.0,
   flocculation varchar(32) DEFAULT 'Medium',
   attenuation real DEFAULT 75.0,
   notes text DEFAULT '',
   best_for varchar(256) DEFAULT '',
   times_cultured integer DEFAULT 0,
   max_reuse integer DEFAULT 10,
   add_to_secondary boolean DEFAULT 0,
   -- Display stuff
   -- Be careful: this will change meaning based on amount_is_weight
   display_unit integer DEFAULT -1,
   display_scale integer DEFAULT -1,
   -- meta data
   deleted boolean DEFAULT 0,
   display boolean DEFAULT 1
);

create table water(
   id integer PRIMARY KEY autoincrement,
   -- BeerXML properties
   name varchar(256) not null DEFAULT '',
   amount real DEFAULT 0.0,
   calcium real DEFAULT 0.0,
   bicarbonate real DEFAULT 0.0,
   sulfate real DEFAULT 0.0,
   chloride real DEFAULT 0.0,
   sodium real DEFAULT 0.0,
   magnesium real DEFAULT 0.0,
   ph real DEFAULT 7.0,
   notes text DEFAULT '',
   -- metadata
   deleted boolean DEFAULT 0,
   display boolean DEFAULT 1
);

-- The following bt_* tables simply point to ingredients provided by brewtarget.
-- This is to make updating and pushing new ingredients easy.
-- NOTE: they MUST be named bt_<table>, where <table> is the table name that
-- they refer to, and they MUST contain fields 'id' and '<table>_id'.

create table bt_equipment(
   id integer PRIMARY KEY autoincrement,
   equipment_id integer,
   foreign key(equipment_id) references equipment(id)
);

create table bt_fermentable(
   id integer PRIMARY KEY autoincrement,
   fermentable_id integer,
   foreign key(fermentable_id) references fermentable(id)
);

create table bt_hop(
   id integer PRIMARY KEY autoincrement,
   hop_id integer,
   foreign key(hop_id) references hop(id)
);

create table bt_misc(
   id integer PRIMARY KEY autoincrement,
   misc_id integer,
   foreign key(misc_id) references misc(id)
);

create table bt_style(
   id integer PRIMARY KEY autoincrement,
   style_id integer,
   foreign key(style_id) references style(id)
);

create table bt_yeast(
   id integer PRIMARY KEY autoincrement,
   yeast_id integer,
   foreign key(yeast_id) references yeast(id)
);

create table bt_water(
   id integer PRIMARY KEY autoincrement,
   water_id integer,
   foreign key(water_id) references water(id)
);

create table mash(
   id integer PRIMARY KEY autoincrement,
   -- BeerXML properties
   name varchar(256) DEFAULT '',
   grain_temp real DEFAULT 20.0,
   notes text DEFAULT '',
   tun_temp real DEFAULT 20.0,
   sparge_temp real DEFAULT 74.0,
   ph real DEFAULT 7.0,
   tun_weight real DEFAULT 0.0,
   tun_specific_heat real DEFAULT 0.0,
   equip_adjust boolean DEFAULT 1,
   -- Metadata
   deleted boolean DEFAULT 0,
   display boolean DEFAULT 1
);

create table mashstep(
   id integer PRIMARY KEY autoincrement,
   -- BeerXML properties
   name varchar(256) not null DEFAULT '',
   mstype varchar(32) DEFAULT 'Infusion',
   infuse_amount real DEFAULT 0.0,
   step_temp real DEFAULT 67.0,
   step_time real DEFAULT 0.0,
   ramp_time real DEFAULT 0.0,
   end_temp real DEFAULT 67.0,
   infuse_temp real DEFAULT 67.0,
   decoction_amount real DEFAULT 0.0,
   -- Display stuff
   -- we have three display fields in this table. I don't like my solution,
   -- but really don't want to deal with another table and lookup
   display_unit integer DEFAULT -1,
   display_scale integer DEFAULT -1,
   display_temp_unit integer DEFAULT -1,
   -- Meta data
   deleted boolean DEFAULT 0,
   display boolean DEFAULT 1,
   -- Our step number is unique within our parent mash.
   mash_id integer,
   step_number integer DEFAULT 0,
   foreign key(mash_id) references mash(id)
   -- This is not necessary since we manage these internally in Brewtarget.
   -- unique( mash_id, step_number )
);

-- Completely new non-BeerXML type.
create table brewnote(
   id integer PRIMARY KEY autoincrement,
   brewDate datetime DEFAULT CURRENT_DATETIME,
   fermentDate datetime DEFAULT CURRENT_DATETIME,
   sg real DEFAULT 1.0,
   volume_into_bk real DEFAULT 0.0,
   strike_temp real DEFAULT 70.0,
   mash_final_temp real DEFAULT 67.0,
   og real DEFAULT 1.0,
   post_boil_volume real DEFAULT 0.0,
   volume_into_fermenter real DEFAULT 0.0,
   pitch_temp real DEFAULT 20.0,
   fg real DEFAULT 1.0,
   eff_into_bk real DEFAULT 70.0,
   abv real DEFAULT 0.0,
   predicted_og real DEFAULT 1.0,
   brewhouse_eff real DEFAULT 70.0,
   predicted_abv real DEFAULT 0.0,
   projected_boil_grav real DEFAULT 1.0,
   projected_strike_temp real DEFAULT 70.0,
   projected_fin_temp real DEFAULT 67.0,
   projected_mash_fin_temp real DEFAULT 67.0,
   projected_vol_into_bk real DEFAULT 0.0,
   projected_og real DEFAULT 1.0,
   projected_vol_into_ferm real DEFAULT 0.0,
   projected_fg real DEFAULT 1.0,
   projected_eff real DEFAULT 70.0,
   projected_abv real DEFAULT 0.0,
   projected_atten real DEFAULT 75.0,
   projected_points real DEFAULT 0.0,
   boil_off real DEFAULT 0.0,
   final_volume real DEFAULT 0.0,
   notes text DEFAULT '',
   deleted boolean DEFAULT 0,
   display boolean DEFAULT 1,
   recipe_id integer,
   foreign key(recipe_id) references recipe(id)
);

-- Completely new non-BeerXML type.
create table instruction(
   id integer PRIMARY KEY autoincrement,
   name varchar(256) not null DEFAULT '',
   directions text DEFAULT '',
   hasTimer boolean DEFAULT 0,
   timerValue varchar(16) DEFAULT '00:00:00',
   completed boolean DEFAULT 0,
   interval real DEFAULT 0.0,
   deleted boolean DEFAULT 0,
   display boolean DEFAULT 1,
   recipe_id integer,
   -- The order of this instruction in the recipe.
   instruction_number integer default 0,
   foreign key(recipe_id) references recipe(id),
   unique(recipe_id,instruction_number)
);

-- When inserting a new instruction record, makes sure the instruction number
-- is largest.
CREATE TRIGGER update_ins_num AFTER INSERT ON instruction
BEGIN
   UPDATE instruction SET instruction_number = 
      (SELECT max(instruction_number) FROM instruction) + 1 
      WHERE rowid = new.rowid;
END;

create table recipe(
   id integer PRIMARY KEY autoincrement,
   -- BeerXML properties
   name varchar(256) not null DEFAULT '',
   type varchar(32) DEFAULT 'All Grain',
   brewer varchar(1024) DEFAULT '',
   assistant_brewer varchar(1024) DEFAULT 'Brewtarget: free beer software',
   batch_size real DEFAULT 0.0,
   boil_size real DEFAULT 0.0,
   boil_time real DEFAULT 0.0,
   efficiency real DEFAULT 70.0,
   og real DEFAULT 1.0,
   fg real DEFAULT 1.0,
   fermentation_stages int DEFAULT 1,
   primary_age real DEFAULT 0.0,
   primary_temp real DEFAULT 20.0,
   secondary_age real DEFAULT 0.0,
   secondary_temp real DEFAULT 20.0,
   tertiary_age real DEFAULT 0.0,
   tertiary_temp real DEFAULT 20.0,
   age real DEFAULT 0.0,
   age_temp real DEFAULT 20.0,
   date date  DEFAULT CURRENT_DATE,
   carb_volume real DEFAULT 0.0,
   forced_carb boolean DEFAULT 0,
   priming_sugar_name varchar(128) DEFAULT '',
   carbonationTemp_c real DEFAULT 20.0,
   priming_sugar_equiv real DEFAULT 1.0,
   keg_priming_factor real DEFAULT 1.0,
   notes text DEFAULT '',
   taste_notes text DEFAULT '',
   taste_rating real DEFAULT 0.0,
   -- Metadata
   deleted boolean DEFAULT 0,
   display boolean DEFAULT 1,
   -- Relational members
   style_id integer,
   mash_id integer,
   equipment_id integer,
   foreign key(style_id) references style(id),
   foreign key(mash_id) references mash(id),
   foreign key(equipment_id) references equipment(id)
);

create table fermentable_in_recipe(
   id integer primary key autoincrement,
   fermentable_id integer,
   recipe_id integer,
   foreign key(fermentable_id) references fermentable(id),
   foreign key(recipe_id) references recipe(id)
);

create table hop_in_recipe(
   id integer PRIMARY KEY autoincrement,
   hop_id integer,
   recipe_id integer,
   foreign key(hop_id) references hop(id),
   foreign key(recipe_id) references recipe(id)
);

create table misc_in_recipe(
   id integer PRIMARY KEY autoincrement,
   misc_id integer,
   recipe_id integer,
   foreign key(misc_id) references misc(id),
   foreign key(recipe_id) references recipe(id)
);

create table water_in_recipe(
   id integer PRIMARY KEY autoincrement,
   water_id integer,
   recipe_id integer,
   foreign key(water_id) references water(id),
   foreign key(recipe_id) references recipe(id)
);

create table yeast_in_recipe(
   id integer PRIMARY KEY autoincrement,
   yeast_id integer,
   recipe_id integer,
   foreign key(yeast_id) references yeast(id),
   foreign key(recipe_id) references recipe(id)
);

-- Ingredient inheritance tables

create table equipment_children(
   id integer PRIMARY KEY autoincrement,
   parent_id integer,
   child_id integer,
   foreign key(parent_id) references equipment(id),
   foreign key(child_id)  references equipment(id)
);

create table fermentable_children(
   id integer PRIMARY KEY autoincrement,
   parent_id integer,
   child_id integer,
   foreign key(parent_id) references fermentable(id),
   foreign key(child_id)  references fermentable(id)
);

create table hop_children(
   id integer PRIMARY KEY autoincrement,
   parent_id integer,
   child_id integer,
   foreign key(parent_id) references hop(id),
   foreign key(child_id)  references hop(id)
);

create table misc_children(
   id integer PRIMARY KEY autoincrement,
   parent_id integer,
   child_id integer,
   foreign key(parent_id) references misc(id),
   foreign key(child_id)  references misc(id)
);

create table recipe_children(
   id integer PRIMARY KEY autoincrement,
   parent_id integer,
   child_id integer,
   foreign key(parent_id) references recipe(id),
   foreign key(child_id)  references recipe(id)
);

create table style_children(
   id integer PRIMARY KEY autoincrement,
   parent_id integer,
   child_id integer,
   foreign key(parent_id) references style(id),
   foreign key(child_id)  references style(id)
);

create table water_children(
   id integer PRIMARY KEY autoincrement,
   parent_id integer,
   child_id integer,
   foreign key(parent_id) references water(id),
   foreign key(child_id)  references water(id)
);

create table yeast_children(
   id integer PRIMARY KEY autoincrement,
   parent_id integer,
   child_id integer,
   foreign key(parent_id) references yeast(id),
   foreign key(child_id)  references yeast(id)
);

-- Inventory tables for the future.

create table fermentable_in_inventory(
   id integer PRIMARY KEY autoincrement,
   fermentable_id integer,
   amount real DEFAULT 0.0,
   foreign key(fermentable_id) references fermentable(id)
);

create table hop_in_inventory(
   id integer PRIMARY KEY autoincrement,
   hop_id integer,
   amount real DEFAULT 0.0,
   foreign key(hop_id) references hop(id)
);

create table misc_in_inventory(
   id integer PRIMARY KEY autoincrement,
   misc_id integer,
   amount real DEFAULT 0.0,
   foreign key(misc_id) references misc(id)
);

-- For yeast, homebrewers don't usually keep stores of yeast. They keep
-- packets or vials or some other type of discrete integer quantity. So, I
-- don't know how useful a real-valued inventory amount would be for yeast.
create table yeast_in_inventory(
   id integer PRIMARY KEY autoincrement,
   yeast_id integer,
   --amount real DEFAULT 0.0,
   quanta integer DEFAULT 0,
   foreign key(yeast_id) references yeast(id)
);
