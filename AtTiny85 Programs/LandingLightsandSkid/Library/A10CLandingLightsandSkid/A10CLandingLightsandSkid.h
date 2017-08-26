/*
  A10CLandingLightsandSkid.h - I2C switch used for Landing Gear on A-10C
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
#ifndef A10CLandingLightsandSkid_h
#define A10CLandingLightsandSkid_h

#include <inttypes.h>

#define READSWITCH    0x0A
#define SETADDRESSREG 0x03
#define RETRYCOUNT    3

class A10CLandingLightsandSkid
{
private:
    uint8_t deviceaddress;
	
public:
    A10CLandingLightsandSkid(uint8_t address);
    uint8_t readSwitch();
    bool setAddress(uint8_t newaddress);
};

#endif