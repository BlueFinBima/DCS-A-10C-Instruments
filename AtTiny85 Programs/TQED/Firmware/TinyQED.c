/***************************************************************************
* File              : TinyQED
* Compiler          : AVRstudio 5.1
* Revision          : 1.1
* Date              : Tuesday, March 23, 2012
* Revised by        : Adriaan Swanepoel
*                   : Adapted from Dan Gates I2C analogue slave
*					  http://www.avrfreaks.net/index.php?name=PNphpBB2&file=viewtopic&t=51467
*                     and Jim Remington's Quadrature encoder for the O 
*					  http://forum.pololu.com/viewtopic.php?t=484
*
* Target device		: ATtiny85 (although should fit on an ATtiny45)
*
* Connections
*                               ATtiny85
*                 +--------------------------------+
*                 | 1 pb5 reset              VCC 8 | VCC
*       Encoder A | 2 pb3                    pb2 7 | SCL
*       Encoder B | 3 pb4                    pb1 6 | Encoder push switch
*             GND | 4 GND                    pb0 5 | SDA
*                 +--------------------------------+
****************************************************************************/

#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <avr/pgmspace.h>
#include "usiTwiSlave.h"

#define F_CPU 8000000UL // tell the compiler we're running 8MHz


#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#define DEFAULTADDRESS 0x10

//If the memory sections are defined but not referenced in the application code, the "Garbage collect unused sections" option in Project Properties->Linker->Optimization must be unchecked. Otherwise the linker will not include the sections in the .elf file.

FUSES = {
	.low = (FUSE_CKSEL0 & FUSE_CKSEL2 & FUSE_CKSEL3 & FUSE_SUT0),
	.high = HFUSE_DEFAULT,
	.extended = EFUSE_DEFAULT
//SELFPRGEN = [ ]
//RSTDISBL = [ ]
//DWEN = [ ]
//SPIEN = [X]
//WDTON = [ ]
//EESAVE = [ ]
//BODLEVEL = DISABLED
//CKDIV8 = [ ]
//CKOUT = [ ]
//SUT_CKSEL = INTRCOSC_8MHZ_6CK_14CK_64MS
//
//EXTENDED = 0xFF (valid)
//HIGH = 0xDF (valid)
//LOW = 0xE2 (valid)	
};

union doublebyte
{
  unsigned int value;
  unsigned char bytes[2];
};

volatile union doublebyte enc_pos0;
volatile union doublebyte enc_pos1;
volatile union doublebyte enc_delta0;
volatile union doublebyte enc_delta1;

unsigned char EEMEM slaveaddress = DEFAULTADDRESS;  //Default address - This is flashed into the EEPROM, but can be modified over the I2C bus
unsigned char EEMEM programTitle[30] = "TinyQED for DCS A-10C Cockpit\0";  // Place text into the EEPROM

/******************************************************************************
 *
 * Interupt routine for encoder moving
 *
 * Description:	This uses an array of valid transitions to change a counter
 *              for both a switched and unswitched movement of the encoder 
 * ARGS:		none
 * RETURN:		none
 *
 *****************************************************************************/
ISR (PCINT0_vect)
{
	// this table lookup approach to interupt driven rotary encoders is from 
	// https://www.circuitsathome.com/mcu/rotary-encoder-interrupt-service-routine-for-avr-micros
	//
  static uint8_t old_AB0 = 3;  //lookup table index
  static uint8_t old_AB1 = 3;  //lookup table index
  static const int8_t enc_states [] PROGMEM =
  {0,-1,1,0,1,0,0,-1,-1,0,0,1,0,1,-1,0};  //encoder lookup table
  // This routine requires the encoder to be on consecutive pins and
  // we use the lowest pin number as the number of bits to shift the values read
  //
  // We run two counters, the first is when the switch is not pressed and the other
  // one is used when the switch is pressed
  // If the encoder has a switch, it is on PB1
  //
  // First we want to de-bounce the contacts.  It is done in the interrupt to avoid
  // another interrupt caused by the bounce. 
  // 
  // Interrupts should be disabled when entering this code and reenabled on exit - I have not checked this but this is why the delay is in the interrupt routine.
  //if(debounceCount == 0 ){
  if ((PINB & (1 << PINB1))){
		  old_AB0 <<=2;  //remember previous state by shifting it up 2 places
		  old_AB0 |= ((PINB & (3 << PINB3)) >> PINB3);  // OR in the cleaned up new encoder state
		  enc_pos0.value += (int8_t)pgm_read_byte(&(enc_states[( old_AB0 & 0x0f )])); //add the new valid encoder change to the counter
		  enc_delta0.value += (int8_t)pgm_read_byte(&(enc_states[( old_AB0 & 0x0f )])); //add the new valid encoder change to the counter since the last time it was requested
  }
	  else {
		old_AB1 <<=2;  //remember previous state by shifting it up 2 places
		old_AB1 |= ((PINB & (3 << PINB3)) >> PINB3);  // OR in the cleaned up new encoder state
		enc_pos1.value += (int8_t)pgm_read_byte(&(enc_states[( old_AB1 & 0x0f )])); //add the new valid encoder change to the counter
		enc_delta1.value += (int8_t)pgm_read_byte(&(enc_states[( old_AB1 & 0x0f )])); //add the new valid encoder change to the counter since the last time it was requested

	  }
}

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
	enc_pos0.value = 0;
	enc_pos1.value = 0;
	enc_delta0.value = 0;
	enc_delta1.value = 0;

	PCMSK |= (1 << PCINT3); // tell pin change mask to listen to pin2
	GIMSK |= (1 << PCIE);   // enable PCINT interrupt in the general interrupt mask
  
	sei();

	cbi(DDRB, DDB1);        // PB1 set up as input - This is for the push switch
	cbi(DDRB, DDB3);        // PB3 set up as input
	cbi(DDRB, DDB4);        // PB4 set up as input
	sbi(PORTB, PB1);	    // Set PB1 internal pull up - This is for the push switch
	sbi(PORTB, PB3);	    // Set PB3 internal pull up
	sbi(PORTB, PB4);	    // Set PB4 internal pull up

	// setaddress(DEFAULTADDRESS);  // This should be in the EEPROM when it was flashed, there is no reason to do this

	uint8_t address = readaddress();  // Get this device's address from the EEPROM
	//Fix for new MCU and/or user error
	if (address >= 127)
	{
		address = DEFAULTADDRESS;
		setaddress(address);
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
				//Reset the counter
				case 1 :
						 temp = usiTwiReceiveByte();
                         if(temp == 0){
							 enc_pos0.value = 0;
							 enc_delta0.value = 0;
						 } else {
							 enc_pos1.value = 0;
							 enc_delta1.value = 0;							 
						 }
						 break; 

				//Center counter value
				case 2 : 
						 temp = usiTwiReceiveByte();
                         if(temp == 0){
							 enc_pos0.value = 32767;
							 enc_delta0.value = 0;
						 } else {
							 enc_pos1.value = 32767;							 
							 enc_delta1.value = 0;
						 }
						 break;

				//Set address
				case 3 : setaddress(usiTwiReceiveByte());
						 break;

				//Send the counter
				case 10:
						 temp = usiTwiReceiveByte();
                         if(temp == 0){
								usiTwiTransmitByte(enc_pos0.bytes[0]);
								usiTwiTransmitByte(enc_pos0.bytes[1]);
						 } else {
								usiTwiTransmitByte(enc_pos1.bytes[0]);
								usiTwiTransmitByte(enc_pos1.bytes[1]);							 
						 }
						break;
				//Send the counter of pulses since the last send
				case 11:
						 temp = usiTwiReceiveByte();
                         if(temp == 0){
								usiTwiTransmitByte(enc_delta0.bytes[0]);
								usiTwiTransmitByte(enc_delta0.bytes[1]);
								enc_delta0.value = 0;						 
						 } else {
								usiTwiTransmitByte(enc_delta1.bytes[0]);
								usiTwiTransmitByte(enc_delta1.bytes[1]);
								enc_delta1.value = 0;							 
						 }
				break;
		
				default : //Do nothing
						 break;
			}	  
		}
		asm volatile ("NOP"::);
	}
}

