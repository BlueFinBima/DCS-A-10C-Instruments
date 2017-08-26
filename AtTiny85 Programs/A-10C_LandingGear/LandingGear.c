/***************************************************************************
* File              : A-10C_LandingGear
* Compiler          : AVRstudio 7.0
* Revision          : 1.1
* Date              : October 26th, 2015
* Revised by        : Neil Larmour
*                   : Adapted from Dan Gates I2C analogue slave
*					  http://www.avrfreaks.net/index.php?name=PNphpBB2&file=viewtopic&t=51467
*                     and Jim Remington's Quadrature encoder for the O 
*					  http://forum.pololu.com/viewtopic.php?t=484
*
* Target device		: ATtiny85 (although should fit on an ATtiny45)
*
* avrdude -p t45 -P /dev/tty.usbserial-A700ejMH -b 9600 -c avrisp -U lfuse:w:0xe2:m -U hfuse:w:0xdf:m -U efuse:w:0xff:m
*
* Connections
*                               ATtiny85
*                   +--------------------------------+
*                   | 1 pb5 reset              VCC 8 | VCC
*       Gear Switch | 2 pb3                    pb2 7 | SCL
*Master Warning LED | 3 pb4                    pb1 6 | Gear LED
*               GND | 4 GND                    pb0 5 | SDA
*                   +--------------------------------+
****************************************************************************/

#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <avr/pgmspace.h>
#include "usiTwiSlave.h"

#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#define DEFAULTADDRESS 0x1E

unsigned char EEMEM slaveaddress = 0xFF;  //Invalid address

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
	uint8_t gearLEDState = 0;
	uint8_t cautionLEDState = 0;
	uint8_t ledState = 0;

	cbi(DDRB, DDB3);        // PB3 set up as input -  for switch state
	sbi(DDRB, DDB1);        // PB4 set up as output - for Gear LED
	sbi(DDRB, DDB4);        // PB1 set up as output - for Master Warning LED
	sbi(PORTB, PB3);	    // Set PB3 internal pull up
	//sbi(PORTB, PB4);	    // Set PB4 internal pull up

	sbi(PORTB,PB1);        // turn off the Gear LED
	cbi(PORTB,PB4);        // turn off the Master Warning LED
	
	sei();  // Enable interupts. This still needs to be done to allow the I2C to work.

	//setaddress(DEFAULTADDRESS);
	eeprom_write_byte(&slaveaddress+1, DEFAULTADDRESS);

	//Fix for new MCU and/or user error
	uint8_t address = readaddress();
	if (address >= 127)
	{
		//  if the EEPROM address is invalid, then we will use the default address for this board
		//  if we do not come in here, then it means that the EEPROM address was valid so we use that
		//  because it was set via i2c command

		address = DEFAULTADDRESS;
		//setaddress(address);
	}

	usiTwiSlaveInit(address); //readaddress());

	for (;;)
	{	
		
		if (usiTwiDataInReceiveBuffer())
		{
			temp = usiTwiReceiveByte();
     
			//which register requested
			//1..9   Reserved Commands
			//10..19 Reserved Data
			switch (temp)
			{
				//LED general Off
				case 1 :
						 temp = usiTwiReceiveByte();
						 if (temp & 0x02){
							 sbi(PORTB,PB1);
							 gearLEDState = 0;
						 }
						 if (temp & 0x08) {
							 cbi(PORTB,PB4);
							 cautionLEDState = 0;
						 }
						 break; 
				//LED general On
				case 2 : 
						temp = usiTwiReceiveByte();
						 if (temp & 0x02){
	 						cbi(PORTB,PB1);
							gearLEDState = 1;
							 }
						if (temp & 0x08) {
	 						sbi(PORTB,PB4);
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
						cbi(PORTB,PB4);
						cautionLEDState = 0;
				break;

				//LED Master Caution LED On
				case 5 :
						sbi(PORTB,PB4);
						cautionLEDState = 2;
				break;
				//LED Gear On
				case 7 :
						cbi(PORTB,PB1);
						gearLEDState = 1;
						break;
				//LED Gear Off
				case 8 :
						sbi(PORTB,PB1);
						gearLEDState = 0;
						break;			

				//Send the switch state
				case 10:
					// we return the state of the switch and the two LEDs in the one value
					usiTwiTransmitByte((uint8_t)((PINB & (1<<PINB3))<<(7-PINB3)) | cautionLEDState | gearLEDState );  //switch state is in the high bit, LED states are in the low order bits
				break;
	
				default : //Do nothing
						 break;
			}	  
		}

		asm volatile ("NOP"::);
	}
}

