/*
  A10CLandingGear.h - I2C switch used for Landing Gear on A-10C
  Revision 1.1
  Copyright (c) 2015 Neil Larmour.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/
#ifndef A10CLandingGear_h
#define A10CLandingGear_h

#include <inttypes.h>

#define READSWITCH    0x0A
#define RESETLED      0x01
#define RESETGLED     0x07
#define SETLED        0x02
#define SETGLED       0x08
#define RESETMCLED    0x04
#define SETMCLED      0x05
#define SETADDRESSREG 0x03
#define RETRYCOUNT    3
#define MASTERCAUTIONLED 0x08
#define GEARWARNINGLED 0x02


class A10CLANDINGGEAR
{
private:
    uint8_t deviceaddress;
	
public:
    A10CLANDINGGEAR(uint8_t address);
    uint8_t readSwitch();
    bool resetLED(uint8_t lednum);
    bool setLED(uint8_t lednum);
    bool resetGearLED();
    bool setGearLED();
    bool resetMasterCautionLED();
    bool setMasterCautionLED();
    bool setAddress(uint8_t newaddress);
};

#endif