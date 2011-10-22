create table equipment(
   name varchar(256) not null,
   boil_size real,
   batch_size real,
   tun_volume real,
   tun_weight real,
   tun_specific_heat real,
   top_up_water real,
   trub_chiller_loss real,
   evap_rate real,
   real_evap_rate real,
   boil_time real,
   calc_boil_volume boolean,
   lauter_deadspace real,
   top_up_kettle real,
   hop_utilization real,
   boiling_point real,
   absorption real,
   notes text,
   -- it's metadata all the way down
   deleted boolean,
   display boolean,
   PRIMARY KEY (name,version)
);

create table fermentable(
   name varchar(256) not null,
   ftype varchar(128),
   amount real,
   yield real,
   color real,
   add_after_boil boolean,
   origin varchar(32),
   supplier varchar(256),
   notes text,
   coarse_fine_diff real,
   moisture real,
   diastatic_power real,
   protein real,
   max_in_batch real,
   recommend_mash boolean,
   is_mashed boolean,
   ibu_gal_per_lb real,
   deleted boolean,
   display boolean,
   PRIMARY KEY (name,version)
);

create table hop(
   name varchar(256) not null,
   alpha real,
   amount real,
   use varchar(64),
   time real,
   notes text,
   htype varchar(32),
   form  varchar(32),
   beta real,
   hsi real,
   origin varchar(32),
   substitutes text,
   humulene real,
   caryophyllene real,
   cohumulone real,
   myrcene real,
   deleted boolean,
   display boolean,
   PRIMARY KEY (name,version)
);

create table misc(
   name varchar(256) not null,
   mtype varchar(64),
   use varchar(64),
   time real,
   amount real, 
   amount_is_weight boolean,
   use_for text,
   notes text,
   deleted boolean,
   display boolean,
   PRIMARY KEY (name,version)
);

create table style(
   name varchar(256) not null,
   version integer,
   s_type varchar(64),
   category varchar(256),
   category_number integer,
   style_letter varchar(1),
   style_guide varchar(1024),
   stype varchar(32),
   og_min real,
   og_max real,
   fg_min real,
   fg_max real,
   ibu_min real,
   ibu_max real,
   color_min real,
   color_max real,
   abv_min real,
   abv_max real,
   carb_min real,
   carb_max real,
   notes text,
   profile text,
   ingredients text,
   examples text,
   deleted boolean,
   display boolean,
   PRIMARY KEY (name,version)
);

create table yeast(
   name varchar(256) not null,
   ytype varchar(32),
   form varchar(32),
   amount real,
   amount_is_weight boolean,
   laboratory varchar(32),
   product_id varchar(32),
   min_temperature real,
   max_temperature real,
   flocculation varchar(32),
   attenuation real,
   notes text,
   best_for varchar(256),
   times_cultured integer,
   max_reuse integer,
   add_to_secondary boolean,
   inventory real,
   deleted boolean,
   display boolean,
   PRIMARY KEY (name,version)
);

create table mashstep(
   name varchar(256) not null,
   mstype varchar(32),
   infuse_amount real,
   step_temp real,
   step_time real,
   ramp_time real,
   end_temp real,
   infuse_temp real,
   decoction_amount real,
   deleted boolean,
   display boolean,
   PRIMARY KEY (name,version)
);

-- unlike some of the other tables, you can have a mash with no name.
-- which is gonna mess my nice primary key ideas up
create table mash(
   mid integer PRIMARY KEY autoincrement,
   name varchar(256),
   grain_temp real,
   notes text,
   tun_temp real,
   sparge_temp real,
   ph real,
   tun_weight real,
   tun_specific_heat real,
   equip_adjust boolean,
   deleted boolean,
   display boolean
);

-- since the relationship of recipes to brewnotes is at most one-to-many
-- I am putting the recipes key in here as a foreign key, instead of
-- using another table
create table brewnote(
   brewDate varchar(32) PRIMARY KEY,
   fermentDate varchar(32),
   sg real,
   volume_into_bk real,
   strike_temp real,
   mash_final_temp real,
   og real,
   post_boil_volume real,
   volume_into_fermenter real,
   pitch_temp real,
   fg real,
   eff_into_bk real,
   actual_abv real,
   predicted_og real,
   brewhouse_eff real,
   predicted_abv real,
   projected_boil_grav real,
   projected_strike_temp real,
   projected_fin_temp real,
   projected_mash_fin_temp real,
   projected_vol_into_bk real,
   projected_og real,
   projected_vol_into_ferm real,
   projected_fg real,
   projected_eff real,
   projected_abv real,
   projected_atten real,
   projected_points real,
   boil_off real,
   final_volume real,
   notes text,
   deleted boolean,
   display boolean,
   recipe_version integer,
   recipe_name varchar(256),
   foreign key(recipe_name,recipe_version) references recipe(name,version)
);

create table water(
   name varchar(256) not null,
   amount real,
   calcium real,
   bicarbonate real,
   sulfate real,
   chloride real,
   sodium real,
   magnesium real,
   ph real,
   notes text,
   deleted boolean,
   display boolean,
   PRIMARY KEY (name,version)
);

-- instructions are many-to-one for recipes. 
create table instruction(
   iid integer PRIMARY KEY autoincrement,
   name varchar(256) not null,
   number integer, -- Which instruction number in the list.
   directions text,
   has_timer boolean,
   timer_value real,
   completed boolean,
   interval real,
   deleted boolean,
   display boolean,
   recipe_version integer,
   recipe_name varchar(256),
   foreign key(recipe_name,recipe_version) references recipe(name,version)
);

-- The relationship of styles to recipe is one to many, as is the mash and
-- equipment. It just makes most sense for the recipe to carry that around
-- instead of using another table
create table recipe(
   name varchar(256) not null,
   rtype varchar(32),
   brewer varchar(1024),
   assistant_brewer varchar(1024),
   batch_size real,
   boil_size real,
   boil_time real,
   efficiency real,
   og real,
   fg real,
   fermentation_stages int,
   primary_age real,
   primary_temp real,
   secondary_age real,
   secondary_temp real,
   tertiary_age real,
   tertiary_temp real,
   age real,
   age_temp real,
   brewdate date,
   carb_volume real,
   forced_carb boolean,
   priming_sugar_name varchar(128),
   carb_temp real,
   priming_sugar_equiv real,
   keg_priming_factor real,
   taste_notes text,
   taste_rating real,
   deleted boolean,
   display boolean,
   style_name varchar(256),
   style_version integer,
   mash_id integer,
   equipment_name varchar(256),
   equipment_version integer,
   foreign key(style_name,style_version) references style(name,version),
   foreign key(mash_id) references mash(mid),
   foreign key(equipment_name,equipment_version) references equipment(name,version),
   PRIMARY KEY (name,version)
);

create table mash_to_mashstep(
   mmid integer PRIMARY KEY autoincrement,

   mash integer,
   mashstep_name varchar(256),
   mashstep_version integer,
   foreign key(mash) references mash(mid),
   foreign key(mashstep_name,mashstep_version) references mashstep(name,version)
);

create table fermentable_in_recipe(
   hrid integer primary key autoincrement,
   fermentable_name varchar(256),
   fermentable_version integer,
   recipe_name varchar(256),
   recipe_version integer,
   foreign key(fermentable_name,fermentable_version) references fermentable(name,version),
   foreign key(recipe_name,recipe_version) references recipe(name,version)
);

create table hop_in_recipe(
   hrid integer PRIMARY KEY autoincrement,
   hop_name varchar(256),
   hop_version integer,
   recipe_name varchar(256),
   recipe_version integer,
   foreign key(hop_name,hop_version) references hop(name,version),
   foreign key(recipe_name,recipe_version) references recipe(name,version)
);

create table misc_in_recipe(
   mrid integer PRIMARY KEY autoincrement,
   misc_name varchar(256),
   misc_version,
   recipe_name varchar(256),
   recipe_version,
   foreign key(misc_name,misc_version) references misc(name,version),
   foreign key(recipe_name,recipe_version) references recipe(name,version)
);

create table water_in_recipe(
   wrid integer PRIMARY KEY autoincrement,
   water_name varchar(256),
   water_version,
   recipe_name varchar(256),
   recipe_version,
   foreign key(water_name,water_version) references water(name,version),
   foreign key(recipe_name,recipe_version) references recipe(name,version)
);

create table yeast_in_recipe(
   yrid integer PRIMARY KEY autoincrement,
   yeast_name varchar(256),
   yeast_version,
   recipe_name varchar(256),
   recipe_version,
   foreign key(yeast_name,yeast_version) references yeast(name,version),
   foreign key(recipe_name,recipe_version) references recipe(name,version)
);
