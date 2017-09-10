/*Begining of Auto generated code by Atmel studio */
#include <Arduino.h>

/*End of auto generated code by Atmel studio */

//
//   This is the Arduino script for managing the switch and indicator boards for DCS A-10C.
//   It interfaces to a UDP listener program running on the host which then talks serial 
//   to the arduino Leonardo pro micro (this script) which then communicates mainly via i2c
//   to various boards.
//6
//   In October 2015, some of the boards were revised to work with the full scale infrastructure
//   being rolled out in part due to the unreliability of the mini toggles from China, and because
//   full sized toggles were being placed into the MFD panel which I had laser cut.
//
//   There are two encoders on the CMSC board and these are on PCINT0 - PCINT3
//
//   Revision History
//   ----------------
//   20151030 - Changes for I2C ATTiny85 board for the Gear Switch and Gear Warning running A-10C_LandingGear.c 
//   20160416 - This change relates to the removal of the Arduino Pro Micro and replacing it with an 32U4 on the CMSC
//              at the same time the CMSC used more data lines to remove the need for a dedicated HT16K33 on that board 
//   20160920 - Implement a mechanism which allows switch states to be queried.  This is used to ensure that the pit switches
//              match the emulator.  This is done by using the arg value for the switch in clickabledata 
//   20161009 - Changes to support the new version 3.0 UFC board with additional LEDs and ATTiny85 Master Caution LED  
//
//  Undiagnosed problems
//  --------------------
//  Does not come back online properly after a software reset
//  Encoders are a little eratic
//  possibly does nor respond to serial port properly after a software reset (although this might be the calling program
//  
//
#include <avr/pgmspace.h>           // This is needed to allow constants to be placed into flash and be read back
#include <Wire.h>                   // This is needed for i2c communications
#include <OLED.h>                   // Library for the OLED display without serial board 4-bit
#include <OLED_I2C.h>               // Library for the OLED display without serial board 4-bit
#include <A10CLandingGear.h>        // Library for the I2C Landing Gear Switch
#include <A10CLandingLightsandSkid.h>  // Library for the I2C Landing landing lights and skid switches
#include <A10CMasterCaution.h>      // Master Caution LED
#include <TQED.h>                   // This is for the Tiny QED encoder https://github.com/renbotics/TQED    
//#include <Encoder.h>              // This is from http://www.pjrc.com/
//Beginning of Auto generated function prototypes by Atmel Studio
void setup();
void loop();
void ProcessSerialPort();
void ProcessQuery();
void ProcessData();
void GearWarning(String parm);
void MasterCaution(String parm);
void checkLight(uint8_t i2c_address, int16_t x, int16_t y, String parm);
void setLight(int16_t x, int16_t y,bool state);
void drawPixel(uint8_t i2c_address, int16_t x, int16_t y, uint16_t color);
void displayHT16K33(uint8_t i2c_address);
void initHT16K33(uint8_t i2c_address);
void displayClear(uint8_t i2c_address);
boolean readSwitches(uint8_t i2c_address);
void getOtherSwitchData(uint8_t);
boolean switchcodeQuery(int16_t);
int keydataavailable(uint8_t i2c_address);
void getSwitchData(uint8_t i2c_address);
void keystackPush(int16_t key, boolean pressed);
void keystackPop(int16_t *sp);
int keystackCount();
void keystackPeek(int16_t *sp);
boolean convertKey(int16_t *sp, int16_t keycode);
int16_t realKey(int16_t keycode);
void specialActions(int16_t *sk);
boolean ProcessEncoders(void);
boolean CheckEncoders(void);
void testAll();
void testCMSC(uint8_t);
void testLEDs(uint8_t i2c_address);
void testCaution(uint8_t i2c_address);
void testGeneral(void);
void getDefaultKeys(uint8_t i2c_address);
void CMSCEncoders(void);
void software_Reset(void);
void breathStatus(void);
boolean checkDefaultKeys(uint8_t i2c_address);
int freeRam ();
//End of Auto generated function prototypes by Atmel Studio

//
//  This section is the master list of all of the 12c devices in use in the project
//
const uint8_t i2c_addr_other = 0x78;     // dummy i2c address for the simulated HT16K33 used for oddly connected switches
const uint8_t i2c_addr_cp    = 0x77;     // i2c address of the HT16K33 used for the Caution Panel (this board only has indicators)
const uint8_t i2c_addr_top   = 0x76;     // i2c address of the HT16K33 used for the gear and the top line of indicators  *** this board no longer exists ***
const uint8_t i2c_addr_ufc   = 0x75;     // i2c address of the HT16K33 used for the up front controller functions (This now has 8 LEDs)
const uint8_t i2c_addr_elec  = 0x74;     // i2c address of the HT16K33 used for the CMSP and Electrical panel
const uint8_t i2c_addr_cmsp  = 0x74;     // i2c address of the HT16K33 used for the CMSP and Electrical panel
const uint8_t i2c_addr_lhs   = 0x73;     // i2c address of the HT16K33 used for the LHS switches (This is a general switch board)
const uint8_t i2c_addr_rhs   = 0x72;     // i2c address of the HT16K33 used for the RHS switches (This is a general switch board)
const uint8_t i2c_addr_nmsp  = 0x71;     // i2c address of the HT16K33 used for the NMSP unit (Navigation)
const uint8_t i2c_addr_test  = 0x70;     // i2c address of the HT16K33 used to test new functions
//
const uint8_t i2c_addr_oled2 = 0x20;     // i2c address of the PCF8574 used for the OLED2 display
const uint8_t i2c_addr_enc0  = 0x10;     // i2c address of the ATtiny85 used for rotary encoder 0
const uint8_t i2c_addr_enc1  = 0x11;     // i2c address of the ATtiny85 used for rotary encoder 1
const uint8_t i2c_addr_enc2  = 0x12;     // i2c address of the ATtiny85 used for rotary encoder 2
const uint8_t i2c_addr_caution = 0x1F;   // i2c address of the ATtiny85 used for the I2C Master Caution LED
const uint8_t i2c_addr_gear  = 0x1E;     // i2c address of the ATtiny85 used for the I2C Landing Gear Switch
const uint8_t i2c_addr_skid  = 0x1D;     // i2c address of the ATtiny85 used for the I2C Landing Lightd/Skid switches
//
const uint8_t i2c_addr_cmsc = 0x00;    // This is a dummy i2c address of the CMSC board which is a controller
//
OLED OLED1(11, 9, 6, 7, 12, 8);   // Instantiate the OLED display on the correct pins
OLED_I2C OLED2( (uint8_t) i2c_addr_oled2,16,2);
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
// These are the buttons on the CMSC board
#define CMSC_PRI 19
#define CMSC_SEP 20
#define CMSC_UNK 21
#define CMSC_BOT 22
#define CMSC_TOP 23
#define CMSC_PRI_LED 13
#define CMSC_ML_LED 4
#define CMSC_UNK_LED 5

// List here the highest and lowest i2c address for the HT16K33 chips.  These are managed to 
// ensure that only the correct number of buffers are allocated.
#define LOWEST_I2C_ADDR 0x71            //this is the address of the lowest i2c HT16K33
#define HIGHEST_I2C_ADDR 0x78           //this is the address of the highest real i2c HT16K33 +1
//
const char boardNames0[] PROGMEM = "Test\0";
const char boardNames1[] PROGMEM = "NMSP\0";
const char boardNames2[] PROGMEM = "SW RHS\0";
const char boardNames3[] PROGMEM = "SW LHS\0";
const char boardNames4[] PROGMEM = "Elec / CMSP\0";
const char boardNames5[] PROGMEM = "Up Front\0";
const char boardNames6[] PROGMEM = "Top Line\0";
const char boardNames7[] PROGMEM = "Caution Panel\0";
const char boardNames8[] PROGMEM = "Others\0";
PGM_P const boardNames[] PROGMEM = {boardNames0,boardNames1,boardNames2,boardNames3,boardNames4,boardNames5,boardNames6,boardNames7,boardNames8};
//
boolean HT16K33Push[HIGHEST_I2C_ADDR-LOWEST_I2C_ADDR]; // Used to force a write to a particular HT16K33 chip
//
uint16_t displaybuffer[HIGHEST_I2C_ADDR-LOWEST_I2C_ADDR][8];  
uint8_t keys[HIGHEST_I2C_ADDR-LOWEST_I2C_ADDR+1][6];                // allocate space for all the key states plus one for the non-HT16K33 keys
uint8_t lastkeys[HIGHEST_I2C_ADDR-LOWEST_I2C_ADDR+1][6];            // allocate space for all the key states plus one for the non-HT16K33 keys 
unsigned long switchLastIntTime[HIGHEST_I2C_ADDR-LOWEST_I2C_ADDR+1];  // this is the time when the last interupt was seen from the device
//
unsigned long time1 = 0;                     // Used to calculate loop delay
unsigned long time2 = 0;                     // Used to calculate loop delay
//
// Place the static text into flash instead of RAM
const char Msg1[] PROGMEM = "    A-10C CMSC  1   \0"; 
const char Msg2[] PROGMEM = "  A-10C CMSP  2 \0"; 
const char Msg3[] PROGMEM = "   Comms Stopped    \0";
const char Msg4[] PROGMEM = "    Shutting Down   \0";
const char Msg5[] PROGMEM = "Incor Switch:\0";
PGM_P const Msgs[] PROGMEM = {
    Msg1,Msg2,Msg3,Msg4,Msg5
};
//
// General String variables
String command ;
String value ;
boolean query = false;   // used to indicate that the current command is a query request
String UHFFreq;
//
// these are the LED coordinates for the caution panel
const uint8_t cautionPanel [2][48] PROGMEM= {
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
    15,15,15},
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
    2,1,0  }
};
const uint8_t generalIndicators [3][24] PROGMEM= {  // these are the LED positions for various indicators
  { 0,1,2,                // CMSC Missile Launch, Pri, Unk
    15,15,15,             // Gear good
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
const uint8_t cmscPosn [2][3] PROGMEM = {  // these are positions for text on the CMSC display
  { 12,0,0},              //
  { 0,0,1}
};
const uint8_t cmspPosn [2][5] PROGMEM = {  // these are positions for text on the CMSP display
  { 0,0,4,8,12},              //
  { 0,1,1,1,1}
};
#define MAXKEYS 106
const int16_t  keyMappings [5][ MAXKEYS ] PROGMEM = {  // this is the lookup table for the up front controller keys and other switches
{801,802,803,396,397     //  1   | Anti Skid | Land Taxi Lights | Stabil augmentation
,398,399,335,334,319     //  2    | Master Arm | Gun Arm Mode
,318,336,337,320,339     //  3    | Laser Arm | TGP | Alt Source
,338,322,306,340,324     //  4    | HUD Day/Night | HUD Normal | CICU | JTRS
,309,308,324,406,401     //  5    | IFFCC | IFFCC | Set Takeoff Trim | Flood | Battery
,404,402,403,405,421     //  6    | Inverter | R Gen | APU Gen | L Gen | Master CMS Mode Select
,420,419,418,417,442     //  7    | Master CMS Mode Select | Master CMS Mode Select | Master CMS Mode Select | Master CMS Mode Select | Arrow
,443,440,444,424,423     //  8    | Arrow | Arrow | Arrow | Up | Return
,428,426,427,441,439     //  9    | Down | Jettison | Unused | Missile Warning System (MWS). Left mouse - ON/OFF, Right mouse - MENU (No Function)
,437,438,435,436,433     //  10    | Electronic Countermeasure Jammer (JMR). Left mouse - ON/OFF, Right mouse - MENU (No Function) | Radar Warning Receiver (RWR). Left mouse - ON/OFF, Right mouse - MENU | Countermeasure Dispenser (DISP). Left mouse - ON/OFF, Right mouse - MENU
,434,507,523,506,505     //  11    | STEER-UP | STEER-DN | UFC-1 | UFC-2
,504,522,521,520,538     //  12    | UFC-3 | UFC-4 | UFC-5 | UFC-6 | UFC-7
,537,536,503,519,535     //  13    | UFC-8 | UFC-9 | UFC-HACK | UFC-0 | UFC-SPC
,502,518,534,501,517     //  14    | UFC-FUNC | UFC-LTR | UFC-CLR | UFC-ENT | UFC-MK
,533,512,528,544,529     //  15    | UFC-ALT | Master Caution | UFC-FWD | ADD-4 | UFC-MID
,545,527,543,510,526     //  16    | ADD-5 | UFC-AFT | ADD-6 | UFC-DATA-UP | UFC-DATA-DN
,509,525,541,542,524     //  17    | UFC-SEL-UP | UFC-SEL-DN | UFC-INTEN-RGT | UFC-INTEN-LFT | UFC-DEPR-UP
,540,326,310,312,342     //  18    | UFC-DEPR-DN | Pump -  Wing - L | Pump -  Wing - R | Pump -  Main - L | Pump -  Main - R
,840,808,809,810,813     //  19    | Eng-?? | Gear | CMSC-RWR-JMR | CMSC-RWR-MWS | CMSC-RWR-PRI
,812,811,108,106,105     //  20    | CMSC-RWR-SEP | CMSC-RWR-UNK | PTR-NMSP-HARS | PTR-NMSP-EGI | PTR-NMSP-TISL
,120,119,118,117,107     //  21    | PTR-NMSP-STARTPT | PTR-NMSP-ANCHR | PTR-NMSP-TCN | PTR-NMSP-ILS | PTR-NMSP-ABLE-STOW
                         //  22
,0                       //  End of array marker
},
{28,14,14,3,5            //  1  
,7,9,1,1,2               //  2
,2,3,3,4,5               //  3
,5,6,7,8,9               //  4
,10,10,12,7,6            //  5
,2,5,1,4,18              //  6
,18,18,18,18,1           //  7
,2,3,4,5,7               //  8
,6,8,0,10,10             //  9
,12,12,14,14,16          //  10
,16,20,21,1,2            //  11
,3,4,5,6,7               //  12
,8,9,12,10,11            //  13
,13,14,15,16,17          //  14
,18,1,30,33,31           //  15
,34,32,35,22,23          //  16
,24,25,28,29,26          //  17
,27,5,6,7,8              //  18
,5,1,1,2,3               //  19
,4,5,1,2,3               //  20
,4,5,6,7,8               //  21
                         //  22
},
{1,12,10,1,1             //  1  
,1,1,12,10,12            //  2
,10,12,10,1,12           //  3
,10,1,1,1,1              //  4
,12,10,1,1,1             //  5
,1,1,1,1,44              //  6
,43,42,41,40,1           //  7
,1,1,1,1,1               //  8
,1,1,0,12,10             //  9
,12,10,12,10,12          //  10
,10,1,-1,1,1             //  11
,1,1,1,1,1               //  12
,1,1,1,1,1               //  13
,1,1,1,1,1               //  14
,1,1,1,1,1               //  15
,1,1,1,1,-1              //  16
,1,-1,1,-1,1             //  17
,-1,1,1,1,1              //  18
,1,1,1,1,1               //  19
,1,1,1,1,1               //  20
,1,1,1,1,1               //  21
                         //  22
},
{38,49,49,38,38          //  1  
,38,38,7,7,7             //  2
,7,7,7,7,7               //  3
,7,7,7,7,7               //  4
,7,7,38,49,1             //  5
,1,1,1,1,4               //  6
,4,4,4,4,4               //  7
,4,4,4,4,4               //  8
,4,4,4,4,4               //  9
,4,4,4,4,4               //  10
,4,8,8,8,8               //  11
,8,8,8,8,8               //  12
,8,8,8,8,8               //  13
,8,8,8,8,8               //  14
,8,24,8,8,8              //  15
,8,8,8,8,8               //  16
,8,8,8,8,8               //  17
,8,36,36,36,36           //  18
,37,39,5,5,5             //  19
,5,5,46,46,46            //  20
,46,46,46,46,46          //  21
                         //  22
},
{654,655,655,0,0         //  1   | Anti Skid | Land Taxi Lights | Stabil augmentation
,0,0,375,375,376         //  2    | Master Arm | Gun Arm Mode
,376,377,377,378,379     //  3    | Laser Arm | TGP | Alt Source
,379,380,381,382,383     //  4    | HUD Day/Night | HUD Normal | CICU | JTRS
,384,384,0,243,246       //  5    | IFFCC | IFFCC | Set Takeoff Trim | Flood | Battery
,242,245,241,244,364     //  6    | Inverter | R Gen | APU Gen | L Gen | Master CMS Mode Select
,364,364,364,364,0       //  7    | Master CMS Mode Select | Master CMS Mode Select | Master CMS Mode Select | Master CMS Mode Select | Arrow
,0,0,0,0,0               //  8    | Arrow | Arrow | Arrow | Up | Return
,0,358,0,360,360         //  9    | Down | Jettison | Unused | Missile Warning System (MWS). Left mouse - ON/OFF, Right mouse - MENU (No Function)
,361,361,362,362,363     //  10    | Electronic Countermeasure Jammer (JMR). Left mouse - ON/OFF, Right mouse - MENU (No Function) | Radar Warning Receiver (RWR). Left mouse - ON/OFF, Right mouse - MENU | Countermeasure Dispenser (DISP). Left mouse - ON/OFF, Right mouse - MENU
,363,0,0,0,0             //  11    | STEER-UP | STEER-DN | UFC-1 | UFC-2
,0,0,0,0,0               //  12    | UFC-3 | UFC-4 | UFC-5 | UFC-6 | UFC-7
,0,0,0,0,0               //  13    | UFC-8 | UFC-9 | UFC-HACK | UFC-0 | UFC-SPC
,0,0,0,0,0               //  14    | UFC-FUNC | UFC-LTR | UFC-CLR | UFC-ENT | UFC-MK
,0,0,0,0,0               //  15    | UFC-ALT | Master Caution | UFC-FWD | ADD-4 | UFC-MID
,0,0,0,0,0               //  16    | ADD-5 | UFC-AFT | ADD-6 | UFC-DATA-UP | UFC-DATA-DN
,0,0,0,0,0               //  17    | UFC-SEL-UP | UFC-SEL-DN | UFC-INTEN-RGT | UFC-INTEN-LFT | UFC-DEPR-UP
,0,0,0,0,0               //  18    | UFC-DEPR-DN | Pump -  Wing - L | Pump -  Wing - R | Pump -  Main - L | Pump -  Main - R
,0,716,0,0,0             //  19    | Eng-?? | Gear | CMSC-RWR-JMR | CMSC-RWR-MWS | CMSC-RWR-PRI
,0,0,0,0,0               //  20    | CMSC-RWR-SEP | CMSC-RWR-UNK | PTR-NMSP-HARS | PTR-NMSP-EGI | PTR-NMSP-TISL
,0,0,0,0,621             //  21    | PTR-NMSP-STARTPT | PTR-NMSP-ANCHR | PTR-NMSP-TCN | PTR-NMSP-ILS | PTR-NMSP-ABLE-STOW
                         //  22
}
};
const uint8_t defaultKeyPositions [8][6] PROGMEM = {  // these are the positions that the keys should be in when the plane is sitting on the apron with power off
  {0x40,0x00,0x00,0x00,0x00,0x00},  //71 
  {0x00,0x00,0x00,0x00,0x00,0x00},  //72 
  {0xA0,0x00,0x20,0x00,0x00,0x00},  //73 
  {0x00,0x00,0x01,0x04,0x6A,0x00},  //74 
  {0x00,0x00,0x00,0x00,0x00,0x00},  //75 
  {0x00,0x00,0x00,0x00,0x00,0x00},  //76 
  {0x00,0x00,0x00,0x00,0x00,0x00},  //77
  {0x00,0x00,0x00,0x00,0x00,0x00}   //78
};
int16_t  kk[3];            // this is the integer array to contain a keycode and value pair
int16_t  keystack[3][64];  // this is the FIFO stack of key presses, key values and devices
uint8_t  keystackNext;     // this is the next keypress to be processed
uint8_t  keystackFree;     // this is the first free slot in the keystack 

boolean readkeys = false;
boolean batteryState = false;
boolean inverterState = false;

unsigned long LastActivityTime=0;
boolean breath = false;
boolean breathDir = true;
uint8_t breathVal = 0;

#ifdef DEBUG
boolean nldebug = false;
#endif

char displayBuffer[21];

TQED qedILS1(i2c_addr_enc0);
TQED qedILS0(i2c_addr_enc1);
TQED qedTacan(i2c_addr_enc2);
A10CLANDINGGEAR gear(i2c_addr_gear);
A10CLandingLightsandSkid skid(i2c_addr_skid);
A10CMASTERCAUTION MasterCautionController(i2c_addr_caution);

uint8_t gearLastState = 0;
uint8_t skidLastState = 0;
uint8_t CMSCLastState = 0;

// These are the two counters used for the encoders on the CMSC
volatile int CMSCBrtCounter = 0;
volatile int CMSCAudCounter = 0;
volatile bool CMSCAudChange = false;
volatile bool CMSCBrtChange = false;


void setup()
{ 
  //MasterCautionController.setAddress(0x48);
  uint8_t i;
  uint8_t j;
  for (i=0;i<HIGHEST_I2C_ADDR-LOWEST_I2C_ADDR;i++) HT16K33Push[i] = false;  // This flag is used to indicate that the LED buffer needs to be written to the HT16K33
  for (i =LOWEST_I2C_ADDR;i< HIGHEST_I2C_ADDR;i++) initHT16K33(i);            // set up the HT16K33 i2c chips
  //initialise the key buffers
  for (i=0;i<HIGHEST_I2C_ADDR-LOWEST_I2C_ADDR+1;i++) {
    for (j = 0;j<6;j++) {
      keys[i][j] = (uint8_t) 0;
    }
  }
  Serial.begin(115200);                  // start the serial port
  //delay(10000);
  //This is used to obtain the default key settings so keep it here incase we need to check for changes to the default.
  for(uint8_t ii = LOWEST_I2C_ADDR;ii<HIGHEST_I2C_ADDR+1;ii++){  //this obtains a print of all the key ram
    getDefaultKeys(ii);
  }
  OLED1.begin(20,2);                  // start the screen for the CMSC
  OLED2.begin(16,2);                    // start the screen for the CMSP

// setup the pins on the CMSC board
  pinMode(CMSC_PRI_LED,OUTPUT);
  pinMode(CMSC_UNK_LED,OUTPUT);
  pinMode(CMSC_ML_LED,OUTPUT);

  pinMode(CMSC_PRI,INPUT_PULLUP);   // This is a digital mapping for the analog pin
  pinMode(CMSC_SEP,INPUT_PULLUP);   // This is a digital mapping for the analog pin
  pinMode(CMSC_UNK,INPUT_PULLUP);   // This is a digital mapping for the analog pin
  pinMode(CMSC_TOP,INPUT_PULLUP);   // This is a digital mapping for the analog pin
  pinMode(CMSC_BOT,INPUT_PULLUP);   // This is a digital mapping for the analog pin


  for(uint8_t ii = LOWEST_I2C_ADDR;ii<HIGHEST_I2C_ADDR;ii++){  //this obtains a print of all the key ram
    if (!checkDefaultKeys(ii)){
      OLED1.clear();                  // Clear the OLED screen
      strcpy_P(displayBuffer, (char*)pgm_read_word(&(Msgs[4])));
      OLED1.setCursor(0,0);
      OLED1.print(displayBuffer);
      
      strcpy_P(displayBuffer, (char*)pgm_read_word(&(boardNames[ii-0x70])));
      OLED1.setCursor(0,1);
      OLED1.print(displayBuffer);
      delay(1000);
    }
  }

  OLED1.noAutoscroll();  
  OLED1.clear();                  // Clear the OLED screen
  OLED1.setCursor(0,0);
  strcpy_P(displayBuffer, (char*)pgm_read_word(&(Msgs[0])));
  OLED1.print(displayBuffer); 

  OLED2.clear();                  // Clear the OLED screen
  OLED2.setCursor(0,0);
  strcpy_P(displayBuffer, (char*)pgm_read_word(&(Msgs[1])));
  OLED2.print(displayBuffer); 

#ifdef DEBUG
  // allocate memory for common strings
  command.reserve(8);
  value.reserve(48);
#endif

  //perform the startup tests

  gear.setLED(MASTERCAUTIONLED | GEARWARNINGLED);  // turn on the Gear warning and old unused master caution LEDs    
  MasterCautionController.setLED(MASTERCAUTIONLED); // turn on the master caution LED on the UFC
  checkLight(i2c_addr_ufc,10,4,value);  // turn on the one of the UFC LEDs
  displayHT16K33(i2c_addr_ufc);
  checkLight(i2c_addr_lhs,15,7,value);  // turn on the indicator LED on LHS switch board
  displayHT16K33(i2c_addr_lhs);
  checkLight(i2c_addr_rhs,15,7,value);  // turn on the indicator LED on RHS switch board
  displayHT16K33(i2c_addr_rhs);
  checkLight(i2c_addr_nmsp,15,7,value);  // turn on the indicator LED on NMSP board
  displayHT16K33(i2c_addr_nmsp);
  checkLight(i2c_addr_cmsp,15,7,value);  // turn on the indicator LED on CMSP board
  displayHT16K33(i2c_addr_cmsp);
  testAll();
  displayClear(i2c_addr_ufc);     //turn off LEDs on the UFC HT16K33 
  //displayClear(i2c_addr_top);   //turn off LEDs on the top HT16K33 
  displayClear(i2c_addr_lhs);     //turn off LEDs on the LHS switch HT16K33 
  displayClear(i2c_addr_rhs);     //turn off LEDs on the LHS switch HT16K33 
  displayClear(i2c_addr_cp);      //turn off LEDs on the caution panel HT16K33 
  displayClear(i2c_addr_nmsp);    //turn off LEDs on the NMSP HT16K33 
  displayClear(i2c_addr_cmsp);    //turn off LEDs on the CMSP HT16K33 
  command="";
  value="";
  OLED1.clear();                  // Clear the OLED screen
  OLED2.clear();                  // Clear the OLED screen
  gear.resetLED(MASTERCAUTIONLED | GEARWARNINGLED);  // turn off the gear LED & turn off now unused master caution LED
  MasterCautionController.resetLED(MASTERCAUTIONLED); // turn off the master caution LED on the UFC
  // Reset all of the counters in the encoder units
  for(i=0;i<=1;i++){
  qedILS0.resetCount(i);
  qedILS1.resetCount(i);
  qedTacan.resetCount(i);
  }
  // prepare the two onboard encoders
  DDRB &= ~((1<<PB0) | (1<<PB1) | (1<<PB2) | (1<<PB3));    // Set encoder bits to inputs 
  //PCMSK0 |= (1<<PCINT0)|(1<<PCINT1)|(1<<PCINT2)|(PCINT3);  // set the pin change mask for the 2 CMSC encoders
  PCMSK0 |= (1<<PCINT0)|(1<<PCINT2);                        // set the pin change mask for one pin each on the 2 CMSC encoders
  PCICR |= (1<<PCIE0);                                     // Pin Change interrupt control register
  sei();                                                   // enable interrupts
  // this is just to remove instability and then reset the encoders
  delay(1000);
  CMSCAudCounter = 0;
  CMSCBrtCounter = 0;
  CMSCAudChange = false;
  CMSCBrtChange = false;

  //  These are set here as a temporary measure while we do not have real switches for the electrical power
  //  the lines can simply be removed once the real switches have been reinstalled.
  inverterState = true;
  batteryState = true;

}
/******************************************************************************
**
**  Main processing loop starts here.
**
*******************************************************************************/
void loop()
{ CheckEncoders();  // encoders do not stack their results so just call them
  if (readSwitches(i2c_addr_ufc)| readSwitches(i2c_addr_elec)| readSwitches(i2c_addr_lhs)| readSwitches(i2c_addr_rhs) | readSwitches(i2c_addr_nmsp) | readSwitches(i2c_addr_other))  {
    // one or more keys have been placed onto the keystack and need to be processed
    while (keystackCount()>0) {
//      breath = false;
      keystackPop(kk);
      //  Now send the key info to the serial port
      //  This is done in multiple prints to avoid the use of a string  
      specialActions(kk);
      Serial.print("C,");
      Serial.print(kk[2]);
      Serial.print(",");
      Serial.print(kk[0]);
      Serial.print(",");
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
      switch(kk[1]){
        case 0:
        case 1:
        case -1:
          Serial.println((float)kk[1]);
        break;
        case 10:
        case 11:
        case 12:
          // this handles the tumbler
          Serial.println((float)(kk[1]-10)/10);
        break;        
        case 40:
        case 41:
        case 42:
        case 43:
        case 44:
          // this handles the 5 position rotary switch
          Serial.println((float)(kk[1]-40)/10);
        break;        
        default:
          Serial.println((float)kk[1]);
        break;
      }
    }
  }
  if (Serial.available()) {
    // We reset a counter here which stops the breath LED from activating
//    breath = false;
    ProcessSerialPort();
  }
  // Breathing is to give a visual indication that the board is still powered on but not doing anything for a game
//  if(!breath){
//    if ( LastActivityTime ==0) analogWrite(CMSC_UNK_LED,0);  //turn off the LED if we've just found activity after breathing
//    LastActivityTime = millis();                             // we've just had some game activity so we set the timer
//    breath = true;                                           // and turn off the activity flag
//  } else if (LastActivityTime == 0 || millis()-LastActivityTime > 120000) breathStatus();
}
void ProcessSerialPort(){
// Used as markers for parsing incoming commands
boolean start= false;
boolean mid = false;
    while (Serial.available()) {
    uint8_t c=Serial.read();  // This might actually need to be an int

    // The majority of the data received will be to set indicators or displays eg *123-01#
    // A query request can also be received and this is used to investigate the state of one of the connected devices
    // it is the same format as a normal command except the command code is prefixed by a question mark eg *?123-xx#
    // We don't force the simulator to match the switches because the sim might have started in mid-air so we do not
    // want to be turning of needed functions.  The calling program will simply report the difference giving the human
    // the ability to change the state of the switches to match the sim. 
   
    switch(char(c)){
      case '*':
        start = true;
        mid = false;
        query = false;
        value = "";
        command = "";
        break;
      case '-':
        if (mid || !start || command == "" || command.toInt() == 0){
          start = false;
          mid = false;
          command = "";
          value = "";
        } else
          mid = true;
          value = "";
        break;
      case '#':
        if(mid && start && command !="" && value != "" && command.toInt() >0)
        {
          if(!query){
          ProcessData();
          } else {
          ProcessQuery();
          query = false;            
          }
          start = false;
          mid = false;
          command = "";
          value = "";        
        }
        else
        {
          start = false;
          mid = false;
          command = "";
          value = "";
        }
        break;
      case '\r':
      case '\n':
      case '\0':
          break;
      default:
        if(start && !mid)
        {
          if(char(c) == '?'){
            query = true;
          } else {
          command += char(c);
          }
        }
        else if (start && mid)
        {
          value += char(c);
        }
        else
        {
          start = false;
          mid = false;
          command = "";
          value = "";          
        }  
        break;
    }
  }
}
void ProcessQuery()
{
  // This routine attempts to respond to requests which want to know the state of the switches which can be incorrectly set.  This will usually exclude
  // momentary push switches unless they are broken.
  int16_t m;
  int cmdcode;
  int valcode;
  float valfloat;
  int16_t keyIndex;
  cmdcode = command.toInt();
  valcode = value.toInt();
  valfloat = value.toFloat();
  //
  // we will try to avoid any i2c activity to determine key states but this should not usually be necessary
  // Most switches are on HT16K33 devices, but there are a few, notably on the CMSC and Landing gear panels which are not
  //
  for(keyIndex = 0;keyIndex<MAXKEYS;keyIndex++){
    if (pgm_read_word_near(&keyMappings[4][keyIndex]) == cmdcode){
      //  we use the lastkey array to get the switch data because this should be recent enough for our needs and it avoids going to the i2c bus
      //  we also want to try to send a single response to every request so we use the decade of the value to determine
      //  how many switchcodes we need to consider  
      Serial.print('?');
      Serial.print(cmdcode);
      Serial.print('-');
        switch(pgm_read_word_near(&keyMappings[2][keyIndex])){
          // these are generally 2 position toggles, the -1 is because the switch might be wired backwards  
          case 0:
          case 1:
          case -1:
               if(switchcodeQuery(keyIndex)) {
                    m = pgm_read_word_near(&keyMappings[2][keyIndex]);                
               } else {
                    m = 0;
               }               
               if(pgm_read_word_near(&keyMappings[2][keyIndex]) <0 ) m++;  //  if this is a backwards switch then adjust the value
               if(m!=valcode){
                   Serial.print(valcode);Serial.print("-");                // print out what the key should be
                   Serial.println(m);                                      // print out what it actually is
               } else Serial.println("=");                                 // if match, then just print equals sign

          break;
          // these are generally 3 position toggles, the 11 is the centre position whis is inferred by the other two positions being off
          // 11 is a dummy code to indicate that neither of the poles of the 
          // the toggle switch are set (ie in the middle position).  This means
          // that we can expect one or other of the switchcodes to be true or both
          // are false.
          case 10:
          case 11:
          case 12:
            // this handles the tumbler
               if(switchcodeQuery(keyIndex)) {
                    m = pgm_read_word_near(&keyMappings[2][keyIndex]);                
               } else if (switchcodeQuery(keyIndex+1)){
                    m = pgm_read_word_near(&keyMappings[2][keyIndex+1]);                
               } else {
                    m = 11;
               }

               if(((float)m-10)/10 != valfloat){
                   Serial.print(valfloat);Serial.print("-");  // print out what the key should be
                   Serial.println(((float)m-10)/10);          // print out what it actually is
               } else Serial.println("=");
             break;        
          // these will generally be 5 position rotary switches
          case 40:
          case 41:
          case 42:
          case 43:
          case 44:
            // this handles the 5 position rotary switch          
            // we should only come in here for the first of the switch codes, and 
            // only one of the switchcodes should be true          
               for(uint8_t i = 0;i <5;i++){
                  if(switchcodeQuery(keyIndex)){
                    m = pgm_read_word_near(&keyMappings[2][keyIndex]);

               if(((float)m-40)/10 != valfloat){
                   Serial.print(valfloat);Serial.print("-");  // print out what the key should be
                   Serial.println(((float)m-40)/10);          // print out what it actually is
               } else Serial.println("=");
                    break;  // leave the for loop because we should only have one switchcode set and we've found it  
                  }
                  keyIndex++;  // have not found a switchcode which is set yet so try the next one
               }
          break;        
          default:
              m = pgm_read_word_near(&keyMappings[2][keyIndex]);
              Serial.println((float)m);  
          break;
      }
      break;  // break out of the for loop
  }
  }
}
boolean switchcodeQuery(int16_t ki){
// this is to return whether a particular switch is set or not  
    uint8_t i = (pgm_read_word_near(&keyMappings[0][ki])/100);  //get the i2c index for this key
    uint8_t j = pgm_read_word_near(&keyMappings[0][ki])-(i*100);  //get the keycode for this key
    i= i+0x70-LOWEST_I2C_ADDR;
    // we now want to identify the byte and bit which represents the switch code of interest.  We encode it thusly (byte*8)+bit+1 so this needs to be reversed
    // all but simple toggles will have multiple switch codes. 
    j--;
    uint8_t k = j/8;      // this is our byte number
    j = j-(k*8);  // this is our bit number 
    return (lastkeys[i][k]>>j)&1;
}
void ProcessData()
{
  // this routine maps all of the exported lights and other indicators to real world devices
  // info for this is in ??\DCS World\Mods\aircrafts\A-10C\Cockpit\Scripts\mainpanel_init.lua
  // and also in the argument values withing the clickabledata.lua file

  uint8_t i;
  int cmdcode;
  int valcode;
  cmdcode = command.toInt();
  valcode = value.toInt();
  switch (cmdcode) {
  case 404:
    // Master Caution
    MasterCaution("1");
    break;
    //  This section is for the landing gear
  case 659: // these are the landing gear lights LANDING_GEAR_N_SAFE
  case 660: // LANDING_GEAR_L_SAFE
  case 661: // LANDING_GEAR_R_SAFE
    checkLight(pgm_read_byte(&generalIndicators[2][cmdcode-659+3]),pgm_read_byte(&generalIndicators[0][cmdcode-659+3]),pgm_read_byte(&generalIndicators[1][cmdcode-659+3]),"1");  // turn on the LED      
    break;
  case 737: // landing gear warning light
    GearWarning("1");
    break;    
    //  This section is for the lights on the top dashboard
  case 662:     // GUN_READY
  case 663:     // NOSEWHEEL_STEERING
  case 664:     // MARKE R_BEACON    
  case 665:     // CANOPY_UNLOCKED
      #ifdef DEBUG
      if(cmdcode ==663){
      // Use NWS indicator as the debug toggle
      if(value=="1"){nldebug=true;}else{nldebug = false;}
      }
      #endif
      // There are two independant LEDs for each of these indicators and they are +4 in the array from the first LED 
      checkLight(pgm_read_byte(&generalIndicators[2][cmdcode-662+16]),pgm_read_byte(&generalIndicators[0][cmdcode-662+16]),pgm_read_byte(&generalIndicators[1][cmdcode-662+16]),"1");      
      checkLight(pgm_read_byte(&generalIndicators[2][cmdcode-662+20]),pgm_read_byte(&generalIndicators[0][cmdcode-662+20]),pgm_read_byte(&generalIndicators[1][cmdcode-662+20]),"1");      
    break;
  case 372:     // CMSC_MissileLaunch
  case 373:     // CMSC_PriorityStatus
  case 374:     // CMSC_UnknownStatus
    checkLight(pgm_read_byte(&generalIndicators[2][cmdcode-372+0]),pgm_read_byte(&generalIndicators[0][cmdcode-372+0]),pgm_read_byte(&generalIndicators[1][cmdcode-372+0]),"1");      
    break;
      //  This section is for the AoA indicators
  case 540: // High
  case 541: // OK
  case 542: // Low
    // These are no longer implemented
    // checkLight(pgm_read_byte(&generalIndicators[2][cmdcode-540+4]),pgm_read_byte(&generalIndicators[0][cmdcode-540+4]),pgm_read_byte(&generalIndicators[1][cmdcode-540+4]),"1");  // turn on the LED      
    break;
      //  This section is for the Air refuel Indicators on RHS
  case 730: // AIR_REFUEL_READY
  case 731: // REFUEL_LATCHED
  case 732: // AIR_REFUEL_DISCONNECT
    // These are no longer implemented
    // checkLight(pgm_read_byte(&generalIndicators[2][cmdcode-730+14]),pgm_read_byte(&generalIndicators[0][cmdcode-730+14]),pgm_read_byte(&generalIndicators[1][cmdcode-730+14]),"1");  // turn on the LED      
    break;
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
    checkLight(i2c_addr_cp,pgm_read_byte(&cautionPanel[0][cmdcode-480]),pgm_read_byte(&cautionPanel[1][cmdcode-480]),"1");    
    break;
  case 606:  //HARS
  case 608:  //EGI 
  case 610:  //TISL
  case 612:  //STRPT
  case 614:  //ANCHR
  case 616:  //TCN
  case 618:  //ILS
  case 619:  //UHF
  case 620:  //FM
    if(cmdcode == 619) cmdcode = 622;  // this is to help cope with the fact that the UHF is the only odd code - so the order of UHF and FM has swapped over by this approach
    cmdcode = cmdcode / 2; 
    checkLight(pgm_read_byte(&generalIndicators[2][cmdcode-303+7]),pgm_read_byte(&generalIndicators[0][cmdcode-303+7]),pgm_read_byte(&generalIndicators[1][cmdcode-303+7]),"1");  // turn on the LED      
    break;
   
  case 2000:      // UHF Frequency
    UHFFreq = value;  // the frequency can change and be displayed without the display on and will not be resent
    if(batteryState){
      OLED1.setCursor(12,1);  //bottom row on rhs
      OLED1.print(value);
    }
  break;
  case 20871:  //CMSP
  case 20872:  //CMSP
  case 20873:  //CMSP
  case 20874:  //CMSP
  case 20875:  //CMSP
    if (inverterState && batteryState){

      OLED2.setCursor(pgm_read_byte(&cmspPosn[0][cmdcode-20871]),pgm_read_byte(&cmspPosn[1][cmdcode-20871]));
      OLED2.print(value);
    }
  break;
  
  case 20881:  //CMSC CHAFF_FLARE
  case 20882:  //CMSC JMR
  case 20883:  //CMSC MWS
    if (inverterState && batteryState){
      OLED1.setCursor(pgm_read_byte(&cmscPosn[0][cmdcode-20881]),pgm_read_byte(&cmscPosn[1][cmdcode-20881]));
      OLED1.print(value);
    }
  break;

  case 9999:  // this is the control code for the board functions
  {
    valcode = value.toInt();  // values for 9999 are always integers
    switch(valcode)
    {
      case 0:
      {
        /*  In this clause, we simply turn the displays off */
        OLED1.clear();
        OLED1.home();
        OLED1.setCursor(0,1);
        strcpy_P(displayBuffer, (char*)pgm_read_word(&(Msgs[0])));
        OLED1.print(displayBuffer); 
        delay(2000);
        OLED1.clear();
        OLED2.clear();
      } 
      break; //case 0
      case 90:
      {
        /*  In this clause, we need to resend the current state for all of the switches */
            for(uint8_t i2c_address = LOWEST_I2C_ADDR;i2c_address<=HIGHEST_I2C_ADDR;i2c_address++){
            getSwitchData(i2c_address);  //get the six bytes of key memory
              for (uint8_t i=0; i<6; i++) {
                uint8_t trans = keys[i2c_address-LOWEST_I2C_ADDR][i];
                // New keys being pressed
                  for (uint8_t j=0;j<8;j++) {
                   int16_t keyIndex = realKey(int16_t (i*8)+j+1+((i2c_address-0x70)*100));
                   if (keyIndex >= 0) {  //only push keys that are real
                      Serial.print((i*8)+j+1+((i2c_address-0x70)*100));
                      Serial.print(" | ");
                      Serial.print((int16_t)pgm_read_word_near(&keyMappings[2][keyIndex]));
                      Serial.print(" | ");
                      Serial.println(0x01 & (trans>>j));
                      if (0x01 & (trans>>j)){
                        // the bit for the switch is set.
                        if (pgm_read_word_near(&keyMappings[2][keyIndex])<0){
                          keystackPush((i*8)+j+1+((i2c_address-0x70)*100),false);  //place a keycode onto the stack as released
                        }else{
                          keystackPush((i*8)+j+1+((i2c_address-0x70)*100),true);  //place a keycode onto the stack as released
                        }
                      }else{
                        // the bit for the the switch is not set so the default will be released
                        if (pgm_read_word_near(&keyMappings[2][keyIndex])<0){
                          keystackPush((i*8)+j+1+((i2c_address-0x70)*100),true);  //place a keycode onto the stack as released
                        }else{
                          keystackPush((i*8)+j+1+((i2c_address-0x70)*100),false);  //place a keycode onto the stack as released
                        }

                      }
                    
                   }
                }
            }
          }
      } //case 90
      break; //case 90
      case 91:
      {
        /*  In this clause, we re-run the check for all of the keys being in the default positions */
          for(uint8_t ii = LOWEST_I2C_ADDR;ii<=HIGHEST_I2C_ADDR;ii++){  //this obtains a print of all the key ram
            if (!checkDefaultKeys(ii)){
            OLED2.clear();                  // Clear the OLED screen
            strcpy_P(displayBuffer, (char*)pgm_read_word(&(Msgs[4])));
            OLED2.setCursor(0,0);
            OLED2.print(displayBuffer);
            
            strcpy_P(displayBuffer, (char*)pgm_read_word(&(boardNames[ii-0x70])));
            OLED2.setCursor(0,1);
            OLED2.print(displayBuffer);
            delay(1000);
          }
        }
      } 
      break;  //case 91
      case 98:
      {
        /*  In this clause, we simply inform that the serial port has been stopped */
        OLED1.clear();
        OLED2.clear();
        OLED1.setCursor(0,1);
        strcpy_P(displayBuffer, (char*)pgm_read_word(&(Msgs[2])));
        OLED1.print(displayBuffer); 
        OLED1.begin(20,2);
      }  
      break; //case 98
      case 99:
      {  // this means that the serial port is about to be closed.
        /*  In this clause, we simply turn everything off */
        gear.resetLED(MASTERCAUTIONLED | GEARWARNINGLED);   // turn off the gear warning and the old unused master caution LEDs
        MasterCautionController.resetLED(MASTERCAUTIONLED);   // turn off the new master caution LED
        displayClear(i2c_addr_test);   //turn off the HT16K33            
        //displayClear(i2c_addr_top);  //turn off the HT16K33            
        displayClear(i2c_addr_cp);     //turn off the HT16K33 
        displayClear(i2c_addr_ufc);    //turn off the HT16K33
        displayClear(i2c_addr_nmsp);   //turn off the HT16K33
        displayClear(i2c_addr_lhs);    //turn off the HT16K33
        displayClear(i2c_addr_rhs);    //turn off the HT16K33
        OLED1.clear(); 
        OLED2.clear(); 
        OLED1.setCursor(0,1);
        strcpy_P(displayBuffer, (char*)pgm_read_word(&(Msgs[3])));
        OLED1.print(displayBuffer); 
        delay(2000); // wait 2 seconds
        OLED1.begin(20,2);
        OLED2.begin(16,2);
        Serial.end();
        //setup();
        software_Reset();
      }  
      break; //case 99      
      default:  
      break;
    } //switch on valcodes for 9999
  } // end of cmdcode 9999
    break;  // cmdcode 9999
 
  default:  // cmdcode
    break;
  } // switch for cmdcode
  command="";
  value="";
  // We now flush out the LED changes to the devices which need it
  for (i=0;i<HIGHEST_I2C_ADDR-LOWEST_I2C_ADDR;i++) {
    if (HT16K33Push[i] ) {
      displayHT16K33(LOWEST_I2C_ADDR+i);
      HT16K33Push[i] = false;
    }
  }
}
void GearWarning(String parm){
  // This special case code is needed because of the way the toggle switch with integrated LED had to be wired with a permanent 5V on it.
    if (value.equals(parm)) {
      gear.setLED(GEARWARNINGLED);  // Turn on the Gear Warning LED
  } 
  else {
     gear.resetLED(GEARWARNINGLED);  // Turn off the Gear Warning LED
  }
}
void MasterCaution(String parm){
  // This special case code is needed because of the way the toggle switch with integrated LED had to be wired with a permanent 5V on it.
  if (value.equals(parm)) {
    MasterCautionController.setLED(MASTERCAUTIONLED); // turn on the master caution LED on the UFC    
  } 
  else {
    MasterCautionController.resetLED(MASTERCAUTIONLED); // turn on the master caution LED on the UFC    
  }
}
void checkLight(uint8_t i2c_address, int16_t x, int16_t y, String parm)
{
  if (value.equals(parm)) {
    if (i2c_address == i2c_addr_cmsc){
    // The light is not on an HT16K33 
      setLight(x, y,HIGH);
    }
    else {
      drawPixel(i2c_address, x,y,LED_ON);
      HT16K33Push[i2c_address-LOWEST_I2C_ADDR] = true;
    }
  } 
  else {
    if (i2c_address == i2c_addr_cmsc){
    // The light is not on an HT16K33     
        setLight(x, y,LOW);
    }
    else {
      drawPixel(i2c_address,x,y,LED_OFF);
      HT16K33Push[i2c_address-LOWEST_I2C_ADDR] = true;
    }
  }
}
void setLight(int16_t x, int16_t y,bool state)
{
  //  this routine is used for LEDs which are not on
  //  HT16K33s
  //  x is the light and y is the board
  switch(y){
    case 0:
      switch(x){
        case 0:
          digitalWrite(CMSC_ML_LED,state);
        break;
        case 1:
          digitalWrite(CMSC_PRI_LED,state);
        break;
        case 2:
          digitalWrite(CMSC_UNK_LED,state);
        break;
        default:
        break;
      }
    break;
    default:
    break;
  }
}
void drawPixel(uint8_t i2c_address, int16_t x, int16_t y, uint16_t color) {
  if ((y < 0) || (y >= 8)) return;
  if ((x < 0) || (x >= 16)) return;

  if (color) {
    displaybuffer[i2c_address-LOWEST_I2C_ADDR][y] |= 1 << x;
  } 
  else {
    displaybuffer[i2c_address-LOWEST_I2C_ADDR][y] &= ~(1 << x);
  }
}
void displayHT16K33(uint8_t i2c_address)
{
  Wire.beginTransmission(i2c_address);
  Wire.write((uint8_t)0x00); // start at address $00
  for (uint8_t i=0; i<8; i++) {
    Wire.write(displaybuffer[i2c_address-LOWEST_I2C_ADDR][i] & 0xFF);    
    Wire.write(displaybuffer[i2c_address-LOWEST_I2C_ADDR][i] >> 8); 
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
void displayClear(uint8_t i2c_address){
  Wire.beginTransmission(i2c_address);
  Wire.write((uint8_t)0x00); // start at address $00
  for (uint8_t i=0; i<8; i++) {
    displaybuffer[i2c_address-LOWEST_I2C_ADDR][i] = 0;
    Wire.write(displaybuffer[i2c_address-LOWEST_I2C_ADDR][i] & 0xFF);
    Wire.write(displaybuffer[i2c_address-LOWEST_I2C_ADDR][i] >> 8);
  }   
  Wire.endTransmission();
}
boolean readSwitches(uint8_t i2c_address) {
  // testing suggests that once the switches are read, it takes around 19ms before another interupt
  // will occur, even if the key is held down for the whole time.
 
 // this code can be in one of 3 states
 // 1 - no keys are pressed and everything dealt with (this is the normal state)
 // 2 - Interupt set and one or more keys have been pressed - The interupt might be set for a new key and we need to spot that an old key has been released
 // 3 - No interupt, but keys have been recently pressed  - this should mean that all of the keys which have been pressed have now been released
 //
 // For this reason, we need to maintain a map of all of the keys which have transitioned from on to off, so that we can raise a keypress event
 // and communicate that the key has now been released.
 // Because we do not get an interupt for a key being released, we need to remember that there is work to be done for the scan after the last 
 // interupt.   
 
  uint8_t i=0;
  uint8_t trans;
  boolean keypressed = false;
  boolean keychanged = false;
  uint8_t keydataavail;

  keydataavail = keydataavailable(i2c_address);
  if (keydataavail == SWITCHSET) {
    // At least one key has been pressed
    memcpy(lastkeys[i2c_address-LOWEST_I2C_ADDR], keys[i2c_address-LOWEST_I2C_ADDR], sizeof(keys[i2c_address-LOWEST_I2C_ADDR])); //save the previous set of keys  
    switch(i2c_address){
      case i2c_addr_other:
        getOtherSwitchData(i2c_address);
        break;
      default:
        // used for HT16K33's
        getSwitchData(i2c_address);
      break;
    }
    for (i = 0;i<6;i++) {if (lastkeys[i2c_address-LOWEST_I2C_ADDR][i]!=keys[i2c_address-LOWEST_I2C_ADDR][i]) keychanged = true; }       
    if (keychanged){
      for (uint8_t i=0; i<6; i++) {
        // we are about to do two passes of the data
        // the first is to find keys which have just been pressed
        // the second is to find keys which have just been released
        //
        // New keys being pressed
        trans = ~lastkeys[i2c_address-LOWEST_I2C_ADDR][i] & keys[i2c_address-LOWEST_I2C_ADDR][i] ;  // find out if any keys were newly pressed
        if (trans != 0x00) {
          //printkeys();
          // we have found at least one key which has been newly pressed ie it was not set last time but is set now.
          for (uint8_t j=0;j<8;j++) {
            if (0x01 & (trans>>j)) {
              keystackPush((i*8)+j+1+((i2c_address-0x70)*100),true);  //place a keycode onto the stack as pressed - This is the lowest possible address rather than the current lowest.
              keypressed = true;
            }
          }
        }
        // New keys which have been released
        trans = ~keys[i2c_address-LOWEST_I2C_ADDR][i] & lastkeys[i2c_address-LOWEST_I2C_ADDR][i] ;  // find out if any keys were newly released
        if (trans != 0x00) {
          //printkeys();
          // we have found at least one key which has been newly released ie it was set last time but is not set now.
          for (uint8_t j=0;j<8;j++) {
            if (0x01 & (trans>>j)) {
              keystackPush((i*8)+j+1+((i2c_address-0x70)*100),false);  //place a keycode onto the stack as released
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
  }
  else{
    // no interupt was reported and this can be because there really was no interupt or 
    // because we tried to test the interupt too early.
    if (keydataavail == SWITCHWAIT){
      // SWITCHWAIT says too early to detect an interupt so we will just return reporting that we have
      // not done anything.
      keypressed = false;            
    }
    else {
      //this is for when we have validly checked an interupt and none was set.

      //* * * not sure why this next line was there, but it was causing problems with the special keys so I removed it, which fixed the problem, but there may be some unforseen problems.
      //for(i=0;i<6;i++) keys[i2c_address-LOWEST_I2C_ADDR][i] = 0x00;  // rather than do an i2c read, we just set the current keys to 0
      keypressed = false;
    }
  }
  return keypressed;  // let caller know that we have some key data
}
void getOtherSwitchData(uint8_t i2c_address){
// this function is to read any special switches which have been wired differently
// the general approach is to only report the transitions, and to make it simpler for 
// the processing of the key presses, we use a dummy code from an HT16K33

    keys[i2c_address-LOWEST_I2C_ADDR][0] = gear.readSwitch() | skid.readSwitch();
    // gear = 0x80
    // landing light = 0x04
    // taxi light = 0x02
    // antiskid = 0x01  

    // We now read the switches on the CMSC board which are all on port F
    uint8_t CurrState = (~(PINF) & 0b01110011);
    keys[i2c_address-LOWEST_I2C_ADDR][1] = (CurrState>>2)|(CurrState & 0b00000011);   // this results in the low order 5 bits being the keystates for the CMSC
  return;
}
boolean CheckEncoders(){
boolean ret=false;
unsigned long currTime = micros();           // only do a call to the clock once
  if (currTime-time2 > 100000){  // only check the switch state once per interval
    time2 = currTime;
    ret = ProcessEncoders();                    // check the i2c encoders
    if(CMSCAudChange | CMSCBrtChange) {   // These two flags are set by the interrupt routine
      CMSCEncoders();
      ret = true;  // we do not need anything pased back because the interupt flags tell us there was a change.
    }
  } else{
    ret = false;   // we'll declare no encoder activity because the time period has not expired and we don't want to check them too often
  }
  return ret;
}
int keydataavailable(uint8_t i2c_address){
  //
  // Testing suggests that once the switches are read, it takes around 19ms before another interupt
  // will occur, even if the key is held down for the whole time.
  //
  // This routine also attempts to mimic an interupt for the last key being released so the first
  // time the interupt flag is checked after it has been set, we will still report that the interupt
  // has been set to give a chance to push the final releases onto the stack 
  //
  uint8_t tmpkeys[6] = {0};                    // initialize array to 0 
  byte interupt_flag =0;
  time1 = micros();                            // only do a call to the clock once
  if (time1 - switchLastIntTime[i2c_address-LOWEST_I2C_ADDR] > 20000){  // only check the interupt if it has had time to get set
    switch(i2c_address){
      case i2c_addr_other:
        // for this pseudo-device there is no interupt to test and it is multiple devices in reality
        // we want to avoid duplicate work (and using more variables)
        memcpy(tmpkeys, keys[i2c_address-LOWEST_I2C_ADDR], sizeof(keys[i2c_address-LOWEST_I2C_ADDR])); //save the previous set of keys  
        getOtherSwitchData(i2c_address);  // this fetches new key values into the keys array, it is a bit wasteful because if there is key action we'll go get it again.
        for(uint8_t ii=0;ii<6;ii++){
           if(keys[i2c_address-LOWEST_I2C_ADDR][ii] != tmpkeys[ii]){            
              interupt_flag = 0xff;
              memcpy(keys[i2c_address-LOWEST_I2C_ADDR],tmpkeys, sizeof(keys[i2c_address-LOWEST_I2C_ADDR])); // copy back the original key values
              break;  // leave the for loop
           }
        }
        break;
      default:
        // this is for the HT16K33 based devices which have an interupt to tell us a key has been pressed
        Wire.beginTransmission(i2c_address);
        Wire.write(HT16K33_KEY_INTERUPT_ADDR | 0);
        Wire.endTransmission();
        Wire.requestFrom(i2c_address, (byte)1);    // read the interupt status
        while (Wire.available()) {                 // slave may send less than requested
          interupt_flag = Wire.read();             // receive a byte as character
        }
      break;
    }

    if (interupt_flag) {
      switchLastIntTime[i2c_address-LOWEST_I2C_ADDR] = time1;   //  if the interupt was set then we need to save the time so that we don't recheck it too quickly
      return SWITCHSET;                              // this means that there was a valid interupt
    }
    else{
      // so no interupt and we've left enough time for it to have occurred/
      // if the interupt had been pressed on the last pass then we will return an interupt
      // so that the released keys can be processed, otherwise we simply return a no keys response
      // we use the last interupt time to determine this.  
      if (switchLastIntTime[i2c_address-LOWEST_I2C_ADDR] == 0){
        return SWITCHNONE;                        // no switch was pressed
      }
      else {
        switchLastIntTime[i2c_address-LOWEST_I2C_ADDR] = 0;  // say no dummy interupt is needed next time
        return SWITCHSET;                         // say a key was pressed  even though none was to allow the released key to be processed
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
void getSwitchData(uint8_t i2c_address){
  // get the six bytes of switch memory from the i2c bus
  uint8_t i;
  Wire.beginTransmission(i2c_address);
  Wire.write(HT16K33_KEY_RAM_ADDR);
  Wire.endTransmission();
  Wire.requestFrom(i2c_address, (byte)6);  // try to read the six bytes containing the key presses
  i=0;
  while (Wire.available()) {                  // slave may send less than requested
    keys[i2c_address-LOWEST_I2C_ADDR][i++] = Wire.read();  // receive a byte as character 
  }
}
void keystackPush(int16_t key, boolean pressed){
  boolean ignoreKey = false;
  int16_t kk[3] = {0,0,0};  // this is used for the keycode value set
  uint8_t i = 0;
  //test
  //Serial.print("Keycode :  ");
  //Serial.println(key);
  //end test
  if(convertKey(kk,key)){  // translate the keycode into it's proper command code and associated settings
    if (pressed) {
      keystack[1][keystackFree] = kk[1];  //this is the value for a pressed key
    }
    else {
      switch(kk[1]){
        case 0:
        // This handles the key being released  
          keystack[1][keystackFree] = 1 ;  // this is the reversed value for a released key
          break;
        case 10:
        case 12:
        // This handles the Tumbler switches which return a 10 or 12 so the off is 11 for released  
          keystack[1][keystackFree] = 11 ;
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
          keystack[1][keystackFree] = 0 ;  // this is the default value for a released key
          break;
      }
    }
    if (!ignoreKey){
      keystack[2][keystackFree] = kk[2]; //this is the device for the keycode
      keystack[0][keystackFree++] = kk[0]; //this is the keycode
    if (keystackFree ==64) {
      // this code checks if we're about to fall off the end of the stack and if we are then it compresses the stack
      while ( keystackNext <= keystackFree & keystackNext != 0) {
        keystack[2][i] = keystack[2][keystackNext];
        keystack[1][i] = keystack[1][keystackNext];
        keystack[0][i++] = keystack[0][keystackNext++];
      }
      keystackNext = 0;
      keystackFree = i;
    }
    }
  } else {
    Serial.print("Missing key: ");  // debug
    Serial.println(key);  // debug    
  }
}
void keystackPop(int16_t *sp){
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
  else{ 
    sp[0] = 0;  // indicate no keys
    sp[1] = 0;
    sp[2] = 0;
  }
}
int keystackCount(){
  //returns the number of items on the key stack
  return keystackFree-keystackNext;
}
void keystackPeek(int16_t *sp){
  //returns the next keycode without removing it from the stack or zero if stack is empty
  if (keystackNext < keystackFree) {
    sp[0] = keystack[0][keystackNext];
    sp[1] = keystack[1][keystackNext];
    sp[2] = keystack[2][keystackNext];
  } 
  else{ 
    sp[0] = 0;  // indicate no keys
    sp[1] = 0;
    sp[2] = 0;
  }
}
boolean convertKey(int16_t *sp, int16_t keycode){
// this function returns a pointer to an array containing the key information found from the HT16K33 
uint8_t i = 0;
  for (i = 0;i< MAXKEYS && pgm_read_word_near(&keyMappings[0][i]) > 0;i++){
    if(pgm_read_word_near(&keyMappings[0][i]) == keycode){
      sp[0] = pgm_read_word_near(&keyMappings[1][i])+3000;
      sp[1] = pgm_read_word_near(&keyMappings[2][i]);
      sp[2] = pgm_read_word_near(&keyMappings[3][i]);
      return true;
    }
  }
  sp[0] = 0;
  sp[1] = 0;
  sp[2] = 0;
  return false;
}
int16_t realKey(int16_t keycode){
// this function returns the index into the keycode array if found or -1 if not found

  for (int16_t i = 0;i< MAXKEYS && pgm_read_word_near(&keyMappings[0][i]) > 0;i++){
    if(pgm_read_word_near(&keyMappings[0][i]) == keycode){
      return i;
    }
  }
return -1;
}
void specialActions(int16_t *sk){
  // this is called just before we pass the keycode back to the 
  // interface.
  // this is checks for buttons like the various power buttons being
  // pressed so that displays etc can be blanked because the game does 
  // not notify us when this happens.
  switch(sk[0]){
    case 3006:
      if(sk[2] == 1){
        // so this is device 1 and code 3006 - Battery
        // whether the battery is turned on or off, we want to clear the OLED displays
        OLED1.clear();
        OLED2.clear();
        //if the battery is turned off then we want to blank all of the indicators
        if(sk[1] == 0){
          batteryState = false;
          displayClear(i2c_addr_cp);    //turn off LEDs on the caution Panel HT16K33 
          displayHT16K33(i2c_addr_cp);
          displayClear(i2c_addr_top);
          displayHT16K33(i2c_addr_top);
          value = "";
          MasterCaution("1");
          GearWarning("1");
        } else {
          batteryState = true;
        }
      } 
    break;
    case 3002:
      if(sk[2] == 1){
        // so this is device 1 and code 3002 - Inverter
        if(sk[1] == 0){
          inverterState = false;
        } else {
          inverterState = true;
        }
      } 
    break;

    case 3018:
      if(sk[2]== 4 && sk[1] == 20){
        // if CMSP rotary switch is at the off position then we turn off the two displays
        OLED1.clear();
        OLED2.clear();
      }
    break;  
    default:
    break;
  }
}
boolean ProcessEncoders(void){
// this routine gets current values from I2C encoders
    //Display the counter value
    boolean ret = false;
    int16_t tempEncoderVal;
    int8_t tempDir;
    float reportValue;
    tempEncoderVal = qedILS0.getDelta(0); // get the switched version of counter
    if (tempEncoderVal != 0){
       //Serial.print("ILS0 0:");Serial.println(tempEncoderVal);
       tempEncoderVal < 0 ? tempDir = -1 : tempDir = 1;
       reportValue = tempEncoderVal * tempEncoderVal * tempDir *0.01; // cheap way to square value without losing the sign
       if (reportValue > 1)reportValue = 1;       
       else if (reportValue < -1)reportValue = -1;
       Serial.print("C,53,3001,");Serial.println(reportValue);  //ILS left knob
       ret = true;
   }
    tempEncoderVal = qedILS0.getDelta(1); // get the switched version of counter
    if (tempEncoderVal != 0){
       //Serial.print("ILS0 1:");Serial.println(tempEncoderVal);
       tempEncoderVal < 0 ? tempDir = -1 : tempDir = 1;
       reportValue = tempEncoderVal * tempEncoderVal * tempDir *0.01; // cheap way to square value without losing the sign
       if (reportValue > 1)reportValue = 1;       
       else if (reportValue < -1)reportValue = -1;
       Serial.print("C,53,3002,");Serial.println(reportValue);  //ILS left knob
       ret = true;
   }
    tempEncoderVal = qedILS1.getDelta(0); // get the switched version of counter
    if (tempEncoderVal != 0){
      //Serial.print("ILS1 0:");Serial.println(tempEncoderVal);
      tempEncoderVal < 0 ? tempDir = -1 : tempDir = 1;
       reportValue = tempEncoderVal * tempEncoderVal * tempDir*0.01; // cheap way to square value without losing the sign
       if (reportValue > 1)reportValue = 1;       
       else if (reportValue < -1)reportValue = -1;
       Serial.print("C,53,3004,");Serial.println(reportValue);  //ILS right knob
       ret = true;
   }
    tempEncoderVal = qedILS1.getDelta(1); // get the switched version of counter
    if (tempEncoderVal != 0){
      // Serial.print("ILS1 1:");Serial.println(tempEncoderVal);
      tempEncoderVal < 0 ? tempDir = -1 : tempDir = 1;
       reportValue = tempEncoderVal * tempEncoderVal * tempDir*0.01; // cheap way to square value without losing the sign
       if (reportValue > 1)reportValue = 1;       
       else if (reportValue < -1)reportValue = -1;
       Serial.print("C,53,3003,");Serial.println(reportValue);  //ILS right knob
       ret = true;
   }
    tempEncoderVal = qedTacan.getDelta(1); // get the switched version of counter
    if (tempEncoderVal != 0){
      tempEncoderVal < 0 ? tempDir = -1 : tempDir = 1;
       reportValue = tempEncoderVal * tempEncoderVal * tempDir*0.01; // cheap way to square value without losing the sign
       if (reportValue > 1)reportValue = 1;       
       else if (reportValue < -1)reportValue = -1;
       Serial.print("C,51,3001,");Serial.println(reportValue);   //TACAN - This is the 10's
       ret = true;
   }
   return ret;
}
void CMSCEncoders(void)
{
float reportValue;
int8_t tempDir;
int16_t EncVal;
//
//  I have many encoder types in operation with different PP Rot and 
//  bounce characteristics - each one needing different handling :-(
//  It looks like AUD has half the PP Rot thab BRT
//
const float BRTMULT = 0.286;  // this is the max value minus the min value for the count of the encoder
const float AUDMULT = 0.167;  // this is the max value minus the min value for the count of the encoder
 
  if (CMSCBrtChange){
    // we appear to have an encoder to report
    EncVal =  CMSCBrtCounter;    
    CMSCBrtChange = false; // turn it off
    CMSCBrtCounter = 0;  // we only want to see the delta since we last processed the encoder
    if (EncVal !=0){
       EncVal < 0 ? tempDir = -1 : tempDir = 1; // save the direction of the rotiation     
       if (EncVal> 0 && EncVal< 4) EncVal = 4; else if (EncVal <0 && EncVal > -4) EncVal = -4;
       //reportValue = EncVal * tempDir *0.01;  // cheap way to square value without losing the sign
       reportValue = (EncVal-(3*tempDir)) * BRTMULT/4;      // remove one less than the min value
       if(reportValue > 1) reportValue = 1; else if (reportValue < -1) reportValue = -1;  // clip high and low
       Serial.print("C,45,3002,");Serial.println(reportValue);   //HSI - This is course setting on the HSI   
     }
  }
  if (CMSCAudChange){
    // we appear to have an encoder to report
    EncVal =  CMSCAudCounter; 
    CMSCAudChange = false; // turn it off
    CMSCAudCounter = 0;  // we only want to see the delta since we last processed the encoder
    if (EncVal !=0){
       EncVal < 0 ? tempDir = -1 : tempDir = 1;
       if (EncVal> 0 && EncVal< 2) EncVal = 2; else if (EncVal <0 && EncVal > -2) EncVal = -2;     
       reportValue = (EncVal-(1*tempDir)) * AUDMULT;      // remove one less than the min value
       if(reportValue > 1) reportValue = 1; else if (reportValue < -1) reportValue = -1;  // clip high and low       
       Serial.print("C,35,3001,");Serial.println(reportValue);   //FM_Proxy - This is used for the altimeter pressure adjustment AAU34 which is set for 25 clicks per revolution
    }
  }
}
void testAll(){
  // this routine is to test all of the sub-section tests, sets a delay and then turns off the test
  testCMSC(HIGH);
  testGeneral();
  testCaution(i2c_addr_cp);
  delay(2000); 
//  displayClear(i2c_addr_top);
//  displayHT16K33(i2c_addr_top);
  displayClear(i2c_addr_cp);
  displayHT16K33(i2c_addr_cp);
  displayClear(i2c_addr_ufc);
  displayHT16K33(i2c_addr_ufc);
  displayClear(i2c_addr_lhs);
  displayHT16K33(i2c_addr_lhs);
  displayClear(i2c_addr_rhs);
  displayHT16K33(i2c_addr_rhs);
  displayClear(i2c_addr_nmsp);
  displayHT16K33(i2c_addr_nmsp);
  displayHT16K33(i2c_addr_cmsp);
  testCMSC(LOW);
}
void testLEDs(uint8_t i2c_address){
  const uint8_t ledPosn[24] = {
    2,4,2,5,2,6,2,7,0,7,0,6,0,5,0,4,1,6,1,5,1,4,1,7  };
  uint8_t ii = 0;
  while (ii < 24) {
    drawPixel(i2c_address,pgm_read_byte(&ledPosn[ii++]),pgm_read_byte(&ledPosn[ii++]),LED_ON);
    displayHT16K33(i2c_address);      
    delay(10);
  }
}
void testCMSC(uint8_t testCMSCmode){
  if(testCMSCmode == HIGH){
  pinMode(CMSC_PRI_LED,OUTPUT);
  pinMode(CMSC_UNK_LED,OUTPUT);
  pinMode(CMSC_ML_LED,OUTPUT);
  pinMode(CMSC_PRI,INPUT_PULLUP);  // This is a digital mapping for the analog pin
  pinMode(CMSC_SEP,INPUT_PULLUP);  // This is a digital mapping for the analog pin
  pinMode(CMSC_UNK,INPUT_PULLUP);  // This is a digital mapping for the analog pin
  pinMode(CMSC_TOP,INPUT_PULLUP); // This is a digital mapping for the analog pin
  pinMode(CMSC_BOT,INPUT_PULLUP);   // This is a digital mapping for the analog pin
  } 
digitalWrite(CMSC_ML_LED,testCMSCmode);
digitalWrite(CMSC_UNK_LED,testCMSCmode);
digitalWrite(CMSC_PRI_LED,testCMSCmode);
}
void testCaution(uint8_t i2c_address){ 
  for (uint8_t k=0;k<1;k++) {
    for (uint16_t i=480;i<528;i++) {
      drawPixel(i2c_address,pgm_read_byte(&cautionPanel[0][i-480]),pgm_read_byte(&cautionPanel[1][i-480]),LED_ON);   
      displayHT16K33(i2c_address);
      delay(10);
    }
  }
}
void testGeneral(void){ 
    // top panel
    for (uint8_t i=0;i<24;i++) {
      drawPixel(pgm_read_byte(&generalIndicators[2][i]),pgm_read_byte(&generalIndicators[0][i]),pgm_read_byte(&generalIndicators[1][i]),LED_ON);   
      displayHT16K33(pgm_read_byte(&generalIndicators[2][i]));
      delay(20);
    }
}
void getDefaultKeys(uint8_t i2c_address){
    byte interupt_flag;
    Wire.beginTransmission(i2c_address);
    Wire.write(HT16K33_KEY_RAM_ADDR);
    Wire.endTransmission();
    Wire.requestFrom(i2c_address, (byte)6);  // try to read the six bytes containing the key presses
    Serial.print("{");
    while (Wire.available()) {                 // slave may send less than requested
      interupt_flag = Wire.read();             // receive a byte as character
      Serial.print("0x");
      Serial.print(interupt_flag,HEX);
      Serial.print(",");
    }
    Serial.print("},  //");
    Serial.print(i2c_address,HEX);
    Serial.println(" ");

}
boolean checkDefaultKeys(uint8_t i2c_address){
  boolean correctKeys = true;
  byte b;
  //byte c;
  uint8_t i = 0;
    byte interupt_flag;
    Wire.beginTransmission(i2c_address);
    Wire.write(HT16K33_KEY_RAM_ADDR);
    Wire.endTransmission();
    Wire.requestFrom(i2c_address, (byte)6);  // try to read the six bytes containing the key presses
    while (Wire.available()) {                 // slave may send less than requested
      interupt_flag = Wire.read();             // receive a byte as character
      b = interupt_flag ^ pgm_read_byte(&defaultKeyPositions[i2c_address-LOWEST_I2C_ADDR][i++]);  // xor to find the mismatches
      if (b!= (byte)0){             
        Serial.print(i2c_address,HEX);Serial.print("==> ");Serial.println(b,HEX);
        correctKeys = false;
      }
    }
    return(correctKeys);
}
void software_Reset() // Restarts program from beginning but does not reset the peripherals and registers
{
asm volatile ("  jmp 0");  
}  
int freeRam () 
{
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}
ISR (PCINT0_vect)
{
  // This is the interrupt service routine for the two rotary encoders on PB 0 - 3 which 
  // raise interrupts PCINT0-3
  //
  // this table lookup approach to interupt driven rotary encoders is from 
  // https://www.circuitsathome.com/mcu/rotary-encoder-interrupt-service-routine-for-avr-micros
  //
  static uint8_t old_PINB;     // The only way to tell which pin caused the interrupt is to save the previous value 
  static uint8_t cur_PINB;     // only look at the actual pins once
  static uint8_t tAB;          // this is used incase there is an invalid transition so the previous state can be restored
  static uint8_t old_AB0 = 3;  //lookup table index
  static uint8_t old_AB1 = 3;  //lookup table index
  static int8_t tVal;
  static const int8_t enc_states [] PROGMEM =
  {0,-1,1,0,1,0,0,-1,-1,0,0,1,0,1,-1,0};  //encoder lookup table
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
      if ((cur_PINB^old_PINB) & 0b00000011){
        // Process changes to encoder on PB0&1
        tAB = old_AB0; // keep in case we need to restore it
        old_AB0 <<=2;  //remember previous state by shifting it up 2 places
        old_AB0 |= ((cur_PINB & (3 << PINB0)) >> PINB0);  // OR in the cleaned up new encoder state
        tVal = (int8_t)pgm_read_byte(&(enc_states[( old_AB0 & 0x0f )])); //add the new valid encoder change to the counter
        if (tVal != 0){
        // this is to ensure we only process counter changes and not the invalid transitions
        // the pins for this encoder are reversed so we -= rather than +=
          CMSCBrtCounter -= tVal; //add the new valid encoder change to the counter
          CMSCBrtChange = true;
        } else {
          // there was an invalid transition so we restore the old save value
          old_AB0 = tAB;
          }
      }
      if ((cur_PINB^old_PINB) & 0b00001100){
        // Process changes to encoder on PB2&3
        tAB = old_AB1; // keep in case we need to restore it
        old_AB1 <<=2;  //remember previous state by shifting it up 2 places
        old_AB1 |= ((cur_PINB & (3 << PINB2)) >> PINB2);  // OR in the cleaned up new encoder state
        tVal = (int8_t)pgm_read_byte(&(enc_states[( old_AB1 & 0x0f )])); //add the new valid encoder change to the counter
        if (tVal != 0){
          // this is to ensure we only process counter changes
          CMSCAudCounter += tVal; //add the new valid encoder change to the counter
          CMSCAudChange = true;
        } else {
          // there was an invalid transition so we restore the old save value
          old_AB1 = tAB;
          }
      }
old_PINB = cur_PINB;  // save the last pin state
}

void breathStatus(void){
    LastActivityTime = 0;  // this is to indicate that we have started to breath - this is to avoid too many calls to the clock
    if(breathDir){ 
      analogWrite(CMSC_UNK_LED,breathVal++);  // this is the only LED on the CMSC which does software PWM
      if (breathVal == 16) breathDir = !breathDir;
    } else {
      analogWrite(CMSC_UNK_LED,breathVal--);  // this is the only LED on the CMSC which does software PWM
      if (breathVal == 2) breathDir = !breathDir;
    }
    delay(50);  
}

