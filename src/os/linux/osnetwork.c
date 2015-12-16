/*
 * File   : osfileapi.c
 *
 * Author : Nicholas Yanchik / NASA GSFC Code 582.0
 * Modifications/enhanced : Aitor Viana / ESA-ESTEC
 * Purpose: 
 *         This file  contains some of the OS APIs abstraction layer 
 *         implementation for POSIX, specifically for Linux / Mac OS X.
 *
 */

/****************************************************************************************
                                    INCLUDE FILES
****************************************************************************************/

#include "stdio.h"
#include "unistd.h"
#include "stdlib.h"

#include "osal/osapi.h"



/****************************************************************************************
                                     DEFINES
****************************************************************************************/



/****************************************************************************************
                                   GLOBAL DATA
****************************************************************************************/


/****************************************************************************************
                                INITIALIZATION FUNCTION
****************************************************************************************/


/****************************************************************************************
                                    Network API
****************************************************************************************/
/*--------------------------------------------------------------------------------------
    Name: OS_NetworkGetID
    
    Purpose: Gets the ID of the current Network 

    Returns: OS_STATUS_EERR if the  host id could not be found
             a 32 bit host id if success
---------------------------------------------------------------------------------------*/
int32_t OS_NetworkGetID             (void)
{
  int    host_id;

   host_id = gethostid();
   if (host_id == -1)
       return OS_STATUS_EERR;
   
    return (host_id);
    
}/* end OS_NetworkGetID */
/*--------------------------------------------------------------------------------------
    Name: OS_NetworkGetHostName
    
    Purpose: Gets the name of the current host

    Returns: OS_STATUS_EERR if the  host name could not be found
             OS_STATUS_SUCCESS if the name was copied to host_name successfully
---------------------------------------------------------------------------------------*/

int32_t OS_NetworkGetHostName       (char *host_name, uint32_t name_len)
{
   int    retval;
   uint32_t return_code;
   
   if ( host_name == NULL)
   {
      return_code = OS_STATUS_EINVAL;
   }
   else if ( name_len == 0 )
   {
      return_code = OS_STATUS_EERR;
   }
   else
   {
      retval = gethostname( host_name, name_len);
      if ( retval == -1 )
      {
      
         return_code = OS_STATUS_EERR;
      }
      else
      {
         return_code = OS_STATUS_SUCCESS;
      }
   }

   return(return_code);
}/* end OS_NetworkGetHostName */



