#pragma large																				// Variables in XDATA
#include <REG51F380.H>																		// Include 8051 Registers
#include "config_platform.h"																// Include Config Platform Module
#include "serial_io.h"																		// Include Serial_IO / UART Module
#include "eeprom_i2c.h"																		// Include EEPROM Module
#include "i2c_bitbang.h"																	// Include I2C_BitBang Module
#include <stdio.h>																			// Include I/O Module

char xdata* xdata rdata _at_ 0x500;															// Write Buffer
char xdata* xdata wdata _at_ 0x600;															// Read Buffer

// *****************************************************************
// ****                     MAIN FUNCTION                       ****
// *****************************************************************

void main(void)																				
{	
	int number, addr, i, ret, cmd;															// Variables
	Device_init();																			// Init Device
	SerialIO_init();																		// Init UART		
	I2C_init();																				// Init I2C
	EA = 1;																					// Enable Geral Interrupt-
	
	while(1)																				// Main Loop
	{
		if (get_number(&cmd))																// if received a char via UART
		{
			switch(cmd)
			{
				case 1:																		// WRITE MEMMORY									
					printf("\r\nADDR to write: ");
					while (!get_number(&cmd));													
					addr = cmd;																// Update ADDR
					printf("\r\nInsert the number of bytes you are going to write: ");
					while (!get_number(&cmd));
					number = cmd;															// Update number of bytes that we gonna write
					for(i = 0; i < number; i++) {
						printf("\r\n%dº byte: ", i+1);
						while (!get_number(&cmd));
						wdata[i+1] = cmd;													// Fill Array
					}
					ret = eeprom_write(wdata, number, addr);								// Write on EEPROM
					if (ret < 0)															// If occurred an error
						printf("\r\nError!");												// Print Error message
					else
						printf("\r\n%d bytes written successfully!\r\n", ret);					// Print Number of bytes written successfuly
					break;
				case 2:																		// READ MEMMORY
					printf("\r\nADDR to read: ");
					while (!get_number(&cmd));
					addr = cmd;																// Update ADDR
					printf("\r\nInsert the number of bytes you are going to read: ");
					while (!get_number(&cmd));
					number = cmd;															// Update number of bytes that we gonna read
					ret = eeprom_read(rdata, number, addr);									// Read on EEPROM
					if (ret < 0)															// If occurred an error
						printf("\r\nError!");												// Print Error message
					else {
						for (i = addr; i < number + addr; i++)
							printf("\r\n%d", (int)rdata[i-addr]);							// Print the bytes readed
					}
					printf("\r\n");
					break;
				case 3:																		// CHANGE CYPHER KEY
					printf("\r\nInsert the new Cypher Key: ");
					for (i = 0; i < CYPHER_LEN; i++) {
						printf("\r\n%d Byte: ", i+1);
						while (!get_number(&cmd));
						cypher[i] = (char)cmd;												// Fill Cypher Key
					}
					printf("\r\nCypher key has been updated!\r\n");								
					break;
				default:
					printf("\r\nError: Invalid Option!\r\n");
			}
		}
	}
	
}

