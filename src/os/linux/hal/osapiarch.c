/*
** File   :	osapiarch.c
**
** Author :	Ezra Yeheskeli
**
** Purpose:
**		   This file  contains some of the OS APIs abstraction layer.
**         It contains the processor architecture specific calls.
**
**  16-Nov-2003 Ezra Yeheskeli
**          - First Creation.
**
*/

/*
** Include section
*/

#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <string.h>

/*
** User defined include files
*/

#include "common_types.h"
#include "osapi.h"


/*
** EEPROM Defines
*/

#define EEPROM_WRITE_TIMEOUT	0x0000FFFF

/*
** global memory
*/

/*
** Global Memory buffers that represents the hardware memory, eeprom, and I/O space
** 1 Megabyte of memory for mem read/write
*/
unsigned char os_1553_mem_bank[0x8000];
unsigned char os_1553_reg_bank[0x8000];

unsigned long int os_register_bank[10];

unsigned long int os_sram_bank  [128 * 1024];
unsigned long int os_eeprom_bank[128 * 1024];



/*
** Name: OS_MemRead8
**
** Purpose:
**         Read one byte of memory.
**
**
** Parameters:
**	Address : Address to be read
**  Value8  : The address content will be copied to the location pointed by this argument
**
** Global Inputs: None
**
** Global Outputs: None
**
**
**
** Return Values: OS_SUCCESS
*/
int32 OS_MemRead8( uint32 Address, uint8 *Value8 )
{

	(*Value8) = (uint8)*((uint8 *)Address) ;

	return(OS_SUCCESS) ;
}

/*
** Name: OS_MemWrite8
**
** Purpose:
**         Write one byte of memory.
**
**
** Parameters:
**	Address : Address to be written to
**  Value8  : The content pointed by this argument will be copied to the address
**
** Global Inputs: None
**
** Global Outputs: None
**
**
**
** Return Values:
**		OS_SUCCESS
*/
int32 OS_MemWrite8 ( uint32 Address, uint8 Value8 )
{
   *((uint8 *)Address) = Value8;
	return(OS_SUCCESS) ;

}

/*
** Name: OS_MemRead16
**
** Purpose:
**         Read  2 bytes of memory.
**
**
** Parameters:
**	Address : Address to be read
**  Value16 : The address content will be copied to the location pointed by
**            this argument
**
** Global Inputs: None
**
** Global Outputs: None
**
**
**
** Return Values:
**		OS_SUCCESS
**		OS_ERROR_ADD_MISALIGNED The Address is not aligned to 16 bit
**      addressing scheme.
*/
int32 OS_MemRead16( uint32 Address, uint16 *Value16 )
{
	/* check 16 bit alignment  , check the 1st lsb */
	if( Address & 0x00000001)
	{
		return(OS_ERROR_ADDRESS_MISALIGNED) ;
	}
	(*Value16) = (uint16)*((uint16 *)Address) ;
	return(OS_SUCCESS) ;

}


/*
** Name: OS_MemWrite16
**
** Purpose:
**         Write 2 byte of memory.
**
**
** Parameters:
**	Address : Address to be written to
**  Value16 : The content pointed by this argument will be copied to the
**            address
**
** Global Inputs: None
**
** Global Outputs: None
**
**
**
** Return Values:
**      OS_SUCCESS
**		OS_ERROR_ADD_MISALIGNED The Address is not aligned to 16 bit
**      addressing scheme.
*/
int32 OS_MemWrite16 ( uint32 Address, uint16 Value16 )
{
	/* check 16 bit alignment  , check the 1st lsb */
	if( Address & 0x00000001)
	{
		return(OS_ERROR_ADDRESS_MISALIGNED) ;
	}
   *((uint16 *)Address) = Value16;
	return(OS_SUCCESS) ;
}

/*
** Name: OS_MemRead32
**
** Purpose:
**         Read 4 bytes of memory.
**
**
** Parameters:
**	Address : Address to be read
**  Value32 : The address content will be copied to the location pointed by
**            this argument
**
** Global Inputs: None
**
** Global Outputs: None
**
**
**
** Return Values:
**		OS_SUCCESS
**		OS_ERROR_ADD_MISALIGNED The Address is not aligned to 16 bit
**      addressing scheme.
*/
int32 OS_MemRead32( uint32 Address, uint32 *Value32 )
{
	/* check 32 bit alignment  */
	if( Address & 0x00000003)
	{
		return(OS_ERROR_ADDRESS_MISALIGNED) ;
	}
	(*Value32) = *((uint32 *)Address) ;
	return(OS_SUCCESS) ;

}

/*
** Name: OS_MemWrite32
**
** Purpose:
**         Write 4 byte of memory.
**
**
** Parameters:
**	Address : Address to be written to
**  Value32 : The content pointed by this argument will be copied to the
**            address
**
** Global Inputs: None
**
** Global Outputs: None
**
**
**
** Return Values:
**		OS_SUCCESS
**		OS_ERROR_ADD_MISALIGNED The Address is not aligned to 16 bit
**      addressing scheme.
*/
int32 OS_MemWrite32 ( uint32 Address, uint32 Value32 )
{
	/* check 32 bit alignment  */
	if( Address & 0x00000003)
	{
		return(OS_ERROR_ADDRESS_MISALIGNED) ;
	}
   *((uint32 *)Address) = Value32;
	return(OS_SUCCESS) ;

}



/*
** Name: OS_PortRead8
**
** Purpose:
**         Read one byte of memory.
**
**
** Parameters:
**	Address : Address to be read
**  Value8  : The address content will be copied to the location pointed by
**            this argument
**
** Global Inputs: None
**
** Global Outputs: None
**
**
**
** Return Values: OS_SUCCESS
*/
int32 OS_PortRead8( uint32 Address, uint8 *Value8 )
{

	(*Value8) = (uint8)*((uint8 *)Address) ;

	return(OS_SUCCESS) ;
}

/*
** Name: OS_PortWrite8
**
** Purpose:
**         Write one byte of memory.
**
**
** Parameters:
**	Address : Address to be written to
**  Value8  : The content pointed by this argument will be copied to the
**            address
**
** Global Inputs: None
**
** Global Outputs: None
**
**
**
** Return Values:
**		OS_SUCCESS
*/
int32 OS_PortWrite8 ( uint32 Address, uint8 Value8 )
{
   *((uint8 *)Address) = Value8;
	return(OS_SUCCESS) ;

}

/*
** Name: OS_PortRead16
**
** Purpose:
**         Read  2 bytes of memory.
**
**
** Parameters:
**	Address : Address to be read
**  Value16 : The address content will be copied to the location pointed by
**            this argument
**
** Global Inputs: None
**
** Global Outputs: None
**
**
**
** Return Values:
**		OS_SUCCESS
**		OS_ERROR_ADD_MISALIGNED The Address is not aligned to 16 bit
**      addressing scheme.
*/
int32 OS_PortRead16( uint32 Address, uint16 *Value16 )
{
	/* check 16 bit alignment  , check the 1st lsb */
	if( Address & 0x00000001)
	{
		return(OS_ERROR_ADDRESS_MISALIGNED) ;
	}
	(*Value16) = (uint16)*((uint16 *)Address) ;
	return(OS_SUCCESS) ;

}


/*
** Name: OS_PortWrite16
**
** Purpose:
**         Write 2 byte of memory.
**
**
** Parameters:
**	Address : Address to be written to
**  Value16 : The content pointed by this argument will be copied to the
**            address
**
** Global Inputs: None
**
** Global Outputs: None
**
**
**
** Return Values:
**      OS_SUCCESS
**		OS_ERROR_ADD_MISALIGNED The Address is not aligned to 16 bit
**      addressing scheme.
*/
int32 OS_PortWrite16 ( uint32 Address, uint16 Value16 )
{
	/* check 16 bit alignment  , check the 1st lsb */
	if( Address & 0x00000001)
	{
		return(OS_ERROR_ADDRESS_MISALIGNED) ;
	}
   *((uint16 *)Address) = Value16;
	return(OS_SUCCESS) ;
}

/*
** Name: OS_PortRead32
**
** Purpose:
**         Read 4 bytes of memory.
**
**
** Parameters:
**	Address : Address to be read
**  Value32 : The address content will be copied to the location pointed by
**            this argument
**
** Global Inputs: None
**
** Global Outputs: None
**
**
**
** Return Values:
**		OS_SUCCESS
**		OS_ERROR_ADD_MISALIGNED The Address is not aligned to 16 bit
**      addressing scheme.
*/
int32 OS_PortRead32( uint32 Address, uint32 *Value32 )
{
	/* check 32 bit alignment  */
	if( Address & 0x00000003)
	{
		return(OS_ERROR_ADDRESS_MISALIGNED) ;
	}
	(*Value32) = *((uint32 *)Address) ;
	return(OS_SUCCESS) ;

}

/*
** Name: OS_PortWrite32
**
** Purpose:
**         Write 4 byte of memory.
**
**
** Parameters:
**	Address : Address to be written to
**  Value32 : The content pointed by this argument will be copied to the
**            address
**
** Global Inputs: None
**
** Global Outputs: None
**
**
**
** Return Values:
**		OS_SUCCESS
**		OS_ERROR_ADD_MISALIGNED The Address is not aligned to 16 bit
**      addressing scheme.
*/
int32 OS_PortWrite32 ( uint32 Address, uint32 Value32 )
{
	/* check 32 bit alignment  */
	if( Address & 0x00000003)
	{
		return(OS_ERROR_ADDRESS_MISALIGNED) ;
	}
   *((uint32 *)Address) = Value32;
	return(OS_SUCCESS) ;

}

/*
** Name: OS_MemCpy
**
** Purpose:
**	Copies 'size' byte from memory address pointed by 'src' to memory
**  address pointed by ' dst' For now we are using the standard c library
**  call 'memcpy' but if we find we need to make it more efficient then
**  we'll implement it in assembly.
**
** Assumptions and Notes:
**
** Parameters:
**	dst : pointer to an address to copy to
**  src : pointer address to copy from
**
** Global Inputs: None
**
** Global Outputs: None
**
**
** Return Values: OS_SUCCESS
*/
int32 OS_MemCpy ( void *dst, void *src, uint32 size)
{
	memcpy( dst, src, size);
	return(OS_SUCCESS) ;
} ;



/*
** Name: OS_MemSet
**
** Purpose:
**	Copies 'size' number of byte of value 'value' to memory address pointed
**  by 'dst' .For now we are using the standard c library call 'memset'
**  but if we find we need to make it more efficient then we'll implement
**  it in assembly.
**
**
** Assumptions and Notes:
**
** Parameters:
**
** Global Inputs: None
**
** Global Outputs: None
**
**
** Return Values: OS_SUCCESS
*/
/*
** OS_MemSet
*/
int32 OS_MemSet ( void *dst, uint8 value , uint32 size)
{
    memset( dst, (int)value, (size_t)size);
	return(OS_SUCCESS) ;
} ;




/*
** Name: OS_EepromWrite32
**
** Purpose:
**
** Assumptions and Notes:
**
** Parameters:
**
** Global Inputs: None
**
** Global Outputs: None
**
**
** Return Values:
**	 OS_SUCCESS
**	 OS_ERROR_TIMEOUT write operation did not go through after a specific
**   timeout.
**	 OS_ERROR_ADD_MISALIGNED The Address is not aligned to 16 bit addressing
**   scheme.
*/
int32 OS_EepromWrite32( uint32 MemoryAddress, uint32 Value32 )
{
    uint32 ret_value = OS_SUCCESS;

	/* check 32 bit alignment  */
	if( MemoryAddress & 0x00000003)
	{
		return(OS_ERROR_ADDRESS_MISALIGNED) ;
	}

   /* make the Write */
   *((uint32 *)MemoryAddress) = Value32;

	return(ret_value) ;
}


/*
** Name: OS_EepromWrite16
**
** Purpose:
**
** Assumptions and Notes:
**
** Parameters:
**
** Global Inputs: None
**
** Global Outputs: None
**
**
** Return Values:
**   OS_SUCCESS
**	 OS_ERROR_TIMEOUT write operation did not go through after a specific
**   timeout.
**   OS_ERROR_ADD_MISALIGNED The Address is not aligned to 16 bit addressing
**   scheme.
*/
int32 OS_EepromWrite16( uint32 MemoryAddress, uint16 Value16 )
{
	uint32 write32 ,temp32 ;
	uint32 aligned_address ;


	/* check 16 bit alignment  , check the 1st lsb */
	if( MemoryAddress & 0x00000001)
	{
		return(OS_ERROR_ADDRESS_MISALIGNED) ;
	}


	temp32 = Value16 ;

    /* check the 2nd lsb */
	if( MemoryAddress & 0x00000002 )
	{
		/* writting the 16 high bit order of 32 bit field */
		aligned_address = MemoryAddress-2 ;

		OS_MemRead32 ( aligned_address  ,&write32)  ;
		write32 = (write32 & 0x0000FFFF) | (temp32<<16 ) ;
	}
	else
	{
		/* writting the 16 low bit order of 32 bit field */
		aligned_address = MemoryAddress ;

		OS_MemRead32 (  aligned_address, &write32 ) ;
		write32 = (write32 ) | (temp32 & 0xFFFF0000 ) ;
	}


	return(OS_EepromWrite32(aligned_address,write32)) ;

}


/*
** Name: OS_EepromWrite8
**
** Purpose:
**
** Assumptions and Notes:
**
** Parameters:
**
** Global Inputs: None
**
** Global Outputs: None
**
**
** Return Values:
**   OS_SUCCESS
**	 OS_ERROR_TIMEOUT write operation did not go through after a specific
**   timeout.
*/
int32 OS_EepromWrite8( uint32 MemoryAddress, uint8 Value8 )
{
	uint32 aligned_address ;
	uint16 write16 ,temp16;



	temp16 = Value8 ;

    /* check the 1st lsb */
	if( MemoryAddress & 0x00000001)
	{
		/* writting the 8 high bit order of 16 bit field */
		aligned_address = MemoryAddress-1 ;

		OS_MemRead16 ( aligned_address  ,&write16)  ;
		write16 = (write16 & 0x00FF) | (temp16<<8 ) ;
	}
	else
	{
		/* writting the 8 low bit order of 16 bit field */
		aligned_address = MemoryAddress ;

		OS_MemRead16 (  aligned_address, &write16 ) ;
		write16 = (write16 ) | (temp16 & 0xFF00 ) ;
	}


	return(OS_EepromWrite16(aligned_address,write16)) ;

}

/*
Parallel Port layout
--------------------
Port Bit I/O Function Device Effected
- - --     -- ----------------
0 I EEPROM 1 ready signal EEPROM
1 O EEPROM 1 write protect EEPROM
2 O EEPROM reset Both EEPROM
3 O Reserved N/A
4 I Reserved N/A
5 O Reserved N/A
6 O RS-422 Enable RS-422 driver and receiver
7 O Mezzanine reset ASD
8 O Mezzanine power control ASD
9 O SP’ACE subsystem flag SP’ACE (1553)
10 O 1553 transceiver B inhibit SP’ACE (1553)
11 O 1553 transceiver A inhibit SP’ACE (1553)
12 I/O General purpose Available on 50-pin connector
13 I/O General purpose Available on 50-pin connector
14 I/O General purpose Available on 50-pin connector
15 I/O General purpose Available on 50-pin connector
*/

/*
** Name: OS_EepromWriteEnable
**
** Purpose:
**		Eable the eeprom for write operation
**
** Assumptions and Notes:
**
** Parameters:
**
** Global Inputs: None
**
** Global Outputs: None
**
**
** Return Values:
**   OS_SUCCESS
*/
int32 OS_EepromWriteEnable()
{

	return(OS_SUCCESS) ;
}

/*
** Name: OS_EepromWriteDisable
**
** Purpose:
**		Disable  the eeprom from write operation
**
** Assumptions and Notes:
**
** Parameters:
**
** Global Inputs: None
**
** Global Outputs: None
**
**
** Return Values:
**   OS_SUCCESS
*/
int32 OS_EepromWriteDisable()
{

	return(OS_SUCCESS) ;

}


/*
** Name: OS_EepromPowerUp
**
** Purpose:
**		Power up the eeprom
** Assumptions and Notes:
**
** Parameters:
**
** Global Inputs: None
**
** Global Outputs: None
**
**
** Return Values:
**   OS_SUCCESS
*/
int32 OS_EepromPowerUp()
{
	return(OS_SUCCESS) ;
}



/*
** Name: OS_EepromPowerDown
**
** Purpose:
**		Power down the eeprom
** Assumptions and Notes:
**
** Parameters:
**
** Global Inputs: None
**
** Global Outputs: None
**
**
** Return Values:
**   OS_SUCCESS
*/
int32 OS_EepromPowerDown()
{
	return(OS_SUCCESS) ;
}
