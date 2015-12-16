/**
 *  \file   osapi-utils.h
 *  \brief  This file defines all the generic services provided by the OSAL
 *  library
 *
 *  Detailed description starts here.
 *
 *  \author  Aitor Viana Sanchez (avs), Aitor.Viana.Sanchez@esa.int
 *
 *  \internal
 *    Created:  01/04/09
 *   Revision:  $Id: osapi-utils.h 1.4 01/04/09 avs Exp $
 *   Compiler:  gcc/g++
 *    Company:  European Space Agency (ESA-ESTEC)
 *  Copyright:  Copyright (c) 2009, Aitor Viana Sanchez
 *
 *  This source code is released for free distribution under the terms of the
 *  GNU General Public License as published by the Free Software Foundation.
 * =====================================================================================
 */

#ifndef _OSAL_UTILS_H_
#define _OSAL_UTILS_H_

/**
 *  \defgroup OSAL_utils Miscellaneous API
 *
 *  The OSAL library also supports generic utils related to operating system
 *  and embedded software issues.
 */

/**
 *  \ingroup OSAL_utils
 *  \brief This function performs the CRC generation.
 *
 *  The function performs the CRC of the given data block.
 *
 *  \param  pv_Buffer This is the data buffer the CRC will be generated from.
 *  \param  ul_Length     This is the length of the data buffer
 *
 *  \return CRC value
 */
uint32_t	OS_crc_gen( void * pv_Buffer, uint32_t ul_Length);

/**
 *  \ingroup OSAL_utils
 *  \brief This function performs the initialization of the CRC key.
 *
 *  The function may be called before calling \ref OS_crc_gen function but in
 *  case it is not called, \ref OS_crc_gen function will stablish a default CRC
 *  key to work with.
 *
 *  \param  ul_CrcKey  The CRC key
 *
 *  \return Always '0'
 */
int 	OS_crc_init( uint32_t ul_CrcKey);

/**
 *  \ingroup OSAL_utils
 *  \brief ECC generation function.
 *
 *  The function generates the ECC for the given data block. The maximum block
 *  size is 4Gbit (512MBytes). This size should be enough for all cases.
 *  The data block size must be always power of two (e.g. 128, 256, 512, etc).
 *
 *  \param  puc_Vec Data block for ECC calculation
 *  \param  ul_Order   Log2 of the data block size (e.g. 8 for 256byte block)
 *  \param  puc_Ecc     Byte array where the calculated ECC is returned to the
 *  user
 *
 *  The Calculated ECC consist of two parts of ul_Order+3 bits concatenated. First
 *  part is for even parity and second is for odd parity. Bytes are store
 *  low-endian and packed.
 *
 *  As an example, for 512 bytes data block (ul_Order=9), two 12-bit values are
 *  generated. Once packed they use 3 bytes of space for the generated ECC (
 *  (9+3) + (9+3) ).
 *
 *  \return Always '0'
 */
int OS_ecc_gen( uint8_t* puc_Vec, uint32_t ul_Order, uint8_t * puc_Ecc);

/**
 *  \ingroup OSAL_utils
 *  \brief This function performs the Error Checking and Correcting
 *  functionality.
 *
 *  The function perform the ECC functionality. The maximum block
 *  size is 4Gbit (512MBytes). This size should be enough for all cases.
 *  The data block size must be always power of two (e.g. 128, 256, 512, etc).
 *
 *  \param  puc_Vec Data block for ECC calculation
 *  \param  ul_Order   Log2 of the data block size (e.g. 8 for 256byte block).
 *  \param  puc_Ecc     Byte array where the calculated the previously ECC of the
 *  given data block.
 *
 *  The function recalculates the ECC and checks for errors. The function does
 *  the following:
 *
 *  - If no errors, only reporting to the user
 *  - If a single error occur and it is a correctable error in data, the error
 *  is corrected in the puc_Vec array
 *  - If a single error occur and it is a correctable error in puc_Ecc, the error
 *  is corrected in the puc_Ecc array
 *  - If a multiple error occur it is not possible to correct them so the
 *  function reports to the user that a non-correctable error occurred.
 *
 * \return Upon successful the function returns '0' otherwise -1 is returned and
 * os_errno is set to indicate the error.
 */
int OS_ecc_check_and_correct( uint8_t* puc_Vec, uint32_t ul_Order, uint8_t * puc_Ecc);

#endif
