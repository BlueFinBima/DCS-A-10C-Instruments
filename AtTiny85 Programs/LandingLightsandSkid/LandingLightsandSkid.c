/***************************************************************************
* File              : LandingLightsandSkid.c
* Compiler          : AVRstudio 7.0
* Revision          : 1.1
* Date              : August 21st 2016
* Revised by        : Neil Larmour
*                   : Adapted from Dan Gates I2C analogue slave
*					  http://www.avrfreaks.net/index.php?name=PNphpBB2&file=viewtopic&t=51467
* Description       : This is an ATtiny board to report the state of two switches on the Landing
*                     gear panel. It is overkill, but the two ATtiny boards for the panel allows
*                     the panel to be set and checked on two i2c addresses using the rotary 
*                     encoder PCBs which I already had available.
*
* Target device		: ATtiny85 (although should fit on an ATtiny45)
*
* avrdude -p t45 -P /dev/tty.usbserial-A700ejMH -b 9600 -c avrisp -U lfuse:w:0xe2:m -U hfuse:w:0xdf:m -U efuse:w:0xff:m
*
* Connections
*                               ATtiny85
*                      +--------------------------------+
*                      | 1 pb5 reset              VCC 8 | VCC
* Landing Light Switch | 2 pb3                    pb2 7 | SCL
*    Taxi Light switch | 3 pb4                    pb1 6 | Anti Skid Switch
*                  GND | 4 GND                    pb0 5 | SDA
*                      +--------------------------------+
****************************************************************************/

#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <avr/pgmspace.h>
#include "include/usiTwiSlave.h"

#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#define DEFAULTADDRESS 0x1D

unsigned char EEMEM slaveaddress = 0xFF;  //Save "not set" to eeprom

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
    uint8_t tempSwVal;
    uint8_t tempSwVala;
	DDRB &= ~((1<<PB4)|(1<<PB3)|(1<<PB1));  // PB1,3,4 set up as inputs -  for switch state
	
	sei();  // Enable interupts. This still needs to be done to allow the I2C to work.
	//setaddress(DEFAULTADDRESS);
    eeprom_write_byte(&slaveaddress+1, DEFAULTADDRESS);  // place the default address into EEPROM even though we will not use it.
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

	usiTwiSlaveInit(address); 

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
				//Set address
				case 3 : 
					setaddress(usiTwiReceiveByte());
					usiTwiSlaveInit(readaddress());							// now use the new address for this device
					break;

				//Send the switch state
				case 10:
					// we return the state of the three switches
					tempSwVal = PINB >>1;								   // Take a copy of PINB and shift it by one bit
					tempSwVala = ((tempSwVal >>1) & ((1<<PB1)|(1<<PB2)));
					tempSwVal &= (1<<PB0);
					tempSwVal |= tempSwVala;   // or in the other two bits which have been shifted to the correct position
					tempSwVal |= ~((1<<PB2)|(1<<PB1)|(1<<PB0));            // set the other bits to 1
					
					usiTwiTransmitByte(~tempSwVal);                     //switch states are in the 3 low order bits
				break;
	
				default : //Do nothing
						 break;
			}	  
		}

		asm volatile ("NOP"::);
	}
}

