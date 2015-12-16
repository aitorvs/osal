/*
** File   : osnetwork.c
** Author : 
*/

/****************************************************************************************
                                    INCLUDE FILES
****************************************************************************************/

#include "stdio.h"
#include "unistd.h"
#include "stdlib.h"

#include "common_types.h"
#include "osapi.h"



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

    Returns: OS_ERROR if the  host id could not be found
             a 32 bit host id if success
---------------------------------------------------------------------------------------*/
int32 OS_NetworkGetID             (void)
{
  int    host_id;

   host_id = gethostid();
   if (host_id == -1)
       return OS_ERROR;
   
    return (host_id);
    
}/* end OS_NetworkGetID */
/*--------------------------------------------------------------------------------------
    Name: OS_NetworkGetHostName
    
    Purpose: Gets the name of the current host

    Returns: OS_ERROR if the  host name could not be found
             OS_SUCCESS if the name was copied to host_name successfully
---------------------------------------------------------------------------------------*/

int32 OS_NetworkGetHostName       (char *host_name, uint32 name_len)
{
   int    retval;
   uint32 return_code;
   
   if ( host_name == NULL)
   {
      return_code = OS_INVALID_POINTER;
   }
   else if ( name_len == 0 )
   {
      return_code = OS_ERROR;
   }
   else
   {
      retval = gethostname( host_name, name_len);
      if ( retval == -1 )
      {
      
         return_code = OS_ERROR;
      }
      else
      {
         return_code = OS_SUCCESS;
      }
   }

   return(return_code);
}/* end OS_NetworkGetHostName */



