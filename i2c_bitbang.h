#ifndef _I2C_BITBANG_
#define _I2C_BITBANG_
#define I2C_TRANSFER_LEN 8

// *****************************************************************
// ****                    STATES ENUMERATION                   ****
// *****************************************************************

typedef enum STATES
{
	Idle = 0, 
	Start_L, Start_H, 													// Start State's
	Byte0_L, Byte0_H,													// Byte0 State's
	RecvACK0_L, RecvACK0_H,												// Receive ACK0 State's
	RecvByte_L, RecvByte_H,												// Receive Byte State's
	SendByte_L, SendByte_H,												// Send Byte State's
	SendACK_L, SendACK_H,												// Send ACK State's
	RecvACK_L, RecvACK_H,												// Receive ACK State's
	SendNACK_L, SendNACK_H,												// Send NACK State's
	Stop_L, Stop_H														// Stop State's
} states_t;


// *****************************************************************
// ****                     I2C_TRANSFER STRUCT                 ****
// *****************************************************************

typedef struct I2C_TRANSFER
{
	unsigned char byte0;												// Slave ADDR
	unsigned char len;													// Number of Bytes to Read/Write
	unsigned char byte_count;											// For Bytes iteration
	unsigned char xdata *payload;										// Buffer:  Write - read from here to write | Read - save here
} i2c_transfer_t;


// *****************************************************************
// ****                I2C_TRANSFER_BUFFER STRUCT               ****
// *****************************************************************

typedef struct I2C_TRANSFER_BUFFER
{
	i2c_transfer_t xdata* buffer[I2C_TRANSFER_LEN];						// I2C Transfers Buffer
	unsigned char start;												// Start Buffer
	unsigned char end;													// End Buffer
	unsigned char len;													// Buffer Length
} i2c_transfer_buffer_t;


// *****************************************************************
// ****                   I2C_BITBANG FUNCTIONS                 ****
// *****************************************************************

void I2C_init(void);													// Init I2C
int submit_transfer(struct I2C_TRANSFER xdata *transfer);				// Submit a transfer


#endif