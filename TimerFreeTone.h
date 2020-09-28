// TimerFreeTone Library - v1.5 - 09/12/2016
//
// Created by Tim Eckel - teckel@leethost.com
// Copyright 2016 License: GNU GPL v3 http://www.gnu.org/licenses/gpl-3.0.html
// Project home: https://bitbucket.org/teckel12/arduino-timer-free-tone
// Blog: http://forum.arduino.cc/index.php?topic=235774.0
//
// PURPOSE:
// Doesn't use timers which frees up conflicts with other libraries. Compatible
// with all ATmega, ATtiny and ARM-based microcontrollers. About 1,500 bytes
// smaller binary sketch size than the standard tone library. Exclusive use of
// port registers for AVR-based microcontrollers for fastest and smallest code.
// Close to a plug-in replacement for the standard Tone library.
//
// SYNTAX:
//   TimerFreeTone( pin, frequency, duration [, volume ] ) - Play a note on pin at frequency in Hz for duration in milliseconds.
//     Parameters:
//       * pin        - Pin speaker is wired to (other wire to ground, be sure to add an inline 100 ohm resistor).
//       * frequency  - Play the specified frequency (should work fairly well in the 100 to 15000 Hz range).
//       * duration   - Set the duration to play in milliseconds. Range: 0 to 65535 (65.5 seconds).
//       * volume     - Optionally set the tone volume level (from 1 to 10), defaults to full volume (10).

#ifndef TimerFreeTone_h
	#define TimerFreeTone_h
 	#include <Arduino.h>

 	void TimerFreeTone(uint8_t pin, unsigned long frequency, unsigned int duration, uint8_t volume = 10);
#endif

//end
