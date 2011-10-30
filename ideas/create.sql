create table equipment(
   eid integer PRIMARY KEY autoincrement,
   name varchar(256) not null,
   version integer,
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
   display boolean
);

create table fermentable(
   fid integer PRIMARY KEY autoincrement,
   name varchar(256) not null,
   version integer,
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
   -- meta data
   deleted boolean,
   display boolean
);

create table hop(
   hid integer PRIMARY KEY autoincrement,
   name varchar(256) not null,
   version integer,
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
   -- meta data
   deleted boolean,
   display boolean
);

create table misc(
   mid integer PRIMARY KEY autoincrement,
   name varchar(256) not null,
   version integer,
   mtype varchar(64),
   use varchar(64),
   time real,
   amount real, 
   amount_is_weight boolean,
   use_for text,
   notes text,
   -- meta data
   deleted boolean,
   display boolean
);

create table style(
   sid integer PRIMARY KEY autoincrement,
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
   -- meta data
   deleted boolean,
   display boolean
);

create table yeast(
   yid integer PRIMARY KEY autoincrement,
   name varchar(256) not null,
   version integer,
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
   -- meta data
   deleted boolean,
   display boolean
);

create table mashstep(
   msid integer PRIMARY KEY autoincrement,
   name varchar(256) not null,
   version integer,
   mstype varchar(32),
   infuse_amount real,
   step_temp real,
   step_time real,
   ramp_time real,
   end_temp real,
   infuse_temp real,
   decoction_amount real,
   deleted boolean,
   display boolean
);

-- unlike some of the other tables, you can have a mash with no name.
-- which is gonna mess my nice primary key ideas up
create table mash(
   maid integer PRIMARY KEY autoincrement,
   name varchar(256),
   version integer,
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
   recipe_id integer,
   foreign key(recipe_id) references recipe(rid)
);

create table water(
   wid integer PRIMARY KEY autoincrement,
   name varchar(256) not null,
   version integer,
   amount real,
   calcium real,
   bicarbonate real,
   sulfate real,
   chloride real,
   sodium real,
   magnesium real,
   ph real,
   notes text,
   -- metadata
   deleted boolean,
   display boolean
);

-- instructions are many-to-one for recipes. 
create table instruction(
   iid integer PRIMARY KEY autoincrement,
   name varchar(256) not null,
   version integer,
   directions text,
   has_timer boolean,
   timer_value real,
   completed boolean,
   interval real,
   deleted boolean,
   display boolean,
   recipe_id integer,
   foreign key(recipe_id) references recipe(rid)
);

-- The relationship of styles to recipe is one to many, as is the mash and
-- equipment. It just makes most sense for the recipe to carry that around
-- instead of using another table
create table recipe(
   rid integer PRIMARY KEY autoincrement,
   name varchar(256) not null,
   version integer,
   rtype varchar(32),
   brewer varchar(1024),
   assistant_brewer varchar(1024),
   batch_size real,
   boil_size real,
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
   brewdate integer,
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
   style_id integer,
   mash_id integer,
   equipment_id integer,
   foreign key(style_id) references style(sid),
   foreign key(mash_id) references mash(maid),
   foreign key(equipment_id) references equipment(eid)
);

create table mash_to_mashstep(
   mmid integer PRIMARY KEY autoincrement,
   mash_id integer,
   mashstep_id integer,
   foreign key(mash_id) references mash(maid),
   foreign key(mashstep_id) references mashstep(msid)
);

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

