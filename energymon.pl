#!/usr/bin/perl

use strict;
use Dancer;
use DBI;
use POSIX qw(strftime);

set port => 80;
set logger => 'console';

my $db = "DBI:mysql:database=energy;host=localhost";
my $dbu = "root";
my $dbp = "password";

get '/gas' => sub {
  my $unit = params->{unit};
  my $num = params->{num};
  my $sec_diff = 0;
  if ($unit eq "sec") {
    $sec_diff = $num;
  } elsif ($unit eq "min") {
    $sec_diff = $num * 60;
  } elsif ($unit eq "hour") {
    $sec_diff = $num * 60 * 60;
  } elsif ($unit eq "day") {
    $sec_diff = $num * 60 * 60 * 24;
  } elsif ($unit eq "month") {
    $sec_diff = $num * 60 * 60 * 24 * 31;
  } elsif ($unit eq "year") {
    $sec_diff = $num * 60 * 60 * 24 * 365;
  }
  my $beg = strftime('%Y-%m-%d %H:%M:%S', localtime(time() - $sec_diff));
  my $end = strftime('%Y-%m-%d %H:%M:%S', localtime(time));
  my $dbh = DBI->connect($db, $dbu, $dbp);
  my $sth = $dbh->prepare("select sum(delta) from gas where time between '$beg' and '$end'");
  $sth->execute();
  my @result = $sth->fetchrow_array;
  return $result[0];
};

start();
