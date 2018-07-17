#!/usr/bin/perl 
#===============================================================================
#
#         FILE: parse_xml.pl
#
#        USAGE: ./parse_xml.pl  
#
#  DESCRIPTION: 
#
#      OPTIONS: ---
# REQUIREMENTS: ---
#         BUGS: ---
#        NOTES: ---
#       AUTHOR: YOUR NAME (), 
#      COMPANY: 
#      VERSION: 1.0
#      CREATED: 10/06/2011 10:18:58 PM
#     REVISION: ---
#===============================================================================

use strict;
use warnings;

use XML::Simple;
use DBI;
use Data::Dumper;
use Getopt::Long;

my $verbose = 0;

sub help {
   print << "EOF";
usage: $0 [-p PATH] [-admr] [database]
  where:
    -p PATH  -- the path to all of the XML files (defaults to .)
    -a       -- parses and imports all records (default)
    -d       -- parses and imports only the database.xml
    -m       -- parses and imports only the mashs.xml 
    -r       -- parses and imports only the recipes.xml
    -v       -- verbose
    -h       -- shows this menu
    database -- the prepared sqlite3 database
EOF

}

sub parse_cli {
   my $opts = shift;
   my $rc;

   %{$opts} = (
      path => '.',
      all  => 1,
      data => 0,
      mash => 0,
      recipe => 0,
      verbose => 0,
      help => 0
   );
   $rc = GetOptions( $opts,
         'path|p=s',
         'all|a',
         'data|d',
         'mash|m',
         'recipe|r',
         'verbose|v',
         'help|h',
      );

   if ( ! $rc ) {
      help();
      die "Invalid options\n";
   }

   if ( $opts->{help} ) {
      help();
      exit;
   }

   if ( $opts->{data} || $opts->{mash} || $opts->{recipe} ) {
      $opts->{all} = 0;
   }

   $verbose = $opts->{verbose};
}

# I have hidden all the noise of the hashes in subs so that I can do syntax
# folding and hide the noise :)
sub set_database {
   return (
      EQUIPMENT => {
         NAME              => { column => 'name',             type => 'string' },
         VERSION           => { column => 'version',          type => 'number' },
         NOTES             => { column => 'notes',            type => 'string' },
         BOIL_SIZE         => { column => 'boil_size',        type => 'number' },
         BATCH_SIZE        => { column => 'batch_size',       type => 'number' },
         TUN_VOLUME        => { column => 'tun_volume',       type => 'number' },
         WHIRLPOOL_TIME    => { column => 'whirlpool_time',   type => 'number' },
         HOP_EST_WHIRLPOOL => { column => 'hop_est_whirlpool',type => 'number' },
         TUN_DIAMETER      => { column => 'tun_diameter',     type => 'number' },
         TUN_WEIGHT        => { column => 'tun_weight',       type => 'number' },
         TUN_SPECIFIC_HEAT => { column => 'tun_specific_heat',type => 'number' },
         TOP_UP_WATER      => { column => 'top_up_water',     type => 'number' },
         TRUB_CHILLER_LOSS => { column => 'trub_chiller_loss',type => 'number' },
         EVAP_RATE         => { column => 'evap_rate',        type => 'number' },
         REAL_EVAP_RATE    => { column => 'real_evap_rate',   type => 'number' },
         BOIL_TIME         => { column => 'boil_time',        type => 'number' },
         BOILING_POINT     => { column => 'boiling_point',    type => 'number' },
         ABSORPTION        => { column => 'absorption',       type => 'number' },
         CALC_BOIL_VOLUME  => { column => 'calc_boil_volume', type => 'boolean' },
         LAUTER_DEADSPACE  => { column => 'lauter_deadspace', type => 'number' },
         TOP_UP_KETTLE     => { column => 'top_up_kettle',    type => 'number' },
         HOP_UTILIZATION   => { column => 'hop_utilization',  type => 'number' },
         EQUIP_ADJUST      => { column => 'equip_adjust',     type => 'boolean' },
      },
      FERMENTABLE => {
         NAME             => { column => 'name',             type => 'string' },
         VERSION          => { column => 'version',          type => 'number' },
         TYPE             => { column => 'ftype',            type => 'string' },
         AMOUNT           => { column => 'amount',           type => 'number' },
         YIELD            => { column => 'yield',            type => 'number' },
         COLOR            => { column => 'color',            type => 'number' },
         ADD_AFTER_BOIL   => { column => 'add_after_boil',   type => 'boolean' },
         ORIGIN           => { column => 'origin',           type => 'string' },
         SUPPLIER         => { column => 'supplier',         type => 'string' },
         NOTES            => { column => 'notes',            type => 'string' },
         COARSE_FINE_DIFF => { column => 'coarse_fine_diff', type => 'number' },
         MOISTURE         => { column => 'moisture',         type => 'number' },
         DIASTATIC_POWER  => { column => 'diastatic_power',  type => 'number' },
         PROTEIN          => { column => 'protein',          type => 'number' },
         MAX_IN_BATCH     => { column => 'max_in_batch',     type => 'number' },
         RECOMMEND_MASH   => { column => 'recommend_mash',   type => 'boolean' },
         IS_MASHED        => { column => 'is_mashed',        type => 'boolean' },
         IBU_GAL_PER_LB   => { column => 'ibu_gal_per_lb',   type => 'number' },
      },
      HOP => {
         NAME           => { column => 'name',           type => 'string' },
         VERSION        => { column => 'version',        type => 'number' },
         ALPHA          => { column => 'alpha',          type => 'number' },
         AMOUNT         => { column => 'amount',         type => 'number' },
         USE            => { column => 'use',            type => 'string' },
         TIME           => { column => 'time',           type => 'number' },
         NOTES          => { column => 'notes',          type => 'string' },
         TYPE           => { column => 'htype',          type => 'string' },
         FORM           => { column => 'form',           type => 'string' },
         BETA           => { column => 'beta',           type => 'number' },
         HSI            => { column => 'hsi',            type => 'number' },
         ORIGIN         => { column => 'origin',         type => 'string' },
         SUBSTITUTES    => { column => 'substitutes',    type => 'string' },
         HUMULENE       => { column => 'humulene',       type => 'number' },
         COHUMULONE     => { column => 'cohumulone',     type => 'number' },
         CARYOPHYLLENE  => { column => 'caryophyllene',  type => 'number' },
         MYRCENE        => { column => 'myrcene',        type => 'number' },
      },
      MASH_STEP => {
         NAME             => { column => 'name',             type => 'string' },
         VERSION          => { column => 'version',          type => 'number' },
         TYPE             => { column => 'mstype',           type => 'string' },
         INFUSE_AMOUNT    => { column => 'infuse_amount',    type => 'number' },
         STEP_TEMP        => { column => 'step_temp',        type => 'number' },
         STEP_TIME        => { column => 'step_time',        type => 'number' },
         RAMP_TIME        => { column => 'ramp_time',        type => 'number' },
         END_TEMP         => { column => 'end_temp',         type => 'number' },
         INFUSE_TEMP      => { column => 'infuse_temp',      type => 'number' },
         DECOCTION_AMOUNT => { column => 'decoction_amount', type => 'number' },
      },
      MISC => {
         NAME              => { column => 'name',             type => 'string' },
         VERSION           => { column => 'version',          type => 'number' },
         TYPE              => { column => 'mtype',            type => 'string' },
         USE               => { column => 'use',              type => 'string' },
         TIME              => { column => 'time',             type => 'number' },
         AMOUNT            => { column => 'amount',           type => 'number' },
         AMOUNT_IS_WEIGHT  => { column => 'amount_is_weight', type => 'boolean' },
         USE_FOR           => { column => 'use_for',          type => 'string' },
         NOTES             => { column => 'notes',            type => 'string' },
      },
      STYLE => {
         NAME            => { column => 'name',            type => 'string' },
         VERSION         => { column => 'version',         type => 'number' },
         TYPE            => { column => 's_type',          type => 'string' },
         CATEGORY        => { column => 'category',        type => 'string' },
         CATEGORY_NUMBER => { column => 'category_number', type => 'number' },
         STYLE_LETTER    => { column => 'style_letter',    type => 'string' },
         STYLE_GUIDE     => { column => 'style_guide',     type => 'string' },
         STYPE           => { column => 'stype',           type => 'string' },
         OG_MIN          => { column => 'og_min',          type => 'number' },
         OG_MAX          => { column => 'og_max',          type => 'number' },
         FG_MIN          => { column => 'fg_min',          type => 'number' },
         FG_MAX          => { column => 'fg_max',          type => 'number' },
         IBU_MIN         => { column => 'ibu_min',         type => 'number' },
         IBU_MAX         => { column => 'ibu_max',         type => 'number' },
         COLOR_MIN       => { column => 'color_min',       type => 'number' },
         COLOR_MAX       => { column => 'color_max',       type => 'number' },
         ABV_MIN         => { column => 'abv_min',         type => 'number' },
         ABV_MAX         => { column => 'abv_max',         type => 'number' },
         CARB_MIN        => { column => 'carb_min',        type => 'number' },
         CARB_MAX        => { column => 'carb_max',        type => 'number' },
         NOTES           => { column => 'notes',           type => 'string' },
         PROFILE         => { column => 'profile',         type => 'string' },
         INGREDIENTS     => { column => 'ingredients',     type => 'string' },
         EXAMPLES        => { column => 'examples',        type => 'string' },
      },
      WATER => {
         NAME        => { column => 'name',        type => 'string' },
         VERSION     => { column => 'version',     type => 'number' },
         AMOUNT      => { column => 'amount',      type => 'number' },
         CALCIUM     => { column => 'calcium',     type => 'number' },
         BICARBONATE => { column => 'bicarbonate', type => 'number' },
         SULFATE     => { column => 'sulfate',     type => 'number' },
         CHLORIDE    => { column => 'chloride',    type => 'number' },
         SODIUM      => { column => 'sodium',      type => 'number' },
         MAGNESIUM   => { column => 'magnesium',   type => 'number' },
         PH          => { column => 'ph',          type => 'number' },
         NOTES       => { column => 'notes',       type => 'string' },
      },
      YEAST => {
         NAME              => { column => 'name',             type => 'string' },
         VERSION           => { column => 'version',          type => 'number' },
         TYPE              => { column => 'ytype',            type => 'string' },
         FORM              => { column => 'form',             type => 'string' },
         AMOUNT            => { column => 'amount',           type => 'number' },
         AMOUNT_IS_WEIGHT  => { column => 'amount_is_weight', type => 'boolean' },
         LABORATORY        => { column => 'laboratory',       type => 'string' },
         PRODUCT_ID        => { column => 'product_id',       type => 'string' },
         MIN_TEMPERATURE   => { column => 'min_temperature',  type => 'number' },
         MAX_TEMPERATURE   => { column => 'max_temperature',  type => 'number' },
         FLOCCULATION      => { column => 'flocculation',     type => 'string' },
         ATTENUATION       => { column => 'attenuation',      type => 'number' },
         NOTES             => { column => 'notes',            type => 'string' },
         BEST_FOR          => { column => 'best_for',         type => 'string' },
         TIMES_CULTURED    => { column => 'times_cultured',   type => 'number' },
         MAX_REUSE         => { column => 'max_reuse',        type => 'number' },
         ADD_TO_SECONDARY  => { column => 'add_to_secondary', type => 'boolean' },
      },
   );
}

# Three files, three hashes.
sub set_mashses {
   return (
      MASH => {
         NAME              => { column => 'name',              type => 'string' },
         VERSION           => { column => 'version',           type => 'number' },
         NOTES             => { column => 'notes',             type => 'string' },
         GRAIN_TEMP        => { column => 'grain_temp',        type => 'number' },
         TUN_TEMP          => { column => 'tun_temp',          type => 'number' },
         SPARGE_TEMP       => { column => 'sparge_temp',       type => 'number' },
         PH                => { column => 'ph',                type => 'number' },
         TUN_WEIGHT        => { column => 'tun_weight',        type => 'number' },
         TUN_SPECIFIC_HEAT => { column => 'tun_specific_heat', type => 'number' },
         EQUIP_ADJUST      => { column => 'equip_adjust',      type => 'boolean' },

      },
      MASHSTEP => {
         NAME             => { column => 'name',             type => 'string' },
         VERSION          => { column => 'version',          type => 'number' },
         TYPE             => { column => 'mstype',           type => 'string' },
         INFUSE_AMOUNT    => { column => 'infuse_amount',    type => 'number' },
         STEP_TEMP        => { column => 'step_temp',        type => 'number' },
         STEP_TIME        => { column => 'step_time',        type => 'number' },
         RAMP_TIME        => { column => 'ramp_time',        type => 'number' },
         END_TEMP         => { column => 'end_temp',         type => 'number' },
         INFUSE_TEMP      => { column => 'infuse_temp',      type => 'number' },
         DECOCTION_AMOUNT => { column => 'decoction_amount', type => 'number' },
      },
      mash_to_mashstep => {
         mash             => { column => 'mash',             type => 'number' },
         mashstep_name    => { column => 'mashstep_name',    type => 'string' },
         mashstep_version => { column => 'mashstep_version', type => 'number' },
      },
   );
}

# Uhh. Wow
sub set_recipe {
   return (
      RECIPE => {
         NAME                => { column => 'name',                type => 'string' },
         VERSION             => { column => 'version',             type => 'number' },
         TYPE                => { column => 'rtype',               type => 'string' },
         BREWER              => { column => 'brewer',              type => 'string' },
         ASSISTANT_BREWER    => { column => 'assistant_brewer',    type => 'string' },
         BATCH_SIZE          => { column => 'batch_size',          type => 'number' },
         BOIL_SIZE           => { column => 'boil_size',           type => 'number' },
         EFFICIENCY          => { column => 'efficiency',          type => 'number' },
         OG                  => { column => 'og',                  type => 'number' },
         FG                  => { column => 'fg',                  type => 'number' },
         FERMENTATION_STAGES => { column => 'fermentation_stages', type => 'number' },
         PRIMARY_AGE         => { column => 'primary_age',         type => 'number' },
         PRIMARY_TEMP        => { column => 'primary_temp',        type => 'number' },
         SECONDARY_AGE       => { column => 'secondary_age',       type => 'number' },
         SECONDARY_TEMP      => { column => 'secondary_temp',      type => 'number' },
         TERTIARY_AGE        => { column => 'tertiary_age',        type => 'number' },
         TERTIARY_TEMP       => { column => 'tertiary_temp',       type => 'number' },
         AGE                 => { column => 'age',                 type => 'number' },
         AGE_TEMP            => { column => 'age_temp',            type => 'number' },
         BREWDATE            => { column => 'brewdate',            type => 'number' },
         CARB_VOLUME         => { column => 'carb_volume',         type => 'number' },
         FORCED_CARB         => { column => 'forced_carb',         type => 'boolean'},
         PRIMING_SUGAR_NAME  => { column => 'priming_sugar_name',  type => 'string' },
         CARB_TEMP           => { column => 'carb_temp',           type => 'number' },
         PRIMING_SUGAR_EQUIV => { column => 'priming_sugar_equiv', type => 'number' },
         KEG_PRIMING_FACTOR  => { column => 'keg_priming_factor',  type => 'number' },
         TASTE_NOTES         => { column => 'taste_notes',         type => 'string' },
         TASTE_RATING        => { column => 'taste_rating',        type => 'number' },
      },
      BREWNOTE => {
         NAME                    => { column => 'name',                    type => 'string' },
         VERSION                 => { column => 'version',                 type => 'number' },
         BREWDATE                => { column => 'brewDate',                type => 'string' },
         DATE_FERMENTED_OUT      => { column => 'fermentDate',             type => 'string' },
         SG                      => { column => 'sg',                      type => 'number' },
         VOLUME_INTO_BK          => { column => 'volume_into_bk',          type => 'number' },
         STRIKE_TEMP             => { column => 'strike_temp',             type => 'number' },
         MASH_FINAL_TEMP         => { column => 'mash_final_temp',         type => 'number' },
         OG                      => { column => 'og',                      type => 'number' },
         POST_BOIL_VOLUME        => { column => 'post_boil_volume',        type => 'number' },
         VOLUME_INTO_FERMENTER   => { column => 'volume_into_fermenter',   type => 'number' },
         PITCH_TEMP              => { column => 'pitch_temp',              type => 'number' },
         FG                      => { column => 'fg',                      type => 'number' },
         ACTUAL_ABV              => { column => 'actual_abv',              type => 'number' },
         EFF_INTO_BK             => { column => 'eff_into_bk',             type => 'number' },
         PREDICTED_OG            => { column => 'predicted_og',            type => 'number' },
         BREWHOUSE_EFF           => { column => 'brewhouse_eff',           type => 'number' },
         PREDICTED_ABV           => { column => 'predicted_abv',           type => 'number' },
         PROJECTED_BOIL_GRAV     => { column => 'projected_boil_grav',     type => 'number' },
         PROJECTED_STRIKE_TEMP   => { column => 'projected_strike_temp',   type => 'number' },
         PROJECTED_FIN_TEMP      => { column => 'projected_fin_temp',      type => 'number' },
         PROJECTED_MASH_FIN_TEMP => { column => 'projected_mash_fin_temp', type => 'number' },
         PROJECTED_VOL_INTO_BK   => { column => 'projected_vol_into_bk',   type => 'number' },
         PROJECTED_OG            => { column => 'projected_og',            type => 'number' },
         PROJECTED_VOL_INTO_FERM => { column => 'projected_vol_into_ferm', type => 'number' },
         PROJECTED_FG            => { column => 'projected_fg',            type => 'number' },
         PROJECTED_EFF           => { column => 'projected_eff',           type => 'number' },
         PROJECTED_ABV           => { column => 'projected_abv',           type => 'number' },
         PROJECTED_ATTEN         => { column => 'projected_atten',         type => 'number' },
         PROJECTED_POINTS        => { column => 'projected_points',        type => 'number' },
         BOIL_OFF                => { column => 'boil_off',                type => 'number' },
         FINAL_VOLUME            => { column => 'final_volume',            type => 'number' },
         NOTES                   => { column => 'notes',                   type => 'string' },
      },
      INSTRUCTION => {
         NAME        => { column => 'name',        type => 'string'  },
         VERSION     => { column => 'version',     type => 'number'  },
         DIRECTIONS  => { column => 'directions',  type => 'string'  },
         HAS_TIMER   => { column => 'has_timer',   type => 'boolean' },
         TIMER_VALUE => { column => 'timer_value', type => 'string'  },
         COMPLETED   => { column => 'completed',   type => 'boolean' },
         INTERVAL    => { column => 'interval',    type => 'number'  },
      },
      hop_in_recipe => {
         hop_name       => { column => 'hop_name',       type => 'string' },
         hop_version    => { column => 'hop_version',    type => 'number' },
         recipe_name    => { column => 'recipe_name',    type => 'string' },
         recipe_version => { column => 'recipe_version', type => 'number' },

      },
   );
}

# Convenience function to do make sure the fields get quoted properly.
sub convert_value {
   my ($section,$xmlref,$key,$dbh) = @_;
   my $value = '';

   if ( defined $section->{$key} ) {
      if ( defined $xmlref->{$key}[0] ) {
         if ( $section->{$key}{type} eq 'string' ) {
            $value = $xmlref->{$key}[0];
            $value =~ s/\n/ /sg;
            $value = $dbh->quote($value) . ",";
         }
         elsif ( $section->{$key}{type} eq 'boolean' ) { 
            $value = $xmlref->{$key}[0] eq 'TRUE' ? "1," : "0,";
         }
         else {
            $value = "$xmlref->{$key}[0],";
         }
      }
   }
   else {
      die "convert_value: Unrecognized key $key\n";
   }
   return $value;
}

# Returns the name of the column. It just makes the code cleaner
sub convert_name {
   my ($section,$key) = @_;
   my $insert = '';

   if ( defined $section->{$key} ) {
      $insert = "$section->{$key}{column},";
   }
   else {
      die "convert_name: unrecognized key $key\n";
   }
  
   return $insert;
}

# Executes one query
sub lookup_value {
   my ($name, $sth, $dbh) = @_;
   my ($value);

   $sth->execute($name);
   $sth->bind_col(1,\$value);
   $sth->fetchrow_arrayref;
   $sth->finish();

   return $value;
}

# Okay. This one translates the database.xml into the database. It is the
# easiest one of the lot because it has to do the least amount of work. I can
# assume, based on the previous data structure, that everything is unique.
sub convert_database {
   my ($dbh, $xsref, $translate) = @_;
   my $whirlygig = 1;
   my %ids = ( EQUIPMENT   => 'eid',
               FERMENTABLE => 'fid',
               HOP         => 'hid',
               MISC        => 'mid',
               STYLE       => 'sid',
               WATER       => 'wid',
               YEAST       => 'yid'
            );

   local $| = 1;
   for my $section ( keys %$xsref ) {
      die "Unknown section: $section\n" unless defined $translate->{$section};
      my $name = $section eq 'MASH_STEP' ? 'MASHSTEP' : $section;
      my $lname = lc $name;

      for my $ref ( @{$xsref->{$section}} ) {
         if ( ! $ref->{NAME}[0] ) {
            warn "No name entry found in $section. Skipping\n";
            next;
         }

         # The style guide for American Pale Ale and Brown Ale will cause problems.
         # Fix them in transit
         if ( $section eq 'STYLE' ) {
            if ($ref->{NAME}[0] eq 'American Pale Ale' ) {
               $ref->{CATEGORY}[0] = 'American Ale';
               $ref->{CATEGORY_NUMBER}[0] = 10;
               $ref->{STYLE_LETTER}[0] = 'A';
            }
            elsif ( $ref->{NAME}[0] eq 'Mild' ) {
               $ref->{CATEGORY}[0] = 'English Ale';
               $ref->{CATEGORY_NUMBER}[0] = 11;
               $ref->{STYLE_LETTER}[0] = 'A';
            }
         }

         # The notes fields seem to cause issues. This should fix them.
         if ( defined $ref->{NOTES} ) {
            $ref->{NOTES}[0] =~ s/[\r\n]/ /msg;
         }

         # Since this is the initial load, assume everything is the root
         # object.
         my $insert = "insert into $name (deleted,display,";
         my $values = "values (0,";
         my $parent = 0;

         if ( defined $ids{$section} ) {
            $parent = lookup_value( $ref->{NAME}[0], 
                                    $dbh->prepare("select min($ids{$section}) from $name where name = ?"), 
                                    $dbh );
         }
         if ( $parent ) {
            $values .= "0,";
         }
         else {
            $values .= "1,";
         }

         for my $key ( keys %$ref ) {
            $values .= convert_value( $translate->{$section}, $ref, $key, $dbh);
            $insert .= convert_name( $translate->{$section}, $key );
         }
         $insert =~ s/,$/) /;
         $values =~ s/,$/);/;

         print "$insert $values\n" if $verbose;
         $dbh->do("$insert $values");
         # if this thingy already exists in the database, we need to link it
         # in
         if ($parent) {
            my $id = lookup_value( $ref->{NAME}[0], 
                                   $dbh->prepare("select max($ids{$section}) from $name where name = ?"), 
                                   $dbh );
            $dbh->do("insert into ${lname}_children (parent_id, child_id) values ($parent,$id);");
         }
         $whirlygig++;
         print "$whirlygig\r" if $whirlygig % 10 == 0; 
      }
      $dbh->commit();
   }
}

# This one loads the mashes and is harder than loaded the databases, because
# we need to do some cross references and load the mapping tables. I am hoping
# this will give me an idea of what I will need to do for the recipes, which
# should be the hardest yet.
sub convert_mashes {
   my ($dbh, $xsref,$translate) = @_;
   my (%mashsteps,$mash_id);
   my $whirlygig = 0;

   my $steps = '';

   for my $section ( keys %$xsref ) {
      for my $mash ( @{$xsref->{$section}} ) {

         my $insert_mash = "insert into mash (deleted,display,";
         my $values_mash = "values (0,1,";

         for my $key ( keys %{$mash} ) {
            if ( $key ne 'MASH_STEPS' ) {
               $values_mash .= convert_value( $translate->{$section}, $mash, $key, $dbh);
               $insert_mash .= convert_name( $translate->{$section}, $key );
            }
            else {
               # These need to be processed after we have added the mash
               $steps = $mash->{$key};
            }
         }
         $insert_mash =~ s/,$/) /;
         $values_mash =~ s/,$/);/;
         $dbh->do("$insert_mash $values_mash");

         # Process the mash steps. Parsing XML makes for some really twisted
         # data structures.
         for my $step ( @$steps ) {
            next unless $step;
            for my $key ( keys %{$step} ) {
               for my $mashstep ( @{$step->{$key}} ) {
                  my $insert_step = "insert into mashstep (deleted,display,";
                  my $values_step = "values (0,1,";

                  for my $mstep ( keys %{$mashstep} ) {
                     $values_step .= convert_value( $translate->{'MASHSTEP'}, $mashstep, $mstep, $dbh);
                     $insert_step .= convert_name( $translate->{'MASHSTEP'}, $mstep );
                  }
                  $insert_step =~ s/,$/) /;
                  $values_step =~ s/,$/);/;
                  $dbh->do("$insert_step $values_step");
                  # get the id we just created. I wonder if there is a better
                  # way of doing this?
                  my $name = $mashstep->{NAME}[0];
                  $mashsteps{$name} = lookup_value( $name,
                                              $dbh->prepare('select max(msid) from mashstep where name = ?'), 
                                              $dbh);
               }
            }
         }
         # Now we link all this crap together.
         $mash_id = lookup_value( $mash->{NAME}[0], 
                                  $dbh->prepare("select max(maid) from mash where name = ?"),
                                  $dbh );
         for my $step ( keys %mashsteps ) {
            my $insert_map = "insert into mash_to_mashstep (mash_id,mashstep_id)";
            $insert_map .= " values ($mash_id, $mashsteps{$step});";
            $dbh->do("$insert_map");
         }
         $whirlygig++;
         print "\t$whirlygig\n" if $whirlygig % 10 == 0;
      }
      $dbh->commit();
   }
   return $mash_id || 0;
}

# This is a helper to convert_recipes. I hate long functions
sub convert_brewnotes {
   my ($brewnotes,$rid,$translate,$dbh) = @_;

   return unless $brewnotes->[0];

   for my $bnote ( @{$brewnotes->[0]{BREWNOTE}} ) {
      my $insert = "insert into brewnote (deleted,display,recipe_id,";
      my $values = "values (0,1,$rid,";

      for my $key ( keys %$bnote ) {
         next if $key eq 'VERSION';
         $values .= convert_value( $translate->{BREWNOTE}, $bnote, $key, $dbh);
         $insert .= convert_name( $translate->{BREWNOTE}, $key );
      }
      $insert =~ s/,$/) /;
      $values =~ s/,$/);/;
      $dbh->do("$insert $values");
   }
   $dbh->commit();
}

sub convert_instructions {
   my ($instructions,$rid,$translate,$dbh) = @_;

   return unless $instructions->[0];

   for my $ins ( @{$instructions->[0]{INSTRUCTION}} ) {
      my $insert = "insert into instruction (deleted,display,recipe_id,";
      my $values = "values (0,1,$rid,";

      for my $key ( keys %$ins ) {
         $values .= convert_value( $translate->{INSTRUCTION}, $ins, $key, $dbh);
         $insert .= convert_name( $translate->{INSTRUCTION}, $key );
      }
      $insert =~ s/,$/) /;
      $values =~ s/,$/);/;
      print "$insert $values\n" if $verbose;
      $dbh->do("$insert $values");
   }
   $dbh->commit();
}

sub convert_ingredients {
   my ($ingredients,$section,$rid,$translate,$dbh) = @_;
   my $lsec = lc $section;
   my $link_table = $lsec . '_in_recipe';
   my %ids = ( HOP         => 'hid',
               FERMENTABLE => 'fid',
               MISC        => 'mid',
               WATER       => 'wid',
               YEAST       => 'yid'
            );

   return unless $ingredients->[0];

   for my $ing ( @{$ingredients->[0]{$section}} ) {
      my $insert = "insert into $section (deleted,display,";
      my $values = "values (0,";

      # Get the ingredient in the main tables. The one marked for display has
      # to be the parent
      my $parent = lookup_value( $ing->{NAME}[0], 
                                 $dbh->prepare("select $ids{$section} from $section where name = ? and display = 1"), 
                                 $dbh );

      # If the ingredient is in the main table, set this not to display 
      if ( $parent ) {
         $values .= "0,";
      }
      # Otherwise, display this 
      else {
         $values .= "1,";
      }

      # The notes fields seem to cause issues. This should fix them.
      if ( defined $ing->{NOTES} ) {
         $ing->{NOTES}[0] =~ s/[\n\r]/ /msg;
      }

      for my $key ( keys %$ing ) {
         $values .= convert_value( $translate->{$section}, $ing, $key, $dbh);
         $insert .= convert_name( $translate->{$section}, $key );
      }
      $insert =~ s/,$/) /;
      $values =~ s/,$/);/;
      print "$insert $values\n" if $verbose;
      $dbh->do("$insert $values");
      # grab the ingredient we just created

      my $id = lookup_value( $ing->{NAME}[0], $dbh->prepare("select max($ids{$section}) from $section where name = ?"), $dbh);
      die "Couldn't find myself: $ing->{NAME}[0] $id\n" unless $id;

      # Set up any necessary parent/child relations
      if ( $parent ) {
         $dbh->do("insert into ${lsec}_children (parent_id,child_id) values ($parent,$id)");
      }

      # Now perform the linking.
      $insert = "insert into $link_table (${lsec}_id,recipe_id) ";
      $values = join (",","values ( ", $id, $rid);
      $values =~ s/\( ,/(/;
      print "$insert $values);\n" if $verbose;
      $dbh->do("$insert $values);");

   }
   $dbh->commit();
}

sub convert_withId {
   my ($stuff,$section,$trans,$dbh) = @_;
   my ($name, $id, $parent);
   my %ids = ( EQUIPMENT   => 'eid',
               FERMENTABLE => 'fid',
               HOP         => 'hid',
               MISC        => 'mid',
               STYLE       => 'sid',
               WATER       => 'wid',
               YEAST       => 'yid'
            );


   my $data = $stuff->[0];
   $name = $data->{NAME}[0];

   $parent = lookup_value($name, 
                      $dbh->prepare("select $ids{$section} from $section where name = ? and display = 1"), 
                      $dbh);
   die "Couldn't find parent $name $section $ids{$section} $parent\n" unless $parent;

   my $insert = "insert into $section (deleted,display,";
   my $values = sprintf 'values (0,%s,', 
                        $parent ? "0" : "1";

   # The style guide for American Pale Ale and Brown Ale will cause problems.
   # Fix them in transit
   if ( $section eq 'STYLE' ) {
      if ($data->{NAME}[0] eq 'American Pale Ale' ) {
         $data->{CATEGORY}[0] = 'American Ale';
         $data->{CATEGORY_NUMBER}[0] = 10;
         $data->{STYLE_LETTER}[0] = 'A';
      }
      elsif ( $data->{NAME}[0] eq 'Mild' ) {
         $data->{CATEGORY}[0] = 'English Ale';
         $data->{CATEGORY_NUMBER}[0] = 11;
         $data->{STYLE_LETTER}[0] = 'A';

      }
   }
   # The notes fields seem to cause issues. This should fix them.
   if ( defined $data->{NOTES} ) {
      $data->{NOTES}[0] =~ s/[\r\n]/ /msg;
   }

   for my $key ( keys %$data ) {
      $values .= convert_value( $trans->{$section}, $data, $key, $dbh);
      $insert .= convert_name( $trans->{$section}, $key );
   }
   $insert =~ s/,$/) /;
   $values =~ s/,$/);/;
   print "$insert $values\n" if $verbose;
   $dbh->do("$insert $values");
   $dbh->commit();

   $id = lookup_value($name, $dbh->prepare("select max($ids{$section}) from $section where name = ?"), $dbh);
   # Do the child links if required
   if ( $parent ) {
      my $lsec = lc $section;
      $dbh->do("insert into ${lsec}_children (parent_id,child_id) values ($parent,$id);");
   }
   return $id;
}

# This one will be the hardest. Lots of dependencies, lots of keys and 2
# bazillion tables. I am hoping I can shortcut much of this by calling
# convert_database and convert_mashes as required.
sub convert_recipes {
   my ($dbh, $xsref,$translate,$dbhash,$mshash) = @_;
   my ($parent_id, $rname, $rid);

   local $| = 1;
   # Have I complained about these obscenely deep data structures yet?
   for my $section ( keys %$xsref ) {
      for my $recipe ( @{$xsref->{$section}} ) {
         # I am doing this a little upside down for right now. We use the
         # translations hash to decide which values to extract from the recipe
         # XML
         my $ins_rec = "insert into recipe (deleted,display,";
         my $val_rec = " values (0,";

         # Save this to make later things easy
         $rname = $recipe->{NAME}[0];

         print "\tconverting $rname\n";
         # See if we have a parent
         $parent_id = lookup_value( $rname,
                                    $dbh->prepare('select min(rid) from recipe where name = ? and display = 1'),
                                    $dbh
                                  );

         $val_rec .= $parent_id ? "0," : "1,";

         for my $key ( keys %{$translate->{RECIPE}} ) {
            my ($val,$name);
            if ( $val  = convert_value( $translate->{RECIPE}, $recipe, $key, $dbh) and
                 $name = convert_name( $translate->{RECIPE}, $key ) ) {
               $val_rec .= $val;
               $ins_rec .= $name;
            }
         }

         # We need to find the style, the equipment and do something with the
         # mashes
         my $equip_id = convert_withId($recipe->{EQUIPMENT},'EQUIPMENT',$dbhash,$dbh);
         my $style_id = convert_withId($recipe->{STYLE},'STYLE', $dbhash, $dbh);

         # Round peg, meet square hole. I do not want to completely rewrite
         # convert_mashes, but it expects a few more layers of data structure
         # than the recipe gives me
         my $mash_id = convert_mashes($dbh, { MASH => $recipe->{MASH} }, $mshash);
        
         $ins_rec .= "equipment_id,mash_id,style_id,";
         $val_rec .= join( ",",
                        $equip_id,
                        $mash_id,
                        $style_id);

         # That should be the recipe. Deceptively easy, but I left all the
         # hard work for later.
         $ins_rec =~ s/,$/) /;
         $val_rec .= ');';
         print "$ins_rec $val_rec\n" if $verbose;
         $dbh->do( "$ins_rec $val_rec");

         # Find the recipe we just made
         $rid = lookup_value( $rname,
                              $dbh->prepare('select max(rid) from recipe where name = ?'),
                              $dbh
                           );

         die "Could not find myself: $rname $rid\n" unless $rid;
         # link this one to its parent if we must
         if ( $parent_id ) {
            $dbh->do("insert into recipe_children (parent_id,child_id) values ($parent_id,$rid);");
         }

         # Brewnotes 
         convert_brewnotes($recipe->{BREWNOTES},$rid,$translate,$dbh);
         # Instructions
         convert_instructions($recipe->{INSTRUCTIONS},$rid,$translate,$dbh);
         # Hops 
         convert_ingredients($recipe->{HOPS},'HOP',$rid,$dbhash,$dbh);
         # Fermentables
         convert_ingredients($recipe->{FERMENATBLES},'FERMENATBLE',$rid,$dbhash,$dbh);
         # Misc
         convert_ingredients($recipe->{MISCS},'MISC',$rid,$dbhash,$dbh);
         # Water
         convert_ingredients($recipe->{WATERS},'WATER',$rid,$dbhash,$dbh);
         # Yeast
         convert_ingredients($recipe->{YEASTS},'YEAST',$rid,$dbhash,$dbh);
      }
   }
}

my %options;

parse_cli(\%options);

my $dbfile    = shift @ARGV || '';
my $data_file = join( "/", $options{path}, 'database.xml');
my $mash_file = join( "/", $options{path}, 'mashs.xml');
my $recp_file = join( "/", $options{path}, 'recipes.xml');
my $xsref;

my %dbhash = set_database();
my %mshash = set_mashses();
my %rchash = set_recipe();

if ( ! $dbfile ) {
   help();
   die "No database specified\n";
}

my $xs = XML::Simple->new(SuppressEmpty=> '', ForceArray => 1 );
my $dbh = DBI->connect("dbi:SQLite:dbname=$dbfile","","", {
      AutoCommit => 0,
      RaiseError => 1,
      sqlite_see_if_its_a_number => 1,
   });

$dbh->do("PRAGMA foreign_keys = ON");

if ( $options{all} || $options{data} ) {
   print "parsing database\n";
   $xsref = $xs->XMLin($data_file);
   convert_database($dbh,$xsref,\%dbhash);
}

if ( $options{all} || $options{mash} ) {
   print "parsing mashes\n";
   $xsref = $xs->XMLin($mash_file);
   convert_mashes($dbh,$xsref,\%mshash);
}

if ( $options{all} || $options{recipe} ) {
   print "parsing recipes\n";
   $xsref = $xs->XMLin($recp_file);
   convert_recipes($dbh,$xsref,\%rchash,\%dbhash,\%mshash);
}

$dbh->disconnect();
