/* 
 * Benedikt Sauter <sauter@ixbat.de> 2006-04-10
 */

#include <string.h>
#include <stdlib.h>
#include <ctype.h> /* tolower(), toupper(), isalpha() */
#include <stdio.h>
#include <usb.h>
#include "../config.h"


void at89_reset(usb_dev_handle * usb_handle,char type);
void at89_erase(usb_dev_handle * usb_handle,char type);
void at89_upload(usb_dev_handle * usb_handle,char type,char *file);

   

