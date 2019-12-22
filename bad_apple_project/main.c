#include	<msp430f5529.h>
#include "oled.h"
#include "bmp.h"

int cnt=0,flag=0;
unsigned char buffer[1024]={0}; //data buffer

int main(void)
{
	WDTCTL = WDTPW + WDTHOLD;

	clock_init();			//initialize the clock system
	OLED_Init();			//iniyialize the OLED
	UART_RS232_Init();  	// initialize the UART
	OLED_Clear();
	_EINT();
	UCA1TXBUF = 0xAA;
	//OLED_DrawBMP(0, 0, 128, 8, BMP1);
	OLED_ShowString(30,0,"Bad apple");
	OLED_ShowString(0,3,"Press the key");
	OLED_ShowString(16,5,"to display");
	while (1)
	{
		//UCA1TXBUF = 0xAA; //186985
		if(flag)
			{
				OLED_DrawBMP(0, 0, 128, 8, buffer);
				UCA1TXBUF = 0xAA;  						 //UART response to computer
				flag = 0;
			}
	}
}
/************************ISR********************************/

#pragma vector=USCI_A1_VECTOR
__interrupt void USCI_A1_ISR(void)
{
	switch (__even_in_range(UCA1IV, 4))
	{
	case 0:
		break;
	case 2:                                            	// receive interrupt
	{
		if (UCRXIFG)                                   	//wait for finish receive
		{
			buffer[cnt] = UCA1RXBUF;                  	// read the data
			if(cnt==1023)
			{
				cnt = 0;
				flag=1;
			}
			else
				cnt++;
		}
	}
		break;
	case 4:
		break;
	default:
		break;

	}
}

