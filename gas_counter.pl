#!/usr/bin/perl

use strict;

# wiringpi pin
my $gpio_pin = 0;
# on each impulse we used 0.01qm of gas
my $impulse_count = 0.01;
# persistance interval in seconds
my $persist_interval = 300;
# poll frequency in ms
my $poll_freq = 100;
# number of reads to consider a value stable
my $stable_count = 10;

use DBI;
use Time::HiRes qw(usleep);
use POSIX qw(strftime);

my $db = "DBI:mysql:database=energy;host=localhost";
my $dbu = "root";
my $dbp = "password";

my $pidfile = "/var/run/gas_counter.pid";
open F, ">$pidfile";
print F $$;
close F;

my $logdir = "/var/log/energymon";
my $logfile = "$logdir/gas.log";
if (! -d "$logdir") {
  system "mkdir -p $logdir";
}
open LOG, ">$logfile";

# initialising the gpio pin
#system "gpio mode $gpio_pin in";
system "gpio mode $gpio_pin down";

my $last_t = time;
my $counter = 0.0;

while (1) {
  wait_for_pin(0);
  wait_for_pin(1);
  $counter += $impulse_count;
  my $delta_t = time() - $last_t;
  log("impulse detected, counter = $counter, delta_t = $delta_t");
  if ($delta_t >= $persist_interval) {
    log("persisting...");
    if (persist($counter) == 0) {
      log("done");
      $last_t = time;
      $counter = 0.0;
    } else {
      log("failed");
    }
  }
}

sub persist {
  my $val = shift;
  my $dbh = DBI->connect($db, $dbu, $dbp) or return 1;
  $dbh->do("insert into gas (time, delta) values(now(), $val)");
  $dbh->disconnect();
  return 0;
}

sub wait_for_pin {
  my $val = shift;
  while (1) {
    while ($val != gpio_read()) {
      usleep($poll_freq * 1000);
    }
    if (is_pin_stable($val)) {
      return;
    }
  }
}

sub is_pin_stable {
  my $val = shift;
  my $c = $stable_count;
  while ($c--) {
    if ($val != gpio_read()) {
      return 0;
    }
  }
  return 1;
}

sub gpio_read {
  my $val = `gpio read $gpio_pin`;
  chomp $val;
  return $val;
}

sub log {
  my $msg = shift;
  print LOG "[".strftime('%Y-%m-%d %H:%M:%S', localtime(time))."] $msg\n";
}
