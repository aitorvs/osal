/*
** osplatform.h
**
** Testing
*/

#ifndef _osplatform_
#define _osplatform_

/*
** external definitions for simulated registers
** There should probably be a base set of generic 
** registers for core fsw
*/
extern unsigned long int os_register_bank[10];

extern unsigned long int os_sram_bank  [128 * 1024];
extern unsigned long int os_eeprom_bank[128 * 1024];

extern unsigned char	    os_1553_mem_bank[0x8000];
extern unsigned char     os_1553_reg_bank[0x8000];

/*
** These are the base physical memory addresses for the DDC Shared Ram and Registers
*/
#define  DDC_MEM_BASE_ADDR           ((uint32 )(&(os_1553_mem_bank[0])))
#define  DDC_REG_BASE_ADDR           ((uint32 )(&(os_1553_reg_bank[0])))

/*
** Defines for interrupts
*/
#define OS_1553_INT_VECTOR             (0x100)

/*
** Address ranges for all memory banks.
*/

#define OS_SRAM_BANK_START_ADDRESS     ((uint32 )(&(os_sram_bank[0])))
#define OS_SRAM_BANK_END_ADDRESS       ((uint32 )(&(os_sram_bank[0]) + sizeof(os_sram_bank)))

#define OS_EEPROM_BANK_START_ADDRESS   ((uint32 )(&(os_eeprom_bank[0])))
#define OS_EEPROM_BANK_END_ADDRESS     ((uint32 )(&(os_eeprom_bank[0]) + sizeof(os_eeprom_bank)))


/*
** EEPROM characteristic
*/
#define OS_EEPROM_WRITE_DELAY          20 /* time required for the EEPROM to write itself */


#endif

