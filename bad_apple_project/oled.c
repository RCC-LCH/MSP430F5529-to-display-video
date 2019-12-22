#include "oled.h"

//#include "stdlib.h"
#include "oledfont.h"  	 
//#include "delay.h"
//OLED的显存
//存放格式如下.
//[0]0 1 2 3 ... 127	
//[1]0 1 2 3 ... 127	
//[2]0 1 2 3 ... 127	
//[3]0 1 2 3 ... 127	
//[4]0 1 2 3 ... 127	
//[5]0 1 2 3 ... 127	
//[6]0 1 2 3 ... 127	
//[7]0 1 2 3 ... 127
void Timer_init()
{
	TA0CTL |= MC_1 + TASSEL__ACLK + TACLR;     	//时钟为SMCLK,比较模式，开始时清零计数器
	TA0CCTL0 = CCIE;						//比较器中断使能
	TA0CCR0 = 18173;		//18173Hz
}

void UART_RS232_Init(void)
{

	   P4SEL |= BIT5+BIT4;                       // P4.4,5 = UCA1 TXD/RXD
	   //UCA1CTL0 |= UCSPB ;//+ UCMSB;             // set two stop bit, MSB first
	   UCA1CTL1 |= UCSWRST;                      // reset USCI
	   UCA1CTL1 |= UCSSEL__SMCLK;                // select SMCLK
	   UCA1BR0 = 100;                            // bps= 185185
	   UCA1BR1 = 0;
	   UCA1MCTL |= UCBRS_0 + UCBRF_0;
	   UCA1CTL1 &= ~UCSWRST;                     // stop reset
	   UCA1IE |= UCRXIE;                         // enable interrupt

}

void clock_init()
{
	/*P1DIR |= BIT0;                            // ACLK set out to pins
		  P1SEL |= BIT0;
		  P2DIR |= BIT2;                            // SMCLK set out to pins
		  P2SEL |= BIT2;*/
	UCSCTL6 &= ~XT1OFF; //XT1
	P5SEL |= BIT2 + BIT3; //XT2 selected
	UCSCTL6 &= ~XT2OFF;          //open XT2
	__bis_SR_register(SCG0);
	UCSCTL0 = DCO0 + DCO1 + DCO2 + DCO3 + DCO4;
	UCSCTL1 = DCORSEL_4;
	UCSCTL2 = FLLD_5 + 1;       //D=16，N=1
	UCSCTL3 = SELREF_5 + FLLREFDIV_3; //n=8,DCOCLK=D*(N+1)*(FLLREFCLK/n);DCOCLKDIV=(N+1)*(FLLREFCLK/n);
	UCSCTL4 = SELA_4 + SELS_3 + SELM_3; //ACLK
	UCSCTL5 = DIVA_5 + DIVS_0;      //ACLK
	//MCLK:16MHZ,SMCLK:19.2307692MHZ,ACLK:18.5185185MHZ

	//	 __bic_SR_register(SCG0);                   //Enable the FLL control loop

}
void delay_ms(unsigned int ms)
{
	unsigned int a;
	while (ms)
	{
		a = 1800;
		while (a--)
			;
		ms--;
	}
	return;
}

void IIC_Start()
{

	OLED_SCLK_Set();
	OLED_SDIN_Set();
	OLED_SDIN_Clr();
	OLED_SCLK_Clr();
}

/**********************************************
 //IIC Stop
 **********************************************/
void IIC_Stop()
{
	OLED_SCLK_Set();
//	OLED_SCLK_Clr();
	OLED_SDIN_Clr();
	OLED_SDIN_Set();

}

void IIC_Wait_Ack()
{

	//GPIOB->CRH &= 0XFFF0FFFF;	//éè??PB12?aé?à-ê?è??￡ê?
	//GPIOB->CRH |= 0x00080000;
//	OLED_SDA = 1;
//	delay_us(1);
	//OLED_SCL = 1;
	//delay_us(50000);
	/*	while(1)
	 {
	 if(!OLED_SDA)				//?D??ê?·??óê?μ?OLED ó|′eD?o?
	 {
	 //GPIOB->CRH &= 0XFFF0FFFF;	//éè??PB12?aí¨ó?í??aê?3??￡ê?
	 //GPIOB->CRH |= 0x00030000;
	 return;
	 }
	 }
	 */
	OLED_SCLK_Set();
	OLED_SCLK_Clr();
}
/**********************************************
 // IIC Write byte
 **********************************************/

void Write_IIC_Byte(unsigned char IIC_Byte)
{
	unsigned char i;
	unsigned char m, da;
	da = IIC_Byte;
	OLED_SCLK_Clr();
	for (i = 0; i < 8; i++)
	{
		m = da;
		//	OLED_SCLK_Clr();
		m = m & 0x80;
		if (m == 0x80)
		{
			OLED_SDIN_Set();
		}
		else
			OLED_SDIN_Clr();

		da = da << 1;
		OLED_SCLK_Set();
		OLED_SCLK_Clr();
	}

}
/**********************************************
 // IIC Write Command
 **********************************************/
void Write_IIC_Command(unsigned char IIC_Command)
{
	IIC_Start();
	Write_IIC_Byte(0x78);            //Slave address,SA0=0
	IIC_Wait_Ack();
	Write_IIC_Byte(0x00);			//write command
	IIC_Wait_Ack();
	Write_IIC_Byte(IIC_Command);
	IIC_Wait_Ack();
	IIC_Stop();
}
/**********************************************
 // IIC Write Data
 **********************************************/
void Write_IIC_Data(unsigned char IIC_Data)
{
	IIC_Start();
	Write_IIC_Byte(0x78);			//D/C#=0; R/W#=0
	IIC_Wait_Ack();
	Write_IIC_Byte(0x40);			//write data
	IIC_Wait_Ack();
	Write_IIC_Byte(IIC_Data);
	IIC_Wait_Ack();
	IIC_Stop();
}
//向SSD1306写入一个字节。
//dat:要写入的数据/命令
//cmd:数据/命令标志 0,表示命令;1,表示数据;
void OLED_WR_Byte(u8 dat, u8 cmd)
{
	if (cmd)
	{

		Write_IIC_Data(dat);

	}
	else
	{
		Write_IIC_Command(dat);

	}
}

void OLED_Set_Pos(unsigned char x, unsigned char y)
{
	OLED_WR_Byte(0xb0 + y, OLED_CMD);
	OLED_WR_Byte(((x & 0xf0) >> 4) | 0x10, OLED_CMD);
	OLED_WR_Byte((x & 0x0f) | 0x01, OLED_CMD);
}
//开启OLED显示
void OLED_Display_On(void)
{
	OLED_WR_Byte(0X8D, OLED_CMD);  //SET DCDC命令
	OLED_WR_Byte(0X14, OLED_CMD);  //DCDC ON
	OLED_WR_Byte(0XAF, OLED_CMD);  //DISPLAY ON
}
//关闭OLED显示
void OLED_Display_Off(void)
{
	OLED_WR_Byte(0X8D, OLED_CMD);  //SET DCDC命令
	OLED_WR_Byte(0X10, OLED_CMD);  //DCDC OFF
	OLED_WR_Byte(0XAE, OLED_CMD);  //DISPLAY OFF
}
//清屏函数,清完屏,整个屏幕是黑色的!和没点亮一样!!!
void OLED_Clear(void)
{
	u8 i, n;
	for (i = 0; i < 8; i++)
	{
		OLED_WR_Byte(0xb0 + i, OLED_CMD);    //设置页地址（0~7）
		OLED_WR_Byte(0x00, OLED_CMD);      //设置显示位置—列低地址
		OLED_WR_Byte(0x10, OLED_CMD);      //设置显示位置—列高地址
		for (n = 0; n < 128; n++)
			OLED_WR_Byte(0, OLED_DATA);
	} //更新显示
}

//在指定位置显示一个字符,包括部分字符
//x:0~127
//y:0~63
//mode:0,反白显示;1,正常显示
//size:选择字体 16/12
void OLED_ShowChar(u8 x, u8 y, u8 chr)
{
	unsigned char c = 0, i = 0;
	c = chr - ' '; //得到偏移后的值
	if (x > Max_Column - 1)
	{
		x = 0;
		y = y + 2;
	}
	if (SIZE == 16)
	{
		OLED_Set_Pos(x, y);
		for (i = 0; i < 8; i++)
			OLED_WR_Byte(F8X16[c * 16 + i], OLED_DATA);
		OLED_Set_Pos(x, y + 1);
		for (i = 0; i < 8; i++)
			OLED_WR_Byte(F8X16[c * 16 + i + 8], OLED_DATA);
	}
	else
	{
		OLED_Set_Pos(x, y + 1);
		for (i = 0; i < 6; i++)
			OLED_WR_Byte(F6x8[c][i], OLED_DATA);

	}
}

//m^n函数
u32 oled_pow(u8 m, u8 n)
{
	u32 result = 1;
	while (n--)
		result *= m;
	return result;
}
//显示2个数字
//x,y :起点坐标
//len :数字的位数
//size:字体大小
//mode:模式	0,填充模式;1,叠加模式
//num:数值(0~4294967295);
void OLED_ShowNum(u8 x, u8 y, u32 num, u8 len, u8 size2)
{
	u8 t, temp;
	u8 enshow = 0;
	for (t = 0; t < len; t++)
	{
		temp = (num / oled_pow(10, len - t - 1)) % 10;
		if (enshow == 0 && t < (len - 1))
		{
			if (temp == 0)
			{
				OLED_ShowChar(x + (size2 / 2) * t, y, ' ');
				continue;
			}
			else
				enshow = 1;

		}
		OLED_ShowChar(x + (size2 / 2) * t, y, temp + '0');
	}
}
//显示一个字符号串
void OLED_ShowString(u8 x, u8 y, u8 *chr)
{
	unsigned char j = 0;
	while (chr[j] != '\0')
	{
		OLED_ShowChar(x, y, chr[j]);
		x += 8;
		if (x > 120)
		{
			x = 0;
			y += 2;
		}
		j++;
	}
}
/*
//显示汉字
void OLED_ShowCHinese(u8 x, u8 y, u8 no)
{
	u8 t, adder = 0;
	OLED_Set_Pos(x, y);
	for (t = 0; t < 16; t++)
	{
		OLED_WR_Byte(Hzk[2 * no][t], OLED_DATA);
		adder += 1;
	}
	OLED_Set_Pos(x, y + 1);
	for (t = 0; t < 16; t++)
	{
		OLED_WR_Byte(Hzk[2 * no + 1][t], OLED_DATA);
		adder += 1;
	}
}*/
/***********功能描述：显示显示BMP图片128×64起始点坐标(x,y),x的范围0～127，y为页的范围0～7*****************/
void OLED_DrawBMP(unsigned char x0, unsigned char y0, unsigned char x1,
					unsigned char y1, unsigned char BMP[])
{
	unsigned int j = 0;
	unsigned char x, y;

	if (y1 % 8 == 0)
		y = y1 / 8;
	else
		y = y1 / 8 + 1;
	for (y = y0; y < y1; y++)
	{
		OLED_Set_Pos(x0, y);
		for (x = x0; x < x1; x++)
		{
			OLED_WR_Byte(BMP[j++], OLED_DATA);
		}
	}
}

//初始化SSD1306
void OLED_Init(void)
{
	OLED_SSD1306_SCLK_IO_INIT;
	OLED_SSD1306_SDIN_IO_INIT;
	OLED_SSD1306_DC_IO_INIT;
	OLED_SSD1306_CE_IO_INIT;
	OLED_SSD1306_RST_IO_INIT;

	OLED_RST_Set();
	delay_ms(100);
	OLED_RST_Clr();
	delay_ms(100);
	OLED_RST_Set();
	/*				  
	 OLED_WR_Byte(0xAE,OLED_CMD);//--turn off oled panel
	 OLED_WR_Byte(0x00,OLED_CMD);//---set low column address
	 OLED_WR_Byte(0x10,OLED_CMD);//---set high column address
	 OLED_WR_Byte(0x40,OLED_CMD);//--set start line address  Set Mapping RAM Display Start Line (0x00~0x3F)
	 OLED_WR_Byte(0x81,OLED_CMD);//--set contrast control register
	 OLED_WR_Byte(0xCF,OLED_CMD); // Set SEG Output Current Brightness
	 OLED_WR_Byte(0xA1,OLED_CMD);//--Set SEG/Column Mapping     0xa0左右反置 0xa1正常
	 OLED_WR_Byte(0xC8,OLED_CMD);//Set COM/Row Scan Direction   0xc0上下反置 0xc8正常
	 OLED_WR_Byte(0xA6,OLED_CMD);//--set normal display
	 OLED_WR_Byte(0xA8,OLED_CMD);//--set multiplex ratio(1 to 64)
	 OLED_WR_Byte(0x3f,OLED_CMD);//--1/64 duty
	 OLED_WR_Byte(0xD3,OLED_CMD);//-set display offset	Shift Mapping RAM Counter (0x00~0x3F)
	 OLED_WR_Byte(0x00,OLED_CMD);//-not offset
	 OLED_WR_Byte(0xd5,OLED_CMD);//--set display clock divide ratio/oscillator frequency
	 OLED_WR_Byte(0x80,OLED_CMD);//--set divide ratio, Set Clock as 100 Frames/Sec
	 OLED_WR_Byte(0xD9,OLED_CMD);//--set pre-charge period
	 OLED_WR_Byte(0xF1,OLED_CMD);//Set Pre-Charge as 15 Clocks & Discharge as 1 Clock
	 OLED_WR_Byte(0xDA,OLED_CMD);//--set com pins hardware configuration
	 OLED_WR_Byte(0x12,OLED_CMD);
	 OLED_WR_Byte(0xDB,OLED_CMD);//--set vcomh
	 OLED_WR_Byte(0x40,OLED_CMD);//Set VCOM Deselect Level
	 OLED_WR_Byte(0x20,OLED_CMD);//-Set Page Addressing Mode (0x00/0x01/0x02)
	 OLED_WR_Byte(0x02,OLED_CMD);//
	 OLED_WR_Byte(0x8D,OLED_CMD);//--set Charge Pump enable/disable
	 OLED_WR_Byte(0x14,OLED_CMD);//--set(0x10) disable
	 OLED_WR_Byte(0xA4,OLED_CMD);// Disable Entire Display On (0xa4/0xa5)
	 OLED_WR_Byte(0xA6,OLED_CMD);// Disable Inverse Display On (0xa6/a7)
	 OLED_WR_Byte(0xAF,OLED_CMD);//--turn on oled panel
	 */

	OLED_WR_Byte(0xAE, OLED_CMD); //--turn off oled panel
	OLED_WR_Byte(0x00, OLED_CMD); //---set low column address
	OLED_WR_Byte(0x10, OLED_CMD); //---set high column address
	OLED_WR_Byte(0x40, OLED_CMD); //--set start line address  Set Mapping RAM Display Start Line (0x00~0x3F)
	OLED_WR_Byte(0x81, OLED_CMD); //--set contrast control register
	OLED_WR_Byte(0xCF, OLED_CMD); // Set SEG Output Current Brightness
	OLED_WR_Byte(0xA1, OLED_CMD); //--Set SEG/Column Mapping     0xa0左右反置 0xa1正常
	OLED_WR_Byte(0xC8, OLED_CMD); //Set COM/Row Scan Direction   0xc0上下反置 0xc8正常
	OLED_WR_Byte(0xA6, OLED_CMD); //--set normal display
	OLED_WR_Byte(0xA8, OLED_CMD); //--set multiplex ratio(1 to 64)
	OLED_WR_Byte(0x3f, OLED_CMD); //--1/64 duty
	OLED_WR_Byte(0xD3, OLED_CMD); //-set display offset	Shift Mapping RAM Counter (0x00~0x3F)
	OLED_WR_Byte(0x00, OLED_CMD); //-not offset
	OLED_WR_Byte(0xd5, OLED_CMD); //--set display clock divide ratio/oscillator frequency
	OLED_WR_Byte(0x80, OLED_CMD); //--set divide ratio, Set Clock as 100 Frames/Sec
	OLED_WR_Byte(0xD9, OLED_CMD); //--set pre-charge period
	OLED_WR_Byte(0xF1, OLED_CMD); //Set Pre-Charge as 15 Clocks & Discharge as 1 Clock
	OLED_WR_Byte(0xDA, OLED_CMD); //--set com pins hardware configuration
	OLED_WR_Byte(0x12, OLED_CMD);
	OLED_WR_Byte(0xDB, OLED_CMD); //--set vcomh
	OLED_WR_Byte(0x40, OLED_CMD); //Set VCOM Deselect Level
	OLED_WR_Byte(0x20, OLED_CMD); //-Set Page Addressing Mode (0x00/0x01/0x02)
	OLED_WR_Byte(0x02, OLED_CMD); //
	OLED_WR_Byte(0x8D, OLED_CMD); //--set Charge Pump enable/disable
	OLED_WR_Byte(0x14, OLED_CMD); //--set(0x10) disable
	OLED_WR_Byte(0xA4, OLED_CMD); // Disable Entire Display On (0xa4/0xa5)
	OLED_WR_Byte(0xA6, OLED_CMD); // Disable Inverse Display On (0xa6/a7)
	OLED_WR_Byte(0xAF, OLED_CMD); //--turn on oled panel

	OLED_WR_Byte(0xAF, OLED_CMD); /*display ON*/
	OLED_Clear();
	OLED_Set_Pos(0, 0);
}

