/**
 *  \file   osapi-hw-core.h
 *  \brief  This file defines the Application Interface for handling hardware
 *  memory resources such as RAM, EEPROM, etc memory devices.
 *
 *  The file defines all the functions the user might use for accessing
 *  physical memory devices. Those function are architecture and hardware,
 *  reason why they are not inside the OSAL core functions source file and
 *  shall ben implemented inside the architecture directory.
 *
 *  Architectures which dont implement those function shall return an ERROR
 *  code.
 *
 *  \author  Ezra Yeheksli -Code 582/Raytheon
 *  \author  Aitor Viana Sanchez (avs), Aitor.Viana.Sanchez@esa.int
 *
 *  \internal
 *    Created:  29/01/08
 *   Revision:  $Id: osapi-hw-core.h 1.4 29/01/08 avs Exp $
 *   Compiler:  gcc/g++
 *
 * =====================================================================================
 */
/*
 * Author : Ezra Yeheskeli
 * Author : Alan Cudmore - Code 852
 * Modifications/enhanced : Aitor Viana / ESA-ESTEC
 * Purpose: 
 *         This file  contains some of the OS APIs abstraction layer 
 *         implementation for POSIX, specifically for Linux / Mac OS X.
 *
 */

#ifndef _osapi_hw_core_
#define _osapi_hw_core_


/**
 *  \ingroup OSAL
 *  \defgroup HW_API Library Hardware API
 *
 *  This API allows the user to deal with some hardware interfaces.
 *
 */
typedef enum 
{
   OS_PROM,
   OS_EEPROM
} os_prom_type ;

/*============================================= PORT INTERFACE    */

int32_t OS_PortRead8         (uint32_t PortAddress, uint8_t *value);
int32_t OS_PortWrite8        (uint32_t PortAddress, uint8_t value);
int32_t OS_PortRead16        (uint32_t PortAddress, uint16_t *PortValue);
int32_t OS_PortWrite16       (uint32_t PortAddress, uint16_t value);
int32_t OS_PortRead32        (uint32_t PortAddress, uint32_t *PortValue);
int32_t OS_PortWrite32       (uint32_t PortAddress, uint32_t value);
int32_t OS_PortSetAttributes (uint32_t key, uint32_t value);
int32_t OS_PortGetAttributes (uint32_t key, uint32_t *value);

/*============================================= RAM  INTERFACE    */

/**
 *  \ingroup HW_API
 *  \brief This function provides the library-user the way to read 8-bits value
 *  from RAM memory.
 *
 *  \param  MemoryAddress is the memory address to be read
 *  \param  value is the read value
 *
 * \return Upon successful the function returns '0' otherwise -1 is returned and
 * os_errno is set to indicate the error.
 */
int32_t OS_MemRead8          (uint32_t MemoryAddress, uint8_t *value);

/**
 *  \ingroup HW_API
 *  \brief This function provides the library-user the way to write 8-bits value
 *  to RAM memory.
 *
 *  \param  MemoryAddress is the memory address to be written
 *  \param  value is the value to be written
 *
 * \return Upon successful the function returns '0' otherwise -1 is returned and
 * os_errno is set to indicate the error.
 */
int32_t OS_MemWrite8         (uint32_t MemoryAddress, uint8_t value);

/**
 *  \ingroup HW_API
 *  \brief This function provides the library-user the way to read 16-bits value
 *  from RAM memory.
 *
 *  \param  MemoryAddress is the memory address to be read
 *  \param  value is the read value
 *
 * \return Upon successful the function returns '0' otherwise -1 is returned and
 * os_errno is set to indicate the error.
 */
int32_t OS_MemRead16         (uint32_t MemoryAddress, uint16_t *value);

/**
 *  \ingroup HW_API
 *  \brief This function provides the library-user the way to write 16-bits value
 *  to RAM memory.
 *
 *  \param  MemoryAddress is the memory address to be written. The address
 *  shall be aligned to 16 bit.
 *  \param  value is the value to be written. 
 *
 * \return Upon successful the function returns '0' otherwise -1 is returned and
 * os_errno is set to indicate the error.
 */
int32_t OS_MemWrite16        (uint32_t MemoryAddress, uint16_t value);

/**
 *  \ingroup HW_API
 *  \brief This function provides the library-user the way to read 32-bits value
 *  from RAM memory.
 *
 *  \param  MemoryAddress is the memory address to be read
 *  \param  value is the read value
 *
 * \return Upon successful the function returns '0' otherwise -1 is returned and
 * os_errno is set to indicate the error.
 */
int32_t OS_MemRead32         (uint32_t MemoryAddress, uint32_t *value);

/**
 *  \ingroup HW_API
 *  \brief This function provides the library-user the way to write 32-bits value
 *  to RAM memory.
 *
 *  \param  MemoryAddress is the memory address to be written. The address
 *  shall be aligned to 16 bit.
 *  \param  value is the value to be written. 
 *
 * \return Upon successful the function returns '0' otherwise -1 is returned and
 * os_errno is set to indicate the error.
 */
int32_t OS_MemWrite32        (uint32_t MemoryAddress, uint32_t value);

/*============================================= EEPROM INTERFACE  */

int32_t OS_EepromWrite8      (uint32_t MemoryAddress, uint8_t value);
int32_t OS_EepromWrite16     (uint32_t MemoryAddress, uint16_t value);
int32_t OS_EepromWrite32     (uint32_t MemoryAddress, uint32_t value);
int32_t OS_EepromWriteEnable (void);
int32_t OS_EepromWriteDisable(void);
int32_t OS_EepromPowerUp     (void);
int32_t OS_EepromPowerDown   (void);

int32_t OS_MemCheckRange     (uint32_t Address, uint32_t Size);
int32_t OS_MemSetAttributes  (uint32_t key, uint32_t value);
int32_t OS_MemGetAttributes  (uint32_t key, uint32_t *value);

/*============================================= UTILS  INTERFACE  */

int32_t OS_MemCpy            (void *dest, void *src, uint32_t n);
int32_t OS_MemSet            (void *dest, uint8_t value, uint32_t n);


#endif

