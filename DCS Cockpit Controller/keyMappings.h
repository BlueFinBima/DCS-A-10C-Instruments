// keyMappings.h
//
// This file is created from the output of a spreadsheet macro.
//
#ifndef _KEYMAPPINGS_h
#define _KEYMAPPINGS_h
// 
#define MAXKEYS 114
const int16_t  keyMappings[5][MAXKEYS] = {  // this is the lookup table for the up front controller keys and other switches
	{ 801,802,803,396,397     //  1   | Anti Skid | Land Taxi Lights | Stabil augmentation
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
	,840,808,809,810,811     //  19    | Eng-?? | Gear | CMSC-RWR-JMR | CMSC-RWR-MWS | CMSC-RWR-PRI
	,812,813,108,106,105     //  20    | CMSC-RWR-SEP | CMSC-RWR-UNK | PTR-NMSP-HARS | PTR-NMSP-EGI | PTR-NMSP-TISL
	,120,119,118,117,107     //  21    | PTR-NMSP-STARTPT | PTR-NMSP-ANCHR | PTR-NMSP-TCN | PTR-NMSP-ILS | PTR-NMSP-ABLE-STOW
	,850,851,852,853,854     //  22    | CMSC_BRT to HSI Crs Setting | CMSC_AUD to Alt pressure adjust | ENC_ILS0 | ENC_ILS0s | ENC_ILS1
	,855,856                 //  23    | ENC_ILS1s | ENC_TACAN
	,0                       //  End of array marker
},
{ 28,14,14,3,5            //  1  
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
,2,1,1,2,3               //  22
,4,1                     //  23
},
{ 1,12,10,1,1             //  1  
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
,50,51,52,53,54          //  22
,55,56                   //  23
},
{ 38,49,49,38,38          //  1  
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
,45,35,53,53,53          //  22
,53,51                   //  23
},
{ 654,655,655,0,0         //  1   | Anti Skid | Land Taxi Lights | Stabil augmentation
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
,0,0,0,0,0               //  22    | CMSC_BRT to HSI Crs Setting | CMSC_AUD to Alt pressure adjust | ENC_ILS0 | ENC_ILS0s | ENC_ILS1
,0,0                     //  23    | ENC_ILS1s | ENC_TACAN
}
};


#endif

