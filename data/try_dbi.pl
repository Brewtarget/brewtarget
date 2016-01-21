#!/usr/bin/perl

use strict;
use warnings;

use DBI;
use Data::Dumper;

my @tables =  (
   [qw/settings/],
   [qw/brewnote/],
   [qw/hop/],
   [qw/style/],
   [qw/instruction hasTimer completed/],
   [qw/water/],
   [qw/mash equip_adjust/],
   [qw/equipment calc_boil_volume/],
   [qw/mashstep/],
   [qw/yeast amount_is_weight add_to_secondary/],
   [qw/misc amount_is_weight/],
   [qw/fermentable add_after_boil recommend_mash is_mashed/],
   [qw/recipe forced_carb/],

   [qw/bt_equipment/],
   [qw/bt_fermentable/],
   [qw/bt_hop/],
   [qw/bt_misc/],
   [qw/bt_style/],
   [qw/bt_water/],
   [qw/bt_yeast/],

   [qw/fermentable_in_recipe/],
   [qw/recipe_children/],
   [qw/hop_children/],
   [qw/hop_in_inventory/],
   [qw/hop_in_recipe/],
   [qw/style_children/],
   [qw/instruction_in_recipe/],
   [qw/water_children/],
   [qw/water_in_recipe/],
   [qw/equipment_children/],
   [qw/yeast_children/],
   [qw/misc_children/],
   [qw/yeast_in_inventory/],
   [qw/fermentable_children/],
   [qw/misc_in_inventory/],
   [qw/yeast_in_recipe/],
   [qw/fermentable_in_inventory/],
   [qw/misc_in_recipe/],
);

my $dbname = shift || '';
die "usage: $0 [sqlite database]\n" unless $dbname;

my $sqlite = DBI->connect("dbi:SQLite:dbname=$dbname","","") or die $DBI::errstr;
my $pgsql  = DBI->connect("dbi:Pg:dbname=brewtarget;host=localhost",
                          'brewtarget',
                          'wdyw2b2day?',
                          {AutoCommit => 1, RaiseError => 1, PrintError => 0}) or die $DBI::errstr;



foreach my $tref ( @tables ) {
   my $name = shift @$tref;

   my $qry = "select * from $name";
   print "DEBUG: qry = $qry\n";
   my $qry_sth = $sqlite->prepare($qry);
   $qry_sth->execute();

   my ($ins, $ins_sth) = (0,0);
   $pgsql->begin_work();

   while ( my $vref = $qry_sth->fetchrow_hashref) {
      my @vals;
      if ( scalar( @$tref ) ) {
         foreach my $key ( @$tref ) {
            if ( $vref->{$key} eq '0' or $vref->{$key} eq '1' ) {
               $vref->{$key} = $vref->{$key} ? 'true' : 'false';
            }
         }
      }
      if ( defined $vref->{display} ) {
         $vref->{display} = $vref->{display} ? 'true' : 'false';
      }
      if ( defined $vref->{deleted} ) {
         $vref->{deleted} = $vref->{deleted} ? 'true' : 'false';
      }
      # if we haven't prepared
      if ( ! $ins ) {
         my ($colnames,$qmarks) = ("","");
         foreach my $col ( sort keys %$vref ) {
            push @vals, $vref->{$col};
            if ( $colnames ) {
               $colnames .= ",$col";
               $qmarks .= ",?";
            }
            else {
               $colnames = $col;
               $qmarks = "?";
            }
         }
         print "DEBUG: insert into $name ($colnames) VALUES($qmarks)\n";
         $ins_sth = $pgsql->prepare("insert into $name ($colnames) VALUES($qmarks)");
      }

      print "DEBUG: ", join("|", @vals), "\n";
      $ins_sth->execute(@vals); # oh sweet mercy
   }
   $pgsql->commit();
}

$sqlite->disconnect();
$pgsql->disconnect();

