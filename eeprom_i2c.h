#ifndef _EEPROM_I2C_
#define _EEPROM_I2C_

// *****************************************************************
// ****                     EEPROM DEFINES                      ****
// *****************************************************************

#define EEPROM_WR 0xA0
#define EEPROM_RD 0xA1
#define PAGE_SIZE 16
#define PAGE_NUMBER 256
#define MAX_LENGTH 64
#define CYPHER_LEN 4

extern char xdata cypher[CYPHER_LEN];


// *****************************************************************
// ****                     EEPROM WRITE FUNCTION              	****
// *****************************************************************
int eeprom_write(char xdata* wdata, unsigned char len, unsigned char addr);


// *****************************************************************
// ****                    EEPROM READ FUNCTION                 ****
// *****************************************************************
int eeprom_read(char xdata* rdata, unsigned char len, unsigned char addr);


#endif