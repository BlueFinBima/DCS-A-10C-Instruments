# DCS-A-10C-Instruments

Contains source for the DCS A-10C Instruments which live on I2C bus

DCS World from Eagle Dynamics is a fantastic flight simulator, and as I began playing around with additional ways to control the simulator, I began to build circuit boards and experiment with various ways to interact with the program.  Originally I had a C# program on windows which took the output from the simulator over UDP, and used a serial connection to speak to a board running an Atmel 32U4 which controlled all of the devices which sit on an i2c bus.  The intermediate program was never something I liked, and there was not enough memory on the 32U4 to all this version to expand.

As I began to experiment with the ESP8266, it became feasible to get rid of the c# program, and have all of the code running in the ESP8266.

This is what is now at the heart of the project.

The Atmel 32U4 part of the project which used to run as part of the A-10C CMSC was rewritten to use the same hardware, but implemented as
an i2c slave to control just the CMSC and nothing else.

This project contains many different code and design artefacts for this ever evloving project.

It should be noted that I have neither the time, money or the space to have a full sized simulator, so the items are intended to be
functionally represetative of the actual instruments as opposed to replicas.

Currently implemented are:
  Up Front Controller
  Warning Light Panel
  CMSP
  CMSC
  Landing Gear Panel
  HACP
  NMSP
  Electrical Panel
  general purpose encoder boards
