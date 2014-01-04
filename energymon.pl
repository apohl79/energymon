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
  return get_data(params->{unit}, params->{num}, "gas");
};

get '/power' => sub {
  return get_data(params->{unit}, params->{num}, "power");
};

get '/temp' => sub {
  return get_temp("temp_".params->{t});
};

start();

sub get_data {
  my $unit = shift;
  my $num = shift;
  my $table = shift;
  my $sec_diff = 0;
  my $beg;
  my $end;
  if ($unit eq "openhab") {
    $sec_diff = 5 * 60; # 5min for openhab
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
  } elsif ($unit eq "yesterday") {
    $beg ||= strftime('%Y-%m-%d 00:00:00', localtime(time() - 86400));
    $end ||= strftime('%Y-%m-%d 23:59:59', localtime(time() - 86400));
  }
  $beg ||= strftime('%Y-%m-%d %H:%M:%S', localtime(time() - $sec_diff));
  $end ||= strftime('%Y-%m-%d %H:%M:%S', localtime(time));
  my $dbh = DBI->connect($db, $dbu, $dbp);
  my $sth = $dbh->prepare("select sum(delta) from $table where time between '$beg' and '$end'");
  $sth->execute();
  my @result = $sth->fetchrow_array;
  my $val = $result[0];
  $val ||= 0;
  if ($val && $unit eq "openhab") {
    $val /= 5;
  }
  return $val;
}

sub get_temp {
  my $temp = shift;
  my $dbh = DBI->connect($db, $dbu, $dbp);
  my $sth = $dbh->prepare("select temp from $temp order by time desc limit 1");
  $sth->execute();
  my @result = $sth->fetchrow_array;
  return $result[0];
}
