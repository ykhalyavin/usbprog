#!/usr/bin/perl
#-------------------------------------------------------------------------
#* CreateRegfile.pl
#* Copyright (C) 2008 Martin Lang <Martin.Lang@rwth-aachen.de>
#*
#* This program is free software; you can redistribute it and/or modify
#* it under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 2 of the License, or
#* (at your option) any later version.
#*
#* This program is distributed in the hope that it will be useful,
#* but WITHOUT ANY WARRANTY; without even the implied warranty of
#* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#* GNU General Public License for more details.
#*
#* You should have received a copy of the GNU General Public License
#* along with this program; if not, write to the Free Software
#* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#*
#*----------------------------------------------------------------------

## Processing File to generate registerfiles for the evaluation script out of copy & paste data from the device's datasheet.

$| = 1;
if ($#ARGV != 0) {
	print "Usage: CreateRegfile.pl <filename>\n";
	exit;
}

open (INPUT, "< $ARGV[0]") or die("Cannot open given input file");

while ($line = <INPUT>) {
	# first we parse the essential information (Register id and Name)
	$line =~ s/–/-/g;
	
	if ($line =~ /^\s*\$([0-9a-fA-F]+)\s*\S+?\s+([A-Z0-9]+).*$/) {
		my $regaddr = $1;
		my $regname = $2;
		my @Bitnames = ("", "", "", "", "", "", "", "" );
		
		# check for register names
		if ($line =~ /^\s*\$[0-9a-fA-F]+\s*\S+?\s+[A-Z0-9]+\s+([A-Z0-9]+|-)\s+([A-Z0-9]+|-)\s+([A-Z0-9]+|-)\s+([A-Z0-9]+|-)\s+([A-Z0-9]+|-)\s+([A-Z0-9]+|-)\s+([A-Z0-9]+|-)\s+([A-Z0-9]+|-).*$/) {
			$Bitnames[0] = $8;
			$Bitnames[1] = $7;
			$Bitnames[2] = $6;
			$Bitnames[3] = $5;
			$Bitnames[4] = $4;
			$Bitnames[5] = $3;
			$Bitnames[6] = $2;
			$Bitnames[7] = $1;
			for ($i = 0; $i < 8; $i++) {
				if ($Bitnames[$i] eq "-") {
					$Bitnames[$i] = "";
				}
			}
		}
		print "$regaddr;$regname;$Bitnames[0];$Bitnames[1];$Bitnames[2];$Bitnames[3];$Bitnames[4];$Bitnames[5];$Bitnames[6];$Bitnames[7];\n";
	}


}

close INPUT;
