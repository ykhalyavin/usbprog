#!/usr/bin/perl

use warnings;
use strict;

while(my $line = <>) {
  chomp $line;
  exit 0 if $line =~ /q/;
  system("make download");
}
