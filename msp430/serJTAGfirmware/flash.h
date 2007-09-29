#ifndef FLASH_H
#define FLASH_H

//Function prototypes for the functions in flash.S
//part of serJTAGfirmware
//http://mspgcc.sf.net
//chris <cliechti@gmx.net>

word flashWriteDW(void *address, word data);                            //Write a word (16 bits) to the flash memory.
void flashWriteBlock(void *sourceaddr, void *destaddr, word numbytes);  //write a complete block
void flashErase(void *address);                                         //erase the block containing the address

#endif /*FLASH_H*/
