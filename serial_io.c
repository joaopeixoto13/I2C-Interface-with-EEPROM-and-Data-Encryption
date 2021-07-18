#pragma large
#include "serial_io.h"
#include <REG51F380.H>
#define RI1 0
#define TI1 1
#define ES1 1
#define ERROR -1

fifo_t xdata Rx;
fifo_t xdata Tx;

bit TxActive = 0;

// *****************************************************************
// ****                         UART INIT                       ****
// *****************************************************************

void SerialIO_init(void)
{
	Rx.start = 0;
	Rx.end = 0;
	Rx.len = 0;
	Tx.start = 0;
	Tx.end = 0;
	Tx.len = 0;
	TxActive = 0;
}

// *****************************************************************
// ****                     GETKEY() FUNCTION                   ****
// *****************************************************************

#pragma disable
int _getkey()
{
	if (!Rx.len)
		return ERROR;	
	Rx.len--;
	return (Rx.buffer[Rx.start++ & (LEN-1)]);	
}


// *****************************************************************
// ****              UART 1 INTERRUPT SERVICE ROUTINE           ****
// *****************************************************************

void device_driver_isr(void) interrupt 16 using 2
{
	if (SCON1 & (1<<RI1)) 																		// RI1 = 1 ?
	{
		if (Rx.len < LEN) {																		// se não estiver cheio
			Rx.buffer[Rx.end++ & (LEN-1)] = SBUF1;												// colocar no buffer de receção o conteúdo de SBUF1
			Rx.len++;
		}
		SCON1 &= ~(1<<RI1);																		// limpar flag RI1
	}
	if (SCON1 & (1<<TI1))																		// TI1 = 1 ?
	{	
		SCON1 &= ~(1<<TI1);																		// limpar flag TI1
		if (Tx.len) {																			// se não estiver vazio  
			SBUF1 = Tx.buffer[Tx.start++ & (LEN-1)];											// colocar em SBUF1 elemento do buffer de transmissão
			Tx.len--;
		}
		else
			TxActive = 0;																		// sendactive = 0 -> não tem nada no buffer para transmitir									
	}
}


// *****************************************************************
// ****                     PUTCHAR() FUNCTION                  ****
// *****************************************************************

char putchar(char c)
{
	while(Tx.len >= LEN);																		// enquanto estiver cheio -> espera até esvaziar 
	if (!TxActive)																				// se não tiver nada para transmitir 
	{
		SBUF1 = c;																				// coloca em SBUF1 o caractere 
		TxActive = 1;																			// flag sendactive = 1
	}
	else
	{
		EA = 0;													
		Tx.buffer[Tx.end++ & (LEN-1)] = c;														// coloca o caractere no buffer de transmissão
		Tx.len++;
		EA = 1;														
	}
	return (c);
}

// *****************************************************************
// ****                TRY_RECEIVE_MESSAGE FUNCTION             ****
// *****************************************************************

bit try_receive_message()
{   
	bit RXactive = 0;
    char i;
    if(Rx.len >= LEN)                                        									//Se esta cheio de lixo
        SerialIO_init();
    for(i = 0; i < Rx.len && RXactive == 0 ; i++)
        RXactive = ~((bit)(Rx.buffer[(Rx.start + i)  & (LEN-1)] ^ 0x0A));
    return RXactive;
}

// *****************************************************************
// ****                   GET_NUMBER() FUNCTION                 ****
// *****************************************************************

bit get_number(int* cmd)
{ 
	char i;
	char temp[2];
	temp[0] = temp[1] = 13;                         
							
	if(!try_receive_message()) 																	//Esperar o imput do numero
		return 0;
	
	for(i = 0;Rx.buffer[Rx.start & (LEN - 1)] != '\r'; i++)
	{																							//Ler o numero
		if(i < 2)
			temp[i] = (_getkey() - 48);
		else
			_getkey();
	}
	_getkey();       
	_getkey();	

	if(temp[1] == 13){                                                              			//Caso seja apenas um digito troca as posições
		temp[1] = temp[0];																												
		temp[0] = 0;																															
	}				
	if(temp[0] < 0 || temp[0] > 9 || temp[1] < 0 || temp[1] > 9 )								//Caso seja um numero invalido
		return 0;		
	
	*cmd = temp[0]*10 + temp[1];															

	return 1;
}