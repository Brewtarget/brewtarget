#!/usr/bin/perl

use strict;
use warnings;

use DBI;
use Getopt::Long qw(:config no_ignore_case);
use Data::Dumper;
use File::Basename;

# this is order dependent due to foreign key constraints
my @tables =  (
   [qw/settings/],
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
   [qw/brewnote/],

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

sub help {
   my $long = shift || 0;
   my $filename = basename($0);

   my $desc = << "EOF";
   where
      -s [sqlite db] -- the SQLite database file
      -p [password]  -- password for the user 
      -h [hostname]  -- hostname for the PgSQL server, defaults to localhost
      -P [port]      -- port number for the PgSQL server, defaults to 5432
      -n [database]  -- database name, defaults to "brewtarget"
      -u [username]  -- user with create table access in the PgSQL server,
                        defaults to brewtarget
      -h             -- prints this screen and exits
EOF
   print "Usage: $filename -s sqlite db -p password [-u username] [-h hostname] [-P port] [-n database] [-h]\n";
   print $desc if $long;
}

sub parse_cli {
   my $ref = shift;
   my $rc;

   %{$ref} = (
      sqlite => '',
      password => '',
      username => 'brewtarget',
      hostname => 'localhost',
      port => 5432,
      name => 'brewtarget',
      help => 0,
   );

   $rc = GetOptions( $ref,
            'sqlite|s=s',
            'password|p=s',
            'username|u=s',
            'hostname|h=s',
            'port|P=i',
            'name|n=s',
            'help|h!',
         );
   if ( not $rc ) {
      help();
      die "Error parsing command line\n";
   }

   if ( $ref->{help} ) {
      help(1);
      exit;
   }

   # make sure the sqlite database is specified and exists
   if ( not $ref->{sqlite} or not -e $ref->{sqlite} ) {
      help();
      die "Invalid sqlite database. The -s flag is required and must point to a file\n";
   }

   if ( not $ref->{password} ) {
      help();
      die "The -p flag is required\n";
   }
}

my %opts;

parse_cli(\%opts);

my $sqlite = DBI->connect("dbi:SQLite:dbname=$opts{sqlite}","","") or die $DBI::errstr;
my $pgsql  = DBI->connect("dbi:Pg:dbname=$opts{name};host=$opts{hostname}",
                          $opts{username},
                          $opts{password},
                          {AutoCommit => 1, RaiseError => 1, PrintError => 0}) or die $DBI::errstr;


foreach my $tref ( @tables ) {
   my $name = shift @$tref;
   my $maxid = 0;

   my $qry = "select * from $name";
   print "DEBUG: qry = $qry\n";
   my $qry_sth = $sqlite->prepare($qry);
   $qry_sth->execute();

   my ($ins, $ins_sth) = (0,0);
   $pgsql->begin_work();

   while ( my $vref = $qry_sth->fetchrow_hashref) {
      my @vals;

      # worry about translating 0/1 to false/true
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
      # get the maxid if required.
      if ( defined $vref->{id} and $vref->{id} > $maxid ) {
         $maxid = $vref->{id};
      }

      # some special cases to translate datestamps. I don't like this
      # solution, to be honest but I can't seem to find anything else that
      # works.
      if ( defined $vref->{fermentDate} and $vref->{fermentDate} eq 'CURRENT_DATETIME' ) {
         $vref->{fermentDate} = 'now()';
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
   # last step is to reset the sequence if there is one.
   if ( $name ne 'settings' and $maxid) {
      my $setval = "SELECT setval('${name}_id_seq',$maxid)";
      print "DEBUG: $setval\n";

      $pgsql->do($setval);
   }

}

$sqlite->disconnect();
$pgsql->disconnect();

