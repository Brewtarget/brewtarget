CREATE TABLE settings( 
   id integer primary key,
   version varchar(256), 
   repopulateChildrenOnNextStart int DEFAULT 0);

CREATE TABLE equipment(
   id SERIAL PRIMARY KEY,
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
   calc_boil_volume boolean DEFAULT FALSE,
   lauter_deadspace real DEFAULT 0.0,
   top_up_kettle real DEFAULT 0.0,
   hop_utilization real DEFAULT 100.0,
   notes text DEFAULT '',
   -- Out BeerXML extensions
   real_evap_rate real DEFAULT 0.0,
   boiling_point real DEFAULT 100.0,
   absorption real DEFAULT 1.085,
   -- Metadata
   deleted boolean DEFAULT FALSE,
   display boolean DEFAULT TRUE,
   folder varchar(256) DEFAULT ''
);
CREATE TABLE fermentable(
   id SERIAL PRIMARY KEY,
   -- BeerXML properties
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
   -- Display stuff
   display_unit integer DEFAULT -1,
   display_scale integer DEFAULT -1,
   -- meta data
   deleted boolean DEFAULT FALSE,
   display boolean DEFAULT TRUE,
   folder varchar(256) DEFAULT ''
);
CREATE TABLE hop(
   id SERIAL PRIMARY KEY,
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
   deleted boolean DEFAULT FALSE,
   display boolean DEFAULT TRUE,
   folder varchar(256) DEFAULT ''
);
CREATE TABLE misc(
   id SERIAL PRIMARY KEY,
   -- BeerXML properties
   name varchar(256) not null DEFAULT '',
   mtype varchar(32) DEFAULT 'Other',
   use varchar(32) DEFAULT 'Boil',
   time real DEFAULT 0.0,
   amount real DEFAULT 0.0,
   amount_is_weight boolean DEFAULT TRUE,
   use_for text DEFAULT '',
   notes text DEFAULT '',
   -- Display stuff.
   -- Be careful: this will change meaning based on amount_is_weight
   display_unit integer DEFAULT -1,
   display_scale integer DEFAULT -1,
   -- meta data
   deleted boolean DEFAULT FALSE,
   display boolean DEFAULT TRUE,
   folder varchar(256) DEFAULT ''
);
CREATE TABLE style(
   id SERIAL PRIMARY KEY,
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
   deleted boolean DEFAULT FALSE,
   display boolean DEFAULT TRUE,
   folder varchar(256) DEFAULT ''
);
CREATE TABLE yeast(
   id SERIAL PRIMARY KEY,
   -- BeerXML properties
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
   best_for text DEFAULT '',
   times_cultured integer DEFAULT 0,
   max_reuse integer DEFAULT 10,
   add_to_secondary boolean DEFAULT FALSE,
   -- Display stuff
   -- Be careful: this will change meaning based on amount_is_weight
   display_unit integer DEFAULT -1,
   display_scale integer DEFAULT -1,
   -- meta data
   deleted boolean DEFAULT FALSE,
   display boolean DEFAULT TRUE,
   folder varchar(256) DEFAULT ''
);
CREATE TABLE water(
   id SERIAL PRIMARY KEY,
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
   deleted boolean DEFAULT FALSE,
   display boolean DEFAULT TRUE,
   folder varchar(256) DEFAULT ''
);
CREATE TABLE bt_equipment(
   id SERIAL PRIMARY KEY,
   equipment_id integer,
   foreign key(equipment_id) references equipment(id)
);
CREATE TABLE bt_fermentable(
   id SERIAL PRIMARY KEY,
   fermentable_id integer,
   foreign key(fermentable_id) references fermentable(id)
);
CREATE TABLE bt_hop(
   id SERIAL PRIMARY KEY,
   hop_id integer,
   foreign key(hop_id) references hop(id)
);
CREATE TABLE bt_misc(
   id SERIAL PRIMARY KEY,
   misc_id integer,
   foreign key(misc_id) references misc(id)
);
CREATE TABLE bt_style(
   id SERIAL PRIMARY KEY,
   style_id integer,
   foreign key(style_id) references style(id)
);
CREATE TABLE bt_yeast(
   id SERIAL PRIMARY KEY,
   yeast_id integer,
   foreign key(yeast_id) references yeast(id)
);
CREATE TABLE bt_water(
   id SERIAL PRIMARY KEY,
   water_id integer,
   foreign key(water_id) references water(id)
);
CREATE TABLE mash(
   id SERIAL PRIMARY KEY,
   -- BeerXML properties
   name varchar(256) DEFAULT '',
   grain_temp real DEFAULT 20.0,
   notes text DEFAULT '',
   tun_temp real DEFAULT 20.0,
   sparge_temp real DEFAULT 74.0,
   ph real DEFAULT 7.0,
   tun_weight real DEFAULT 0.0,
   tun_specific_heat real DEFAULT 0.0,
   equip_adjust boolean DEFAULT TRUE,
   -- Metadata
   deleted boolean DEFAULT FALSE,
   -- Mashes default to be undisplayed until they are named
   display boolean DEFAULT FALSE,
   -- Does this make any sense?
   folder varchar(256) DEFAULT ''
);
CREATE TABLE mashstep(
   id SERIAL PRIMARY KEY,
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
   deleted boolean DEFAULT FALSE,
   display boolean DEFAULT TRUE,
   -- Our step number is unique within our parent mash.
   mash_id integer,
   step_number integer DEFAULT 0,
   foreign key(mash_id) references mash(id)
   -- This is not necessary since we manage these internally in Brewtarget.
   -- unique( mash_id, step_number )
   -- mashsteps don't get folders, because they don't separate from their mash
);

CREATE TABLE recipe(
   id SERIAL PRIMARY KEY,
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
   forced_carb boolean DEFAULT FALSE,
   priming_sugar_name varchar(128) DEFAULT '',
   carbonationTemp_c real DEFAULT 20.0,
   priming_sugar_equiv real DEFAULT 1.0,
   keg_priming_factor real DEFAULT 1.0,
   notes text DEFAULT '',
   taste_notes text DEFAULT '',
   taste_rating real DEFAULT 0.0,
   -- Metadata
   deleted boolean DEFAULT FALSE,
   display boolean DEFAULT TRUE,
   folder varchar(256) DEFAULT '',
   -- Relational members
   style_id integer,
   mash_id integer,
   equipment_id integer,
   foreign key(style_id) references style(id),
   foreign key(mash_id) references mash(id),
   foreign key(equipment_id) references equipment(id)
);
CREATE TABLE brewnote(
   id SERIAL PRIMARY KEY,
   brewDate timestamp DEFAULT now(),
   fermentDate timestamp DEFAULT now(),
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
   projected_ferm_points real DEFAULT 0.0,
   boil_off real DEFAULT 0.0,
   final_volume real DEFAULT 0.0,
   notes text DEFAULT '',
   deleted boolean DEFAULT FALSE,
   display boolean DEFAULT TRUE,
   folder varchar(256) DEFAULT '',
   recipe_id integer,
   foreign key(recipe_id) references recipe(id)

);
CREATE TABLE instruction(
   id SERIAL PRIMARY KEY,
   name varchar(256) not null DEFAULT '',
   directions text DEFAULT '',
   hasTimer boolean DEFAULT FALSE,
   timerValue varchar(16) DEFAULT '00:00:00',
   completed boolean DEFAULT FALSE,
   interval real DEFAULT 0.0,
   deleted boolean DEFAULT FALSE,
   display boolean DEFAULT TRUE
   -- instructions aren't displayed in trees, and get no folder
);
CREATE TABLE fermentable_in_recipe(
   id SERIAL primary key,
   fermentable_id integer,
   recipe_id integer,
   foreign key(fermentable_id) references fermentable(id),
   foreign key(recipe_id) references recipe(id)
);
CREATE TABLE hop_in_recipe(
   id SERIAL PRIMARY KEY,
   hop_id integer,
   recipe_id integer,
   foreign key(hop_id) references hop(id),
   foreign key(recipe_id) references recipe(id)
);
CREATE TABLE misc_in_recipe(
   id SERIAL PRIMARY KEY,
   misc_id integer,
   recipe_id integer,
   foreign key(misc_id) references misc(id),
   foreign key(recipe_id) references recipe(id)
);
CREATE TABLE water_in_recipe(
   id SERIAL PRIMARY KEY,
   water_id integer,
   recipe_id integer,
   foreign key(water_id) references water(id),
   foreign key(recipe_id) references recipe(id)
);
CREATE TABLE yeast_in_recipe(
   id SERIAL PRIMARY KEY,
   yeast_id integer,
   recipe_id integer,
   foreign key(yeast_id) references yeast(id),
   foreign key(recipe_id) references recipe(id)
);
CREATE TABLE instruction_in_recipe(
   id SERIAL PRIMARY KEY,
   instruction_id integer,
   recipe_id integer,
   -- instruction_number is the order of the instruction in the recipe.
   instruction_number integer DEFAULT 0,
   foreign key(instruction_id) references instruction(id),
   foreign key(recipe_id) references recipe(id)
);

CREATE OR REPLACE FUNCTION increment_instruction_num() RETURNS TRIGGER AS $BODY$
BEGIN
   UPDATE instruction_in_recipe SET instruction_number = 
      (SELECT max(instruction_number) FROM instruction_in_recipe WHERE recipe_id = NEW.recipe_id) + 1 
      WHERE id = NEW.id;
   return NULL;
END;
$BODY$
 LANGUAGE plpgsql;

CREATE TRIGGER inc_ins_num AFTER INSERT ON instruction_in_recipe
   FOR EACH ROW EXECUTE PROCEDURE increment_instruction_num();

CREATE OR REPLACE FUNCTION decrement_instruction_num() RETURNS TRIGGER AS $BODY$
BEGIN
   UPDATE instruction_in_recipe SET instruction_number = instruction_number - 1
      WHERE recipe_id = OLD.recipe_id AND instruction_id > OLD.instruction_id;
   return NULL;
END;
$BODY$
  LANGUAGE plpgsql;

CREATE TRIGGER dec_ins_num AFTER DELETE ON instruction_in_recipe
   FOR EACH ROW EXECUTE PROCEDURE decrement_instruction_num();

CREATE TABLE equipment_children( id SERIAL PRIMARY KEY, parent_id integer, child_id integer UNIQUE, foreign key(parent_id) references equipment(id), foreign key(child_id)  references equipment(id));
CREATE TABLE fermentable_children( id SERIAL PRIMARY KEY, parent_id integer, child_id integer UNIQUE, foreign key(parent_id) references fermentable(id), foreign key(child_id)  references fermentable(id));
CREATE TABLE hop_children( id SERIAL PRIMARY KEY, parent_id integer, child_id integer UNIQUE, foreign key(parent_id) references hop(id), foreign key(child_id)  references hop(id));
CREATE TABLE misc_children( id SERIAL PRIMARY KEY, parent_id integer, child_id integer UNIQUE, foreign key(parent_id) references misc(id), foreign key(child_id)  references misc(id));
CREATE TABLE recipe_children( id SERIAL PRIMARY KEY, parent_id integer, child_id integer UNIQUE, foreign key(parent_id) references recipe(id), foreign key(child_id)  references recipe(id));
CREATE TABLE style_children( id SERIAL PRIMARY KEY, parent_id integer, child_id integer UNIQUE, foreign key(parent_id) references style(id), foreign key(child_id)  references style(id));
CREATE TABLE water_children( id SERIAL PRIMARY KEY, parent_id integer, child_id integer UNIQUE, foreign key(parent_id) references water(id), foreign key(child_id)  references water(id));
CREATE TABLE yeast_children( id SERIAL PRIMARY KEY, parent_id integer, child_id integer UNIQUE, foreign key(parent_id) references yeast(id), foreign key(child_id)  references yeast(id));
CREATE TABLE fermentable_in_inventory( id SERIAL PRIMARY KEY, fermentable_id integer UNIQUE, amount real DEFAULT 0.0, foreign key(fermentable_id) references fermentable(id));
CREATE TABLE hop_in_inventory(id SERIAL PRIMARY KEY, hop_id integer UNIQUE, amount real DEFAULT 0.0, foreign key(hop_id) references hop(id));
CREATE TABLE misc_in_inventory(id SERIAL PRIMARY KEY, misc_id integer UNIQUE, amount real DEFAULT 0.0, foreign key(misc_id) references misc(id));
CREATE TABLE yeast_in_inventory(id SERIAL PRIMARY KEY, yeast_id integer UNIQUE, quanta integer DEFAULT 0, foreign key(yeast_id) references yeast(id));
