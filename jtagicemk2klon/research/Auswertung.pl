#!/usr/bin/perl
#-------------------------------------------------------------------------
#* Auswertung.pl
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

## Definitionen

## JTAG Statemachine 

@JTAGStates = (
	"TEST-LOGIC-RESET",	#0
	"RUN-TEST/IDLE", 	#1 
	"SELECT-DR-SCAN",  	#2
	"SELECT-IR-SCAN",  	#3
	"CAPTURE DR",		#4
	"SHIFT-DR",			#5
	"EXIT1-DR",			#6	
	"PAUSE-DR",			#7
	"EXIT2-DR",			#8
	"UPDATE-DR",		#9
	"CAPTURE-IR",		#10
	"SHIFT-IR",			#11
	"EXIT1-IR",			#12
	"PAUSE-IR",			#13
	"EXIT2-IR",			#14
	"UPDATE-IR"			#15
);
# Format STATE => ( 0 Transition Target , 1 Transition Target)
@JTAGTransition = (
	[ 1, 0 ],	#0
	[ 1, 2 ],	#1
	[ 4, 3 ],	#2
	[ 10, 0 ],	#3
	[ 5, 6 ],	#4
	[ 5, 6 ],	#5
	[ 7, 9 ],	#6
	[ 7, 8 ],	#7
	[ 5, 9 ], 	#8
	[ 1, 2 ], 	#9
	[ 11, 12 ],	#10
	[ 11, 12 ],	#11
	[ 13, 15 ],	#12
	[ 13, 14 ],	#13
	[ 11, 15 ],	#14
	[ 1, 2 ], 	#15
);

%JTAGHandlers = (
	1 => \&ProcessState_TestRunIDLE,
	4 => \&ProcessState_CaptureDR,
	5 => \&ProcessState_ShiftDR,
	9 => \&ProcessState_UpdateDR,
	10 => \&ProcessState_CaptureIR,
	11 => \&ProcessState_ShiftIR,
	15 => \&ProcessState_UpdateIR
);

%JTAGInstructions = (
	0 => "EXTEST",
	1 => "IDCODE",
	2 => "SAMPLE_PRELOAD",
	4 => "AVR Prog Enable",
	5 => "AVR Prog Cmd",
	6 => "AVR Pageload",
	7 => "AVR Pageread",
	8 => "AVR Stop",
	9 => "AVR Run",
	10 => "Exec AVR Instruction",
	11 => "Access OCD Registers",
	12 => "AVR Reset",
	15 => "BYPASS"
);

%OCDRegisters = (
	0 => "PSB0",
	1 => "PSB1",
	2 => "PDMSB",
	3 => "PDSB",
	8 => "BCR",
	9 => "BSR",
	12 => "OCDR",
	13 => "OCDCSR"
);

%OCDBitnames = (
	8 => [ "BCR0", "BCR1", "BCR2", "PDSB_MODE0", "PDSB_MODE1", "PDMSB_MODE0", "PDMSB_MODE1", "EN_PDSB", "EN_PDMSB", "BREAK_MASK", "BPSB1", "BPSB0", "BFLOW", "BSTEP", "PCMOD", "TIMERRUN" ],
	9 => [ "SOFTB", "FORCEB", "BSR2", "PDSBB", "PDMSBB", "PSB1B", "PSB0B", "FLOWB", "BSR8", "BSR9", "BSR10", "BSR11", "BSR12", "BSR13", "BSR14", "BSR15" ],
	13 => [ "OCDCTL0", "OCDCTL1", "RESNA0", "RESNA1", "OCDRDIRTY", "OCDCTL5", "OCDCTL6", "OCDCTL7", "OCDCTL8", "OCDCTL9", "OCDCTL10", "OCDCTL11", "OCDCTL12", "OCDCTL13", "OCDCTL14", "OCDRE" ]
);

$JTAGStart = 2; # JTAGICE seems to idle in SELECT-DR-SCAN State

## /Definitionen


## INFO
# JTAG Clock Idles at high state. Data is latched at time of the rising edge and updated at time of the falling edge
# Deshalb wird die JTAG Statemachine immer bei der steigenden Flanke getriggert, suche also nach steigenden Flanken in TCK
# Lese nun alle Zeilen aus der Eingabe Datei, öffne Ausgabe Datei und triggere bei steigenden Flanken die State Machine.

## /INFO

## init
$| = 1;
if ($#ARGV < 0) {
	print "Usage: Auswertung.pl [ -v | -sb=*.sb | -start=JTAG-STATE ] <filename>\n";
	exit;
}
my $JtagState = $JTAGStart;
my $TDI = 0;
my $TDO = 0;
my $TMS = 0;
my $oldTCK = 0;

$Symbolfile = 0;
%Regnames = ();
%Bitnames = ();
@Registerfile = (-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1);

$CurrentIR = "";
$CurrentDRIn = "";
$CurrentDROut = "";

$LatchedInstruction = 0;
$LatchedData = 0;
$OutputData = 0;

$OCDPrelatchReg = 0;

#LoadProcessorSymbolicFile("mega16.sb");
# Parse command line arguments
$filename = "";
$Verbose = 0;
for ($i = 0; $i <= $#ARGV; $i++) {
	if ($ARGV[$i] =~ /^-v$/) {
		$Verbose = 1;
	}
	elsif ($ARGV[$i] =~ /^-sb=(.*\.sb)$/) {
		LoadProcessorSymbolicFile($1);
	}
	elsif ($ARGV[$i] =~ /^-start=([-A-Z]+)$/) {
		# Find matching JTAG State
		$JtagState = -1;
		for ($c = 0; $c <= $#JTAGStates; $c++) {
			if ($1 eq $JTAGStates[$c]) {
				$JtagState = $c;
				last;
			}
		}
		
		if ($JtagState == -1) {
			die("Invalid JTAG start state given!");
		}
	}
	else {
		# treat this as filename
		$filename = $ARGV[$i];
	}
}

## /init

open (INPUT, "< $filename") or die("Cannot open given file");
#open (OUTPUT, "> $filename.out") or die("Cannot open an output file");


while ($line = <INPUT>) {

	# Parse line of Format Line#,Time,TCK,TMS,TDO,TDI
	
	if ($line =~ /^\d+,(\d+),(\d),(\d),(\d),(\d)/) {
	
		$TMS = int($3);
		$TDO = int($4);
		$TDI = int($5);
		
		if ($oldTCK == 0 && int($2) == 1) {
			# TRIGGER JTAG State Machine
			
			# Process Data in context of current JTAG State
			
			if (exists($JTAGHandlers{$JtagState})) {
				## Call it with current TDO and TDI
				&{$JTAGHandlers{$JtagState}}($TDO,$TDI);
			}
			
			## Calculate next JTAG State
			$JtagState = $JTAGTransition[$JtagState][$TMS];
			
			if ($Verbose == 1) {
				print "Next JTAG State: $JTAGStates[$JtagState]\n";
			}
		
		}
		
		$oldTCK = int($2);
	}


}

close INPUT;
#close OUTPUT;

sub ProcessState_TestRunIDLE {
	print "JTAG Test RUN / IDLE\n";
	## TODO: Check whether this is an avr instruction and disassemble it
	my $disasm = "";
	
	if ($LatchedInstruction == 10) {
		open (TMP, "> tmp") or die("Error opening tempfile");
		binmode TMP;
		print TMP pack("v",$LatchedData);
		close TMP;
		system("avr-objdump -D --architecture avr:5 --target binary tmp > restmp");
		open (TMP, "< restmp") or die("Cannot open resulting file");
		while ($resline = <TMP>) {
			# Parse output of avr-objdump
			if ($resline =~ /^\s+0:\s+[A-Fa-f0-9]{2}\s[A-Fa-f0-9]{2}\s+(.*)$/) {
				$disasm = $1;
				last;
			}
		}
		close TMP;	
		$disasm = PostprocessDisassembler($disasm);
		print "AVR Instruction: \t\t" . $disasm . "\n";
	}
}

sub ProcessState_CaptureDR {
	$CurrentDRIn = "";
	$CurrentDROut = "";
}

sub ProcessState_ShiftDR {
	my $TDO = shift;
	my $TDI = shift;
	
	if ((length($CurrentDRIn) % 5) == 4) {
		$CurrentDRIn = "_" . $CurrentDRIn;
		$CurrentDROut = "_" . $CurrentDROut;
	}
	
	$CurrentDRIn = "$TDI" . $CurrentDRIn;
	$CurrentDROut = "$TDO" . $CurrentDROut;
}

sub ProcessState_UpdateDR {
	my $PrintDRIn = $CurrentDRIn;
	my $PrintDROut = $CurrentDROut;
	
	$CurrentDRIn =~ s/_//g; # Remove break symbols which are only present for easier reading of binary values
	$CurrentDROut =~ s/_//g;
	
	my $Reglen = length($CurrentDRIn);
	
	while (length($CurrentDRIn) < 32) {
		$CurrentDRIn = "0" . $CurrentDRIn;
	}
	$LatchedData = unpack("N", pack("B32",$CurrentDRIn));
	while (length($CurrentDROut) < 32) {
		$CurrentDROut = "0" . $CurrentDROut;
	}
	$OutputData = unpack("N", pack("B32",$CurrentDROut));
	#printf "Data: %x\n", $LatchedData; 
	
	## Print out what have been seen
	printf "TDI: $PrintDRIn %x\nTDO: $PrintDROut %x\n", $LatchedData, $OutputData;
	
	## Test for Access on OCD Registers
	if ($LatchedInstruction == 11) {
		if (($Reglen == 5) && !($LatchedData & 0x10)) { # this is prelatching of read access to register
			#print "Prelatch $LatchedData\n";
			$OCDPrelatchReg = $LatchedData;
		}
		elsif (($Reglen == 16)) {
			printf "AVR Debug Access:\t\t$OCDRegisters{$OCDPrelatchReg} is %x " . GetOCDRegisterNameString($OCDPrelatchReg,$OutputData) . "\n", $OutputData;
		}
		elsif (($Reglen == 21)) {
			# Check whether this is also read access for the prelatched register
			my $Regid = ($LatchedData>>16) & 0xF;
			
			if ($Regid == $OCDPrelatchReg) {
				# This is also read access for before latched register
				printf "AVR Debug Access:\t\t$OCDRegisters{$OCDPrelatchReg} is %x " . GetOCDRegisterNameString($OCDPrelatchReg,($OutputData & 0xFFFF)) . "\n", ($OutputData & 0xFFFF);
			}
		
			if ($LatchedData & 0x100000) { 
				## This is write access to one of the registers 
				printf "AVR Debug Access:\t\t$OCDRegisters{$Regid} set to %x " . GetOCDRegisterNameString($Regid,($LatchedData & 0xFFFF)) . "\n", ($LatchedData & 0xFFFF);
			}
		}
		#print "Len: $Reglen\n";
	}
}

sub ProcessState_CaptureIR {
	$CurrentIR = "";
}

sub ProcessState_ShiftIR {
	my $TDO = shift;
	my $TDI = shift;
	
	$CurrentIR = "$TDI" . $CurrentIR;
}

sub ProcessState_UpdateIR {
	## Reset the prelatching of OCD Register Read
	$OCDPrelatchReg = 0xFF;

	$LatchedInstruction = ord(pack('B8', "0000$CurrentIR"));
	print "\nJTAG IR: $JTAGInstructions{$LatchedInstruction}\n";
	
	# When the instruction was "AVR Run" reset the debug register set value tracking
	if ($LatchedInstruction == 9) {
		for (my $i = 0; $i < 32; $i++) {
			$Registerfile[$i] = -1;
		}
	}
}

sub LoadProcessorSymbolicFile {
	my $filename = shift;
	
	open (SBF, "< $filename") or die ("Error opening processor symbolic file!");
	
	my $lin = "";
	while ($lin = <SBF>)  {
		if ($lin =~ /^([0-9a-fA-F]+);(.*);(.*);(.*);(.*);(.*);(.*);(.*);(.*);(.*);$/) {
			$Regnames{hex($1)} = $2;
			my @Bitnamesloc = ( $3, $4 , $5, $6, $7, $8, $9, $10 );
			$Bitnames{hex($1)} = \@Bitnamesloc;
		}
	
	}
	
	$Symbolfile = 1;
	close SBF;
}

sub PostprocessDisassembler {
	my $asm = shift;
	
	if ($Symbolfile == 1) {
		#print "Symbolfile found: \"$asm\"\n";
		## Write registernames to in and out instruction
		if ($asm =~ /^out\s0x([0-9A-Fa-f]{2}),\sr([0-9]{1,2}).*$/i) {
			if (exists($Regnames{hex($1)})) {
				# When there is a value in value tracking display matching bitnames
				if ($Registerfile[int($2)] >= 0) {
					$asm = "out $Regnames{hex($1)} (0x$1), r$2\t; $Regnames{hex($1)} = " . GetBitnameString(hex($1),$Registerfile[int($2)]);
				}
				else {
					$asm = "out $Regnames{hex($1)} (0x$1), r$2";
				}
			}
		}
		elsif ($asm =~ /^st\s(X|Y|Z),\sr([0-9]{1,2}).*/) {
			# check whether the associated address registers are both in value tracking
			my $address = -1;
			
			if ($1 eq "X") {
				if ($Registerfile[26] != -1 && $Registerfile[27] != -1) {
					$address = ($Registerfile[27] * 0x100) | $Registerfile[26]; 
				}
			}
			elsif ($1 eq "Y") {
				if ($Registerfile[28] != -1 && $Registerfile[29] != -1) {
					$address = ($Registerfile[29] * 0x100) | $Registerfile[28]; 
				}
			}
			elsif ($1 eq "Z") {
				if ($Registerfile[30] != -1 && $Registerfile[31] != -1) {
					$address = ($Registerfile[31] * 0x100) | $Registerfile[30]; 
				}
			}
			
			# when there is a valid address check whether the register is named
			if ($address >= 0) {
				$address -= 0x20;
				
				if (exists($Regnames{$address})) {
					$asm = $asm . "\t\t; " . $Regnames{$address} . " = ";
					
					if ($Registerfile[int($2)] != -1) {
						$asm = $asm . GetBitnameString($address,$Registerfile[int($2)]);
					}
					else {
						$asm = $asm . "r$2";
					}
				}
				else {
					if ($Registerfile[int($2)] != -1) {
						$asm = sprintf("%s\t\t; %x = %x", $asm, $address, $Registerfile[int($2)]);
					}
					else {
						$asm = sprintf("%s\t\t; %x = ?", $asm, $address);
					}
				}
			
			}
		}
	}
	
	if ($asm =~ /^in\sr([0-9]{1,2}),\s0x([0-9A-Fa-f]{2}).*$/i) {
		# Remove from valuetracking because it has not known number
		$Registerfile[int($1)] = -1;
		
		if (exists($Regnames{hex($2)})) {
			$asm = "in r$1, $Regnames{hex($2)} (0x$2)";
		}
	}
	
	# Value Tracking for values which are set while debugging session
	if ($asm =~ /^ldi\sr([0-9]{1,2}),\s0x([0-9a-fA-F]{2}).*$/) {
		# Assign the value to internal value tracking system
		#print "Valuetracking: $1 = $2\n";
		$Registerfile[int($1)] = hex($2);
	}
	
	if ($asm  =~ /^mov\sr([0-9]{1,2}),\s([0-9]{1,2}).*/) {
		$Registerfile[int($1)] = $Registerfile[int($2)];
	}
	
	return $asm;
}

sub GetBitnameString {
	my $reg = shift;
	my $val = shift;
	
	my $str = "";
	my $i;
	my $mask = 1;
	
	for ($i = 0; $i < 8; $i++) {
		if ($val & $mask) {
			if (exists($Bitnames{$reg}) && $Bitnames{$reg}->[$i] ne "") {
				$str = $str . " | " . $Bitnames{$reg}->[$i];
			}
		}
		$mask = $mask * 2;
	}
	
	$str =~ s/ \| //; # Remove first |
	if ($str eq "") {
		$str = sprintf("0x%x",$val);
	}
	return $str;
}

sub GetOCDRegisterNameString {
	my $reg = shift;
	my $val = shift;
	
	my $str = "";
	my $i;
	my $mask = 1;
	
	if (not(exists($OCDBitnames{$reg}))) {
		return "";
	}
	
	for ($i = 0; $i < 16; $i++) {
		if ($val & $mask) {
			$str = $str . " | " . $OCDBitnames{$reg}->[$i];
		}
		$mask = $mask * 2;
	}
	
	$str =~ s/ \| //; # Remove first |
	return "( " . $str . " )";
}
	
