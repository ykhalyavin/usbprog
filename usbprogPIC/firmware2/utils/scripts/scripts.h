/*****************************************************************************
* file: scripts.h
* description: Scripting functions headerfile
*	
* This file is part of the UsbprogPIC software, written by Nico van Leeuwen
* (c) 2008, www.liondesign.nl
* 
* Usbprog is a development of Benedikt Sauter, www.embedded-projects.net
******************************************************************************/

struct {						// Script table - keeps track of scripts in the Script Buffer.
	byte	Length;
	int	StartIndex;	// offset from uc_script_buffer[0] of beginning of script.
} ScriptTable[SCRIPT_ENTRIES];

extern void SendScriptChecksums(void);
extern void RunScript(byte scriptnumber, byte repeat);
extern void ClearScriptTable(void);
extern void StoreScriptInBuffer(byte *usbindex);
extern void ScriptEngine(byte *scriptstart_ptr, byte scriptlength);
