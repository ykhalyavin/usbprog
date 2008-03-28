#!/usr/bin/perl

# Diese Datei dient der Auswertung von mit einem Logic Analyzer ausgezeichnetem JTAG Traffic. 

## Definitionen

## JTAG Statemachine 

@JTAGStates = (
	"TEST LOGIC RESET",	#0
	"RUN TEST / IDLE", 	#1 
	"SELECT DR SCAN",  	#2
	"SELECT IR SCAN",  	#3
	"CAPTURE DR",		#4
	"SHIFT DR",			#5
	"EXIT1 DR",			#6	
	"PAUSE DR",			#7
	"EXIT2 DR",			#8
	"UPDATE DR",		#9
	"CAPTURE IR",		#10
	"SHIFT IR",			#11
	"EXIT1 IR",			#12
	"PAUSE IR",			#13
	"EXIT2 IR",			#14
	"UPDATE IR"			#15
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
	2 => "PSMSB",
	3 => "PDSB",
	8 => "BCR",
	9 => "BSR",
	12 => "OCDR",
	13 => "OCDCSR"
);

$JTAGStart = 1; # Der Startzustand ist TEST RUN / IDLE

## /Definitionen


## INFO
# JTAG Clock Idles at high state. Data is latched at time of the rising edge and updated at time of the falling edge
# Deshalb wird die JTAG Statemachine immer bei der steigenden Flanke getriggert, suche also nach steigenden Flanken in TCK
# Lese nun alle Zeilen aus der Eingabe Datei, öffne Ausgabe Datei und triggere bei steigenden Flanken die State Machine.

## /INFO

## init
$| = 1;
if ($#ARGV != 0) {
	print "Usage: Auswertung.pl <filename>\n";
	exit;
}
my $filename = $ARGV[0];
my $JtagState = $JTAGStart;
my $TDI = 0;
my $TDO = 0;
my $TMS = 0;
my $oldTCK = 0;

$CurrentIR = "";
$CurrentDRIn = "";
$CurrentDROut = "";

$LatchedInstruction = 0;
$LatchedData = 0;
$OutputData = 0;

$OCDPrelatchReg = 0;


## /init

open (INPUT, "< $filename") or die("Cannot open given file");
open (OUTPUT, "> $filename.out") or die("Cannot open an output file");


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
			
			#print "TDI: $TDI\nTDO: $TDI\nNew JTAG State: $JTAGStates[$JtagState]\n";
		
		}
		
		$oldTCK = int($2);
	}


}

close INPUT;
close OUTPUT;

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
		print "AVR Instruction: " . $disasm . "\n";
	}
}

sub ProcessState_CaptureDR {
	$CurrentDRIn = "";
	$CurrentDROut = "";
}

sub ProcessState_ShiftDR {
	my $TDO = shift;
	my $TDI = shift;
	
	$CurrentDRIn = "$TDI" . $CurrentDRIn;
	$CurrentDROut = "$TDO" . $CurrentDROut;
}

sub ProcessState_UpdateDR {
	print "TDI: $CurrentDRIn\nTDO: $CurrentDROut\n";
	
	my $Reglen = length($CurrentDRIn);
	
	while (length($CurrentDRIn) < 32) {
		$CurrentDRIn = "0" . $CurrentDRIn;
	}
	$LatchedData = unpack("N", pack("B32",$CurrentDRIn));
	while (length($CurrentDROut) < 32) {
		$CurrentDROut = "0" . $CurrentDROut;
	}
	$OutputData = unpack("N", pack("B32",$CurrentDROut));
	printf "Data: %x\n", $LatchedData; 
	
	## Test for Access on OCD Registers
	if ($LatchedInstruction == 11) {
		if (($Reglen == 5) && !($LatchedData & 0x10)) { # this is prelatching of read access to register
			print "Prelatch $LatchedData\n";
			$OCDPrelatchReg = $LatchedData;
		}
		elsif (($Reglen == 16)) {
			printf "$OCDRegisters{$OCDPrelatchReg} reads as %x\n", $OutputData;
		}
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
	$LatchedInstruction = ord(pack('B8', "0000$CurrentIR"));
	print "JTAG Instruction: $JTAGInstructions{$LatchedInstruction}\n";
}

	
