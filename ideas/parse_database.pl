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

my %translate = (
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
   EQUIPMENT => {
      NAME              => { column => 'name',             type => 'string' },
      VERSION           => { column => 'version',          type => 'number' },
      NOTES             => { column => 'notes',            type => 'string' },
      BOIL_SIZE         => { column => 'boil_size',        type => 'number' },
      BATCH_SIZE        => { column => 'batch_size',       type => 'number' },
      TUN_VOLUME        => { column => 'tun_volume',       type => 'number' },
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
   },
);

my $dbfile  = shift @ARGV || '';
my $xmlfile = shift @ARGV || '';

die "Usage: parse_xml.pl <db file> <XML database>\n" unless $xmlfile and $dbfile;

my $xs = XML::Simple->new(SuppressEmpty=> '', ForceArray => 1 );
my $ref = $xs->XMLin($xmlfile);

my $dbh = DBI->connect("dbi:SQLite:dbname=$dbfile","","", {
      AutoCommit => 0,
      RaiseError => 1,
      sqlite_see_if_its_a_number => 1,
   });

$dbh->do("PRAGMA foreign_keys = ON");

for my $section ( keys %$ref ) {
   die "Unknown section: $section\n" unless defined $translate{$section};

   for my $ref ( @{$ref->{$section}} ) {
      if ( ! $ref->{NAME}[0] ) {
         warn "No name entry found in $section. Skipping\n";
         next;
      }

      my $insert = "insert into $section (";
      my $values = "values (";

      for my $key ( keys %$ref ) {
         if ( defined $translate{$section}{$key} ) {
            my $value;
            $insert .= "$translate{$section}{$key}{column},";
            if ( $translate{$section}{$key}{type} eq 'string' ) {
               $value = $ref->{$key}[0];
               $value =~ s/\n/ /sg;
               $value = $dbh->quote($value);
            }
            elsif ( $translate{$section}{$key}{type} eq 'boolean' ) { 
               $value = $ref->{$key}[0];
               if ( $value eq 'FALSE' ) {
                  $value = "0";
               }
               elsif ( $value eq 'TRUE' ) {
                  $value = "1";
               }
            }
            else {
               $value = "$ref->{$key}[0]";
            }
            $values .= "$value,";
         }
         else {
            die "Unrecognized key $key in $section\n";
         }
      }
      $insert =~ s/,$/) /;
      $values =~ s/,$/);/;
      $dbh->do("$insert $values");
   }
   $dbh->commit();
}
