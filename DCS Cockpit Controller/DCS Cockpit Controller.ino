/*
*  This sketch is the first iteration of the WiFi DCS A-10C cockpit controller based on the ESP8266
*
*/
//   This is an I2C controller running on an ESP8266 based board which enables the control or real devices for DCS A-10C cockpit.
//   The board communicates with the simulator via UDP on WiFi, and controls most of the devices on I2C bus.
//
//   This is the Arduino sketch using the ESP NodeMCU 12E board description and acts as an I2C master to control
//   pretty much everything.
//
//   Build Info
//   ----------
//   Board runs on an ESP8266 embedded on a NodeMCU board.  The board also contains an OLED dispay connected via ISP
//   In vMicro's Vistual Studio plugin, the file name and the project name must be the same to enable 
//   the build buttons to work.
//
//   Revision History
//   ----------------
//   20170824 - First completed version of the board  
//
//  Undiagnosed problems
//  --------------------
//  AtTiny85 devices will not work if they have an I2C address >= 0x40
//  Encoders are a little eratic
//  ESP8266 have a single engine so they get a bit cranky if they are starved of CPU, memory, or time to do WiFi
//  Note the ESP8266 is a 3V3 device so needs to have an I2C friendly level shifter on the I2C bus
//  
//

#include "keyMappings.h"
#include <arduino.h>
#include <U8g2lib.h>                  // Graphics for the OLED graphic display 
#include <ESP8266WiFi.h>              // ESP8266 wireless code
#include <WiFiUdp.h>                  // UDP over WiFi 
#include <Wire.h>                     // This is needed for i2c communications
#include <OLED_I2C.h>                 // Library for the OLED display without serial board 4-bit
#include <A10CLandingGear.h>          // Library for the I2C Landing Gear Switch
#include <A10CLandingLightsandSkid.h> // Library for the I2C Landing landing lights and skid switches
#include <A10CMasterCaution.h>        // Master Caution LED
#include <TQED.h>                     // This is for the Tiny QED encoder https://github.com/renbotics/TQED    
#include "keyMappings.h"              // this is the keyMapping master file.

// WiFi network name and password:
const char * networkName = "***REMOVED***";
const char * networkPswd = "***REMOVED***";

//IP address to send UDP data to:
// either use the ip address of the server or 
// a network broadcast address
const char * udpAddress = "10.1.1.2";
const uint16_t udpPort = 3339;
uint16_t udpOutBndPort = 0;
char packetBuffer[576];               // network buffer to hold incoming packet
char cmdBuffer[576];                  // this is a buffer to hold a single command (there can be long text command values
char replyBuffer[32];                 // a string to send back
boolean connected = false;            // Indicate if we are currently connected
byte cmscLastSwitchState = 0;         // This holds the state of the switches from last time so that we can report changes only
									  // Constructor for the UDP library class
WiFiUDP udp;

//
//This section is for the APU dials which live directly on the ISP interface of the ESP8266
//
/* Constructor */
// i2c version
//U8G2_SH1106_128X64_NONAME_1_SW_I2C oledAPUu8g2(U8G2_R0,D1,D2);
// isp version using hardware port 
U8G2_SH1106_128X64_NONAME_1_4W_HW_SPI oledAPUu8g2(U8G2_R0, 15, 2, 0);
float APUSpdDial = 0.0;
float APUEGTDial = 0.0;
boolean APUdialsWrite = false; // this is a flag to indicate that we have changed the dial value and we need to write them out
							   //
unsigned long time1 = 0;                     // Used to calculate loop delay
unsigned long time2 = 0;                     // Used to calculate loop delay
											 //
											 // These are the response which checking keys can return
#define SWITCHNONE 0 
#define SWITCHWAIT 1
#define SWITCHSET 2
#define SWITCHALREADYDONE 3

											 //
											 //
											 //  i2c Constants
											 //
											 //  This section is the master list of all of the 12c devices in use in the project
											 //
const uint8_t i2c_addr_cmsc = 0x79;     // i2c address for the CMSC device
const uint8_t i2c_addr_other = 0x78;     // dummy i2c address for the simulated HT16K33 used for oddly connected switches
const uint8_t i2c_addr_cp = 0x77;     // i2c address of the HT16K33 used for the Caution Panel (this board only has indicators)
const uint8_t i2c_addr_top = 0x76;     // i2c address of the HT16K33 used for the gear and the top line of indicators  *** this board no longer exists ***
const uint8_t i2c_addr_ufc = 0x75;     // i2c address of the HT16K33 used for the up front controller functions (This has no indicators)
const uint8_t i2c_addr_elec = 0x74;     // i2c address of the HT16K33 used for the CMSP and Electrical panel
const uint8_t i2c_addr_cmsp = 0x74;     // i2c address of the HT16K33 used for the CMSP and Electrical panel
const uint8_t i2c_addr_lhs = 0x73;     // i2c address of the HT16K33 used for the LHS switches (This is a general switch board)
const uint8_t i2c_addr_rhs = 0x72;     // i2c address of the HT16K33 used for the RHS switches (This is a general switch board)
const uint8_t i2c_addr_nmsp = 0x71;     // i2c address of the HT16K33 used for the NMSP unit (Navigation)
const uint8_t i2c_addr_test = 0x70;     // i2c address of the HT16K33 used to test new functions										//
const uint8_t i2c_addr_oled2 = 0x20;     // i2c address of the PCF8574 used for the cmspDisplay display
										 //const uint8_t i2c_addr_caution = 0x3F;   // i2c address of the ATtiny85 used for the I2C Master Caution LED (test)
										 // Note:  The ATTiny85 devices had to move below address 0x40 to avoid a problem with the ESP8266 controller which for some
										 //        unknown reason would not talk to ATTiny85 devices at higher addresses.  They worked ok with the ATMEL controller
const uint8_t i2c_addr_caution = 0x1F;   // i2c address of the ATtiny85 used for the I2C Master Caution LED
const uint8_t i2c_addr_gear = 0x1E;     // i2c address of the ATtiny85 used for the I2C Landing Gear Switch
const uint8_t i2c_addr_skid = 0x1D;     // i2c address of the ATtiny85 used for the I2C Landing Lightd/Skid switches
const uint8_t i2c_addr_enc0 = 0x10;     // i2c address of the ATtiny85 used for rotary encoder 0
const uint8_t i2c_addr_enc1 = 0x11;     // i2c address of the ATtiny85 used for rotary encoder 1
const uint8_t i2c_addr_enc2 = 0x12;     // i2c address of the ATtiny85 used for rotary encoder 2

										//
										//
										//
										// List here the highest and lowest i2c address for the HT16K33 chips.  These are managed to 
										// ensure that only the correct number of buffers are allocated.
#define LOWEST_I2C_ADDR 0x71            //this is the address of the lowest i2c HT16K33
#define HIGHEST_I2C_ADDR 0x7A           //this is the address of the highest i2c HT16K33 +2
										//


										/* * * Something very wierd seems to be going on with these class variables for deviceaddress so the order is critical (possibly due to a stack over write because the problem seems to have disappeared after an array was properly dimensioned ) * * */
										/*
										A10CLandingLightsandSkid skid(i2c_addr_skid);
										A10CMasterCaution MasterCautionController(i2c_addr_caution);
										A10CLandingGear gear(i2c_addr_gear);
										*/
A10CMasterCaution MasterCautionController(i2c_addr_caution);
A10CLandingGear gear(i2c_addr_gear);
A10CLandingLightsandSkid skid(i2c_addr_skid);

TQED qedILS0(i2c_addr_enc2);
TQED qedILS1(i2c_addr_enc1);
TQED qedTacan(i2c_addr_enc0);

boolean readkeys = false;
//  HT16K33 Constants
//
// Various HT16K33 constants etc 
#define HT16K33_BLINK_CMD 0x80
#define HT16K33_BLINK_DISPLAYON 0x01
#define HT16K33_BLINK_OFF 0
#define HT16K33_BLINK_2HZ  1
#define HT16K33_BLINK_1HZ  2
#define HT16K33_BLINK_HALFHZ  3
#define HT16K33_CMD_BRIGHTNESS 0xE0
#define HT16K33_KEY_RAM_ADDR 0x40        //this is the six bytes which hold the keys which have been pressed
#define HT16K33_KEY_INTERUPT_ADDR 0x60   
#define HT16K33_KEY_ROWINT_ADDR 0xA0   
//
//
uint16_t displaybuffer[HIGHEST_I2C_ADDR - LOWEST_I2C_ADDR][8];
boolean HT16K33Push[HIGHEST_I2C_ADDR - LOWEST_I2C_ADDR]; // Used to force a write to a particular HT16K33 chip
int16_t  kk[3];            // this is the integer array to contain a keycode and value pair
int16_t  keystack[3][64];  // this is the FIFO stack of key presses, key values and devices
uint8_t  keystackNext;     // this is the next keypress to be processed
uint8_t  keystackFree;     // this is the first free slot in the keystack 
uint8_t keys[HIGHEST_I2C_ADDR - LOWEST_I2C_ADDR][6];
uint8_t lastkeys[HIGHEST_I2C_ADDR - LOWEST_I2C_ADDR][6];
unsigned long switchLastIntTime[HIGHEST_I2C_ADDR - LOWEST_I2C_ADDR + 1];  // this is the time when the last interrupt was seen from the device (incl CMSC)
void connectToWiFi(const char *, const char *);
void writeCmscScreen(uint8_t, uint8_t, char *);
void MasterCaution(int);
void GearWarning(int);

//
void setLightCmsc(uint8_t, uint8_t, boolean);
//
// these are the LED coordinates for the Caution Panel
const uint8_t cautionPanel[2][48] = {
	{
		4,4,4,4,5,
		5,5,5,6,6,
	6,6,7,7,7,
	7,8,8,8,8,
	9,9,9,9,10,
	10,10,10,11,11,
	11,11,12,12,12,
	12,13,13,13,13,
	14,14,14,14,15,
	15,15,15 },
	{
		3,2,1,0,3,
		2,1,0,3,2,
	1,0,3,2,1,
	0,3,2,1,0,
	3,2,1,0,3,
	2,1,0,3,2,
	1,0,3,2,1,
	0,3,2,1,0,
	3,2,1,0,3,
	2,1,0 }
};
const uint8_t generalIndicators[3][24] = {  // these are the LED positions for various indicators
	{ 0,1,2,                // CMSC Missile Launch, Pri, Unk
	15,15,15,             // Gear good x3 These remain on an HT16K33
	10,                   // Gear Warning *** not certain this is still needed     
	10,8,7,               // NMSP backlit push buttons
	6,5,4,                // NMSP backlit push buttons
	3,11,12,              // NMSP backlit push buttons
	10,8,6,4,             // indicators on the UFC
	9,7,5,3               // indicators on the UFC
	},
	{ 0,0,0,
	5,4,6,
	7,
	4,4,4,
	4,4,4,
	4,4,4,
	4,5,6,7,
	4,5,6,7

	},
	{
		i2c_addr_cmsc,i2c_addr_cmsc,i2c_addr_cmsc,
		i2c_addr_lhs,i2c_addr_lhs,i2c_addr_lhs,
	i2c_addr_lhs,
	i2c_addr_nmsp,i2c_addr_nmsp,i2c_addr_nmsp,
	i2c_addr_nmsp,i2c_addr_nmsp,i2c_addr_nmsp,
	i2c_addr_nmsp,i2c_addr_nmsp,i2c_addr_nmsp,
	i2c_addr_ufc,i2c_addr_ufc,i2c_addr_ufc,i2c_addr_ufc,
	i2c_addr_ufc,i2c_addr_ufc,i2c_addr_ufc,i2c_addr_ufc
	}
};
const uint8_t defaultKeyPositions[7][6] = {  // these are the positions that the keys should be in when the plane is sitting on the apron with power off
	{ 0x40,0x00,0x00,0x00,0x00,0x00 },  //71 
	{ 0x00,0x00,0x00,0x00,0x00,0x00 },  //72 
	{ 0xA0,0x00,0x20,0x00,0x00,0x00 },  //73 
	{ 0x00,0x00,0x01,0x04,0x6A,0x00 },  //74 
	{ 0x00,0x00,0x00,0x00,0x00,0x00 },  //75 
	{ 0x00,0x00,0x00,0x00,0x00,0x00 },  //76 
	{ 0x00,0x00,0x00,0x00,0x00,0x00 }   //77
};

//
// Place the static text into an array
const char *Msgs[] = { "    A-10C CMSC  1   \0",
"  A-10C CMSP  2 \0",
"   Comms Stopped    \0",
"    Shutting Down   \0",
"Incor Switch:\0",
"   A-10C CMSC Test  \0" };
//

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
#define CMSC_DEV_ENC_AUD    0x01
#define CMSC_DEV_ENC_BRT    0x02
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
#define CMSC_DEV_SCREEN_CLR 0x00
// These are only used with a STATUS request to address and change one of more LED
#define CMSC_DEV_LED_1      0b00000001
#define CMSC_DEV_LED_2      0b00000010
#define CMSC_DEV_LED_3      0b00000100
#define CMSC_DEV_LED_1_VAL  0b00001000
#define CMSC_DEV_LED_2_VAL  0b00010000
#define CMSC_DEV_LED_3_VAL  0b00100000
// These are the codes for the key mappings
#define CMSC_BRT            850
#define CMSC_AUD            851
#define CMSC_BTN_BASE       809
/*
** End of the defines for the CMSC communications
*/

const uint8_t cmscPosn[2][3] = {         // these are positions for text on the CMSC display
	{ 12,0,0 },                              //
	{ 0,0,1 }
};
const uint8_t cmspPosn[2][5] = {         // these are positions for text on the CMSP display
	{ 0,0,4,8,12 },                          //
	{ 0,1,1,1,1 }
};
uint8_t cmscEncodersReady = 0;      // Global flags to indicate that the CMSC encoders need to be read.
float EncoderValues[10];            // This array holds the values of the encoders which get used when the encoders are reporting as if they are keys
									//
const char * boardNames[] = { "Test\0","NMSP\0","SW RHS\0","SW LHS\0","Elec / CMSP\0","Up Front\0","Top Line\0","Caution Panel\0" };
char displayBuffer[21];
char i2cBuffer[128];
// Display constructors
OLED_I2C cmspDisplay((uint8_t)i2c_addr_oled2, 16, 2);
//


#define LED_ON 1
#define LED_OFF 0
//
// General String variables
String command;
String value;
//
int i = 0;
int j = 0;
//
boolean commandsReady = false;
//
boolean inverterState = true;
boolean batteryState = true;
//
//
void setup() {
	Wire.begin(D2, D1);                                                                 //  sda = gpio4/D2   scl=gpio5/D1
	MasterCautionController.setLED(MASTERCAUTIONLED); // turn on the master caution LED on the UFC    
	Wire.endTransmission();   // try to send a stop bit to see if this can wake up a sleeping 12c bus
	for (uint8_t i = 0;i<HIGHEST_I2C_ADDR - LOWEST_I2C_ADDR;i++) HT16K33Push[i] = false;   // This flag is used to indicate that the LED buffer needs to be written to the HT16K33
	for (uint8_t i = LOWEST_I2C_ADDR;i< HIGHEST_I2C_ADDR;i++) initHT16K33(i);            // set up the HT16K33 i2c chips

																						 //initialise the key buffers 
	for (uint8_t i = 0;i<HIGHEST_I2C_ADDR - LOWEST_I2C_ADDR;i++) {
		initHT16K33(LOWEST_I2C_ADDR + i);
		for (uint8_t j = 0;j<6;j++) {
			keys[i][j] = (uint8_t)0;
		}
	}
	/* Debug
	for(uint8_t ii = LOWEST_I2C_ADDR;ii<HIGHEST_I2C_ADDR+1;ii++){  //this obtains a print of all the key ram
	getDefaultKeys(ii);
	}
	*/

	Serial.begin(115200);  // Initilize hardware serial:
	Serial.println("\n");
	testAll();

	oledAPUu8g2.begin();                              //initialize the ISP APU display
													  //Connect to the WiFi network
	connectToWiFi(networkName, networkPswd);

	while (!connected) {
		//oledAPUMsg("Awaiting WiFi",0,40);
		delay(1000);
	}
	oledAPUMsg("WiFi DCS Instruments V0.1", 0, 20);
	delay(1000);
	oledAPUMsg("Connected", 0, 40);
	cmspDisplay.clear();                  // Clear the OLED screen

}

void loop() {
	if (readSwitches(i2c_addr_ufc) | readSwitches(i2c_addr_elec) | readSwitches(i2c_addr_lhs) | readSwitches(i2c_addr_rhs) | readSwitches(i2c_addr_nmsp) | readSwitches(i2c_addr_cmsc) | readSwitches(i2c_addr_other) | ProcessEncoders()) {
		// one or more keys have been placed onto the keystack and need to be processed
		while (keystackCount()>0) {
			//      breath = false;
			keystackPop(kk);
			//  Now send the key info to the serial port
			//  This is done in multiple prints to avoid the use of a string  
			specialKeyActions;
			sprintf(replyBuffer, "C,%d,%d,\0", kk[2], kk[0]);
			// note about values:
			// Generally the decade of the value indicates the number of switchcodes (not positions) the switch has.
			// a code in the 40's has 5 switchcodes, in the 10's has 2 switchcodes, in the 0's has 1 switchcode.
			// A three position toggle will have only two switchcodes because the centre position is inferred.
			// The reason for this is to allow us to infer from the value in the table, how many switch codes are
			// associated with the real switch.
			// All of the switchcodes for a real switch are expected to be contiguous in the array.
			// The prime user of the information about the number of switchcodes is the Query function, in normal
			// operation, communicating a changed state does not care about the number of switchcodes there are.
			// 
			char tbuf[16];
			switch (kk[1]) {
			case 0:
			case 1:
			case -1:
				// unfortunately the float processing in sprintf for ESP8266 does not work!
				sprintf(tbuf, "%d.00\0", kk[1]);
				break;
			case 10:
			case 11:
			case 12:
				// this handles the tumbler
				sprintf(tbuf, "0.%d0\0", kk[1] - 10);
				break;
			case 40:
			case 41:
			case 42:
			case 43:
			case 44:
				// this handles the rotary switch  
				sprintf(tbuf, "0.%d0\0", kk[1] - 40);
				break;
			case 50:
			case 51:
			case 52:
			case 53:
			case 54:
			case 55:
			case 56:
			case 57:
			case 58:
			case 59:
				// this handles the real encoders which have a float for their value
				// Unfortunately the ESP8266 sprintf does not work for floats due to a bug :-(
				// The value minus 50 is the index into the float array which contains the 
				// encoder value. 
				dtostrf(EncoderValues[kk[1] - 50], 2, 2, &tbuf[0]);  // this is needed because of the ESP8266 sprintf problem
				EncoderValues[kk[1] - 50] = 0;  // once we've sent the float, set it to zero.
				break;
			default:
				sprintf(tbuf, "%d.00\0", kk[1]);
				break;
			}
			strncat(replyBuffer, tbuf, strlen(tbuf));
			processOutput(replyBuffer);
		}
	}
	yield();

	//only Receive data when connected
	if (connected) {
		int packetSize = udp.parsePacket();
		if (packetSize >0) {
			udpOutBndPort = udp.remotePort();  // grab the ephemeral port number quickly because it could go away
											   //receive data
			int packetLen = udp.read(packetBuffer, 576);
			if (packetLen > 0) {
				// Serial.print("Recv Data len=");Serial.print(packetLen,DEC);Serial.print(" ->");Serial.println(packetBuffer);
				packetBuffer[packetLen] = '\0';
				parseCmdPacket(packetBuffer, packetLen);
			}
		}
		if (commandsReady) {
			processOutput(replyBuffer);
		}
	}

	//  
	// per loop actions
	//
	if (APUdialsWrite) {  //process the APU dials if they need it
		oledAPUDials(APUSpdDial, APUEGTDial);
	}
	// Flush out the LED changes to the HT16K33 devices which need it
	for (i = 0;i<HIGHEST_I2C_ADDR - LOWEST_I2C_ADDR;i++) {
		if (HT16K33Push[i]) {
			displayHT16K33(LOWEST_I2C_ADDR + i);
			HT16K33Push[i] = false;
		}
	}

	yield();
}

void parseCmdPacket(char * packetBuffer, int packetLen) {
	for (uint16_t ii = 0;ii<packetLen;ii++) {
		if (packetBuffer[ii] == '*') {  // this removes the SIMID (4 bytes) from the start of the packet
			memcpy(packetBuffer, &packetBuffer[ii + 1], packetLen - ii);
			packetLen = packetLen - ii - 1;
			ii = packetLen;
		}
	}
	for (uint16_t ii = 0;ii<packetLen;ii++) {
		if (packetBuffer[ii] == ':') {
			memcpy(cmdBuffer, packetBuffer, ii);  // move the command into the command buffer
			cmdBuffer[ii] = '\0';
			memcpy(packetBuffer, &packetBuffer[ii + 1], packetLen - ii);  // shift the remaining contents to the start of the buffer
			packetLen = packetLen - ii - 1;
			ii = 0;
			processCmd(cmdBuffer);
		}
		else {
			if (ii == packetLen - 1) {
				memcpy(cmdBuffer, packetBuffer, ii + 1);  // move the command into the command buffer when it wasn't terminated by a :
				cmdBuffer[ii + 1] = '\0';
				packetLen = packetLen - ii;
				ii = packetLen + 1;
				processCmd(cmdBuffer);
			}
		}
	}
}
uint16_t getCmd(char * cmdBuffer) {
	//  This returns the command code from the command buffer ie the bit before the =
	char cmdCodeBuffer[5];
	int cmdLen = strlen(cmdBuffer);
	for (uint16_t ii = 0;ii<cmdLen;ii++) {
		if (cmdBuffer[ii] == '=') {
			memcpy(cmdCodeBuffer, cmdBuffer, ii);  // move the command code into its own string
			cmdCodeBuffer[ii] = '\0';
			return(atoi(cmdCodeBuffer));
		}
	}
	return(9999);
}
float getCmdValfloat(char * cmdBuffer) {
	// This returns the command value if it is a float value, ie the bit after the equals.
	char cmdValBuffer[10];
	int cmdLen = strlen(cmdBuffer);
	for (uint16_t ii = 0;ii<cmdLen;ii++) {
		if (cmdBuffer[ii] == '=') {
			memcpy(cmdValBuffer, &cmdBuffer[ii + 1], cmdLen - ii);  // move the command code into its own string
			cmdValBuffer[cmdLen - ii] = '\0';
			return(atof(cmdValBuffer));
		}
	}
	return(0.000001);
}
int getCmdValint(char * cmdBuffer) {
	// This returns the command value if it is intended to be an integer value, ie the bit after the equals.
	// The input still appears as a floating point number and is cast (which can have problems) on the return.
	char cmdValBuffer[10];
	int cmdLen = strlen(cmdBuffer);
	for (uint16_t ii = 0;ii<cmdLen;ii++) {
		if (cmdBuffer[ii] == '=') {
			memcpy(cmdValBuffer, &cmdBuffer[ii + 1], cmdLen - ii);  // move the command code into its own string
			cmdValBuffer[cmdLen - ii] = '\0';
			return((int)atof(cmdValBuffer));
		}
	}
	return(0);
}
char * getCmdValStr(char * cmdBuffer) {
	// This returns the command value as a string, ie the bit after the equals.
	char cmdValBuffer[576];
	int cmdLen = strlen(cmdBuffer);
	for (uint16_t ii = 0;ii<cmdLen;ii++) {
		if (cmdBuffer[ii] == '=') {
			memcpy(cmdValBuffer, &cmdBuffer[ii + 1], cmdLen - ii);  // move the command code into its own string
			cmdValBuffer[cmdLen - ii] = '\0';
			return(cmdValBuffer);
		}
	}
	return("\0");
}
void processCmd(char * cmdBuffer) {
	//
	// this function performs processing on a command which we are interested in.
	// all of the other commands are nops
	//
	float dialVal = 0;
	int cmdCode = getCmd(cmdBuffer);
	int cmdValue = 0;
	if (cmdCode <2080) {
		cmdValue = getCmdValint(cmdBuffer);
	}
	//value = getCmdValStr(cmdBuffer);
	// Serial.print("code: ");Serial.print(cmdCode,DEC);Serial.print(" = ");Serial.println(cmdValue,DEC);
	switch (cmdCode) {
	case 13:
		dialVal = 360 - (235 * getCmdValfloat(cmdBuffer) - 90);
		if (APUSpdDial != dialVal) {
			APUSpdDial = dialVal;
			APUdialsWrite = true;
		}
		break;
	case 14:
		dialVal = 360 - (255 * getCmdValfloat(cmdBuffer) - 90);
		if (APUEGTDial != dialVal) {
			APUEGTDial = dialVal;
			APUdialsWrite = true;
		}
		break;
	case 404: // Master Caution
		MasterCaution(cmdValue);
		break;
		//  This section is for the landing gear
	case 659: // these are the landing gear lights LANDING_GEAR_N_SAFE
	case 660: // LANDING_GEAR_L_SAFE
	case 661: // LANDING_GEAR_R_SAFE
		checkLight(generalIndicators[2][cmdCode - 659 + 3], generalIndicators[0][cmdCode - 659 + 3], generalIndicators[1][cmdCode - 659 + 3], cmdValue);  // turn on the LED      
		break;
	case 737: // landing gear warning light
		GearWarning(cmdValue);
		break;
		//  This section is for the lights on the top dashboard
	case 662:     // GUN_READY
	case 663:     // NOSEWHEEL_STEERING
	case 664:     // MARKE R_BEACON    
	case 665:     // CANOPY_UNLOCKED
#ifdef DEBUG
		if (cmdCode == 663) {
			// Use NWS indicator as the debug toggle
			if (value == "1") { nldebug = true; }
			else { nldebug = false; }
		}
#endif
		// two LEDs per light and each is independently addressable.  
		checkLight(generalIndicators[2][cmdCode - 662 + 16], generalIndicators[0][cmdCode - 662 + 16], generalIndicators[1][cmdCode - 662 + 16], cmdValue);
		checkLight(generalIndicators[2][cmdCode - 662 + 20], generalIndicators[0][cmdCode - 662 + 20], generalIndicators[1][cmdCode - 662 + 20], cmdValue);
		break;
	case 372:     // CMSC_MissileLaunch
	case 373:     // CMSC_PriorityStatus
	case 374:     // CMSC_UnknownStatus
		checkLight(generalIndicators[2][cmdCode - 372 + 0], generalIndicators[0][cmdCode - 372 + 0], generalIndicators[1][cmdCode - 372 + 0], cmdValue);
		break;
		//  This section is for the AoA indicators
	case 540: // High
	case 541: // OK
	case 542: // Low
			  // These are no longer implemented
			  // checkLight(generalIndicators[2][cmdCode-540+4],generalIndicators[0][cmdCode-540+4],generalIndicators[1][cmdCode-540+4],cmdValue);  // turn on the LED      
		break;
		//  This section is for the Air refuel Indicators on RHS
	case 730: // AIR_REFUEL_READY
	case 731: // REFUEL_LATCHED
	case 732: // AIR_REFUEL_DISCONNECT
			  // These are no longer implemented
			  // checkLight(generalIndicators[2][cmdCode-730+14],generalIndicators[0][cmdCode-730+14],generalIndicators[1][cmdCode-730+14],cmdValue);  // turn on the LED      
		break;
		// These values are the Caution Display Panel                     
	case 480:  //ENG START CYCLE
	case 481:  //L HYD PRESS
	case 482:  //R HYD PRESS
	case 483:  //GUN UNSAFE
	case 484:  //ANTISKID
	case 485:  //L HYD RES
	case 486:  //R HYD RES
	case 487:  //OXY LOW
	case 488:  //ELEV DISENG
	case 489:  //VOID1
	case 490:  //SEAT NOT ARMED
	case 491:  //BLEED AIR LEAK
	case 492:  //AIL DISENG
	case 493:  //L AIL TAB
	case 494:  //R AIL TAB
	case 495:  //SERVICE AIR HOT
	case 496:  //PITCH SAS
	case 497:  //L ENG HOT
	case 498:  //R ENG HOT  @@@
	case 499:  //WINDSHIELD HOT
	case 500:  //YAW SAS  @ @ @ 
	case 501:  //L ENG OIL PRESS
	case 502:  //R ENG OIL PRESS
	case 503:  //CICU
	case 504:  //GCAS
	case 505:  //L MAIN PUMP
	case 506:  //R MAIN PUMP
	case 507:  //VOID2
	case 508:  //LASTE
	case 509:  //L WING PUMP
	case 510:  //R WING PUMP
	case 511:  //HARS
	case 512:  //IFF MODE 4
	case 513:  //L MAIN FUEL LOW*
	case 514:  //R MAIN FUEL LOW
	case 515:  //L R TKS UNEQUAL
	case 516:  //EAC
	case 517:  //L FUEL PRESS
	case 518:  //R FUEL PRESS
	case 519:  //NAV
	case 520:  //STALL SYS
	case 521:  //L CONV @ @ @ 
	case 522:  //R CONV
	case 523:  //CADC
	case 524:  //APU GEN
	case 525:  //L GEN
	case 526:  //R GEN
	case 527:  //INST INV   @@@
		checkLight(i2c_addr_cp, cautionPanel[0][cmdCode - 480], cautionPanel[1][cmdCode - 480], cmdValue);
		break;
	case 606:  //NMSP - HARS
	case 608:  //NMSP - EGI 
	case 610:  //NMSP - TISL
	case 612:  //NMSP - STRPT
	case 614:  //NMSP - ANCHR
	case 616:  //NMSP - TCN
	case 618:  //NMSP - ILS
	case 619:  //NMSP - UHF
	case 620:  //NMSP - FM
		if (cmdCode == 619) cmdCode = 622;  // this is to help cope with the fact that the UHF is the only odd code - so the order of UHF and FM has swapped over by this approach        
		cmdCode = cmdCode / 2;             // the code is halved to save space in the array
		checkLight(generalIndicators[2][cmdCode - 303 + 7], generalIndicators[0][cmdCode - 303 + 7], generalIndicators[1][cmdCode - 303 + 7], cmdValue);
		break;
	case 2087:  // this is the CMSP Counter Measures list_indication(7)
				// we have a multiline (terminated by x0A) ascii values to munge into a form to go onto oled2
		processCMS(cmdBuffer);
		break;
	case 2088:  // this is the CMSC Counter Measures list_indication(8)
				// we have a multiline (terminated by x0A) ascii values to munge into a form to go onto the oled on the CMSC device
		processCMS(cmdBuffer);
		break;
	default:
		break;
	}
}

boolean readSwitches(uint8_t i2c_address) {
	// testing suggests that once the switches are read, it takes around 19ms before another interrupt
	// will occur, even if the key is held down for the whole time.

	// this code can be in one of 4 states
	// 1 - no keys are pressed and everything dealt with (this is the normal state)
	// 2 - Interrupt set and one or more keys have been pressed - The interrupt might be set for a new key and we need to spot that an old key has been released
	// 3 - No interrupt, but keys have been recently pressed  - this should mean that all of the keys which have been pressed have now been released
	// 4 - For the CMSC device, there is no interrupt so check for available data also processes it.  This code is for when this happens
	//
	// For this reason, we need to maintain a map of all of the keys which have transitioned from on to off, so that we can raise a keypress event
	// and communicate that the key has now been released.
	// Because we do not get an interupt for a key being released, we need to remember that there is work to be done for the scan after the last 
	// interupt.   

	boolean keypressed = false;
	boolean keychanged = false;
	uint8_t keydataavail;

	keydataavail = keydataavailable(i2c_address);
	//Serial.print(" keydataavailable(): ");Serial.print(i2c_address,HEX);Serial.print(" - ");Serial.println(keydataavail,HEX);

	switch (keydataavail) {
	case SWITCHSET:
		memcpy(lastkeys[i2c_address - LOWEST_I2C_ADDR], keys[i2c_address - LOWEST_I2C_ADDR], sizeof(keys[i2c_address - LOWEST_I2C_ADDR])); //save the previous set of keys  
		switch (i2c_address) {
		case i2c_addr_cmsc:
			// the CMSC should only return SWITCHALREADYDONE or SWITCHNONE when interrogated
			break;
		case i2c_addr_other:
			getOtherSwitchData(i2c_address);
			break;
		default:
			// used for HT16K33's
			getSwitchData(i2c_address);
			break;
		}
		for (uint8_t i = 0;i<6;i++) {
			if (lastkeys[i2c_address - LOWEST_I2C_ADDR][i] != keys[i2c_address - LOWEST_I2C_ADDR][i]) { keychanged = true;break; }
		}
		if (keychanged) {
			for (uint8_t i = 0; i<6; i++) {
				// we are about to do two passes of the data
				// the first is to find keys which have just been pressed
				// the second is to find keys which have just been released
				//
				// New keys being pressed
				uint8_t trans = ~lastkeys[i2c_address - LOWEST_I2C_ADDR][i] & keys[i2c_address - LOWEST_I2C_ADDR][i];  // find out if any keys were newly pressed
				if (trans != 0x00) {
					//printkeys();
					// we have found at least one key which has been newly pressed ie it was not set last time but is set now.
					for (uint8_t j = 0;j<8;j++) {
						if (0x01 & (trans >> j)) {
							keystackPush((i * 8) + j + 1 + ((i2c_address - 0x70) * 100), true);  //place a keycode onto the stack as pressed - This is the lowest possible address rather than the current lowest.
							keypressed = true;
						}
					}
				}
				// New keys which have been released
				trans = ~keys[i2c_address - LOWEST_I2C_ADDR][i] & lastkeys[i2c_address - LOWEST_I2C_ADDR][i];  // find out if any keys were newly released
				if (trans != 0x00) {
					//printkeys();
					// we have found at least one key which has been newly released ie it was set last time but is not set now.
					for (uint8_t j = 0;j<8;j++) {
						if (0x01 & (trans >> j)) {
							keystackPush((i * 8) + j + 1 + ((i2c_address - 0x70) * 100), false);  //place a keycode onto the stack as released
							keypressed = true;
						}
					}
				}
			}
		}
		else {
			// this is for when a key is pressed but it is not a new key
			keypressed = false;
		}
		break;
	case SWITCHWAIT:
		// This happens when we are too early to detect an interupt so we will just return reporting that we have
		// not done anything.
		keypressed = false;
		break;
	case SWITCHALREADYDONE:
		// For devices such as CMSC where the cost of the keys is the same as the cost of an interrupt so the keys get stacked in the keydataavailable() routine
		keypressed = true;
		break;
	case SWITCHNONE:
	default:
		// this is most likely that there are no changed keys
		// this is for when we have validly checked an interrupt and none was set.          
		//for(uint8_t i=0;i<6;i++) keys[i2c_address-LOWEST_I2C_ADDR][i] = 0x00;  // rather than do an i2c read, we just set the current keys to 0
		keypressed = false;
		break;
	}
	return keypressed;  // let caller know that we have some key data
}
void getOtherSwitchData(uint8_t i2c_address) {
	// this function is to read any special switches which have been wired differently
	// the general approach is to only report the transitions, and to make it simpler for 
	// the processing of the key presses, we use a dummy code from an HT16K33
	keys[i2c_address - LOWEST_I2C_ADDR][0] = (gear.readSwitch() & 0b10000000) | skid.readSwitch();
	// gear = 0x80          808
	// landing light = 0x04 803
	// taxi light = 0x02    802
	// antiskid = 0x01      801
	return;
}
int keydataavailable(uint8_t i2c_address) {
	//
	// Testing suggests that once the HT16K33 switches are read, it takes around 19ms before another interrupt
	// will occur, even if the key is held down for the whole time. This only refers to the switches on a HT16K33
	//
	// This routine also attempts to mimic an interrupt for the last key being released so the first
	// time the interrupt flag is checked after it has been set, we will still report that the interrupt
	// has been set to give a chance to push the final releases onto the stack.
	//
	// Toggle switches which are set closed will always present an interrupt rather than just on the transition.
	//
	int retvalue = SWITCHNONE;
	byte interrupt_flag = 0;;
	time1 = micros();                            // only do a call to the clock once
	if (time1 - switchLastIntTime[i2c_address - LOWEST_I2C_ADDR] > 20000) {  // only check the interrupt if it has had time to get set
		switch (i2c_address) {
		case i2c_addr_cmsc:
			// for the CMSC device, there is no interrupt to test per se.  We poll the device to get status, then 
			// we read the switch state in the returned byte
			// we need to ask the whole CMSC device to give us status.
			//
			Wire.beginTransmission(i2c_address);
			Wire.write(CMSC_DEV_STATUS);
			Wire.endTransmission();
			Wire.requestFrom(i2c_address, (byte)1);    // read the interrupt status back from the CMSC device.
													   //
			while (Wire.available()) {                 // Slave may send less than requested
				byte b = Wire.read();                    // receive a byte as character
				byte c = b & 0b00011111;                 // create a byte with only the switch bits
				byte d = c ^ cmscLastSwitchState;        // mask of the different bits
				cmscEncodersReady = b >> 6;                  // move the encoder flags to the low order bits

				if (cmscEncodersReady) {
					processCmscEncoders(cmscEncodersReady);
					retvalue = SWITCHALREADYDONE;
				}   // get the values from the CMSC encoders if they are ready

				if (d) {
					//Serial.print("dif o:");Serial.print(cmscLastSwitchState,BIN);Serial.print(" n: ");Serial.print(c,BIN);Serial.print(" d: ");Serial.println(d,BIN);  
					// one or more of the switch bits has changed
					// we'll find out which ones and report them
					for (uint8_t i = 0;i <= 5;i++) {
						// the keycodes are 809 CMSC-RWR-JMR | 810 CMSC-RWR-MWS | 811 CMSC-RWR-PRI | 812 CMSC-RWR-UNK | 813 CMSC-RWR-ML
						// these keycodes are designed to represent the second byte of bits for device 0x78
						if (0x01 & (d >> i)) {
							// New keys being pressed
							if (0x01 & (c >> i)) {          // We need to determine if it is key up or key down
								keystackPush(i + CMSC_BTN_BASE, true);   // place a keycode onto the stack as pressed - This is the lowest possible address rather than the current lowest
							}
							else {
								// New keys which have been released
								keystackPush(i + CMSC_BTN_BASE, false);  // place a keycode onto the stack as released
							}
						}
					}
					cmscLastSwitchState = c; // save away the value at the time we noticed that it changed last
											 // we'll now say no switches were pressed because we have already processed them
					retvalue = SWITCHALREADYDONE;
				}
				else {

				}
			}
			if (retvalue != 0) {
				//Serial.print("CMSC ");Serial.println(retvalue,HEX);
			}
			return retvalue;
			break;
		case i2c_addr_other:
			// for this pseudo-device there is no interrupt to test and it is multiple devices in reality
			// we want to avoid duplicate work (and using more variables)
			uint8_t tmpkeys[6];                    // get 6 bytes to save the previous keys away to
			memcpy(tmpkeys, keys[i2c_address - LOWEST_I2C_ADDR], sizeof(keys[i2c_address - LOWEST_I2C_ADDR])); //save the previous set of keys  
			getOtherSwitchData(i2c_address);  // this fetches new key values into the keys array, it is a bit wasteful because if there is key action we'll go get it again.
			for (uint8_t i = 0;i<6;i++) {         // we're only really interested in the first byte, but we'll check them all.
				if (keys[i2c_address - LOWEST_I2C_ADDR][i] != tmpkeys[i]) {
					// if we find that there is a change, we raise the interrupt and then restore the previous value             
					interrupt_flag = 0xff;
					memcpy(keys[i2c_address - LOWEST_I2C_ADDR], tmpkeys, sizeof(keys[i2c_address - LOWEST_I2C_ADDR])); // copy back the original key values
					break;  // leave the for loop
				}
			}
			break;
		default:
			// this is for the HT16K33 based devices which have an interrupt to tell us a key has been pressed
			Wire.beginTransmission(i2c_address);
			Wire.write(HT16K33_KEY_INTERUPT_ADDR | 0);
			Wire.endTransmission();
			Wire.requestFrom(i2c_address, (byte)1);    // read the interrupt status
			while (Wire.available()) {                 // slave may send less than requested
				interrupt_flag = Wire.read();             // receive a byte as character
			}
			/*
			if(interrupt_flag){
			Serial.print("HT16K33 Interrupt from ");Serial.print(i2c_address,HEX);Serial.print(" : ");Serial.println(interrupt_flag,HEX);
			}
			*/
			break;
		}
		if (interrupt_flag) {
			switchLastIntTime[i2c_address - LOWEST_I2C_ADDR] = time1;   // if the interrupt was set then we need to save the time so that we don't recheck it too quickly
			return SWITCHSET;                                         // this means that there was a valid interrupt
		}
		else {
			// so no interrupt and we've left enough time for it to have occurred/
			// if the interrupt had been pressed on the last pass then we will return an interrupt
			// so that the released keys can be processed, otherwise we simply return a no keys response
			// we use the last interrupt time to determine this.  
			if (switchLastIntTime[i2c_address - LOWEST_I2C_ADDR] == 0) {
				return SWITCHNONE;                        // no switch was pressed
			}
			else {
				switchLastIntTime[i2c_address - LOWEST_I2C_ADDR] = 0;   // say no dummy interrupt is needed next time
				return SWITCHSET;                                     // say a key was pressed even though none was to allow the released key to be processed
			}
		}
	}
	else {
		// it is too soon for another interupt to be set so we want to return saying 
		// that it is too short a time since the last interupt to process switch data.
		return SWITCHWAIT;  // Too early
	}
	return true;
}
void specialKeyActions(void) {
	// This routine is to handle inter-device events where which the simulator does not communicate to us
	// 
	// Currently the events handled here are:
	// 1) turning off the inverter or the battery switches does not turn off the CMSC display or the CMSP display
	//
	if (kk[0] == 3006 & kk[2] == 1) {
		// this is the battery switch
		if (kk[1] == 1) {
			batteryState = true;
		}
		else {
			batteryState = false;
			cmspDisplay.clear();
			writeCmscScreen(i2c_addr_cmsc, 0xff, "");
		}

	}
	if (kk[0] == 3002 & kk[2] == 1) {
		// this is the inverter switch
		if (kk[1] == 1) {
			inverterState = true;
		}
		else {
			inverterState = false;
			cmspDisplay.clear();
			writeCmscScreen(i2c_addr_cmsc, 0xff, "");
		}
	}
}
void GearWarning(int parm) {
	// This special case code is needed because of the way the toggle switch with integrated LED had to be wired with a permanent 5V on it.
	if (parm == 1) {
		gear.setLED(GEARWARNINGLED);  // Turn on the Gear Warning LED
	}
	else {
		gear.resetLED(GEARWARNINGLED);  // Turn off the Gear Warning LED
	}
}
void MasterCaution(int parm) {
	// This special case code is needed because of the way the toggle switch with integrated LED had to be wired with a permanent 5V on it.
	if (parm == 1) {
		MasterCautionController.setLED(MASTERCAUTIONLED); // turn on the master caution LED on the UFC    
	}
	else {
		MasterCautionController.resetLED(MASTERCAUTIONLED); // turn on the master caution LED on the UFC    
	}
}
void setLightCmsc(uint8_t i2c_address, uint8_t n, boolean s) {
	Wire.beginTransmission(i2c_address);
	if (s) {
		Wire.write(CMSC_DEV_STATUS | (1 << n) | (1 << n + 3));     // set LED n on
	}
	else {
		Wire.write(CMSC_DEV_STATUS | (1 << n) & ~(1 << n + 3));  // set LED n off
	}
	Wire.endTransmission();
	Wire.requestFrom((int)i2c_address, (int)1);           // try to read the byte containing the key presses & encoder states
	uint8_t i = 0;
	while (Wire.available()) {                  // slave may send less than requested
		i2cBuffer[i++] = Wire.read();             // receive a byte as character 
	}
	// we should only ever get a single byte back from this request but we still buffer it.
	i2cBuffer[i++] = '\0';
	//Serial.print("rcvd-");Serial.print(i-1,HEX);Serial.print(":");Serial.println((byte)i2cBuffer[i-2],BIN); 
}
void writeCmscScreen(uint8_t i2c_address, uint8_t n, char * c) {
	const uint8_t cmscScreenZone[3] = { 0x01,0x02,0x03 };
	// Serial.print("CMSC Screen: ");Serial.print(CMSC_DEV_SCREEN|cmscScreenZone[n],HEX);Serial.print(" : ");Serial.print(n,DEC);Serial.print(" |");Serial.print(c);Serial.println("|");
	Wire.beginTransmission(i2c_address);
	if (n == 0xff) {
		Wire.write(CMSC_DEV_SCREEN);                       // zone value of 0 is a clear screen so no message data
	}
	else {
		Wire.write((byte)CMSC_DEV_SCREEN | cmscScreenZone[n]);     // select the correct screen zone to position the cursor
		Wire.write(c, (int)strlen(c));

	}
	Wire.endTransmission();                            // we do not expect data to be sent back
	delay(10);                                         // needed to avoid missing successive screen writes
}
void processCmscEncoders(byte b) {
	/*  On entry we need to find out which CMSC encoders are ready to be read and
	*  get the data out of them.  There are two encoders on the CMSC board and they
	*  both could have data.
	*/
	union { byte b[8];float f[2]; } static encData;  // this is used to get two floats from a byte array received over i2c 
													 // I was consistantly getting ENOSPC when this was a global, so I changed
													 // this so the floats get copied into global float array which seemed to 
													 // cure the problem.
	Wire.beginTransmission(i2c_addr_cmsc);
	Wire.write(CMSC_DEV_ENC | b);                 // Tell the CMSC we want to read the encoders
	Wire.endTransmission();
	int ii = 0;                               // set to zero for the first and both encoder scenario
	int tDataAmt = 0;
	switch (b) {
	case 0b00000011:
		tDataAmt = Wire.requestFrom(i2c_addr_cmsc, (sizeof(float) * 2));  // ask for two floats
		if (tDataAmt != (sizeof(float) * 2)) {
			//Serial.print("Data Recv Mismatch 11: ");Serial.println(tDataAmt,DEC);
		}
		keystackPush(CMSC_AUD, true);
		keystackPush(CMSC_BRT, true);
		break;
	case CMSC_DEV_ENC_BRT:
		tDataAmt = Wire.requestFrom(i2c_addr_cmsc, sizeof(float));  // ask for one float
		if (tDataAmt != (sizeof(float))) {
			//Serial.print("Data Recv Mismatch 01: ");Serial.println(tDataAmt,DEC);
		}
		keystackPush(CMSC_BRT, true);
		break;
	case CMSC_DEV_ENC_AUD:
		tDataAmt = Wire.requestFrom(i2c_addr_cmsc, sizeof(float));  // ask for one float
		if (tDataAmt != (sizeof(float))) {
			//Serial.print("Data Recv Mismatch 10: ");Serial.println(tDataAmt,DEC);
		}
		ii = 4;  // put the data into the second float.
		keystackPush(CMSC_AUD, true);
		break;
	default:
		//Serial.print("No encoders requested: \0");
		break;
	}
	//Serial.print("about to recv: ");Serial.print(ii,DEC);Serial.print(" | ");Serial.println(tDataAmt,DEC);
	int jj;
	for (jj = ii; jj<ii + tDataAmt; jj++) {
		encData.b[jj] = Wire.read();                       // receive the float(s) as byte array
														   //Serial.print(encData.b[jj],HEX);Serial.print(":");
	}
	//Serial.println();
	/*
	for(ii=0;ii<8;ii++){
	Serial.print(encData.b[ii],HEX);
	}
	Serial.println();
	*/
	// We should now have one or two floats in the buffer
	//  save into globals
	EncoderValues[0] = encData.f[0];
	EncoderValues[1] = encData.f[1];
	//Serial.print("enc recv ");Serial.print(encData.f[0]);Serial.print(" : ");Serial.println(encData.f[1]);
}
void processCMS(char * cmdBuffer) {
	// This routine is for the text messages on both the CMSP and the CMSC
	// The variable length of the payload means the message needs to be parsed and split on nl characters
	// The format is sets of three lines, the first is dashes, the second the field name and the last is the field value
	// The values all need to be padded with blanks

	/*
	-----------------------------------------
	txt_UP
	RDY OFF OFF RDY
	-----------------------------------------
	txt_DOWN1
	MWS
	-----------------------------------------
	txt_DOWN2
	JMR
	-----------------------------------------
	txt_DOWN3
	RWR
	-----------------------------------------
	txt_DOWN4
	DISP


	-----------------------------------------
	txt_CHAFF_FLARE
	A240m120
	-----------------------------------------
	txt_JMR
	OFF
	-----------------------------------------
	txt_MWS
	ACTIVE
	*/

	const char * MsgTxtTypes[] = { "UP","DOWN1","DOWN2","DOWN3","DOWN4","CHAFF_FLARE","JMR","MWS" };
	const uint8_t padding[] = { 16,4,4,4,4,8,8,8 };
	char cmsMsgLine[0x30];
	char * cmsMsgPtr1;  char * cmsMsgPtr2;
	//Serial.println("Parsing CMS message");
	cmsMsgPtr1 = cmdBuffer + 5;                // start after the 208x=
	cmsMsgPtr2 = strchr(cmsMsgPtr1, '\n');   // this should find the NL charater at the end of the first line
	while (cmsMsgPtr2 != NULL) {
		memcpy(cmsMsgLine, cmsMsgPtr1, cmsMsgPtr2 - cmsMsgPtr1);
		cmsMsgLine[cmsMsgPtr2 - cmsMsgPtr1] = '\0';
		if (strncmp(cmsMsgLine, "txt_", 4) == 0) {
			for (uint8_t j = 0;j<8;j++) {  // find out which txt_ cmd we're processing
				if (strcmp(&cmsMsgLine[4], MsgTxtTypes[j]) == 0) {
					// get the actual value from the next line
					cmsMsgPtr1 = cmsMsgPtr2 + 1;
					cmsMsgPtr2 = strchr(cmsMsgPtr1, '\n');
					//  We need to do some blank padding, and it would be preferable to do this on the display device, however we need to do this
					//  in this program for the CMSP so we might as well do it for the CMSC as well.
					memcpy(cmsMsgLine, "                 ", padding[j]);  // make sure that we do not leave any characters from the last value
					memcpy(cmsMsgLine, cmsMsgPtr1, cmsMsgPtr2 - cmsMsgPtr1);
					cmsMsgLine[padding[j]] = '\0';
					//cmsMsgLine[4] = '\0';
					if (j<5) {
						// This is for the CMSP
						if (inverterState && batteryState) {
							cmspDisplay.setCursor(cmspPosn[0][j], cmspPosn[1][j]);
							cmspDisplay.print(cmsMsgLine);
						}
						else {
							cmspDisplay.clear();
						}
					}
					else {
						// This must be a CMSC text message so we subtract 5 from the index before referencing
						// the positioning array
						/*
						* Messages are transmitted over i2c us to the
						* 32U4 board which used to contol the whole lot but has been changed to be a slave.
						*
						*/
						//Serial.print(MsgTxtTypes[j]);Serial.print(" : ");Serial.println(cmsMsgLine);       

						if (inverterState && batteryState) {
							writeCmscScreen(i2c_addr_cmsc, j - 5, cmsMsgLine);
						}
						else {
							writeCmscScreen(i2c_addr_cmsc, 0xff, cmsMsgLine);
						}
					}
					break; //leave the checking loop
				}
			}
		}
		cmsMsgPtr1 = cmsMsgPtr2 + 1;
		cmsMsgPtr2 = strchr(cmsMsgPtr1, '\n');
	}
}
/*
* * * This code handles the polling and reporting of the I2C encoders running AtTiny85's
* * * for this version of the code, we use the normal keycode mechanism to report the
* * * values of the encoders, and we do this using value of 50.
*/
// these are the keycodes for the encoders and need to appear in the key mappings table
#define  ENC_ILS0  852
#define  ENC_ILS0s 853
#define  ENC_ILS1  854
#define  ENC_ILS1s 855
#define  ENC_TACAN 856

boolean ProcessEncoders(void) {
	// this routine gets current values from I2C encoders
	//Display the counter value
	boolean ret = false;
	int16_t tempEncoderVal;
	int8_t tempDir;
	float reportValue;
	tempEncoderVal = qedILS0.getDelta(0); // get the switched version of counter
	if (tempEncoderVal != 0) {
		//Serial.print("ILS0 0:");Serial.println(tempEncoderVal);
		tempEncoderVal < 0 ? tempDir = -1 : tempDir = 1;
		reportValue = tempEncoderVal * tempEncoderVal * tempDir *0.01; // cheap way to square value without losing the sign
		if (reportValue > 1)reportValue = 1;
		else if (reportValue < -1)reportValue = -1;
		keystackPush(ENC_ILS0, true);
		EncoderValues[ENC_ILS0 - 850] = reportValue;
		//Serial.print("C,53,3001,");Serial.println(reportValue);  //ILS left knob
		ret = true;
	}
	tempEncoderVal = qedILS0.getDelta(1); // get the switched version of counter
	if (tempEncoderVal != 0) {
		//Serial.print("ILS0 1:");Serial.println(tempEncoderVal);
		tempEncoderVal < 0 ? tempDir = -1 : tempDir = 1;
		reportValue = tempEncoderVal * tempEncoderVal * tempDir *0.01; // cheap way to square value without losing the sign
		if (reportValue > 1)reportValue = 1;
		else if (reportValue < -1)reportValue = -1;
		keystackPush(ENC_ILS0s, true);
		EncoderValues[ENC_ILS0s - 850] = reportValue;
		//Serial.print("C,53,3002,");Serial.println(reportValue);  //ILS left knob
		ret = true;
	}
	tempEncoderVal = qedILS1.getDelta(0); // get the switched version of counter
	if (tempEncoderVal != 0) {
		//Serial.print("ILS1 0:");Serial.println(tempEncoderVal);
		tempEncoderVal < 0 ? tempDir = -1 : tempDir = 1;
		reportValue = tempEncoderVal * tempEncoderVal * tempDir*0.01; // cheap way to square value without losing the sign
		if (reportValue > 1)reportValue = 1;
		else if (reportValue < -1)reportValue = -1;
		keystackPush(ENC_ILS1, true);
		EncoderValues[ENC_ILS1 - 850] = reportValue;
		//Serial.print("C,53,3003,");Serial.println(reportValue);  //ILS right knob
		ret = true;
	}
	tempEncoderVal = qedILS1.getDelta(1); // get the switched version of counter
	if (tempEncoderVal != 0) {
		// Serial.print("ILS1 1:");Serial.println(tempEncoderVal);
		tempEncoderVal < 0 ? tempDir = -1 : tempDir = 1;
		reportValue = tempEncoderVal * tempEncoderVal * tempDir*0.01; // cheap way to square value without losing the sign
		if (reportValue > 1)reportValue = 1;
		else if (reportValue < -1)reportValue = -1;
		keystackPush(ENC_ILS1s, true);
		EncoderValues[ENC_ILS1s - 850] = reportValue;
		//Serial.print("C,53,3004,");Serial.println(reportValue);  //ILS right knob
		ret = true;
	}
	tempEncoderVal = qedTacan.getDelta(0); // get the switched version of counter
	if (tempEncoderVal != 0) {
		tempEncoderVal < 0 ? tempDir = -1 : tempDir = 1;
		reportValue = tempEncoderVal * tempEncoderVal * tempDir*0.01; // cheap way to square value without losing the sign
		if (reportValue > 1)reportValue = 1;
		else if (reportValue < -1)reportValue = -1;
		keystackPush(ENC_TACAN, true);
		EncoderValues[ENC_TACAN - 850] = reportValue;
		// Serial.print("C,51,3001,");Serial.println(reportValue);   //TACAN - This is the 10's
		ret = true;
	}
	return ret;
}
void drawPixel(uint8_t i2c_address, int16_t x, int16_t y, uint16_t color) {
	if ((y < 0) || (y >= 8)) return;
	if ((x < 0) || (x >= 16)) return;

	if (color) {
		displaybuffer[i2c_address - LOWEST_I2C_ADDR][y] |= 1 << x;
	}
	else {
		displaybuffer[i2c_address - LOWEST_I2C_ADDR][y] &= ~(1 << x);
	}
}
void displayHT16K33(uint8_t i2c_address)
{
	Wire.beginTransmission(i2c_address);
	Wire.write((uint8_t)0x00); // start at address $00
	for (uint8_t i = 0; i<8; i++) {
		Wire.write(displaybuffer[i2c_address - LOWEST_I2C_ADDR][i] & 0xFF);
		Wire.write(displaybuffer[i2c_address - LOWEST_I2C_ADDR][i] >> 8);
	}
	Wire.endTransmission();
}
void initHT16K33(uint8_t i2c_address)
{
	Wire.begin();
	Wire.beginTransmission(i2c_address);
	Wire.write(0x21);  // turn on oscillator
	Wire.endTransmission();
	Wire.beginTransmission(i2c_address);
	Wire.write(HT16K33_BLINK_CMD | HT16K33_BLINK_DISPLAYON | (0 << 1));
	Wire.endTransmission();
	Wire.beginTransmission(i2c_address);
	Wire.write(HT16K33_CMD_BRIGHTNESS | 15);
	Wire.endTransmission();
	Wire.beginTransmission(i2c_address);
	Wire.write(HT16K33_KEY_ROWINT_ADDR | 0);
	Wire.endTransmission();
	displayClear(i2c_address);
}
void displayClear(uint8_t i2c_address) {
	Wire.beginTransmission(i2c_address);
	Wire.write((uint8_t)0x00); // start at address $00
	for (uint8_t i = 0; i<8; i++) {
		displaybuffer[i2c_address - LOWEST_I2C_ADDR][i] = 0;
		Wire.write(displaybuffer[i2c_address - LOWEST_I2C_ADDR][i] & 0xFF);
		Wire.write(displaybuffer[i2c_address - LOWEST_I2C_ADDR][i] >> 8);
	}
	Wire.endTransmission();
}
void getSwitchData(uint8_t i2c_address) {
	// get the six bytes of switch memory from the i2c bus
	uint8_t i;
	Wire.beginTransmission(i2c_address);
	Wire.write(HT16K33_KEY_RAM_ADDR);
	Wire.endTransmission();
	Wire.requestFrom(i2c_address, (byte)6);  // try to read the six bytes containing the key presses
	i = 0;
	while (Wire.available()) {                  // slave may send less than requested
		keys[i2c_address - LOWEST_I2C_ADDR][i++] = Wire.read();  // receive a byte as character 
	}
}
void checkLight(uint8_t i2c_address, int16_t x, int16_t y, uint8_t parm)
{
	if (parm == 1) {
		if (i2c_address == i2c_addr_cmsc) {
			// The light is not on an HT16K33 
			setLightCmsc(i2c_addr_cmsc, x, HIGH);
		}
		else {
			drawPixel(i2c_address, x, y, LED_ON);
			HT16K33Push[i2c_address - LOWEST_I2C_ADDR] = true;
		}
	}
	else {
		if (i2c_address == i2c_addr_cmsc) {
			// The light is not on an HT16K33     
			setLightCmsc(i2c_addr_cmsc, x, LOW);
		}
		else {
			drawPixel(i2c_address, x, y, LED_OFF);
			HT16K33Push[i2c_address - LOWEST_I2C_ADDR] = true;
		}
	}
}
void keystackPush(int16_t key, boolean pressed) {
	boolean ignoreKey = false;
	int16_t kk[3] = { 0,0,0 };  // this is used for the keycode value set
	uint8_t i = 0;
	//test
	//Serial.print("Keycode :  ");Serial.println(key);
	//end test
	if (convertKey(kk, key)) {  // translate the keycode into it's proper command code and associated settings
		if (pressed) {
			keystack[1][keystackFree] = kk[1];  //this is the value for a pressed key
		}
		else {
			switch (kk[1]) {
			case 0:
				// This handles the key being released  
				keystack[1][keystackFree] = 1;  // this is the reversed value for a released key
				break;
			case 10:
			case 12:
				// This handles the Tumbler switches which return a 10 or 12 so the off is 11 for released  
				keystack[1][keystackFree] = 11;
				break;
			case 40:
			case 41:
			case 42:
			case 43:
			case 44:
				//this is a released key for a rotary switch and we just want to ignore it because we have a 
				//switchcode for each position, which allows us to care only about the the pressed case.
				ignoreKey = true;
				break;
			default:
				keystack[1][keystackFree] = 0;  // this is the default value for a released key
				break;
			}
		}
		if (!ignoreKey) {
			keystack[2][keystackFree] = kk[2]; //this is the device for the keycode
			keystack[0][keystackFree++] = kk[0]; //this is the keycode
			if (keystackFree == 64) {
				// this code checks if we're about to fall off the end of the stack and if we are then it compresses the stack
				while (keystackNext <= keystackFree & keystackNext != 0) {
					keystack[2][i] = keystack[2][keystackNext];
					keystack[1][i] = keystack[1][keystackNext];
					keystack[0][i++] = keystack[0][keystackNext++];
				}
				keystackNext = 0;
				keystackFree = i;
			}
		}
	}
	else {
		Serial.print("Missing key: ");Serial.println(key);  // debug    
	}
}
void keystackPop(int16_t *sp) {
	if (keystackNext < keystackFree) {
		sp[0] = keystack[0][keystackNext];
		sp[1] = keystack[1][keystackNext];
		sp[2] = keystack[2][keystackNext++];

		if (keystackNext == keystackFree) {
			// if we have just taken the last entry then set both pointers to the top of the stack
			keystackNext = 0;
			keystackFree = 0;
		}
	}
	else {
		sp[0] = 0;  // indicate no keys
		sp[1] = 0;
		sp[2] = 0;
	}
}
int keystackCount() {
	//returns the number of items on the key stack
	return keystackFree - keystackNext;
}
void keystackPeek(int16_t *sp) {
	//returns the next keycode without removing it from the stack or zero if stack is empty
	if (keystackNext < keystackFree) {
		sp[0] = keystack[0][keystackNext];
		sp[1] = keystack[1][keystackNext];
		sp[2] = keystack[2][keystackNext];
	}
	else {
		sp[0] = 0;  // indicate no keys
		sp[1] = 0;
		sp[2] = 0;
	}
}
boolean convertKey(int16_t *sp, int16_t keycode) {
	// this function returns a pointer to an array containing the key information found from the HT16K33 

	for (uint8_t i = 0;i< MAXKEYS && keyMappings[0][i] > 0;i++) {
		if (keyMappings[0][i] == keycode) {
			sp[0] = keyMappings[1][i] + 3000;
			sp[1] = keyMappings[2][i];
			sp[2] = keyMappings[3][i];
			//Serial.print("ConvertKey(): ");Serial.print(i,DEC);Serial.print(":");Serial.print(sp[0]);Serial.print(":");Serial.print(sp[1]);Serial.print(":");Serial.print(sp[2]);Serial.print(":");Serial.println(keycode);
			return true;
		}
	}
	sp[0] = 0;
	sp[1] = 0;
	sp[2] = 0;
	return false;
}
int16_t realKey(int16_t keycode) {
	// this function returns the index into the keycode array if found or -1 if not found

	for (int16_t i = 0;i< MAXKEYS && keyMappings[0][i] > 0;i++) {
		if (keyMappings[0][i] == keycode) {
			return i;
		}
	}
	return -1;
}
void specialkeyActions(void) {
	// we have been called following the kk array being populated with a key value set
	// so that special actions can be performed.  These special actions are typically 
	// needed when the simulator does not notify us of the consequences for one device
	// when another has gone into a particular state.  

	// Clean up after the inverter is turned off
	if (kk[0] == 123) {
		if (kk[1] == 0) {
			inverterState = false;
		}
		else {
			inverterState = true;

		}


	}
	// Clean up after the battery has been turned off
	if (kk[0] == 123) {
		if (kk[1] == 0) {
			batteryState = false;
		}
		else {
			batteryState = true;

		}


	}

}
void connectToWiFi(const char * ssid, const char * pwd) {
	Serial.println("Connecting to WiFi network: " + String(ssid));

	// delete old config
	WiFi.disconnect(true);
	//register event handler
	WiFi.onEvent(WiFiEvent);

	//Initiate connection
	WiFi.config(IPAddress(10, 1, 1, 17), IPAddress(10, 1, 1, 1), IPAddress(255, 255, 255, 0));
	WiFi.begin(ssid, pwd);
	Serial.println("Waiting for WIFI connection...");
	oledAPUMsg("Awaiting WiFi", 0, 40);
}
//wifi event handler
void WiFiEvent(WiFiEvent_t event) {
	switch (event) {
	case WIFI_EVENT_STAMODE_GOT_IP:
		//When connected set 
		Serial.print("WiFi connected! IP address: ");Serial.println(WiFi.localIP());
		//Serial.println("Connected");  
		//oledAPUMsg("Connected WiFi",0,20);  // this causes stack corruption - probably because it is async event
		//initializes the UDP state
		//This initializes the transfer buffer
		//udp.begin(WiFi.localIP(),udpPort);
		if (udp.begin(udpPort) == 1) {
			Serial.print("Listening on port: ");
			Serial.println(udpPort, DEC);
			connected = true;
		}
		else {
			Serial.println("Port open failed");
			//oledAPUMsg("Fail: Port open",0,20);  // this causes stack corruption - probably because it is async event
		}
		break;
	case WIFI_EVENT_STAMODE_DISCONNECTED:
		Serial.println("WiFi lost connection");
		//oledAPUMsg("WiFi lost",0,20);  // this causes stack corruption - probably because it is async event
		connected = false;
		break;
	}
}
void processOutput(char * replyBuffer) {
	if (udpOutBndPort >0) {
		udp.beginPacket(udpAddress, udpOutBndPort);
		//Serial.print("Sending to: ");Serial.print(udpAddress);Serial.print(":");Serial.print(udpOutBndPort,DEC);Serial.print(" bytes: ");Serial.print(strlen(replyBuffer),DEC);Serial.print(" data=");Serial.println(replyBuffer);     
		udp.write(replyBuffer, strlen(replyBuffer));
		udp.endPacket();
		commandsReady = false;
	}
	else {
		// try to get a non-zero port
		udpOutBndPort = udp.remotePort();  // this should be the ephemeral port on the other end
		commandsReady = true; // flag to retry
	}
}
void oledAPUDials(float degAPUSpdDial, float degAPUEGTDial) {
	// this routine plots the two dials with pointers at the positions (in degrees) specified
	APUdialsWrite = false;
	float theta;
	float a = 0.01745329400196656833824975417896;
	oledAPUu8g2.firstPage();
	do {
		oledAPUu8g2.drawDisc(32, 32, 2, U8G2_DRAW_ALL);
		oledAPUu8g2.drawCircle(32, 32, 30, U8G2_DRAW_ALL);
		oledAPUu8g2.drawCircle(32, 32, 31, U8G2_DRAW_ALL);
		oledAPUu8g2.setFont(u8g2_font_4x6_mf);
		oledAPUu8g2.drawStr(44, 26, "APU");
		oledAPUu8g2.drawStr(28, 50, "%RPM");
		theta = degAPUSpdDial*a;
		oledAPUu8g2.drawLine(32, 32, (sin(theta) * 25) + 32, (cos(theta) * 25) + 32);

		oledAPUu8g2.drawDisc(96, 32, 2, U8G2_DRAW_ALL);
		oledAPUu8g2.drawCircle(96, 32, 30, U8G2_DRAW_ALL);
		oledAPUu8g2.drawCircle(96, 32, 31, U8G2_DRAW_ALL);
		oledAPUu8g2.setFont(u8g2_font_4x6_mf);
		oledAPUu8g2.drawStr(84, 16, "APU");
		oledAPUu8g2.drawStr(108, 26, "EGT");
		oledAPUu8g2.drawStr(84, 50, "C x100");
		theta = degAPUEGTDial*a;
		oledAPUu8g2.drawLine(96, 32, (sin(theta) * 25) + 96, (cos(theta) * 25) + 32);

	} while (oledAPUu8g2.nextPage());
	delay(10);
}
void oledAPUMsg(const char * msg, int x, int y) {
	// this routine puts a message on the APU oled display to indicate ESP8266 state
	oledAPUu8g2.firstPage();
	do {
		/*
		oledAPUu8g2.setFont(u8g2_font_ncenB14_tr);
		oledAPUu8g2.setFont(u8g2_font_4x6_mf);
		*/
		oledAPUu8g2.setFont(u8g2_font_5x7_tr);
		oledAPUu8g2.drawStr(x, y, msg);

	} while (oledAPUu8g2.nextPage());
	delay(10);
}
void testAll() {
	/* This test code is to ensure that some of the AtTiny85 boards have been defined at the same addess as they were supposed to be */
	if (MasterCautionController.deviceaddress != i2c_addr_caution) {
		Serial.print("Error with Master Caution devide: ");Serial.print(MasterCautionController.deviceaddress, HEX);Serial.print(" - should be - ");Serial.println(i2c_addr_caution, HEX);
	}
	if (gear.deviceaddress != i2c_addr_gear) {
		Serial.print("Error with Landing Gear device: ");Serial.print(gear.deviceaddress, HEX);Serial.print(" - should be - ");Serial.println(i2c_addr_gear, HEX);
	}
	if (skid.deviceaddress != i2c_addr_skid) {
		Serial.print("Error with Landing Lights & Skid device: ");Serial.print(skid.deviceaddress, HEX);Serial.print(" - should be - ");Serial.println(i2c_addr_skid, HEX);
	}

	// this routine is to test all of the sub-section tests, sets a delay and then turns off the test
	checkLight(i2c_addr_cmsp, 15, 7, 1);                 // turn on the indicator LED on CMSP board
	displayHT16K33(i2c_addr_cmsp);
	checkLight(i2c_addr_ufc, 10, 4, 1);                  // turn on one of the UFC LEDs
	displayHT16K33(i2c_addr_ufc);
	checkLight(i2c_addr_lhs, 15, 7, 1);                  // turn on the indicator LED on LHS switch board
	displayHT16K33(i2c_addr_lhs);
	checkLight(i2c_addr_rhs, 15, 7, 1);                  // turn on the indicator LED on RHS switch board
	displayHT16K33(i2c_addr_rhs);
	checkLight(i2c_addr_nmsp, 15, 7, 1);                 // turn on the indicator LED on NMSP board
	displayHT16K33(i2c_addr_nmsp);

	testCmsc(i2c_addr_cmsc, true);
	testCmsp(i2c_addr_cmsp);
	testGeneral();
	testCaution(i2c_addr_cp);
	gear.setLED(MASTERCAUTIONLED | GEARWARNINGLED);   // turn on the Gear warning and old unused master caution LEDs    
	MasterCautionController.setLED(MASTERCAUTIONLED); // turn on the master caution LED on the UFC

	delay(2000);
	displayClear(i2c_addr_lhs);
	displayHT16K33(i2c_addr_lhs);
	displayClear(i2c_addr_rhs);
	displayHT16K33(i2c_addr_rhs);
	displayClear(i2c_addr_nmsp);
	displayHT16K33(i2c_addr_nmsp);
	displayClear(i2c_addr_cmsp);
	displayHT16K33(i2c_addr_cmsp);
	displayClear(i2c_addr_cp);
	displayHT16K33(i2c_addr_cp);
	displayClear(i2c_addr_ufc);
	displayHT16K33(i2c_addr_ufc);
	testCmsc(i2c_addr_cmsc, false);
	cmspDisplay.clear();
	gear.resetLED(MASTERCAUTIONLED | GEARWARNINGLED);   // turn off the Gear warning and old unused master caution LEDs    
	MasterCautionController.resetLED(MASTERCAUTIONLED); // turn off the master caution LED on the UFC

}
void testLEDs(uint8_t i2c_address) {
	// I think that this has no purpose
	const uint8_t ledPosn[24] = {
		2,4,2,5,2,6,2,7,0,7,0,6,0,5,0,4,1,6,1,5,1,4,1,7 };
	uint8_t ii = 0;
	while (ii < 24) {
		drawPixel(i2c_address, ledPosn[ii++], ledPosn[ii++], LED_ON);
		displayHT16K33(i2c_address);
		delay(20);
	}
}
void testCmsp(uint8_t i2c_address) {
	cmspDisplay.begin(16, 2);                                // start the screen for the CMSP (this seems to be very sensitive to the positioning in the initialisation chain on the ESP8266
	cmspDisplay.clear();                                    // Clear the OLED screen
	cmspDisplay.setCursor(0, 0);
	cmspDisplay.print(Msgs[1]);
}
void testCmsc(uint8_t i2c_address, boolean led) {
	uint8_t i;
	Wire.beginTransmission(i2c_address);
	if (led) {
		Wire.write(CMSC_DEV_STATUS | CMSC_DEV_LED_3_VAL | CMSC_DEV_LED_2_VAL | CMSC_DEV_LED_1_VAL); // test the CMSC comms by turning on the LEDs
	}
	else {
		Wire.write(CMSC_DEV_STATUS | CMSC_DEV_LED_3 | CMSC_DEV_LED_2 | CMSC_DEV_LED_1);  // test the CMSC comms by turning off the LEDs    
	}
	Wire.endTransmission();
	i = 1;
	Wire.requestFrom(i2c_address, i);     // try to read the byte containing the key presses & encoder states
	i = 0;
	while (Wire.available()) {                  // slave may send less than requested
		i2cBuffer[i++] = Wire.read();             // receive a byte as character 
												  //Serial.print((char)i2cBuffer[i-1]);
	}
	if (led) {
		strcpy(displayBuffer, Msgs[5]);
		writeCmscScreen(i2c_addr_cmsc, 2, displayBuffer);        // write an opening message into zone     
	}
	else {
		writeCmscScreen(i2c_addr_cmsc, 0xff, displayBuffer);     // clear the screen         
	}
	i2cBuffer[i++] = '\0';
	//Serial.print("rcvd testCmsc-");Serial.print(i-1,HEX);Serial.print(":");Serial.println((byte)i2cBuffer[i-2],BIN);
}
void testCaution(uint8_t i2c_address) {
	for (uint8_t k = 0;k<1;k++) {
		for (uint16_t i = 480;i<528;i++) {
			drawPixel(i2c_address, cautionPanel[0][i - 480], cautionPanel[1][i - 480], LED_ON);
			displayHT16K33(i2c_address);
			delay(20);
		}
	}
}
void testGeneral(void) {
	// Set various LEDs as defined in an array of oddball i2c LEDs
	for (uint8_t i = 0;i<24;i++) {
		drawPixel(generalIndicators[2][i], generalIndicators[0][i], generalIndicators[1][i], LED_ON);
		displayHT16K33(generalIndicators[2][i]);
		delay(20);
	}
}
void getDefaultKeys(uint8_t i2c_address) {
	byte interrupt_flag;
	Wire.beginTransmission(i2c_address);
	Wire.write(HT16K33_KEY_RAM_ADDR);
	Wire.endTransmission();
	Wire.requestFrom(i2c_address, (byte)6);  // try to read the six bytes containing the key presses
	Serial.print("{");
	while (Wire.available()) {                 // slave may send less than requested
		interrupt_flag = Wire.read();             // receive a byte as character
		Serial.print("0x");
		Serial.print(interrupt_flag, HEX);
		Serial.print(",");
	}
	Serial.print("},  //");
	Serial.print(i2c_address, HEX);
	Serial.println(" ");

}