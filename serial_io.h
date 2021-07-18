#ifndef _SERIAL_IO_
#define _SERIAL_IO_

#define LEN 64

// *****************************************************************
// ****                       UART FIFO STRUCT                  ****
// *****************************************************************

typedef struct Fifo
{
	unsigned char buffer[LEN];
	unsigned char start;
	unsigned char end;
	unsigned char len;
} fifo_t;


// *****************************************************************
// ****                     UART FUNCTIONS                      ****
// *****************************************************************

void SerialIO_init(void);
bit get_number(int* cmd);

#endif
