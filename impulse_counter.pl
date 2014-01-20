#!/usr/bin/perl

use strict;
use DBI;
use Time::HiRes qw(usleep);
use POSIX qw(strftime);

use lib "/opt/energymon";
use impulse_cfg;

my $db = "DBI:mysql:database=energy;host=localhost";
my $dbu = "root";
my $dbp = "password";

# initialising the gpio pin
logmsg("setting up gpio pin $gpio_pin");
system "gpio -g mode $gpio_pin down";
system "gpio export $gpio_pin in";

my $last_t = time;
my $counter = 0.0;

$SIG{TERM} = sub { persist($counter); exit; };

$|++;

while (1) {
  wait_for_pin($pin_off);
  logmsg("waiting for impulse...");
  wait_for_pin($pin_on);
  $counter += $impulse_count;
  my $delta_t = time() - $last_t;
  logmsg("impulse detected, counter = $counter, delta_t = $delta_t");
  if ($delta_t >= $persist_interval) {
    logmsg("persisting...");
    if (persist($counter) == 0) {
      logmsg("done");
      $last_t = time;
      $counter = 0.0;
    } else {
      logmsg("failed");
    }
  }
}

sub persist {
  my $val = shift;
  my $dbh = DBI->connect($db, $dbu, $dbp) or return 1;
  $dbh->do("insert into $db_table (time, delta) values(now(), $val)");
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
    usleep($poll_freq * 1000);
  }
  return 1;
}

sub gpio_read {
  open F, "< /sys/class/gpio/gpio$gpio_pin/value"
    or die "can't open /sys/class/gpio/gpio$gpio_pin/value";
  my $val = <F>;
  close F;
  chomp $val;
  return $val;
}

sub logmsg {
  my $msg = shift;
  print "[".strftime('%Y-%m-%d %H:%M:%S', localtime(time))."] $msg\n";
}
