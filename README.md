# MSP430G2-Geiger-Counter

This device is Geiger-Muller Counter ,and True Random Number Generator(RNG) uses radioactive isotopes.

## Features

- [x] Radiation Detection - CPS, CPM, uSv/h, mSv/Y
- [x] Temperature - degree Celsius (°C)
- [x] True Random Number Generator - Serial Port Output 8bit TRNG

![alt text](https://github.com/GCY/MSP430G2-Geiger-Counter/blob/master/res/20200325_024932.jpg?raw=true)

![alt text](https://github.com/GCY/MSP430G2-Geiger-Counter/blob/master/res/20200325_024958.jpg?raw=true)

![alt text](https://github.com/GCY/MSP430G2-Geiger-Counter/blob/master/res/20200406_174812.jpg?raw=true)


## Hardware

### Key Component List
- Geiger-Muller Tube - J321βγ, M4011, SBM-20...etc
- MSP430G2433 or MSP430G2553
- TLC555 (CMOS 555, DIP or SMD)
- TMP35 (Operate Voltage 3.3v)

### Configure

- J1 - CMOS555 4.2v power supply.
- VR1 - Adjust High-Voltage output 180v~500v.
- IC1 - if this is use LDO, your temperature sensor operate voltage = 3v(TMP35), use forward voltage operate voltage = 4v(LM35).

## Firmware

### Build and Burn The .hex file
- Install Energia 1.6.10E18
- Select board -> MSP430G2553
- Open MSP430Geiger.ino
- Upload

### Key Parameters

## Reference



LICENSE
-------

MIT License

Copyright (c) 2020 Tony Guo

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
