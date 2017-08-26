/*Begining of Auto generated code by Atmel studio */
#include <Arduino.h>
/*End of auto generated code by Atmel studio */
//   This is an I2C CMSC slave device for DCS A-10C and it is based precisely on a previous controller.  
//   This is the Arduino sketch which turns the original CMSC DCS A-10C controller, which was an I2C master
//   and controlled pretty much everything.  It used a serial interface for communication.  This code is for  
//   an arduino Leonardo pro micro (this sketch).
// 
//   There are two encoders on the CMSC board and these are on PCINT0 - PCINT3
//   There is also a 20x2 OLED character display, 5 tactiles, and three LEDs.
//
//   The design of the encoder reporting is a little complex.  When the master requests a device status,
//   the CMSC responds with a single byte contain the state of the switches and whether either of the 
//   encoders has changed.  The master will then explicitly request the state of the encoder(s) it has 
//   been informed about.  In the ISR for the i2c, the encoder(s) values are checked and returned to the 
//   master.  The encoder values are read just before the send, because we do not know how long it will take  
//   for the master to request values after it has been notified so we do not want to lose pulses.  We also
//   only report the encoders requested.  Flags for encoder changes are set in the ISR for the encoders, and 
//   only reset after the value has been read from the encoder (which should be just before the value is sent
//   down the wire.  Pacing of the number of encoder reports might need to me implemented but the current design 
//   means that this can be done implemented by the slave or simply have the master request values less frequently.
//
//   Build Info
//   ----------
//   Board runs on an Atmel 32U4 microcontroller and the Arduino device to build against is a Leonardo
//   In vMicro's Vistual Studio plugin, the file name and the project name must be the same to enable 
//   the build buttons to work.
//
//   Revision History
//   ----------------
//   20170625 - Initial code of the I2C Slave 
//
//  Undiagnosed problems
//  --------------------
//  Does not come back online properly after a software reset
//  Encoders are a little eratic
//  
//
#include <Wire.h>                   // This is needed for i2c communications
#include <OLED.h>                   // Library for the OLED display without serial board 4-bit
#include <TQED.h>                   // This is for the Tiny QED encoder https://github.com/renbotics/TQED    
//Beginning of Auto generated function prototypes by Atmel Studio
void setup();
void loop();
void ProcessComms();
void ProcessQuery();
void ProcessData();
boolean readSwitches(uint8_t i2c_address);
void specialActions(int16_t *sk);
boolean CheckEncoders(void);
void testAll();
void testCMSC(uint8_t);
void getDefaultKeys(uint8_t i2c_address);
void CMSCEncoders(void);
void processReqi2c(void);
void processRcvi2c(void);
uint8_t getSwitchStates(void);
union { float f[2];byte b[8]; } volatile encoderCmsc;
volatile boolean EncodersNeedAttention = false;
void software_Reset(void);
int freeRam();
//End of Auto generated function prototypes by Atmel Studio

//
//  This section is the master list of all of the 12c devices in use in the project
//
const uint8_t i2c_addr_cmsc = 0x79;    // This is a dummy i2c address of the CMSC board which is a controller
									   //
OLED OLED1(11, 9, 6, 7, 12, 8);   // Instantiate the OLED display on the correct pins
								  //
								  //
#define LED_ON 1
#define LED_OFF 0
								  // These are the numbers of the three LEDs on the Pro Micro board
#define LED_RED 1
#define LED_YELLOW 2
#define LED_GREEN 3
								  // These are the response which checking keys can return
#define SWITCHNONE 0 
#define SWITCHWAIT 1
#define SWITCHSET 2
								  // These are the buttons on the CMSC board
#define CMSC_PRI 19
#define CMSC_SEP 20
#define CMSC_UNK 21
#define CMSC_BOT 22
#define CMSC_TOP 23
#define CMSC_PRI_LED 13
#define CMSC_ML_LED 4
#define CMSC_UNK_LED 5
const byte cmscLeds[3] = { CMSC_ML_LED,CMSC_PRI_LED,CMSC_UNK_LED };  // this is to allow indexing of the correct pin

																	 /*
																	 ** These headers are used to communicate with the i2c connected
																	 ** CMSC board which has 5 tactiles, 3 LEDs, 2 encoders and a
																	 ** 2x20 OLED character display.  The interface attempts to keep
																	 ** the amount of data on the i2c bus to a minimum, and also to
																	 ** reduce the instances when we need to turn the bus around.
																	 **
																	 */
																	 // Requests status from the CMSC & it can also set the LEDs
																	 // returns a single byte, 0bEE0SSSSS, with E being a flag to say
																	 // that an encoder has some data for us, and S is the current state 
																	 // of one of the five tactiles.
																	 // The status request flag must always be tested for first because
																	 // it overloads some of the other request bits.
#define CMSC_DEV_STATUS     0x80
																	 // Requests the status of the encoder(s) identified in the low order 
																	 // bits.  If both encoders are requested, twice the amount of data is
																	 // returned.  Requesting without any encoders will reset both the encoders
#define CMSC_DEV_ENC        0x20
#define CMSC_DEV_ENC_1      0x01
#define CMSC_DEV_ENC_2      0x02
#define CMSC_DEV_ENC_BOTH   0x03
#define CMSC_DEV_ENC_RST    0x00
																	 // Writes data to a single section of the 2x20 OLED identified in the low
																	 // order bits.  The areas have predefined positions and lengths so unless
																	 // the request is to clear the screen, there will be character bytes which
																	 // follow this request 
#define CMSC_DEV_SCREEN     0x40
#define CMSC_DEV_SCREEN_1   0x01
#define CMSC_DEV_SCREEN_2   0x02
#define CMSC_DEV_SCREEN_3   0x03
#define CMSC_DEV_SCREEN_CLR 0xFF
																	 // These are only used with a STATUS request to address and change one of more LED
#define CMSC_DEV_LED_1      0b00000001
#define CMSC_DEV_LED_2      0b00000010
#define CMSC_DEV_LED_3      0b00000100
#define CMSC_DEV_LED_1_VAL  0b00001000
#define CMSC_DEV_LED_2_VAL  0b00010000
#define CMSC_DEV_LED_3_VAL  0b00100000
																	 /*
																	 ** End of the defines for the CMSC communications
																	 */


																	 //
unsigned long time1 = 0;                     // Used to calculate loop delay
unsigned long time2 = 0;                     // Used to calculate loop delay
											 //
											 // Place the static text into flash instead of RAM
const char * Msg1 = "  A-10C CMSC  V1.1a \0";
//
// General String variables
String command;
String value;
boolean query = false;   // used to indicate that the current command is a query request
String UHFFreq;
//
const uint8_t generalIndicators[3][3] = {  // these are the LED positions for various indicators
	{ 0,1,2                // CMSC Missile Launch, Pri, Unk
	},
	{ 0,0,0,
	},
	{
		i2c_addr_cmsc,i2c_addr_cmsc,i2c_addr_cmsc
	}
};
const uint8_t cmscPosn[2][3] = {  // these are positions for text on the CMSC display
	{ 12,0,0 },              // order is nother then "CHAFF_FLARE","JMR","MWS"
	{ 0,0,1 }
};
/*
const int16_t  keyMappings [5][ MAXKEYS ] PROGMEM = {  // this is the lookup table for the up front controller keys and other switches
{809,810,813,812,811,0}, //  19     CMSC-RWR-JMR | CMSC-RWR-MWS | CMSC-RWR-PRI | CMSC-RWR-SEP | CMSC-RWR-UNK |
{1,2,3,4,5};
{1,1,1,1,1},
{5,5,5,5,5},
{0,0,0,0,0}
};
*/
boolean readkeys = false;
boolean batteryState = true;
boolean inverterState = true;

unsigned long LastActivityTime = 0;
#ifdef DEBUG
boolean nldebug = false;
#endif

char displayBuffer[21];

uint8_t CMSCLastState = 0;

// These are the two counters used for the encoders on the CMSC
volatile int CMSCBrtCounter = 0;
volatile int CMSCAudCounter = 0;
volatile bool CMSCAudChange = false;
volatile bool CMSCBrtChange = false;
volatile float cmscAudResponse = 0;
volatile float cmscBrtResponse = 0;

volatile byte  cmscRequest = 0;  // this is used to pass the i2c request from the master to the response listener
volatile boolean responseToDo = true;

void setup()
{
	uint8_t i;
	uint8_t j;
	Serial.begin(115200);               // start the serial port
	Wire.begin(i2c_addr_cmsc);
	Wire.onReceive(processRcvi2c);         // I2C Recv Event listener  
	Wire.onRequest(processReqi2c);         // I2C Rsp Event listener  

	delay(1000);
	Serial.print("Board Ready to go:");               // Indicate that we've started on the serial port
	Serial.println(Msg1);                             // Indicate that we've started on the serial port
	OLED1.begin(20, 2);                                // start the screen for the CMSC

													   // setup the pins on the CMSC board
	pinMode(CMSC_PRI_LED, OUTPUT);
	pinMode(CMSC_UNK_LED, OUTPUT);
	pinMode(CMSC_ML_LED, OUTPUT);

	pinMode(CMSC_PRI, INPUT_PULLUP);   // This is a digital mapping for the analog pin
	pinMode(CMSC_SEP, INPUT_PULLUP);   // This is a digital mapping for the analog pin
	pinMode(CMSC_UNK, INPUT_PULLUP);   // This is a digital mapping for the analog pin
	pinMode(CMSC_TOP, INPUT_PULLUP);   // This is a digital mapping for the analog pin
	pinMode(CMSC_BOT, INPUT_PULLUP);   // This is a digital mapping for the analog pin
	testAll();
	OLED1.noAutoscroll();
	OLED1.clear();                  // Clear the OLED screen
	OLED1.setCursor(0, 0);
	strcpy(displayBuffer, Msg1);
	OLED1.print(displayBuffer);
	delay(1000);
#ifdef DEBUG
	// allocate memory for common strings
	command.reserve(8);
	value.reserve(48);
#endif

	//perform the startup tests

	command = "";
	value = "";
	OLED1.clear();                                              // Clear the OLED screen
																// prepare the two onboard encoders
	DDRB &= ~((1 << PB0) | (1 << PB1) | (1 << PB2) | (1 << PB3));       // Set encoder bits to inputs 
																		//PCMSK0 |= (1<<PCINT0)|(1<<PCINT1)|(1<<PCINT2)|(PCINT3);   // set the pin change mask for the 2 CMSC encoders
	PCMSK0 |= (1 << PCINT0) | (1 << PCINT2);                          // set the pin change mask for one pin each on the 2 CMSC encoders
	PCICR |= (1 << PCIE0);                                        // Pin Change interrupt control register
	sei();                                                      // enable interrupts
																// this is just to remove instability and then reset the encoders
	CMSCAudCounter = 0;
	CMSCBrtCounter = 0;
	CMSCAudChange = false;
	CMSCBrtChange = false;

	//  These are set here as a temporary measure while we do not have real switches for the electrical power
	//  the lines can simply be removed once the real switches have been reinstalled.
	inverterState = true;
	batteryState = true;
	responseToDo = false;
}
/******************************************************************************
**
**  Main processing loop starts here.
**
*******************************************************************************/
void loop()
{
	//if(EncodersNeedAttention)processEncInterrupt();  // run main interrupt code.  The ISR only sets a flag
	// this routine only sets two other flags, one for each 
	// encoder.  This is because we do not want to read the
	// encoders unto the i2c master asks us for the values to 
	// ensure that we do not miss any pulses.
	// For each interrupt, we will add to the encoder count
	// and turn off the NeedAttention flag, however the individual
	// encoder flags stay on until their values have been written
	// out on i2c 
	delay(10);
}

void processRcvi2c() {
	// function: i2c Event Listener that executes when data is received from master
	// It is unclear whether this is an ISR or is just scheduled by an ISR but we'll
	// keep it short and simple in case it is an ISR.
	// If a command is received which needs data to be returned, we need to indicate this
	// so that when the request ISR is called, that routine knows what is expected to be
	// sent back.  The request ISR does not seem to be able to tell us how much data is 
	// being expected which is a little annoying.

	byte c = Wire.read();           // receive command byte
									// Serial.print("request: ");Serial.println(c,BIN);
	if (c & CMSC_DEV_STATUS) {
		cmscRequest = CMSC_DEV_STATUS;     // Communicate to the response routine what what to respond with.  This needs to happen first because the other routine might be called straight away.
										   // this is a single byte requesting status of the CMSC and also
										   // potentially setting one or more LEDs so we need to check if
										   // there is LED work to be done
		if (c & (CMSC_DEV_LED_1 | CMSC_DEV_LED_2 | CMSC_DEV_LED_3)) {
			// we have one or more of the LED flags set so 
			// we shift along them and change the LED states
			for (int n = 0;n<3;n++) {
				if (c & (1 << n)) {
					digitalWrite(cmscLeds[n], c & (1 << n + 3));  // Alter the state of the appropriate LED
				}
			}
		}
	}
	else if (c & CMSC_DEV_ENC) {
		//  digitalWrite(CMSC_ML_LED,HIGH);  //debug
		// we've received a request for encoder values
		cmscRequest = c & (CMSC_DEV_ENC | CMSC_DEV_ENC_BOTH);
		if (c & CMSC_DEV_ENC_BOTH) {
			// one or both of the encoders has been requested so we pass the request to the listener
		}
		else {
			// neither of the low order bits was set so a reset has been requested
			// don't forget to write the reset code       
			cmscRequest = 0;
		}
	}
	else if (c & CMSC_DEV_SCREEN) {
		// we want to write something to the screen
		//Serial.println(c,HEX);
		cmscRequest = 0x00;  // tell the response routine that we are not going to be sending back data
							 //Serial.println("Screen command received");
		if ((byte)(c & CMSC_DEV_SCREEN_3) > 0) {  //SCREEN_3 has both bits set so we use this for the any zone test
												  // we have a message to be placed into one of the screen zones so we receive the rest of the bytes
												  //Serial.println("Screen Write");
			int i = 0;
			while (Wire.available()) {           // loop through all the remaining data
				displayBuffer[i++] = Wire.read(); // and add to the display buffer
			}
			displayBuffer[i++] = '\0'; // null terminate the string
									   //Serial.println(displayBuffer);
									   // we have the screen data in the buffer, now we must write it to the correct place
			uint8_t n = c & 0b00000011;
			OLED1.setCursor(cmscPosn[0][n - 1], cmscPosn[1][n - 1]);
			OLED1.print(displayBuffer);
		}
		else {
			// low order bits are 0's so it is a screen clear which is being requested.
			//Serial.println("Screen Clear");
			OLED1.clear();
		}
	}
	else {
		// else for the main commands
		Serial.print("Unrecognised i2c data.");
	}
	while (Wire.available()) {      // loop through all the remaining data (but I am not expecting there to be any
		char c = Wire.read();         // receive byte as a character
		Serial.print(c);              // print the character
	}
}
void processReqi2c() {
	// function: i2c Event Listener that executes when a request for data is received from master
	// this is probably an ISR so we'll keep the processing short and simple.  We only send data if 
	// we have previously received a request, and the request needs to tell this routine what kind of 
	// data needs to be returned.
	// Slaves cannot use begin or end transmissions
	//
	//  * * * do not be tempted to put serial prints in here.  They seem to work but disrupt the i2c comms! * * *
	//
	uint8_t tcmscRequest = cmscRequest;
	responseToDo = false;
	cmscRequest = 0;  // free up the request code in case it gets set again by the ISR (this should not happen)
	switch (tcmscRequest & 0b11100000) {
	case CMSC_DEV_STATUS:
		Wire.write(getSwitchStates());
		break;
	case (CMSC_DEV_ENC) :
		CMSCEncoders(tcmscRequest & 0b00000011);  // obtain the encoder values only for the ones requested by the master.   
		switch (tcmscRequest & 0b00000011) {
		case (CMSC_DEV_ENC_1) :
			CMSCAudChange = false;
			Wire.write((byte *)&encoderCmsc.b[4], 4);
			encoderCmsc.f[1] = 0;
			break;
		case (CMSC_DEV_ENC_2) :
			CMSCBrtChange = false;
			Wire.write((byte *)&encoderCmsc.b[0], 4);
			encoderCmsc.f[0] = 0;
			break;
		case (CMSC_DEV_ENC_BOTH) :
			CMSCAudChange = false;
			CMSCBrtChange = false;
			Wire.write((byte *)&encoderCmsc.b[0], 8);
			encoderCmsc.f[0] = 0;
			encoderCmsc.f[1] = 0;
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}
	// one worry is if exiting this routine causes the library to think that no response is necessary.
	// !!! This looks to be a valid concern !!!
}

uint8_t getSwitchStates(void) {
	// this function is to read any special switches which have been wired differently
	// the general approach is to only report the transitions, and to make it simpler for 
	// the processing of the key presses, we use a dummy code from an HT16K33

	// We now read the switches on the CMSC board which are all on port F
	uint8_t CurrState = (~(PINF)& 0b01110011);
	CurrState = (CurrState >> 2) | (CurrState & 0b00000011);   // this results in the low order 5 bits being the keystates for the CMSC
	if (CMSCAudChange) {
		// tell the master that we're waiting to inform about encoder changes
		CurrState |= 0b01000000;
	}
	if (CMSCBrtChange) {
		// tell the master that we're waiting to inform about encoder changes
		CurrState |= 0b10000000;
	}
	//Serial.println(CurrState,BIN);
	return(CurrState);
}
boolean CheckEncoders() {
	boolean ret = false;
	unsigned long currTime = micros();           // only do a call to the clock once
	if (currTime - time2 > 100000) {  // only check the switch state once per interval
		time2 = currTime;
	}
	else {
		ret = false;   // we'll declare no encoder activity because the time period has not expired and we don't want to check them too often
	}
	return ret;
}
void specialActions(int16_t *sk) {
	/* We use this routine to process special flags, including the ones which should rob the display of power from the simulator.
	*
	*/
	OLED1.clear();
	batteryState = false;
	inverterState = false;
}

void CMSCEncoders(uint8_t encs) {
	float reportValue;
	int8_t tempDir;
	int16_t EncVal;
	//
	//  While not ideal, this function is called from the i2c ISR
	//
	//  I have many encoder types in operation with different PP Rot and 
	//  bounce characteristics - each one needing different handling :-(
	//  It looks like AUD has half the PP Rot than BRT
	//
	const float BRTMULT = 0.286;  // this is the max value minus the min value for the count of the encoder
	const float AUDMULT = 0.167;  // this is the max value minus the min value for the count of the encoder
	if (encs & 0b00000010) {
		// we appear to have an encoder to report
		EncVal = CMSCBrtCounter;
		if (EncVal != 0) {
			EncVal < 0 ? tempDir = -1 : tempDir = 1;   // save the direction of the rotation     
			if (EncVal> 0 && EncVal< 4) EncVal = 4; else if (EncVal <0 && EncVal > -4) EncVal = -4;
			//reportValue = EncVal * tempDir *0.01;    // cheap way to square value without losing the sign
			reportValue = (EncVal - (3 * tempDir)) * BRTMULT / 4;      // remove one less than the min value
			if (reportValue > 1) reportValue = 1; else if (reportValue < -1) reportValue = -1;  // clip high and low
			encoderCmsc.f[0] = reportValue;
		}
		CMSCBrtChange = false; // turn it off
		CMSCBrtCounter = 0;  // we only want to see the delta since we last processed the encoder
	}
	if (encs & 0b00000001) {
		// we appear to have an encoder to report
		EncVal = CMSCAudCounter;
		if (EncVal != 0) {
			EncVal < 0 ? tempDir = -1 : tempDir = 1;  // save the direction of the rotation   
			if (EncVal> 0 && EncVal< 2) EncVal = 2; else if (EncVal <0 && EncVal > -2) EncVal = -2;
			reportValue = (EncVal - (1 * tempDir)) * AUDMULT;      // remove one less than the min value
			if (reportValue > 1) reportValue = 1; else if (reportValue < -1) reportValue = -1;  // clip high and low       
			encoderCmsc.f[1] = reportValue;
		}
		CMSCAudChange = false; // turn it off
		CMSCAudCounter = 0;  // we only want to see the delta since we last processed the encoder
	}
	if (!encs) {
		// this clause deals with the case where no encoders have been requested to we need to reset the encoders    
	}
}
void testAll() {
	// this routine is to test all of the sub-section tests, sets a delay and then turns off the test
	testCMSC(HIGH);
	delay(2000);
	testCMSC(LOW);
}
void testCMSC(uint8_t testCMSCmode) {
	if (testCMSCmode == HIGH) {
		pinMode(CMSC_PRI_LED, OUTPUT);
		pinMode(CMSC_UNK_LED, OUTPUT);
		pinMode(CMSC_ML_LED, OUTPUT);
		pinMode(CMSC_PRI, INPUT_PULLUP);  // This is a digital mapping for the analog pin
		pinMode(CMSC_SEP, INPUT_PULLUP);  // This is a digital mapping for the analog pin
		pinMode(CMSC_UNK, INPUT_PULLUP);  // This is a digital mapping for the analog pin
		pinMode(CMSC_TOP, INPUT_PULLUP); // This is a digital mapping for the analog pin
		pinMode(CMSC_BOT, INPUT_PULLUP);   // This is a digital mapping for the analog pin
	}
	digitalWrite(CMSC_ML_LED, testCMSCmode);
	digitalWrite(CMSC_UNK_LED, testCMSCmode);
	digitalWrite(CMSC_PRI_LED, testCMSCmode);
}
void software_Reset() // Restarts program from beginning but does not reset the peripherals and registers
{
	asm volatile ("  jmp 0");
}
int freeRam()
{
	extern int __heap_start, *__brkval;
	int v;
	return (int)&v - (__brkval == 0 ? (int)&__heap_start : (int)__brkval);
}
ISR(PCINT0_vect) {
	// currently we need this to be very light weight to avoid conflicts with the i2c library
	EncodersNeedAttention = true;
	//digitalWrite(CMSC_UNK_LED,HIGH);
	// This is the interrupt service routine for the two rotary encoders on PB 0 - 3 which 
	// raise interrupts PCINT0-3
	//
	// this table lookup approach to interupt driven rotary encoders is from 
	// https://www.circuitsathome.com/mcu/rotary-encoder-interrupt-service-routine-for-avr-micros
	//
	EncodersNeedAttention = false;
	static uint8_t old_PINB;     // The only way to tell which pin caused the interrupt is to save the previous value 
	static uint8_t cur_PINB;     // only look at the actual pins once
	static uint8_t tAB;          // this is used incase there is an invalid transition so the previous state can be restored
	static uint8_t old_AB0 = 3;  //lookup table index
	static uint8_t old_AB1 = 3;  //lookup table index
	static int8_t tVal;
	static const int8_t enc_states[] =
	{ 0,-1,1,0,1,0,0,-1,-1,0,0,1,0,1,-1,0 };  //encoder lookup table
											  // This encoders are on consecutive pins ABAB
											  //
											  // we use the lowest pin number as the number of bits to shift the values read
											  //
											  // We run two counters, one for each encoder (there are no switches - on each interrupt we process both
											  // encoders.
											  // 
											  // Interrupts are disabled when entering this code and reenabled on exit.
											  // 
											  // * * * debouncing not implemetented and the capacitors did not help * * *
											  // First we want to de-bounce the contacts.  It is done in the interrupt to avoid
											  // another interrupt caused by the bounce.  Bouncing might still cause excessive interupts so 
											  // we might need to pop some 0.1uF caps on the pins to reduce bouncing.
											  //
											  //delay(50);  // wait for 20 milliseconds before reading the port to debounce without interrupts enabled
	cur_PINB = PINB & 0x0f;  // get the values of PB0-3 in one go
							 // First thing is to determine which encoder caused the interrupt
							 // we're only interested in the rising changes but an interupt happens on falling as well
							 // so we xor with old value to find changes then and with the current values to onlt have the rising pins
	if ((cur_PINB^old_PINB) & 0b00000011) {
		// Process changes to encoder on PB0&1
		tAB = old_AB0; // keep in case we need to restore it
		old_AB0 <<= 2;  //remember previous state by shifting it up 2 places
		old_AB0 |= ((cur_PINB & (3 << PINB0)) >> PINB0);  // OR in the cleaned up new encoder state
		tVal = (int8_t)(enc_states[(old_AB0 & 0x0f)]); //add the new valid encoder change to the counter
		if (tVal != 0) {
			// this is to ensure we only process counter changes and not the invalid transitions
			// the pins for this encoder are reversed so we -= rather than +=
			CMSCAudCounter += tVal; //add the new valid encoder change to the counter
			CMSCAudChange = true;
		}
		else {
			// there was an invalid transition so we restore the old save value
			old_AB0 = tAB;
		}
	}
	if ((cur_PINB^old_PINB) & 0b00001100) {
		// Process changes to encoder on PB2&3
		tAB = old_AB1; // keep in case we need to restore it
		old_AB1 <<= 2;  //remember previous state by shifting it up 2 places
		old_AB1 |= ((cur_PINB & (3 << PINB2)) >> PINB2);  // OR in the cleaned up new encoder state
		tVal = (int8_t)(enc_states[(old_AB1 & 0x0f)]); //add the new valid encoder change to the counter
		if (tVal != 0) {
			// this is to ensure we only process counter changes
			CMSCBrtCounter -= tVal; //add the new valid encoder change to the counter
			CMSCBrtChange = true;
		}
		else {
			// there was an invalid transition so we restore the old save value
			old_AB1 = tAB;
		}
	}
	old_PINB = cur_PINB;  // save the last pin state
						  //digitalWrite(CMSC_UNK_LED,LOW);
}

