#pragma large
#include "eeprom_i2c.h"
#include "i2c_bitbang.h"
#include "errno.h"

char xdata cypher[CYPHER_LEN] = {'A', 'A', 'A', 'A'};								// Default Cypher Key

// *****************************************************************
// ****                     CYPHER ALGORITHM                    ****
// *****************************************************************


void encrypt_msg(char xdata* buf, unsigned char buf_len)
{
	unsigned char i;
	for (i = 0; i < buf_len; i++)
		buf[i] ^= cypher[i % CYPHER_LEN];											// Fill and encrypt the buffer with the cypher key
}


// *****************************************************************
// ****                     EEPROM WRITE FUNCTION              	****
// *****************************************************************

int eeprom_write(char xdata* wdata, unsigned char len, unsigned char addr)
{
	int offset, i = 1;
	unsigned int delay;															
	char j = 1, k = 0;
	bit flag = 1;
	int actual_pos, ret = 0;
	char tmp_trfr_len = 0;															// Temporary transfer len
	char xdata aux[64];																
	i2c_transfer_t xdata tmp;
	
	offset = addr + len - 1;														// Calcule final offset
	
	if ((offset > (PAGE_SIZE * PAGE_NUMBER)) || len > MAX_LENGTH)					// Check overload									
		return -EPERM;
	
	for (i = addr; i <= offset; i++, k++) {											// Check if it fits on a page
		if (((i % PAGE_SIZE) == 0 && k == 0) && len > PAGE_SIZE) {					// If is in the First Position on the page and len > PAGE_SIZE -> not fit										 			
			flag = 0; break; 
		}
		else if (((i % PAGE_SIZE) == 0 && k == 0) && len <= PAGE_SIZE) {			// If is in the First Positionon the page and len <= PAGE_SIZE -> fits					
			flag = 1; break;
		}
		if (i % PAGE_SIZE == 0)	{													// When through a multiple of PAGE_SIZE -> not fit
			flag = 0; break;
		}
	}
	if (flag) {																		// If fits on a single page
		tmp.byte0 = EEPROM_WR;														// Byte0 = EEPROM_WR
		tmp.len = len + 1;															// Transfer len 					
		tmp.byte_count = 0;															// Reset Byte Count
		tmp.payload = wdata;														// Update Payload for write operation
		encrypt_msg(wdata, len+1);													// Encrypt the message
		wdata[0] = addr;															// Put on 0 the Word Adress
		ret = submit_transfer(&tmp);												// Submit the transfer
		return (ret - 1);															// Return the number of bytes written successfuly
	}
	
	actual_pos = (addr & ((PAGE_SIZE * PAGE_NUMBER) - 1));							// Calcule actual position
	k = 0;																			// Reset 'k' for array index
	
	while (len)																		// While len exist -> transfers to do
	{
		if (k == 0) {																// First Iteration
			if (actual_pos % PAGE_SIZE != 0) {										// If is in the middle
				i = actual_pos;
				while (i++ % PAGE_SIZE != 0)										// While not found the margin page							
					tmp_trfr_len++;													// Calcule transfer len
			}
			else
				tmp_trfr_len = PAGE_SIZE;											// Transfer len = PAGE_SIZE
		}
		
		else if (len > PAGE_SIZE)													// If not fit on a page
			tmp_trfr_len = PAGE_SIZE;												// Transfer len = 8
		else 
			tmp_trfr_len = len;														// If fit -> Transfer len = len
		
		len -= tmp_trfr_len;														// Update len
		
		tmp.byte0 = EEPROM_WR;														// Byte 0 = EEPROM_WR
		tmp.len = tmp_trfr_len + 1;													// Len transfer
		tmp.byte_count = 0;															// Reset Byte Count for i2c
		
		for (; j <= tmp_trfr_len + k; j++) { 														
			aux[j-k] = wdata[j];													// Fill aux with the transfers caracteres
			aux[j-k] ^= cypher[(j-k) % CYPHER_LEN];									// Encrypt aux array
		}
	
		aux[0] = actual_pos;														// First Byte on payload = WORD_ADRRESS
		k += tmp_trfr_len;															// Update 'k' for array index
		actual_pos += tmp_trfr_len;													// Update actual position
			
		tmp.payload = aux;															// Fill payload

		delay = 50000;																// Delay for memmory -> this delay must exist to work on 24LC08B 
		while (--delay);															
		
		ret += (submit_transfer(&tmp) - 1);											// Submit the transfer and Count the number of bytes that write successfuly									
	}
	
	return ret;																		// Return the number of bytes that write successfuly
}


// *****************************************************************
// ****                    EEPROM READ FUNCTION                 ****
// *****************************************************************

int eeprom_read(char xdata* rdata, unsigned char len, unsigned char addr)
{
	int offset, ret;
	i2c_transfer_t xdata tmp;														// I2C_TRANSFER tmp
	
	offset = addr + len - 1;														// Calcule final offset
	
	if (offset > (PAGE_SIZE * PAGE_NUMBER))											// Check overload
		return -EPERM;
	
	tmp.byte0 = EEPROM_RD;															// Byte0 = EEPROM_RD
	tmp.len = len;																	// Upadte len transfer			
	tmp.byte_count = 0;																// Reset Byte Count
	tmp.payload = rdata;															// Update Payload for read operation
	rdata[0] = addr;																// Put on 0 the Word Adress 
	
	ret = submit_transfer(&tmp);													// Submit the transfer and Count the number of bytes that read successfuly
	
	encrypt_msg(rdata, len);														// Decrypt the message received
	
	return ret;																		// Return the number of bytes that read successfuly 
}
