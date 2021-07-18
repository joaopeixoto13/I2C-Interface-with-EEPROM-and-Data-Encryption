#pragma large
#include <REG51F380.H>
#include "config_platform.h"

// *****************************************************************
// ****                     OSCILATOR INIT                      ****
// *****************************************************************

void Oscilator_init(void)
{
	 FLSCL     = 0x90;
     CLKSEL    = 0x03;		// SYSCLK a 48MHz
}

// *****************************************************************
// ****                       TIMER INIT                        ****
// *****************************************************************

void Timer_init(void)
{
	TMOD      = 0x02;		// Timer 0 8bit auto-reload
	CKCON     = 0x02;		// Prescaled Clock Input -> SYSCLK / 48
	
	TH0 = (-25);
	TL0 = (-25);			// T overflow = 25uS -> f = 40KHz
}

// *****************************************************************
// ****                        UART INIT                        ****
// *****************************************************************

void UART_Init()
{
	SBRLL1 = 0x30;
	SBRLH1 = 0xff;			// BaudRate a 115200 bps
	SCON1 = 0x10;			// Enable UART1 reception -> REN1
	SBCON1 = 0x43;
	EIE2 |= 0x02;			// Enable UART1 Interrupt	
}


// *****************************************************************
// ****                       PCA INIT                          ****
// *****************************************************************

void PCA_init(void)
{
	PCA0MD = 0;				// Disable Watch-Dog Timer
}

// *****************************************************************
// ****                      PORT_IO INIT                       ****
// *****************************************************************

void Port_IO_init(void)
{
    P0SKIP    = 0x0F;		// TxD e RxD a P0_4 e P0_5 
    XBR1      = 0x40;		// CrossBar Enable
    XBR2      = 0x01;		// UART1 Enable on CrossBar
}

// *****************************************************************
// ****                      DEVICE INIT                        ****
// *****************************************************************

void Device_init(void)
{
	Oscilator_init();		// Call Oscilator Init
	UART_Init();			// Call UART Init
	PCA_init();				// Call PCA Init
	Port_IO_init();			// Call Port_IO Init
	Timer_init();			// Call Timer Init
}
