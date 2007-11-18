/*
Demoapplication for libvscopedevice
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
#include "../../lib/vscopedevice.h" 

#define VALUES 1000

#define BYTE unsigned char

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

int main (int argc,char **argv)
{
  VScope *vscope;
  char buf[VALUES];
  
  vscope = openVScope();

  //ActivateEdgeTrigger(vscope,1,LOW);
  ActivatePatternTrigger(vscope,0x0C);
  //DeActivateTrigger(vscope);
  
  Recording(vscope,SAMPLERATE_100US,VALUES,buf); 
  
  //RecordingInternal(vscope,SAMPLERATE_100US); 
  //GetRecordInternal(vscope,buf,1000);
  
  FILE *datei;
  datei = fopen("vscope.vcd", "w");
  fprintf (datei, "$date\n");
  fprintf (datei, "\tMon Jun 15 17:13:54 1998\n"); 
  fprintf (datei, "$end");
  fprintf (datei, "$version\n");
  fprintf (datei, "Chronologic Simulation VCS version 4.0.3\n");
  fprintf (datei, "$end\n");

  fprintf (datei, "$timescale\n");
  fprintf (datei, "\t1ns\n");      
  fprintf (datei, "$end\n");
  fprintf (datei, "$scope module vscope $end\n");
  fprintf (datei, "$var wire       1 !    channel1 $end\n");
  fprintf (datei, "$var wire       1 *    channel2 $end\n");
  fprintf (datei, "$var wire       1 $    channel3 $end\n");
  fprintf (datei, "$var wire       1 (    channel4 $end\n");
  fprintf (datei, "$var wire       1 )    channel5 $end\n");
  fprintf (datei, "$var wire       1 ?    channel6 $end\n");
  fprintf (datei, "$var wire       1 =    channel7 $end\n");
  fprintf (datei, "$var wire       1 +    channel8 $end\n");
  fprintf (datei, "$upscope $end\n");
  fprintf (datei, "$enddefinitions $end\n");

  char sign;
  int i,s=0;
  for(i=0;i<VALUES;i++)
  {
    sign=buf[i];
    fprintf(datei,"#%i\n%i!\n%i*\n%i$\n%i(\n%i)\n%i?\n%i=\n%i+\n",s*2500
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

  printf("Messungen: %x\n",s);
  fclose(datei);
  return 0;
}	

