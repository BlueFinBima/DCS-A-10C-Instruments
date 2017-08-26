/***************************************************************************
* File              : Master Caution on V3 UFC.c
* Compiler          : AVRstudio 7.0
* Created           : 08/10/2016 12:10:35
* Revision          : 1.0
* Revised by        : Neil Larmour
* Description       : This gives the master caution button on the V3 UFC an i2c address of its own.
*                   : It is basically one output (Light) on PB1
*                   :
*                   : Adapted from Dan Gates I2C analogue slave
*					  http://www.avrfreaks.net/index.php?name=PNphpBB2&file=viewtopic&t=51467
*                     and Jim Remington's Quadrature encoder for the O 
*
*
* Target device		: ATtiny85 (although should fit on an ATtiny45)
*
* avrdude -p t45 -P /dev/tty.usbserial-A700ejMH -b 9600 -c avrisp -U lfuse:w:0xe2:m -U hfuse:w:0xdf:m -U efuse:w:0xff:m
*
* Connections
*                               ATtiny85
*                   +--------------------------------+
*                   | 1 pb5 reset              VCC 8 | VCC
*                   | 2 pb3                    pb2 7 | SCL
*                   | 3 pb4                    pb1 6 | Master Warning LED
*               GND | 4 GND                    pb0 5 | SDA
*                   +--------------------------------+
****************************************************************************/

#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <avr/pgmspace.h>
#include "include/usiTwiSlave.h"

#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#define DEFAULTADDRESS 0x1F

unsigned char EEMEM slaveaddress = 0XFF;  //Invalid address

/******************************************************************************
 *
 * setaddress
 *
 * Description:	Updates the I2C address in the internal eeprom
 * ARGS:		The new I2C address
 * RETURN:		none
 *
 *****************************************************************************/
void setaddress(unsigned char address)
{
	eeprom_write_byte(&slaveaddress, (uint8_t*)address);
}

/******************************************************************************
 *
 * readaddress
 *
 * Description:	Reads the preprogrammed I2C address from the internal eeprom
 * ARGS:		none
 * RETURN:		The devices I2C address
 *
 *****************************************************************************/
unsigned char readaddress()
{
	return eeprom_read_byte((uint8_t*)&slaveaddress);
}

/******************************************************************************
 *
 * main
 *
 * Description:	Where it all starts...
 * ARGS:		none
 * RETURN:		none
 *
 *****************************************************************************/
int main(void)
{
	unsigned char temp;
	uint8_t cautionLEDState = 0;
	

	sbi(DDRB, DDB1);        // PB1 set up as output - for Master Warning LED
	sbi(PORTB, PB3);	    // Set PB3 internal pull up
	//sbi(PORTB, PB4);	    // Set PB4 internal pull up

	cbi(PORTB,PB1);        // turn off the Master Warning LED
	
	sei();  // Enable interrupts. This still needs to be done to allow the I2C to work.

	//setaddress(DEFAULTADDRESS);
	eeprom_write_byte(&slaveaddress+1, DEFAULTADDRESS);
	
	//Fix for new MCU and/or user error
	uint8_t address = readaddress();
	if (address >= 127)
	{
		address = DEFAULTADDRESS;
		//setaddress(address);
	}

	usiTwiSlaveInit(address); //readaddress());

	for (;;)
	{	
		
		if (usiTwiAmountDataInReceiveBuffer())
		{

			temp = usiTwiReceiveByte();
     
			//which register requested
			//1..9   Reserved Commands
			//10..19 Reserved Data
			switch (temp)
			{
				//LED general Off  (used if more than one LED)
				case 1 :
						 temp = usiTwiReceiveByte();
						 if (temp & 0x02) {
							 cbi(PORTB,PB1);
							 cautionLEDState = 0;
						 }
						 break; 
				//LED general On (used if more than one LED)
				case 2 : 
						temp = usiTwiReceiveByte();
						if (temp & 0x02) {
	 						sbi(PORTB,PB1);
							cautionLEDState = 1;
						}
						 break;
				//Set address
				case 3 : 
					setaddress(usiTwiReceiveByte());
					usiTwiSlaveInit(readaddress());
						 break;

				//LED Master Caution LED Off
				case 4 :
						cbi(PORTB,PB1);
						cautionLEDState = 0;
				break;

				//LED Master Caution LED On
				case 5 :
				sbi(PORTB,PB1);
				cautionLEDState = 2;
				break;

				//LED Master Caution LED On
				case 255 :
				sbi(PORTB,PB1);
				cautionLEDState = 2;
				break;
				//LED Master Caution LED Off
				case 254 :
				cbi(PORTB,PB1);
				cautionLEDState = 0;
				break;

				default : //Do nothing
						 break;
			}	  
		}

		asm volatile ("NOP"::);
	}
}

