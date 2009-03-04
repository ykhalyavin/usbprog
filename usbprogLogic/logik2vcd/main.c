/*
Demoapplication for liblogicdevice
Copyright (C) 2006 Benedikt Sauter <sauter@ixbat.de>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>


#include "logic.h"


#define TYPE_INTERN   0
#define TYPE_ONLINE   1
#define TYPE_SNAPSHOT 2

#define BYTE unsigned char

struct globalArgs_t {
	int triggertype;			
	int recordtype;			// 0 = intern, 1 = online, 2 = snapshot		
	char *filename;
	FILE *file;
	int channel;			/* channelnumbers */
	long numbers;			/* numbers of samples */
	char *samplerate;		/* # of input files */
	int samplerate_v;		/* # of input files */
	int triggervalue;		/* # of input files */
	int triggerignore;		/* # of input files */
	int append; 			// append data to given file
	int verbose; 			// show all you can 
} globalArgs;

char * progname;
Logic *logic;

static const char *optString = "f:R:T:c:t:i:s:n:vaq?h";

/* Display program usage, and exit.
 */

void display_usage( void )
{
	fprintf(stderr,
	"Usage: %s [options]\n"
	"Options:\n"
	"  -f <vcd-file>			Specify location of data file.\n\n"
	"  -R <record-type>		Specify the record type (online,intern or snapshot).\n"
	"  -T <trigger-type>		Activate Trigger.\n"
	"  -c <channel>		      	Specify channel for edge trigger (1-8).\n"
	"  -t <trigger-value>	      	0 or 1 for edgetrigger.\n"
	"			      	and the port state in hex at pattern-trigger.\n"
	"  -i <trigger-ignorevalue>    	Specify the channels which should be ignored at pattern trigger.\n"
	"  -s <samplerate>            	Specify samplerate 5us|10us|100us|1ms|10ms|100ms\n"
	"  -n <numbers>                	Number of values to sample.\n"
	"  -a                         	Add value to given file. Do not override given file.\n"
	"  -v                         	Verbose output. -v -v for more.\n"
	"  -q                         	Quell progress output. -q -q for less.\n"
	"  -?                         	Display this usage.\n"
	"\nlogic2vcd project: <URL:http://www.ixbat.de/logic2vcd>\n"
	,progname);

	exit( EXIT_FAILURE );
}


// for generate the vcd file
int Bit_Test(BYTE val, BYTE bit) {
	BYTE test_val = 0x01;    /* dezimal 1 / binaer 0000 0001 */
	/* Bit an entsprechende Pos. schieben */
	test_val = (test_val << bit);
	/* 0=Bit nicht gesetzt; 1=Bit gesetzt */
	if ((val & test_val) == 0)
		return 0;      /* Nicht gesetzt */
	else
		return 1;      /* gesetzt */
}


/* Convert the input files to HTML, governed by globalArgs.
 */
void logic2vcd( void )
{
	// check default values
	int errorrecordtype = 0;
	int errortriggertype = 0;
	int errorsamplerate = 0;
	int errornumbers = 0;
	int errorchannel = 0;
	int errorfilename = 0;
	int errors = 0;

	//long values = 0;

	// recordtype
	if((globalArgs.recordtype != TYPE_INTERN) && (globalArgs.recordtype > TYPE_SNAPSHOT))
		errorrecordtype = TYPE_ONLINE;

	// triggertype
	if((globalArgs.triggertype!=TRIGGER_OFF) && (globalArgs.triggertype!=TRIGGER_EDGE)
			&& (globalArgs.triggertype!=TRIGGER_PATTERN))
		errortriggertype = 1;


	// samplerate
	if(strcmp( globalArgs.samplerate, "5us" ) == 0)
		globalArgs.samplerate_v = SAMPLERATE_5US;
	else if (strcmp( globalArgs.samplerate, "10us" ) == 0)
		globalArgs.samplerate_v = SAMPLERATE_10US;
	else if (strcmp( globalArgs.samplerate, "50us" ) == 0)
		globalArgs.samplerate_v = SAMPLERATE_50US;
	else if (strcmp( globalArgs.samplerate, "100us" ) == 0)
		globalArgs.samplerate_v = SAMPLERATE_100US;
	else if (strcmp( globalArgs.samplerate, "1ms" ) == 0)
		globalArgs.samplerate_v = SAMPLERATE_1MS;
	else if (strcmp( globalArgs.samplerate, "10ms" ) == 0)
		globalArgs.samplerate_v = SAMPLERATE_10MS;
	else if (strcmp( globalArgs.samplerate, "100ms" ) == 0)
		globalArgs.samplerate_v = SAMPLERATE_100MS;
	else
		errorsamplerate = 1;




	// Numbers
	if( globalArgs.numbers < 0 )
		errornumbers = 1;
	

	// channel
	if ( (globalArgs.channel < 1 ) && (globalArgs.channel > 8 ))
		errorchannel = 1;


	// filename 
	if(globalArgs.filename!=NULL)
	{
		if(strlen(globalArgs.filename)<=0 )
			errorfilename = 1;
	}
	else
		errorfilename = 1;
	

	if(errorrecordtype)
	{
		fprintf(stderr,"-R Unkown recordtype.\n Please use online, intern or snapshot.\n");
		errors++;
	}
	
	if(errortriggertype)
	{
		fprintf(stderr,"-T Unkown triggertype.\n Please use edge or pattern or remove \
				\n parameter if you doesn't need a trigger start condiditon.\n");
		errors++;
	}
		
	if(errorsamplerate)
	{
		fprintf(stderr,"-s Unkown samplerate.\n \
				\n Please use 5us,10us,50us,100us,1ms,10ms,100ms\n"); 
		errors++;
	}
		
	if(errornumbers)
	{
		fprintf(stderr,"-n To less number of values to sample.\n \
				\n Please give a number greater than 0\n"); 
		errors++;
	}

	if(errorchannel)
	{
		fprintf(stderr,"-c Unknown channel.\n \
				\n Use a channel between 1 and 8.\n"); 
		errors++;
	}

	if(errorfilename)
	{
		fprintf(stderr,"-f Specify location of data file.\n");
		errors++;
	}

	if(errors > 0)
		exit(0);	


	// open connection to device
	logic = openLogic();
	if(logic==0)
	{
		fprintf(stderr,"Device not found!\n");
		exit(0);	
	}


	// check trigger and switch on
	if(globalArgs.triggertype==TRIGGER_EDGE)
	{
		char value=0x00;
		if(globalArgs.triggervalue>0)
			value = 0x01;
		if(globalArgs.verbose)
			fprintf(stderr,"start edge trigger channel %i, value %i\n",
				globalArgs.channel,globalArgs.triggervalue);
		ActivateEdgeTrigger(logic,globalArgs.channel,value);
	}
	else if(globalArgs.triggertype==TRIGGER_PATTERN)
	{
		if(globalArgs.verbose)
			fprintf(stderr,"start pattern trigger value %i\n",
				globalArgs.triggervalue);
		ActivatePatternTrigger(logic,(char)globalArgs.triggervalue,(char)globalArgs.triggerignore);
	}
	else {
		if(globalArgs.verbose)
			fprintf(stderr,"deactivate trigger\n");
		DeActivateTrigger(logic);
	}

	char buf[globalArgs.numbers];

	if(globalArgs.recordtype == TYPE_INTERN)
	{
		if(globalArgs.verbose)
			fprintf(stderr,"Recording intern\n"); // TODO say if n > 1000


		if(globalArgs.verbose)
		  fprintf(stderr, "Starting recording...\n");

		RecordingInternal(logic,globalArgs.samplerate_v);

		if(globalArgs.verbose)
		  fprintf(stderr, "Getting internal recording...\n");

		GetRecordInternal(logic,buf,globalArgs.numbers,globalArgs.samplerate_v); //XXX
	}

	else if(globalArgs.recordtype == TYPE_ONLINE)
	{
		if(globalArgs.verbose)
			fprintf(stderr,"Recording online\n");

		Recording(logic,globalArgs.samplerate_v,globalArgs.numbers,buf);

		StopLogic(logic);

	}
	// snapshot - get one value
	else if(globalArgs.recordtype==TYPE_SNAPSHOT)
	{
		//if(globalArgs.verbose)
		//{
			fprintf(stderr,"Snapshot: comming ...\n");
			exit( EXIT_FAILURE ); //XXX
		//}
	}

	printf("session end reached\n");
	
	// write to file
	  // create file header
	if(globalArgs.append==0){
		// open with w file
		if(globalArgs.verbose)
			fprintf(stderr,"create new vcd file\n");

		globalArgs.file = fopen(globalArgs.filename, "w");
		fprintf (globalArgs.file, "$date\n");
  		fprintf (globalArgs.file, "\tMon Jun 15 17:13:54 1998\n");
  		fprintf (globalArgs.file, "$end");
  		fprintf (globalArgs.file, "$version\n");
  		fprintf (globalArgs.file, "Chronologic Simulation VCS version 4.0.3\n");
  		fprintf (globalArgs.file, "$end\n");

  		fprintf (globalArgs.file, "$timescale\n");
		fprintf (globalArgs.file, "\t1ns\n");
  		fprintf (globalArgs.file, "$end\n");
  		fprintf (globalArgs.file, "$scope module logic $end\n");
  		fprintf (globalArgs.file, "$var wire       1 !    channel1 $end\n");
  		fprintf (globalArgs.file, "$var wire       1 *    channel2 $end\n");
  		fprintf (globalArgs.file, "$var wire       1 $    channel3 $end\n");
  		fprintf (globalArgs.file, "$var wire       1 (    channel4 $end\n");
  		fprintf (globalArgs.file, "$var wire       1 )    channel5 $end\n");
 		fprintf (globalArgs.file, "$var wire       1 ?    channel6 $end\n");
  		fprintf (globalArgs.file, "$var wire       1 =    channel7 $end\n");
  		fprintf (globalArgs.file, "$var wire       1 +    channel8 $end\n");
  		fprintf (globalArgs.file, "$upscope $end\n");
  		fprintf (globalArgs.file, "$enddefinitions $end\n");
		fclose(globalArgs.file);

		// close filehandle
	}

	// open filename with w+
	if(globalArgs.verbose)
		fprintf(stderr,"add data\n");
	globalArgs.file = fopen(globalArgs.filename, "a");
 
	char sign;
  	int i,s=0;
  	for(i=0;i<globalArgs.numbers;i++)
  	{
    		sign=buf[i];
		printf("%d\n", (int) sign);
    		fprintf(globalArgs.file,"#%i\n%i!\n%i*\n%i$\n%i(\n%i)\n%i?\n%i=\n%i+\n",s*2500
                    ,Bit_Test(sign, 0)?1:0
                    ,Bit_Test(sign, 1)?1:0
                    ,Bit_Test(sign, 2)?1:0
                    ,Bit_Test(sign, 3)?1:0
                    ,Bit_Test(sign, 4)?1:0
                    ,Bit_Test(sign, 5)?1:0
                    ,Bit_Test(sign, 6)?1:0
                    ,Bit_Test(sign, 7)?1:0);
    		s++;
  	}
	fclose(globalArgs.file);
	closeLogic(logic);
	
	// write data to file
	printf("Summary: values(%ld), samplerate(%s), file(%s)\n",
			globalArgs.numbers,globalArgs.samplerate,globalArgs.filename);
}

int main( int argc, char *argv[] )
{
	int opt = 0;
	//progname = rindex(argv[0],'/'); rindex() is obsolete
	progname = strrchr(argv[0],'/');
	if (progname)
	  progname++;
	 else
	  progname = argv[0];

	/* Initialize globalArgs before we get to work. */
	globalArgs.triggertype = TRIGGER_OFF;	
	globalArgs.recordtype = 0;	
	globalArgs.filename = NULL;
	globalArgs.file = NULL;
	globalArgs.channel = 1;
	globalArgs.numbers = 1000;	
	globalArgs.samplerate = "100us";
	globalArgs.triggervalue = 0x00;	
	globalArgs.append = 0;
	globalArgs.verbose = 0;

	/* Process the arguments with getopt(), then 
	 * populate globalArgs. 
	 */

	opt = getopt( argc, argv, optString );
	while( opt != -1 ) {
		switch( opt ) {
			case 'f':
				// filename
				globalArgs.filename = optarg;	/* true */
				break;
				
			case 'R':
				//globalArgs.langCode = optarg;
				if(strcmp( optarg, "intern" ) == 0)
				  globalArgs.recordtype = 0;
				else if (strcmp( optarg, "online")== 0)
				  globalArgs.recordtype = 1;
				else if (strcmp( optarg, "snapshot")== 0)
				  globalArgs.recordtype = 2;
				else
				  fprintf(stderr,"-R Unknown recordtype: %s \
						  \n Please user online,intern or snapshot.\n",optarg);
				break;
				
			case 'T':
				if(strcmp( optarg, "edge" ) == 0)
				  globalArgs.triggertype = TRIGGER_EDGE;
				else if (strcmp( optarg, "pattern")== 0)
				  globalArgs.triggertype = TRIGGER_PATTERN;
				else
				fprintf(stderr,"-T Unkown triggertype.\n Use edge,pattern or forget\
						\n parameter if you don't need a start trigger.\n");
				break;
			case 'c':
				globalArgs.channel = atoi(optarg);
				break;
			case 't':
				globalArgs.triggervalue = atoi(optarg);
				break;
			case 'i':
				globalArgs.triggerignore = atoi(optarg);
				break;
			case 's':
				globalArgs.samplerate=optarg;	
				break;
			case 'n':
				globalArgs.numbers = atoi(optarg);	
				break;
			case 'a':
				globalArgs.append=1;
				break;
			case 'v':
				globalArgs.verbose=1;
				break;
	
			case 'h':	/* fall-through is intentional */
			case '?':
				display_usage();
				break;
				
			default:
				/* You won't actually get here. */
				break;
		}
		
		opt = getopt( argc, argv, optString );
	}
	
//	globalArgs.inputFiles = argv + optind;
//	globalArgs.numInputFiles = argc - optind;

	logic2vcd();
	
	return EXIT_SUCCESS;
}
