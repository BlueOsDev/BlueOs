
/********************************************************************************
*Copyright (C) 2013 	Moritz Nagel 	mnagel@htwg-konstanz.de					*
*					Daniel Urbanietz 	daniel@d-urbanietz.de					*
*********************************************************************************
*FILE: blue_hw_atmeg644P.h														*
*																				*
*AUTHOR(S): M. Nagel, D. Urbanietz												*
*																				*
*DESCRIPTION:																	*
*This file contains hardware specific definitions 								*
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

#ifndef BLUE_HW_ATMEG644P_H
#define BLUE_HW_ATMEG644P_H

	#define TIMER_PRESCALER 64UL
	#define TIMER_TICKS_MS (F_CPU / (TIMER_PRESCALER*1000UL))
	#define TIMER_COUNTER_INIT_VALUE (256UL - TIMER_TICKS_MS)
	#define TIMER_US_FACTOR ((TIMER_PRESCALER * 1000000UL) / F_CPU)

	#define TIMER_CONGFIGURATION			TCCR0A = 0x00; \
											TCCR0B = (1 << CS00) | (1 << CS01);

	#define TIMER_MASK_REGISTER				TIMSK0
	#define TIMER_INTERRUPT_MASK			TOIE0
	#define BLUE_OS_INTERRUPT				TIMER0_OVF_vect
	#define TIMER_COUNTER					TCNT0
	#define TIMER_INTERRUPT_FLAG_REGISTER 	TIFR0
	#define TIMER_INTERRUPT_FLAG			TOV0

	// Shell
	#define UART_CONFIGURATION				UBRR0H = (unsigned char)(UART_UBRR_VALUE>>8); \
    										UBRR0L = (unsigned char) UART_UBRR_VALUE; \
											UCSR0B = _BV(RXCIE0)|(1<<RXEN0)|(1<<TXEN0); \
											UCSR0C = (3<<UCSZ00);

	// UART
	#define UART_COUNT						2

	#define UART0_UDR						UDR0
	#define UART0_FE						(UCSR0A & (1<<FE0))  // Framing error
	#define UART0_DOR						(UCSR0A & (1<<DOR0)) // Data overrun
	#define UART0_UPE						(UCSR0A & (1<<UPE0)) // Parity error

	#define UART0_UDRIE_REGISTER			UCSR0B
	#define UART0_TXCIE_REGISTER			UCSR0B
	#define UART0_RXCIE_REGISTER			UCSR0B
	#define UART0_UDRIE_FLAG				UDRIE0
	#define UART0_TXCIE_FLAG				TXCIE0
	#define UART0_RXCIE_FLAG				RXCIE0

	#define UART0_UDRE						(UCSR0A & (1<<UDRE0))

	#define UART1_UDR						UDR1
	#define UART1_FE						(UCSR1A & (1<<FE1))  // Framing error
	#define UART1_DOR						(UCSR1A & (1<<DOR1)) // Data overrun
	#define UART1_UPE						(UCSR1A & (1<<UPE1)) // Parity error

	#define UART1_UDRIE_REGISTER			UCSR1B
	#define UART1_TXCIE_REGISTER			UCSR1B
	#define UART1_RXCIE_REGISTER			UCSR1B
	#define UART1_UDRIE_FLAG				UDRIE1
	#define UART1_TXCIE_FLAG				TXCIE1
	#define UART1_RXCIE_FLAG				RXCIE1

	#define UART1_UDRE						(UCSR1A & (1<<UDRE1))
	
	// Interrupt definitions
	#define INTERRUPT_UART0_RXC				USART0_RX_vect
	#define INTERRUPT_UART0_TXC				USART0_TX_vect
	#define INTERRUPT_UART0_UDRE			USART0_UDRE_vect
	#define INTERRUPT_UART1_RXC				USART1_RX_vect
	#define INTERRUPT_UART1_TXC				USART1_TX_vect
	#define INTERRUPT_UART1_UDRE			USART1_UDRE_vect

	#define TIMER_INTERRUPT_ENABLE			TIMER_MASK_REGISTER |= (1 << TIMER_INTERRUPT_MASK);
	#define TIMER_INTERRUPT_DISABLE			TIMER_MASK_REGISTER &= ~(1 << TIMER_INTERRUPT_MASK);

#endif
