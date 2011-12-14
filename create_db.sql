create table equipment(
   eid integer PRIMARY KEY autoincrement,
   name varchar(256) not null DEFAULT '',
   boil_size real DEFAULT 0.0,
   batch_size real DEFAULT 0.0,
   tun_volume real DEFAULT 0.0,
   tun_weight real DEFAULT 0.0,
   tun_specific_heat real DEFAULT 0.0,
   top_up_water real DEFAULT 0.0,
   trub_chiller_loss real DEFAULT 0.0,
   evap_rate real DEFAULT 0.0,
   real_evap_rate real DEFAULT 0.0,
   boil_time real DEFAULT 0.0,
   calc_boil_volume boolean DEFAULT FALSE,
   lauter_deadspace real DEFAULT 0.0,
   top_up_kettle real DEFAULT 0.0,
   hop_utilization real DEFAULT 0.0,
   boiling_point real DEFAULT 0.0,
   absorption real DEFAULT 0.0,
   notes text DEFAULT '',
   -- it's metadata all the way down
   deleted boolean DEFAULT FALSE,
   display boolean DEFAULT TRUE
);

create table fermentable(
   fid integer PRIMARY KEY autoincrement,
   name varchar(256) not null DEFAULT '',
   ftype varchar(32) DEFAULT 'Grain',
   amount real DEFAULT 0.0,
   yield real DEFAULT 0.0,
   color real DEFAULT 0.0,
   add_after_boil boolean DEFAULT FALSE,
   origin varchar(32) DEFAULT '',
   supplier varchar(256) DEFAULT '',
   notes text DEFAULT '',
   coarse_fine_diff real DEFAULT 0.0,
   moisture real DEFAULT 0.0,
   diastatic_power real DEFAULT 0.0,
   protein real DEFAULT 0.0,
   max_in_batch real DEFAULT 100.0,
   recommend_mash boolean DEFAULT FALSE,
   is_mashed boolean DEFAULT FALSE,
   ibu_gal_per_lb real DEFAULT 0.0,
   -- meta data
   deleted boolean DEFAULT FALSE,
   display boolean DEFAULT TRUE
);

create table hop(
   hid integer PRIMARY KEY autoincrement,
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
   -- meta data
   deleted boolean DEFAULT FALSE,
   display boolean DEFAULT TRUE
);

create table misc(
   mid integer PRIMARY KEY autoincrement,
   name varchar(256) not null DEFAULT '',
   mtype varchar(32) DEFAULT 'Other',
   use varchar(32) DEFAULT 'Boil',
   time real DEFAULT 0.0,
   amount real DEFAULT 0.0,
   amount_is_weight boolean DEFAULT TRUE,
   use_for text DEFAULT '',
   notes text DEFAULT '',
   -- meta data
   deleted boolean DEFAULT FALSE,
   display boolean DEFAULT TRUE
);

create table style(
   sid integer PRIMARY KEY autoincrement,
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
   deleted boolean DEFAULT FALSE,
   display boolean  DEFAULT TRUE
);

create table yeast(
   yid integer PRIMARY KEY autoincrement,
   name varchar(256) not null DEFAULT '',
   ytype varchar(32) DEFAULT 'Ale',
   form varchar(32) DEFAULT 'Liquid',
   amount real DEFAULT 0.0,
   amount_is_weight boolean DEFAULT FALSE,
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
   add_to_secondary boolean DEFAULT FALSE,
   inventory real DEFAULT 0,
   -- meta data
   deleted boolean DEFAULT FALSE,
   display boolean DEFAULT TRUE
);

-- unlike some of the other tables, you can have a mash with no name.
-- which is gonna mess my nice primary key ideas up
create table mash(
   maid integer PRIMARY KEY autoincrement,
   name varchar(256) DEFAULT '',
   grain_temp real DEFAULT 20.0,
   notes text DEFAULT '',
   tun_temp real DEFAULT 20.0,
   sparge_temp real DEFAULT 74.0,
   ph real DEFAULT 7.0,
   tun_weight real DEFAULT 0.0,
   tun_specific_heat real DEFAULT 0.0,
   equip_adjust boolean DEFAULT TRUE,
   deleted boolean DEFAULT FALSE,
   display boolean DEFAULT TRUE
);

create table mashstep(
   msid integer PRIMARY KEY autoincrement,
   name varchar(256) not null DEFAULT '',
   mstype varchar(32) DEFAULT 'Infusion',
   infuse_amount real DEFAULT 0.0,
   step_temp real DEFAULT 67.0,
   step_time real DEFAULT 0.0,
   ramp_time real DEFAULT 0.0,
   end_temp real DEFAULT 67.0,
   infuse_temp real DEFAULT 67.0,
   decoction_amount real DEFAULT 0.0,

   -- Meta data
   deleted boolean DEFAULT FALSE,
   display boolean DEFAULT TRUE,

   -- Our step number is unique within our parent mash.
   mash_id integer,
   step_number integer,
   foreign key(mash_id) references mash(maid),
   unique( mash_id, step_number )
);

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
   deleted boolean DEFAULT FALSE,
   display boolean DEFAULT TRUE,
   recipe_id integer,
   foreign key(recipe_id) references recipe(rid)
);

create table water(
   wid integer PRIMARY KEY autoincrement,
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
   deleted boolean DEFAULT FALSE,
   display boolean DEFAULT TRUE
);

-- instructions are many-to-one for recipes. 
create table instruction(
   iid integer PRIMARY KEY autoincrement,
   name varchar(256) not null DEFAULT '',
   directions text DEFAULT '',
   has_timer boolean DEFAULT FALSE,
   timer_value varchar(16) DEFAULT '00:00:00',
   completed boolean DEFAULT FALSE,
   interval real DEFAULT 0.0,

   deleted boolean DEFAULT FALSE,
   display boolean DEFAULT TRUE,

   recipe_id integer,
   -- The order of this instruction in the recipe.
   instruction_number integer autoincrement,
   foreign key(recipe_id) references recipe(rid),
   unique(recipe_id,instruction_number)
);

-- The relationship of styles to recipe is one to many, as is the mash and
-- equipment. It just makes most sense for the recipe to carry that around
-- instead of using another table
create table recipe(
   rid integer PRIMARY KEY autoincrement,
   name varchar(256) not null DEFAULT '',
   rtype varchar(32) DEFAULT 'All Grain',
   brewer varchar(1024) DEFAULT '',
   assistant_brewer varchar(1024) DEFAULT 'Brewtarget: free beer software',
   batch_size real DEFAULT 0.0,
   boil_size real DEFAULT 0.0,
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
   brewdate date  DEFAULT CURRENT_DATE,
   carb_volume real DEFAULT 0.0,
   forced_carb boolean DEFAULT FALSE,
   priming_sugar_name varchar(128) DEFAULT '',
   carb_temp real DEFAULT 20.0,
   priming_sugar_equiv real DEFAULT 1.0,
   keg_priming_factor real DEFAULT 1.0,
   taste_notes text DEFAULT '',
   taste_rating real DEFAULT 0.0,
   deleted boolean DEFAULT FALSE,
   display boolean DEFAULT TRUE,
   style_id integer,
   mash_id integer,
   equipment_id integer,
   foreign key(style_id) references style(sid),
   foreign key(mash_id) references mash(maid),
   foreign key(equipment_id) references equipment(eid)
);

--create table mash_to_mashstep(
--   mmid integer PRIMARY KEY autoincrement,
--   mash_id integer,
--   mashstep_id integer,
--   foreign key(mash_id) references mash(maid),
--   foreign key(mashstep_id) references mashstep(msid)
--);

create table fermentable_in_recipe(
   hrid integer primary key autoincrement,
   fermentable_id integer,
   recipe_id integer,
   foreign key(fermentable_id) references fermentable(fid),
   foreign key(recipe_id) references recipe(rid)
);

create table hop_in_recipe(
   hrid integer PRIMARY KEY autoincrement,
   hop_id integer,
   recipe_id integer,
   foreign key(hop_id) references hop(hid),
   foreign key(recipe_id) references recipe(rid)
);

create table misc_in_recipe(
   mrid integer PRIMARY KEY autoincrement,
   misc_id integer,
   recipe_id integer,
   foreign key(misc_id) references misc(mid),
   foreign key(recipe_id) references recipe(rid)
);

create table water_in_recipe(
   wrid integer PRIMARY KEY autoincrement,
   water_id integer,
   recipe_id integer,
   foreign key(water_id) references water(wid),
   foreign key(recipe_id) references recipe(rid)
);

create table yeast_in_recipe(
   yrid integer PRIMARY KEY autoincrement,
   yeast_id integer,
   recipe_id integer,
   foreign key(yeast_id) references yeast(yid),
   foreign key(recipe_id) references recipe(rid)
);

create table equipment_children(
   ecid integer PRIMARY KEY autoincrement,
   parent_id integer,
   child_id integer,
   foreign key(parent_id) references equipment(eid),
   foreign key(child_id)  references equipment(eid)
);

create table fermentable_children(
   fcid integer PRIMARY KEY autoincrement,
   parent_id integer,
   child_id integer,
   foreign key(parent_id) references fermentable(fid),
   foreign key(child_id)  references fermentable(fid)
);

create table hop_children(
   hcid integer PRIMARY KEY autoincrement,
   parent_id integer,
   child_id integer,
   foreign key(parent_id) references hop(hid),
   foreign key(child_id)  references hop(hid)
);

create table misc_children(
   mcid integer PRIMARY KEY autoincrement,
   parent_id integer,
   child_id integer,
   foreign key(parent_id) references misc(mid),
   foreign key(child_id)  references misc(mid)
);

create table recipe_children(
   rcid integer PRIMARY KEY autoincrement,
   parent_id integer,
   child_id integer,
   foreign key(parent_id) references recipe(rid),
   foreign key(child_id)  references recipe(rid)
);

create table style_children(
   scid integer PRIMARY KEY autoincrement,
   parent_id integer,
   child_id integer,
   foreign key(parent_id) references style(sid),
   foreign key(child_id)  references style(sid)
);

create table water_children(
   wcid integer PRIMARY KEY autoincrement,
   parent_id integer,
   child_id integer,
   foreign key(parent_id) references water(wid),
   foreign key(child_id)  references water(wid)
);

create table yeast_children(
   ycid integer PRIMARY KEY autoincrement,
   parent_id integer,
   child_id integer,
   foreign key(parent_id) references yeast(yid),
   foreign key(child_id)  references yeast(yid)
);

