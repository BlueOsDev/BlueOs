
/********************************************************************************
*Copyright (C) 2013 	Moritz Nagel 	mnagel@htwg-konstanz.de					*
*					Daniel Urbanietz 	daniel@d-urbanietz.de					*
*********************************************************************************
*FILE: blue_os_shell.h															*
*																				*
*AUTHOR(S): M. Nagel, D. Urbanietz												*
*																				*
*DESCRIPTION:																	*
*This module provides a shell. You can configure it in the						*
*\BlueOS\config\blue_os_config.h . 												*
*																				*
*VERSION: 1.0.																	*
*																				*			
*********************************************************************************
*This file is part of BlueOS.													*
*																				*
*BlueOS is free software: you can redistribute it and/or modify					*
*it under the terms of the GNU Lesser General Public License as published by	*
*the Free Software Foundation, either version 3 of the License, or				*
*(at your option) any later version.											*
*																				*
*BlueOS is distributed in the hope that it will be useful,						*
*but WITHOUT ANY WARRANTY; without even the implied warranty of					*
*MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the					*
*GNU Lesser General Public License for more details.							*
*																				*
*You should have received a copy of the GNU Lesser General Public License		*
*along with BlueOS.  If not, see <http://www.gnu.org/licenses/>.				*
********************************************************************************/

#ifndef BLUE_OS_SHELL_H
#define BLUE_OS_SHELL_H

#include "blue_os_config.h"
#include "blue_os.h"

#ifdef BLUE_OS_USE_SHELL

#include <avr/io.h>
#include <avr/pgmspace.h>

/**
 * This function writes a buffer to the shell.
 *
 * @param buffer Pointer to the buffer, which will be sent.
 * @param length Length of the buffer.
 */
void blueOsShellWrite(const uint8_t* buffer, uint8_t length);

/**
 * This function writes a single character to the shell.
 *
 * @param c Character, that will be written to the shell.
 */
void blueOsShellWriteChar(char c);

/**
 * This function writes a string to the shell.
 *
 * @param string Pointer to the string.
 */
void blueOsWriteString(const char* string);

/**
 * This function writes a string, which is
 * located in the flash, to the shell.
 * If you just want to write a static string,
 * use the makro blueOsWriteStringFlash().
 *
 * @param s Pointer to the string, which is located in the flash.
 */
void blueOsWriteStringFlash_(const char *s );

/**
 * This makro stores a string into the flash memory
 * and calls the function blueOsWriteStringFlash_()
 * with the correct pointer.
 *
 * Use this makro like this: blueOsWriteStringFlash("...")
 *
 * @warning Do not give a pointer as argument, the
 * pointer will be generated by the makro!
 *
 * @param __s Constant string, in the form "..."
 */
#define blueOsWriteStringFlash(__s)       blueOsWriteStringFlash_(PSTR(__s))

/**
 * This function writes an integer value to the shell.
 *
 * @param value This is the value, that will be written.
 * @param leading Number of leading zeros.
 */
void blueOsWriteInt(int value, char leading);

/**
 * This function writes an long value to the shell.
 *
 * @param value This is the value, that will be written.
 * @param leading Number of leading zeros.
 */
void blueOsWriteLong(long value, char leading);

/**
 * This function copyes the received bytes to a buffer
 * with the given length. If less bytes are available,
 * this function will return the number of copyed bytes.
 *
 * @param buffer Pointer to the buffer, where the bytes will
 * be stored.
 * @param length Maximum number of bytes, that will be copyed
 * into the buffer.
 *
 * @return Number of copyed bytes.
 */
uint8_t blueOsShellRead(uint8_t* buffer, uint8_t length);


extern volatile struct BlueOsQueue blueOsShellRxQueue;
extern volatile struct BlueOsQueue blueOsShellTxQueue;

/**
 * Shell synchronization
 */
#ifdef BLUE_OS_USE_SEMA
#include "blue_os_synchronization.h"
volatile BlueOsSema blueOsShellSema;
#endif	

#endif //BLUE_OS_USE_SHELL
#endif //BLUE_OS_SHELL_H