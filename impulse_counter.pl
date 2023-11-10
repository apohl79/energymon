#!/usr/bin/perl

use strict;
use DBI;
use Time::HiRes qw(usleep);
use POSIX qw(strftime);

use lib "/opt/energymon";
use impulse_cfg;
use IO::Socket;

my $DEBUG = 0;

if (defined($ARGV[1] and $ARGV[1] eq "debug")) {
  $DEBUG = 1;
}

my $db = "DBI:mysql:database=energy;host=mneme";
my $dbu = "root";
my $dbp = "password";
my $graphite_host = "mneme";
my $mqtt_cmd = "/usr/local/bin/mosquitto_pub -h mqtt -p 1883 -t $mqtt_topic -m ";

my $graphite = undef;

# initialising the gpio pin
logmsg("setting up gpio pin $gpio_pin");
system "gpio -g mode $gpio_pin down";
system "gpio export $gpio_pin in";

my $last_t_persist = time;
my $last_t_mqtt = time;
my $counter = 0.0;
my $counter_mqtt = 0.0;

$SIG{TERM} = sub { persist($counter); exit; };

$|++;

while (1) {
  wait_for_pin($pin_off);
  logmsg("waiting for impulse...");
  wait_for_pin($pin_on);
  $counter += $impulse_count;
  $counter_mqtt += $impulse_count;
  send_graphite("stats.stats.home.$db_table", $impulse_count);
  my $delta_t_persist = time() - $last_t_persist;
  my $delta_t_mqtt = time() - $last_t_mqtt;
  logmsg("impulse detected, counter = $counter, counter_mqtt = $counter_mqtt, delta_t_persist = $delta_t_persist, delta_t_mqtt = $delta_t_mqtt");
  if ($delta_t_persist >= $persist_interval) {
    logmsg("persisting...");
    if (persist($counter) == 0) {
      logmsg("done");
      $last_t_persist = time;
      $counter = 0.0;
    } else {
      logmsg("failed");
    }
  }
  if ($delta_t_mqtt >= $mqtt_interval and defined($mqtt_topic)) {
    my $curr = sprintf("%.2f", $counter_mqtt / $mqtt_interval * 1000000);
    logmsg("mqtt publish $curr to $mqtt_topic...");
    system($mqtt_cmd.$curr);
    $last_t_mqtt = time;
    $counter_mqtt = 0.0;
  }
}

sub send_graphite {
    my ($name, $val) = @_;
    my $graphite = new IO::Socket::INET(PeerAddr => $graphite_host,
                                        PeerPort => 2003,
                                        Proto    => 'tcp');
    if (defined($graphite)) {
        logmsg("updating graphite");
        print $graphite "$name $val ".time()."\n";
        $graphite->close();
    } else {
        logmsg("graphite not available");
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
  my $ok = open(F, "< /sys/class/gpio/gpio$gpio_pin/value");
  if ($ok) {
      my $val = <F>;
      close F;
      chomp $val;
      return $val;
  } else {
      logmsg("Error: Failed to open /sys/class/gpio/gpio$gpio_pin/value: $!");
  }
  return 0;
}

sub logmsg {
  my $msg = shift;
  if ($DEBUG) {
      print "[".strftime('%Y-%m-%d %H:%M:%S', localtime(time))."] $msg\n";
  }
}
