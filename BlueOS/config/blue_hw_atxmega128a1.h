
/********************************************************************************
*Copyright (C) 2013 	Moritz Nagel 	mnagel@htwg-konstanz.de					*
*					Daniel Urbanietz 	daniel@d-urbanietz.de					*
*********************************************************************************
*FILE: blue_hw_atxmega128a1.h													*
*																				*
*AUTHOR(S): M. Nagel, D. Urbanietz													*
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

#ifndef BLUE_HW_ATXMEGA128A1_H
#define BLUE_HW_ATXMEGA128A1_H


#define BLUE_OS_HW_3BYTE_PROGRAMPOINTER
#define BLUE_OS_HW_WEIRD_INTERRUPTS


	
	#define TIMER_PRESCALER 64UL
	#define TIMER_TICKS_MS (F_CPU / (TIMER_PRESCALER*1000UL))
	#define TIMER_COUNTER_INIT_VALUE (0xFFFF - TIMER_TICKS_MS)
	#define TIMER_US_FACTOR ((TIMER_PRESCALER * 1000000UL) / F_CPU)

	// Prescaler 1/64 (Use PortF timer)
	// Overflow interrupt (Mid level)

	#define BLUE_OS_16_BIT_TIMER

	#define TIMER_CONGFIGURATION			PMIC_CTRL = 0x07;\
											TCF0_CTRLA = 0x05;\
											TCF0_CTRLB = 0x00;\
											TCF0_CTRLC = 0x00;\
											TCF0_CTRLD = 0x00;\
											TCF0_INTCTRLA = 0x00;\
											TCF0_INTCTRLB = 0x00;
											
	
	#define TIMER_COMPARE_VALUE TCF0_CCA
	#define TIMER_ENABLE_COMPARE 	TCF0_INTFLAGS |= 0x10;\
									TCF0_INTCTRLB |= 0x03;\
									TCF0_CTRLB |= 0x10;
	#define TIMER_DISABLE_COMPARE 	TCF0_INTCTRLB &= ~(0x03);\
									TCF0_CTRLB &= ~(0x10);

	#define TIMER_COMPARE_INTERRUPT			TCF0_CCA_vect

	
	#define BLUE_OS_INTERRUPT				TCF0_OVF_vect
	#define TIMER_COUNTER					TCF0_CNT
	#define TIMER_INTERRUPT_FLAG_REGISTER 	TCF0_INTFLAGS
	#define TIMER_INTERRUPT_FLAG			TC0_OVFIF_bp

	// Shell
	#if BLUE_OS_USE_SHELL_UART > 2
		#error This UART does not exist!
	#endif

	//UART Interrupt Level: low
	#define __UART_CONFIGURATION(NR)		USART##NR##_CTRLC = 3; \
    										USART##NR##_BAUDCTRLA =(uint8_t)XMEGA_UART_BAUD; \
											USART##NR##_BAUDCTRLB =(XMEGA_UART_SCALE << USART_BSCALE0_bp)|(XMEGA_UART_BAUD >> 8); \
											USART##NR##_CTRLB |= USART_TXEN_bm;\
											USART##NR##_CTRLB |= USART_RXEN_bm;\
											USART##NR##_CTRLA |= (3<<USART_RXCINTLVL_gp);
	
	#define _UART_CONFIGURATION(NR) __UART_CONFIGURATION(NR)
	#define UART_CONFIGURATION _UART_CONFIGURATION(BLUE_OS_USE_SHELL_UART)
	
	// UART
	#define __UART_UDR(NR)					USART##NR##_DATA
	#define _UART_UDR(NR) __UART_UDR(NR)
	#define UART_UDR _UART_UDR(BLUE_OS_USE_SHELL_UART)
	
	#define __UART_FE(NR)					(USART##NR##_STATUS & USART_FERR_bm)  // Framing error
	#define _UART_FE(NR) __UART_FE(NR)
	#define UART_FE _UART_FE(BLUE_OS_USE_SHELL_UART)

	#define __UART_DOR(NR)					(USART##NR##_STATUS & USART_BUFOVF_bm) // Data overrun
	#define _UART_DOR(NR) __UART_DOR(NR)
	#define UART_DOR _UART_DOR(BLUE_OS_USE_SHELL_UART)
	
	#define __UART_UPE(NR)					(USART##NR##_STATUS & USART_PERR_bm) // Parity error
	#define _UART_UPE(NR) __UART_UPE(NR)
	#define UART_UPE _UART_UPE(BLUE_OS_USE_SHELL_UART)			


	#define __UART_UDRIE_REGISTER(NR)		USART##NR##_CTRLA
	#define _UART_UDRIE_REGISTER(NR) __UART_UDRIE_REGISTER(NR)
	#define UART_UDRIE_REGISTER _UART_UDRIE_REGISTER(BLUE_OS_USE_SHELL_UART)
	#define UART_TXCIE_REGISTER			UART_UDRIE_REGISTER
	#define UART_RXCIE_REGISTER			UART_UDRIE_REGISTER
	#define UART_UDRIE_FLAG				USART_DREINTLVL_gp
	#define UART_TXCIE_FLAG				USART_TXCINTLVL_gp
	#define UART_RXCIE_FLAG				USART_RXCINTLVL_gp

	#define __UART_UDRE(NR)				(USART##NR##_STATUS & USART_DREIF_bm)
	#define _UART_UDRE(NR) __UART_UDRE(NR)
	#define UART_UDRE _UART_UDRE(BLUE_OS_USE_SHELL_UART)


	// Interrupt definitions
	#define __INTERRUPT_UART_RXC(NR)		USART##NR##_RXC_vect
	#define _INTERRUPT_UART_RXC(NR) __INTERRUPT_UART_RXC(NR)
	#define INTERRUPT_UART_RXC _INTERRUPT_UART_RXC(BLUE_OS_USE_SHELL_UART)

	#define __INTERRUPT_UART_TXC(NR)		USART##NR##_TXC_vect
	#define _INTERRUPT_UART_TXC(NR) __INTERRUPT_UART_TXC(NR)
	#define INTERRUPT_UART_TXC _INTERRUPT_UART_TXC(BLUE_OS_USE_SHELL_UART)
	
	#define __INTERRUPT_UART_UDRE(NR)		USART##NR##_DRE_vect
	#define _INTERRUPT_UART_UDRE(NR) __INTERRUPT_UART_UDRE(NR)
	#define INTERRUPT_UART_UDRE _INTERRUPT_UART_UDRE(BLUE_OS_USE_SHELL_UART)


	#define TIMER_INTERRUPT_ENABLE			TCF0_INTCTRLA = 0x03;
	#define TIMER_INTERRUPT_DISABLE			TCF0_INTCTRLA = 0x00;
#endif
