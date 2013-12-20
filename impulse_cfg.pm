package impulse_cfg;

my $mode = $ARGV[0];
if (!defined($mode)) {
  print "Usage: $0 <gas|current>\n";
  exit;
}

# wiringpi pin
our $gpio_pin = -1;
# on each impulse we used 0.01qm of gas
our $impulse_count = 0;
# normal pin state
our $pin_off = 0;
# impulse pin state
our $pin_on = 1;
# persistance interval in seconds
our $persist_interval = 300;
# poll frequency in ms
our $poll_freq = 100;
# number of reads to consider a value stable
our $stable_count = 10;
# db table to persist data to
our $db_table = "";

if ($mode eq "current") {
  $gpio_pin = 18;
  $impulse_count = 0.001;
  $pin_off = 1;
  $pin_on = 0;
  $persist_interval = 300;
  $poll_freq = 3;
  $stable_count = 3;
  $db_table = "current";
} elsif ($mode eq "gas") {
  $gpio_pin = 17;
  $impulse_count = 0.01;
  $db_table = "gas";
}

BEGIN {
  require Exporter;
  our @ISA = qw(Exporter);
  our @EXPORT = qw($gpio_pin $impulse_count $pin_off $pin_on $persist_interval $poll_freq $stable_count $db_table);
}

1;
