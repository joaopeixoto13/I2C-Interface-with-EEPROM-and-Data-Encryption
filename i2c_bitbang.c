#pragma large
#include "i2c_bitbang.h"
#include "errno.h"
#include <REG51F380.H>

// *****************************************************************
// ****                   VARIABLES INITIALIZATION              ****
// *****************************************************************

volatile states_t data state, next_state;																	// State Initialize
volatile i2c_transfer_t xdata * xdata actual_transfer _at_ 0x100;											// Actual Transfer 
volatile i2c_transfer_t xdata dummy_transfer;																// Dummy Transfer

volatile i2c_transfer_buffer_t xdata * xdata i2c_transfer_buffer _at_ 0x200;								// I2C Transfer Buffer

volatile char count_bits = 8;																				// Count bits variable
volatile unsigned char tmp_byte;																			// Temporary byte 
volatile bit transfer_done = 0;																				// Bit that represents if a trasnfer is done

sbit SCL = P3^0;																							// SCL Pin on P3^0
sbit SDA = P3^1;																							// SDA Pin on P3^1


// *****************************************************************
// ****                     I2C_INIT FUNCTION                   ****
// *****************************************************************

void I2C_init(void)																							// Initialize I2C Buffer
{
	i2c_transfer_buffer->start = 0;																			// Start Buffer to 0
	i2c_transfer_buffer->end = 0;																			// End Buffer to 0
	i2c_transfer_buffer->len = 0;																			// Length Buffer to 0
	count_bits = 8;																							// Initialize 'count_bits'
	transfer_done = 1;																						// Initialize 'transfer_done'
}


// *****************************************************************
// ****                  SUBMIT_TRANSFER FUNCTION               ****
// *****************************************************************

int submit_transfer(i2c_transfer_t xdata *p_transfer)														// Function that submit one Transfer				
{
	if (!transfer_done)																						// If one transfer are in progress
		return -EBUSY;
	
	if (i2c_transfer_buffer->len >= I2C_TRANSFER_LEN)														// If buffer space is full
		return -ENOBUFS;
	
	if (p_transfer->byte0 & 1)																				// If is a Read Operation
	{
		dummy_transfer.byte0 = p_transfer->byte0;															
		dummy_transfer.byte0 &= ~(1<<0);																	// Fill Byte0
		dummy_transfer.len = 1;																				// Update transfer len
		dummy_transfer.byte_count = 0;																		// Reset Byte Count 
		dummy_transfer.payload = p_transfer->payload;														// Fill Payload
		i2c_transfer_buffer->buffer[i2c_transfer_buffer->end++ & (I2C_TRANSFER_LEN-1)] = &dummy_transfer;	// Add to I2C Buffer Transfer the Dummy Operation
		i2c_transfer_buffer->len++;																			// Increment Buffer Len
	}
	i2c_transfer_buffer->buffer[i2c_transfer_buffer->end++ & (I2C_TRANSFER_LEN-1)] = p_transfer;			// Add to I2C Transfer Buffer 
	i2c_transfer_buffer->len++;																				// Increment I2C Transfer Buffer
	
	count_bits = 8;																							// Reset Count Bits 
	next_state = Start_L;																					// Update next state
	SCL = 1;																								// Ensure the High Level
	SDA = 1;																								// Ensure the High Level
	ET0 = 1;																								// Ensure the High Level
	
	actual_transfer = i2c_transfer_buffer->buffer[i2c_transfer_buffer->start++ & (I2C_TRANSFER_LEN-1)];		// Update Actual Transfer
	tmp_byte = actual_transfer->byte0;																		// Update Temporary Byte
	
	transfer_done = 0;																						// Reset Transfer Done flag
	TR0 = 1;																								// Set TR0
	EA = 1;																									// Set EA
	
	while (!transfer_done);																					// While is on the transfer
	
	return actual_transfer->byte_count;																		// return number of bytes read / write successfuly
}


// *****************************************************************
// ****                     NEXT_TRANSFER FUNCTION              ****
// *****************************************************************

void next_transfer()																						// Initialize new Transfer
{
	actual_transfer = i2c_transfer_buffer->buffer[i2c_transfer_buffer->start++ & (I2C_TRANSFER_LEN-1)];		// Update Actual Transfer
	tmp_byte = actual_transfer->byte0;																		// Upadte temporary Byte
}


// *****************************************************************
// ****              TIMER 0 INTERRUPT SERVICE ROUTINE          ****
// *****************************************************************

void timer0_isr(void) interrupt 1 using 2
{	
	SCL ^= 1;																								// Switch SCL for State Machine
	state = next_state;																						// Update State
	
	switch(state)																							// Switch State
	{
		// **************** IDLE STATE *********************
		case Idle:
			TR0 = 0;																						// Stop BitBang Timer
			SDA = 1;																						// Ensure that SDA & SCL are in High Level
			SCL = 1;
			transfer_done = 1;																				// Reset flag
			break;
		
		
		// ************* START CONDITION *******************
		case Start_L:
			SDA = 1;																						// Ensure the SDA are in High Level to make the transition
			next_state = Start_H;
			break;
		
		case Start_H:
			SDA = 0;																						// Make the Descendent Transition
			tmp_byte = actual_transfer->byte0;																// Reload 'byte0' 
			next_state = Byte0_L;
			break;
		
		
		// ************** SLAVE ADDRESS ********************
		case Byte0_L:
			SDA = tmp_byte & (1<<7);																		// Grab the MSB bit		
			count_bits--;																					// Decrement 'count_bits'	
			next_state = Byte0_H;
			break;
		
		case Byte0_H:
			if (!count_bits) {																				// Byte complete 
				count_bits = 8;																				// Reload 'count_bits'
				next_state = RecvACK0_L;		
			}
			else {
				tmp_byte <<= 1;																				// Next MSB
				next_state = Byte0_L;
			}
			break;
			
			
		// ************** SLAVE ADDRESS ACK ****************
		case RecvACK0_L:
			SDA = 1;																						// Ensure SDA are High Level to slave pull-down 
			next_state = RecvACK0_H;
			break;
		
		case RecvACK0_H:
			if (!SDA) 																						// If slave pull-down the line -> ACK
			{																	
				if ((actual_transfer->byte0) & 0x01)														// Test Operation -> 1 - Read | 0 - Write
					next_state = RecvByte_L;																// Read Operation	
				else
					next_state = SendByte_L;																// Write Operation
				
				tmp_byte = actual_transfer->payload[actual_transfer->byte_count]; 							// Update Temporary byte
			}
			else																							// Something gonna wrong - NACK
				next_state = Stop_L;
			break;
			
			
		// ************** READ OPERATION ********************
		case RecvByte_L:	
			SDA = 1;																	
			next_state = RecvByte_H;
			count_bits--;																					// Decrement 'count_bits'
			break;
		
		case RecvByte_H:
			tmp_byte <<= 1;																					// Shift to fill the bits of 'n' byte in payload buffer
			tmp_byte |= SDA;																				// Fill the position bit with SDA value
			if (!count_bits)
			{
				count_bits = 8;																				// reload 'count_bits'	
				actual_transfer->payload[actual_transfer->byte_count] = tmp_byte;
				if (++actual_transfer->byte_count == actual_transfer->len)									// If all bytes are fill
					next_state = SendNACK_L;																// SendNack to finish 
				else												
					next_state = SendACK_L;																	// SendACK to continue to fill the others bytes
			}
			else 																							// If complete 8 bits transition
				next_state = RecvByte_L;
			break;
			
		
		// ************** WRITE OPERATION *******************
		case SendByte_L:
			SDA = tmp_byte & (1<<7);																		// Grab the MSB bit
			count_bits--;																					// Decrement 'count_bits'	
			next_state = SendByte_H;
			break;
		
		case SendByte_H:
			if (count_bits)																					// Byte not complete 
			{		
				tmp_byte <<= 1;																				// Shift for next iteration 	
				next_state = SendByte_L;
			}
			else
			{
				count_bits = 8;																				// Reload 'count_bits'
				next_state = RecvACK_L;
			}
			break;
			
			
		// **************** ACK OPERATIONS ******************
		case SendACK_L:
			SDA = 0;																						// Pull the line to zero
			next_state = SendACK_H;
			break;
		
		case SendACK_H:
			next_state = RecvByte_L;
			break;
		
		case RecvACK_L:
			SDA = 1;																						// Ensure SDA to slave pull-down 
			next_state = RecvACK_H;
			break;
		
		case RecvACK_H:
			if (!SDA)																						// If Slave pull-down the line to zero
			{
				if (++actual_transfer->byte_count == actual_transfer->len)									// If all bytes were written
				{	
					i2c_transfer_buffer->len--;
					if (i2c_transfer_buffer->len) 															// If has more transfers to do
					{ 								
						next_transfer();
						next_state = Start_L;	
					}
					else
						next_state = Stop_L;																// Generate Stop Condition
				}
				else
				{
					tmp_byte = actual_transfer->payload[actual_transfer->byte_count]; 						// Update Temporary byte
					next_state = SendByte_L;																// Continue to SendBytes
				}
			}										
			else	
				next_state = Stop_L;																		// Generate Stop Condition
			break;
			
			
		// **************** NACK OPERATIONS *****************
		case SendNACK_L:
			SDA = 1;																						// Ensure SDA is high level to pull-down 
			next_state = SendNACK_H;
			break;
		
		case SendNACK_H:
			if (--i2c_transfer_buffer->len) 																// If exist more Transfers to do - Write | Read
			{
				next_transfer();
				next_state = Start_L;
			}				
			else																							// If complete all Transfers
				next_state = Stop_L;
			break;
			
			
		// ***************** STOP CONDITION *****************
		case Stop_L:
			SDA = 0;
			next_state = Stop_H;
			break;
		
		case Stop_H:
			SDA = 1;																						// Make the Ascendent Transition																			 
			next_state = Idle;
			break;
	}
	TF0 = 0;																								// Ensure that TF0 is cleared
	
}