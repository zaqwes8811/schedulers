#ifndef _MAIN_H
#define _MAIN_H
//----------------------------------------------------------------------
// WILL NEED TO EDIT THIS SECTION FOR EVERY PROJECT
//----------------------------------------------------------------------
// Must include the appropriate microcontroller header file here
//#include <AT89×52.h>
// Must include oscillator / chip details here if delays are used
// -
// Oscillator / resonator frequency (in Hz) e.g. (11059200UL)
#define OSC_FREQ (11059200UL)
// Number of oscillations per instruction (6 or 12)
#define OSC_PER_INST (12)
//----------------------------------------------------------------------
// SHOULD NOT NEED TO EDIT THE SECTIONS BELOW
//----------------------------------------------------------------------
typedef unsigned char tByte;
typedef unsigned int tWord;
typedef unsigned long tLong;
// Misc #defines
#ifndef TRUE
#define FALSE 0
#define TRUE (!FALSE)
#endif
#define RETURN_NORMAL 0//(bit) 0
#define RETURN_ERROR 1//(bit) 1

//----------------------------------------------------------------------
// Interrupts
// – see Chapter 12.
//----------------------------------------------------------------------
// Generic 8051 timer interrupts (used in most schedulers)
#define INTERRUPT_Timer_0_Overflow 1
#define INTERRUPT_Timer_1_Overflow 3
#define INTERRUPT_Timer_2_Overflow 5
// Additional interrupts (used in shared-clock schedulers)
#define INTERRUPT_UART Rx_Tx 4
#define INTERRUPT_CAN_c515c 17
// ---------------------------------------------------------------------

// Error codes
// – see Chapter 13.
//----------------------------------------------------------------------
#define ERROR_SCH_TOO_MANY_TASKS (1)
#define ERROR_SCH_CANNOT_DELETE_TASK (2)
#define ERROR_SCH_WAITING_FOR_SLAVE_TO_ACK (0xAA)
#define ERROR_SCH_WAITING_FOR_START_COMMAND_FROM_MASTER (0xAA)
#define ERROR_SCH_ONE_OR_MORE_SLAVES_DID_NOT_START (0xA0)
#define ERROR_SCH_LOST_SLAVE (0x80)
#define ERROR_I2C_WRITE_BYTE_AT24C64 (11)
#define ERROR_I2C_READ_BYTE_AT24C64 (12)
#define ERROR_I2C_WRITE_BYTE (13)
#define ERROR_I2C_READ_BYTE (14)
#define ERROR_USART_TI (21)
#define ERROR_USART_WRITE_CHAR (22)

// Scheduler
// The maximum number of tasks required at any one time
// during the execution of the program
//
// MUST BE ADJUSTED FOR EACH NEW PROJECT
#define SCH_MAX_TASKS (4)
#endif
