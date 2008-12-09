#!/usr/bin/perl

use warnings;
use strict;
use Time::HiRes qw< sleep >;

sleep 0.5;

my $cmd = `usbprog devices`;

if($cmd =~ /\[\s*(\d+)\](.*?)c620$/m) {
  system("usbprog device $1 upload ../../avrispmk2klon/avrispmk2.bin");
  print "Ok."
} else {
  exit 1;
}
