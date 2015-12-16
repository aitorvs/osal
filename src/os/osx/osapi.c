/*
 * =====================================================================================
 *
 *       Filename:  osapi.c
 *
 *    Description:  This file contains some of the OSAL implementation for
 *    POSIX, for Mac OS X 
 *
 *        Version:  1.0
 *        Created:  11/09/2008 16:00:22
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Aitor Viana Sanchez (avs), aitor.viana.sanchez@esa.int
 *        Company:  European Space Agency (ESA-ESTEC)
 *
 * =====================================================================================
 */

/*
** File   : osapi.c
**
** Author : Alan Cudmore
**
** Purpose: 
**         This file  contains some of the OS APIs abstraction layer 
**         implementation for POSIX, specifically for Linux / Mac OS X.
**
*/
/****************************************************************************************
                                    INCLUDE FILES
****************************************************************************************/
#include <stdio.h>
#include <sys/types.h>
#include <ctype.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include <time.h>
#include <signal.h>
#include <stdarg.h>

#include <errno.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>     
#include <sys/select.h>
#include <sys/time.h>
#include <fcntl.h>
#include <errno.h>

#include <limits.h>


#include <sys/ipc.h> 
#include <sys/shm.h>


/*
** User defined include files
*/
#include "common_types.h"
#include "osapi.h"


/*
** Defines
*/
#define OS_BASE_PORT 43000
#define UNINITIALIZED 0
#define MAX_PRIORITY 255
#ifndef PTHREAD_STACK_MIN
   #define PTHREAD_STACK_MIN 8092
#endif


/*
** Global data for the API
*/

/*  
** Tables for the properties of objects 
*/

/*tasks */
typedef struct
{
    int free;
    pthread_t id;
    int creator;
    uint32 stack_size;
    uint32 priority;
}OS_task_record_t;
    
/* queues */
typedef struct
{
    int free;
    int id;
    int creator;
}OS_queue_record_t;

/* Binary Semaphores */
typedef struct
{
    int free;
    sem_t *id;
    int creator;
}OS_bin_sem_record_t;

/* Binary Semaphores */
typedef struct
{
    int free;
    sem_t *id;
    int creator;
}OS_count_sem_record_t;

/* Mutexes */
typedef struct
{
    int free;
    pthread_mutex_t id;
    int creator;
}OS_mut_sem_record_t;

/* Tables where the OS object information is stored */
OS_task_record_t    OS_task_table          [OS_MAX_TASKS];
OS_queue_record_t   OS_queue_table         [OS_MAX_QUEUES];
OS_bin_sem_record_t OS_bin_sem_table       [OS_MAX_BIN_SEMAPHORES];
OS_count_sem_record_t OS_count_sem_table   [OS_MAX_COUNT_SEMAPHORES];
OS_mut_sem_record_t OS_mut_sem_table       [OS_MAX_MUTEXES];

pthread_key_t    thread_key;

pthread_mutex_t OS_task_table_mut;
pthread_mutex_t OS_queue_table_mut;
pthread_mutex_t OS_bin_sem_table_mut;
pthread_mutex_t OS_mut_sem_table_mut;
pthread_mutex_t OS_count_sem_table_mut;

/*
** Local Function Prototypes
*/
LOCAL uint32  OS_CompAbsDelayedTime( uint32 milli_second , struct timespec * tm);
LOCAL void    OS_ThreadKillHandler(int sig );
LOCAL Result_T OS_TaskRegister (void);

LOCAL unsigned int is_init = FALSE;

Result_T OS_Start(void)
{
    uint32 semid;
    int32 status;
    
    assert(is_init);
    if(!is_init)	return OS_ERROR;
    
    
    if((status = OS_BinSemCreate(&semid, 0, 0)) != OS_SUCCESS)
        OS_MSG_ERR(status, "Idle task sem creation not success", 0);
    
    /*  Idle task to maintain the resources   */
    while(1)
    {
        OS_MSG_DBG("Idle task started");
        if((status = OS_BinSemTake(semid)) != OS_SUCCESS)
            OS_MSG_ERR(status, "Idle task sem take not success\n", 0);

        OS_MSG_DBG("Ups!!! you should not reach this point!!!\n");
    }
    
    return OS_ERROR;	// never reach here
}

/*---------------------------------------------------------------------------------------
   Name: OS_Init

   Purpose: Initialize the tables that the OS API uses to keep track of information
            about objects

   returns: nothing
---------------------------------------------------------------------------------------*/
Result_T OS_Init(void)
{
   int i;
   int ret;

   assert( !is_init );
   if( TRUE == is_init ) return OS_ERROR;

    /* Initialize Task Table */
   
    for(i = 0; i < OS_MAX_TASKS; i++)
    {
        OS_task_table[i].free        = TRUE;
        OS_task_table[i].creator     = UNINITIALIZED;
    }

    /* Initialize Message Queue Table */

    for(i = 0; i < OS_MAX_QUEUES; i++)
    {
        OS_queue_table[i].free        = TRUE;
        OS_queue_table[i].id          = UNINITIALIZED;
        OS_queue_table[i].creator     = UNINITIALIZED;
    }

    /* Initialize Binary Semaphore Table */

    for(i = 0; i < OS_MAX_BIN_SEMAPHORES; i++)
    {
        OS_bin_sem_table[i].free        = TRUE;
        OS_bin_sem_table[i].creator     = UNINITIALIZED;
    }

    /* Initialize Counting Semaphores */
    for(i = 0; i < OS_MAX_BIN_SEMAPHORES; i++)
    {
        OS_count_sem_table[i].free        = TRUE;
        OS_count_sem_table[i].creator     = UNINITIALIZED;
    }


    /* Initialize Mutex Semaphore Table */

    for(i = 0; i < OS_MAX_MUTEXES; i++)
    {
        OS_mut_sem_table[i].free        = TRUE;
        OS_mut_sem_table[i].creator     = UNINITIALIZED;
    }
   
   ret = pthread_key_create(&thread_key, NULL );
   if ( ret != 0 )
   {
      printf("Error creating thread key\n");
   }

   pthread_mutex_init((pthread_mutex_t *) & OS_task_table_mut,NULL); 
   pthread_mutex_init((pthread_mutex_t *) & OS_queue_table_mut,NULL); 
   pthread_mutex_init((pthread_mutex_t *) & OS_bin_sem_table_mut,NULL); 
   pthread_mutex_init((pthread_mutex_t *) & OS_count_sem_table_mut,NULL); 
   pthread_mutex_init((pthread_mutex_t *) & OS_mut_sem_table_mut,NULL); 


//   OS_FS_Init();
   
   is_init = TRUE;
   
   return OS_SUCCESS;
   
}

/*
**********************************************************************************
**          TASK API
**********************************************************************************
*/


/*---------------------------------------------------------------------------------------
   Name: OS_TaskCreate

   Purpose: Creates a task and starts running it.

   returns: OS_INVALID_POINTER if any of the necessary pointers are NULL
            OS_ERR_INVALID_PRIORITY if the priority is bad
            OS_ERR_NO_FREE_IDS if there can be no more tasks created
            OS_ERROR if the operating system calls fail
            OS_SUCCESS if success
            
    NOTES: task_id is passed back to the user as the ID. stack_pointer is usually null.
           Flags are unused at this point.


---------------------------------------------------------------------------------------*/

Result_T OS_TaskCreate (
        uint32 *task_id, 
        const void *function_pointer,
        const uint32 *stack_pointer, 
        uint32 stack_size, 
        uint32 priority,
        uint32 flags, 
        void* arg)
{
    int                return_code = 0;
    pthread_attr_t     custom_attr ;
    struct sched_param priority_holder ;
    int                possible_taskid;
    uint32             local_stack_size;
    int                ret;

    UNUSED(stack_pointer);
    UNUSED(flags);

    /* Check for NULL pointers */
    
    if( (function_pointer == NULL) || (task_id == NULL) )
        return OS_INVALID_POINTER;
    
    /* Check for bad priority */

    if (priority > MAX_PRIORITY)
        return OS_ERR_INVALID_PRIORITY;

    
    /* Check Parameters */


    pthread_mutex_lock(&OS_task_table_mut); 

    for(possible_taskid = 0; possible_taskid < OS_MAX_TASKS; possible_taskid++)
    {
        if (OS_task_table[possible_taskid].free == TRUE)
        {
            break;
        }
    }

    /* Check to see if the id is out of bounds */
    
    if( possible_taskid >= OS_MAX_TASKS || OS_task_table[possible_taskid].free != TRUE)
    {
        pthread_mutex_unlock(&OS_task_table_mut);
        return OS_ERR_NO_FREE_IDS;
    }

    /* Set the possible task Id to not free so that
     * no other task can try to use it */
    OS_task_table[possible_taskid].free = FALSE ;
    
    pthread_mutex_unlock(&OS_task_table_mut);

    if ( stack_size < PTHREAD_STACK_MIN )
       local_stack_size = PTHREAD_STACK_MIN;
    else
        local_stack_size = stack_size;


    /*
    ** Set stack size
    */
    if(pthread_attr_init(&custom_attr))
    {   pthread_mutex_lock(&OS_task_table_mut); 
        OS_task_table[possible_taskid].free = TRUE;
        pthread_mutex_unlock(&OS_task_table_mut); 
        printf("pthread_attr_init error in OS_TaskCreate, Task ID = %d\n",possible_taskid);
        return(OS_ERROR);
    }

    
    if (pthread_attr_setstacksize(&custom_attr, (size_t)local_stack_size ))
    {
        printf("pthread_attr_setstacksize error in OS_TaskCreate, Task ID = %d\n",possible_taskid);
        return OS_ERROR;
    }
        
    /* 
    ** Set priority 
    */
    priority_holder.sched_priority = MAX_PRIORITY - priority ;
    
    if ( (ret = pthread_attr_setschedpolicy(&custom_attr, SCHED_FIFO)) != 0)
    	return OS_ERROR;
    if ( (ret = pthread_attr_setschedparam(&custom_attr,&priority_holder)) != 0 )
    	return OS_ERROR;

    /*
    ** Create thread
    */
    return_code = pthread_create(&(OS_task_table[possible_taskid].id),
                                 &custom_attr,
                                 function_pointer,
                                 arg);
    if (return_code != 0)
    {
        pthread_mutex_lock(&OS_task_table_mut); 
        OS_task_table[possible_taskid].free = FALSE;
        pthread_mutex_unlock(&OS_task_table_mut); 

        printf("pthread_create error in OS_TaskCreate, Task ID = %d\n",possible_taskid);
        return(OS_ERROR);
    }

    *task_id = possible_taskid;

    /* this Id no longer free */

    pthread_mutex_lock(&OS_task_table_mut); 

    OS_task_table[possible_taskid].creator = OS_TaskGetId();
    OS_task_table[possible_taskid].stack_size = stack_size;
    OS_task_table[possible_taskid].priority = MAX_PRIORITY - priority;

    pthread_mutex_unlock(&OS_task_table_mut);

    return OS_SUCCESS;
}/* end OS_TaskCreate */


/*--------------------------------------------------------------------------------------
     Name: OS_TaskDelete

    Purpose: Deletes the specified Task and removes it from the OS_task_table.

    returns: OS_ERR_INVALID_ID if the ID given to it is invalid
             OS_ERROR if the OS delete call fails
             OS_SUCCESS if success
---------------------------------------------------------------------------------------*/
Result_T OS_TaskDelete (uint32 task_id)
{    
#if defined (CONFIG_OS_STATIC_RESOURCE_ALLOCATION)
    return OS_NOT_SUPPORTED;
#else
    /* Check to see if the task_id given is valid */
    int ret;
    if (task_id >= OS_MAX_TASKS || OS_task_table[task_id].free == TRUE)
            return OS_ERR_INVALID_ID;

    /* Try to delete the task */
    
    ret = pthread_cancel(OS_task_table[task_id].id);
    if (ret != 0)
    {
        return OS_ERROR;
    }
    /*
     * Now that the task is deleted, remove its 
     * "presence" in OS_task_table
    */

    pthread_mutex_lock(&OS_task_table_mut); 

    OS_task_table[task_id].free = TRUE;
    OS_task_table[task_id].creator = UNINITIALIZED;
    OS_task_table[task_id].stack_size = UNINITIALIZED;
    OS_task_table[task_id].priority = UNINITIALIZED;
    OS_task_table[task_id].id = UNINITIALIZED;
    
    pthread_mutex_unlock(&OS_task_table_mut);

    return OS_SUCCESS;
#endif
    
}/* end OS_TaskDelete */
/*--------------------------------------------------------------------------------------
     Name: OS_TaskExit

    Purpose: Exits the calling task and removes it from the OS_task_table.

    returns: OS_SUCCESS when the call success
---------------------------------------------------------------------------------------*/

Result_T OS_TaskExit(void)
{
    uint32 task_id;

    task_id = OS_TaskGetId();

    pthread_mutex_lock(&OS_task_table_mut); 

    OS_task_table[task_id].free = TRUE;
    OS_task_table[task_id].creator = UNINITIALIZED;
    OS_task_table[task_id].stack_size = UNINITIALIZED;
    OS_task_table[task_id].priority = UNINITIALIZED;
    OS_task_table[task_id].id = UNINITIALIZED;

    pthread_mutex_unlock(&OS_task_table_mut);

    pthread_exit(NULL);

    return OS_SUCCESS;

}/*end OS_TaskExit */

/*---------------------------------------------------------------------------------------
   Name: OS_TaskDelay

   Purpose: Delay a task for specified amount of milliseconds

   returns: OS_ERROR if sleep fails or millisecond = 0
            OS_SUCCESS if success
---------------------------------------------------------------------------------------*/
Result_T OS_TaskDelay(uint32 millisecond )
{
    if (usleep(millisecond * 1000 ) != 0)
        return OS_ERROR;
    else
        return OS_SUCCESS;
    
    
}/* end OS_TaskDelay */
/*---------------------------------------------------------------------------------------
   Name: OS_TaskSetPriority

   Purpose: Sets the given task to a new priority

    returns: OS_ERR_INVALID_ID if the ID passed to it is invalid
             OS__ERR_INVALID_PRIORITY if the priority is greater than the max 
             allowed
             OS_ERROR if the OS call to change the priority fails
             OS_SUCCESS if success
---------------------------------------------------------------------------------------*/
Result_T OS_TaskSetPriority (uint32 task_id, uint32 new_priority)
{
#if defined(CONFIG_OS_STATIC_RESOURCE_ALLOCATION)
    return OS_NOT_SUPPORTED;
#else
    pthread_attr_t     custom_attr ;
    struct sched_param priority_holder ;

    if(task_id >= OS_MAX_TASKS || OS_task_table[task_id].free == TRUE)
        return OS_ERR_INVALID_ID;

    if (new_priority > MAX_PRIORITY)
        return OS_ERR_INVALID_PRIORITY;

    /* 
     ** Set priority -- This is currently incomplete ..
     */
    priority_holder.sched_priority = MAX_PRIORITY - new_priority ;
    if(pthread_attr_setschedparam(&custom_attr,&priority_holder))
    {
        OS_MSG_DBG("pthread_attr_setschedparam error in OS_TaskSetPriority, Task ID = %lu\n",task_id);
        return(OS_ERROR);
    }

    /* Change the priority in the table as well */
/*     OS_task_table[task_id].priority = MAX_PRIORITY - new_priority;
 */
    OS_task_table[task_id].priority = new_priority;

    return OS_SUCCESS;
#endif
} /* end OS_TaskSetPriority */


/*---------------------------------------------------------------------------------------
   Name: OS_TaskRegister
  
   Purpose: Registers the calling task id with the task by adding the var to the tcb
            It searches the OS_task_table to find the task_id corresponding to the tcb_id
            
   Returns: OS_ERR_INVALID_ID if there the specified ID could not be found
            OS_ERROR if the OS call fails
            OS_SUCCESS if success
---------------------------------------------------------------------------------------*/
LOCAL Result_T OS_TaskRegister (void)
{
    int          i;
    int          ret;
    uint32       task_id;
    pthread_t    pthread_id;


    /* set up a signal handler to be able to cancel this task if needed */
    signal(SIGUSR2, OS_ThreadKillHandler);
    /* 
    ** Get PTHREAD Id
    */
    pthread_id = pthread_self();

    /*
    ** Look our task ID in table 
    */
    for(i = 0; i < OS_MAX_TASKS; i++)
    {
       if(OS_task_table[i].id == pthread_id)
       {
          break;
       }
    }
    task_id = i;

    if(task_id == OS_MAX_TASKS)
    {
        return OS_ERR_INVALID_ID;
    }

    /*
    ** Add pthread variable
    */
    ret = pthread_setspecific(thread_key, (void *)task_id);
    if ( ret != 0 )
    {
       printf("OS_TaskRegister Failed during pthread_setspecific function\n");
       return(OS_ERROR);
    }

    return OS_SUCCESS;
}/* end OS_TaskRegister */


/*---------------------------------------------------------------------------------------
   Name: OS_TaskGetId

   Purpose: This function returns the #defined task id of the calling task

   Notes: The OS_task_key is initialized by the task switch if AND ONLY IF the 
          OS_task_key has been registered via OS_TaskRegister(..).  If this is not 
          called prior to this call, the value will be old and wrong.
---------------------------------------------------------------------------------------*/
Result_T OS_TaskGetId (void)
{ 
    pthread_t    pthread_id;
    uint32 i;  


    pthread_id = pthread_self();
    /* 
     ** Get PTHREAD Id
     */
    for (i = 0; i < OS_MAX_TASKS; i++)
    {
        if (pthread_equal(pthread_id, OS_task_table[i].id) != 0 )
        {
            break;
        }
    }

    return i;

}/* end OS_TaskGetId */

/*---------------------------------------------------------------------------------------
    Name: OS_TaskGetInfo

    Purpose: This function will pass back a pointer to structure that contains 
             all of the relevant info (creator, stack size, priority, name) about the 
             specified task. 

    Returns: OS_ERR_INVALID_ID if the ID passed to it is invalid
             OS_INVALID_POINTER if the task_prop pointer is NULL
             OS_SUCCESS if it copied all of the relevant info over
 
---------------------------------------------------------------------------------------*/
Result_T OS_TaskGetInfo (uint32 task_id, OS_task_prop_t *task_prop)  
{
    /* Check to see that the id given is valid */
    
    if (task_id >= OS_MAX_TASKS || OS_task_table[task_id].free == TRUE)
        return OS_ERR_INVALID_ID;

    if( task_prop == NULL)
        return OS_INVALID_POINTER;

    /* put the info into the stucture */


    pthread_mutex_lock(&OS_task_table_mut); 

    task_prop -> creator =    OS_task_table[task_id].creator;
    task_prop -> stack_size = OS_task_table[task_id].stack_size;
    task_prop -> priority =   OS_task_table[task_id].priority;
    task_prop -> OStask_id =  (uint32) OS_task_table[task_id].id;
    

    pthread_mutex_unlock(&OS_task_table_mut);

    
    return OS_SUCCESS;
    
} /* end OS_TaskGetInfo */

/****************************************************************************************
                                MESSAGE QUEUE API
****************************************************************************************/

/*---------------------------------------------------------------------------------------
   Name: OS_QueueCreate

   Purpose: Create a message queue which can be refered to by name or ID

   Returns: OS_INVALID_POINTER if a pointer passed in is NULL
            OS_ERR_NO_FREE_IDS if there are already the max queues created
            OS_ERROR if the OS create call fails
            OS_SUCCESS if success

   Notes: the flahs parameter is unused.
---------------------------------------------------------------------------------------*/
Result_T OS_QueueCreate (
        uint32 *queue_id, 
        uint32 queue_depth,
        uint32 data_size, 
        uint32 flags)
{
   int                     tmpSkt;
   int                     returnStat;
   struct sockaddr_in      servaddr;
   uint32                  possible_qid;

   UNUSED(flags);

    if ( queue_id == NULL )
        return OS_INVALID_POINTER;

    if( (queue_depth == 0) || (data_size == 0) )
    	return OS_QUEUE_INVALID_SIZE;

   /* Check Parameters */

    pthread_mutex_lock(&OS_queue_table_mut);    
    
    for(possible_qid = 0; possible_qid < OS_MAX_QUEUES; possible_qid++)
    {
        if (OS_queue_table[possible_qid].free == TRUE)
            break;
    }

        
    if( possible_qid >= OS_MAX_QUEUES || OS_queue_table[possible_qid].free != TRUE)
    {
        pthread_mutex_unlock(&OS_queue_table_mut);
        return OS_ERR_NO_FREE_IDS;
    }

    /* Set the possible task Id to not free so that
     * no other task can try to use it */

    OS_queue_table[possible_qid].free = FALSE;
    pthread_mutex_unlock(&OS_queue_table_mut);

    
    tmpSkt = socket(AF_INET, SOCK_DGRAM, 0);

   memset(&servaddr, 0, sizeof(servaddr));
   servaddr.sin_family      = AF_INET;
   servaddr.sin_port        = htons(OS_BASE_PORT + possible_qid);
   servaddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK); 

   /* 
   ** bind the input socket to a pipe
   ** port numbers are OS_BASE_PORT + queue_id
   */
   returnStat = bind(tmpSkt,(struct sockaddr *)&servaddr, sizeof(servaddr));
   
   if ( returnStat == -1 )
   {        
       pthread_mutex_lock(&OS_queue_table_mut);
        OS_queue_table[possible_qid].free = TRUE;
        pthread_mutex_unlock(&OS_queue_table_mut);
      printf("bind failed on OS_QueueCreate. errno = %d\n",errno);
      return OS_ERROR;
   }
   
   /*
   ** store socket handle
   */
   *queue_id = possible_qid;
   
    pthread_mutex_lock(&OS_queue_table_mut);    

   OS_queue_table[*queue_id].id = tmpSkt;
   OS_queue_table[*queue_id].free = FALSE;
   OS_queue_table[*queue_id].creator = OS_TaskGetId();

    pthread_mutex_unlock(&OS_queue_table_mut);

   return OS_SUCCESS;
    
}/* end OS_QueueCreate */


/*--------------------------------------------------------------------------------------
    Name: OS_QueueDelete

    Purpose: Deletes the specified message queue.

    Returns: OS_ERR_INVALID_ID if the id passed in does not exist
             OS_ERROR if the OS call to delete the queue fails 
             OS_SUCCESS if success

    Notes: If There are messages on the queue, they will be lost and any subsequent
           calls to QueueGet or QueuePut to this queue will result in errors
---------------------------------------------------------------------------------------*/


Result_T OS_QueueDelete (uint32 queue_id)
{
#if defined (CONFIG_OS_STATIC_RESOURCE_ALLOCATION)
    return OS_NOT_SUPPORTED;
#else
    /* Check to see if the queue_id given is valid */
    
    if (queue_id >= OS_MAX_QUEUES || OS_queue_table[queue_id].free == TRUE)
            return OS_ERR_INVALID_ID;

    /* Try to delete the queue */

    if(close(OS_queue_table[queue_id].id) !=0)   
        {
            return OS_ERROR;
        }
        
    /* 
     * Now that the queue is deleted, remove its "presence"
     * in OS_message_q_table and OS_message_q_name_table 
    */
        
    pthread_mutex_lock(&OS_queue_table_mut);    

    OS_queue_table[queue_id].free = TRUE;
    OS_queue_table[queue_id].creator = UNINITIALIZED;
    OS_queue_table[queue_id].id = UNINITIALIZED;

    pthread_mutex_unlock(&OS_queue_table_mut);

   
    return OS_SUCCESS;
#endif

} /* end OS_QueueDelete */

/*---------------------------------------------------------------------------------------
   Name: OS_QueueGet

   Purpose: Receive a message on a message queue.  Will pend or timeout on the receive.
   Returns: OS_ERR_INVALID_ID if the given ID does not exist
            OS_ERR_INVALID_POINTER if a pointer passed in is NULL
            OS_QUEUE_EMPTY if the Queue has no messages on it to be recieved
            OS_QUEUE_TIMEOUT if the timeout was OS_PEND and the time expired
            OS_QUEUE_INVALID_SIZE if the size copied from the queue was not correct
            OS_SUCCESS if success
---------------------------------------------------------------------------------------*/
Result_T OS_QueueGet (uint32 queue_id, void *data, uint32 size, uint32 *size_copied, int32 timeout)
{

   int sizeCopied;
   int flags;

    /* Check Parameters */
    if(queue_id >= OS_MAX_QUEUES || OS_queue_table[queue_id].free == TRUE)
        return OS_ERR_INVALID_ID;
    if( (data == NULL) || (size_copied == NULL) )
        return OS_INVALID_POINTER;
    if(size == 0)
    	return OS_QUEUE_INVALID_SIZE;
    
   /*
   ** Read the socket for data
   */
   if (timeout == OS_PEND) 
   {      
      fcntl(OS_queue_table[queue_id].id,F_SETFL,0);
      sizeCopied = recvfrom(OS_queue_table[queue_id].id, data, size, 0, NULL, NULL);
      if(sizeCopied != (int)size )
      {
         *size_copied = 0;
         return(OS_QUEUE_INVALID_SIZE);
      }
   }
   else if (timeout == OS_CHECK)
   {      
      flags = fcntl(OS_queue_table[queue_id].id, F_GETFL, 0);
      fcntl(OS_queue_table[queue_id].id,F_SETFL,flags|O_NONBLOCK);
      
      sizeCopied = recvfrom(OS_queue_table[queue_id].id, data, size, 0, NULL, NULL);

      fcntl(OS_queue_table[queue_id].id,F_SETFL,flags);
      
      if (sizeCopied == -1 && errno == EWOULDBLOCK )
      {
         *size_copied = 0;
         return OS_QUEUE_EMPTY;
      }
      else if(sizeCopied != (int)size )
      {
         *size_copied = 0;
         return(OS_QUEUE_INVALID_SIZE);
      }

   }
   else /* timeout */ 
   {
      int timeloop;
      
      flags = fcntl(OS_queue_table[queue_id].id, F_GETFL, 0);
      fcntl(OS_queue_table[queue_id].id,F_SETFL,flags|O_NONBLOCK);

      /*
      ** This "timeout" will check the socket for data every 100 milliseconds
      ** up until the timeout value. Although this works fine for a desktop environment,
      ** it should be written more efficiently for a flight system.
      ** The proper way will be to use select or poll with a timeout
      */
      for ( timeloop = timeout; timeloop > 0; timeloop = timeloop - 100 )
      {
         sizeCopied = recvfrom(OS_queue_table[queue_id].id, data, size, 0, NULL, NULL);

         if ( sizeCopied == (int)size )
         {
             *size_copied = sizeCopied;
             fcntl(OS_queue_table[queue_id].id,F_SETFL,flags);
             return OS_SUCCESS;
         
         }
         else if (sizeCopied == -1 && errno == EWOULDBLOCK )
         {
            /*
            ** Sleep for 100 milliseconds
            */
            usleep(100 * 1000);
         }
         else
         {
             *size_copied = 0;
             fcntl(OS_queue_table[queue_id].id,F_SETFL,flags);
             return OS_QUEUE_INVALID_SIZE;
         }
      }
      fcntl(OS_queue_table[queue_id].id,F_SETFL,flags);
      return(OS_QUEUE_TIMEOUT);

   } /* END timeout */

   /*
   ** Should never really get here.
   */
   return OS_SUCCESS;

} /* end OS_QueueGet */

/*---------------------------------------------------------------------------------------
   Name: OS_QueuePut

   Purpose: Put a message on a message queue.

   Returns: OS_ERR_INVALID_ID if the queue id passed in is not a valid queue
            OS_INVALID_POINTER if the data pointer is NULL
            OS_QUEUE_FULL if the queue cannot accept another message
            OS_ERROR if the OS call returns an error
            OS_SUCCESS if SUCCESS            
   
   Notes: The flags parameter is not used.  The message put is always configured to
            immediately return an error if the receiving message queue is full.
---------------------------------------------------------------------------------------*/


Result_T OS_QueuePut (uint32 queue_id, void *data, uint32 size, uint32 flags)
{

   struct sockaddr_in serva;
   LOCAL int socketFlags = 0;
   int bytesSent    = 0;
   int tempSkt      = 0;

   UNUSED(flags);

   /*
   ** Check Parameters 
   */
    if(queue_id >= OS_MAX_QUEUES || OS_queue_table[queue_id].free == TRUE)
        return OS_ERR_INVALID_ID;

    if (data == NULL)
        return OS_INVALID_POINTER;
    if( size == 0 )
    	return OS_QUEUE_INVALID_SIZE;
   /* 
   ** specify the IP addres and port number of destination
   */
   memset(&serva, 0, sizeof(serva));
   serva.sin_family      = AF_INET;
   serva.sin_port        = htons(OS_BASE_PORT + queue_id);
   serva.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    /*
    ** open a temporary socket to transfer the packet to MR
    */
    tempSkt = socket(AF_INET, SOCK_DGRAM, 0);

    /* 
    ** send the packet to the message router task (MR)
    */
    bytesSent = sendto(tempSkt,(char *)data, size, socketFlags,
                     (struct sockaddr *)&serva, sizeof(serva));
    if( bytesSent != (int)size )
    {
       return(OS_QUEUE_FULL);
    }

    /* 
    ** close socket
    */
    close(tempSkt);

   return OS_SUCCESS;
} /* end OS_QueuePut */


/*---------------------------------------------------------------------------------------
    Name: OS_QueueGetInfo

    Purpose: This function will pass back a pointer to structure that contains 
             all of the relevant info (name and creator) about the specified queue. 

    Returns: OS_INVALID_POINTER if queue_prop is NULL
             OS_ERR_INVALID_ID if the ID given is not  a valid queue
             OS_SUCCESS if the info was copied over correctly
---------------------------------------------------------------------------------------*/

Result_T OS_QueueGetInfo (uint32 queue_id, OS_queue_prop_t *queue_prop)  
{
    /* Check to see that the id given is valid */
    
    if (queue_prop == NULL)
        return OS_INVALID_POINTER;
    
    if (queue_id >= OS_MAX_QUEUES || OS_queue_table[queue_id].free == TRUE)
        return OS_ERR_INVALID_ID;

    /* put the info into the stucture */
    pthread_mutex_lock(&OS_queue_table_mut);    

    queue_prop -> creator =   OS_queue_table[queue_id].creator;

    pthread_mutex_unlock(&OS_queue_table_mut);


    return OS_SUCCESS;
    
} /* end OS_QueueGetInfo */

/****************************************************************************************
                                  SEMAPHORE API
****************************************************************************************/

/*---------------------------------------------------------------------------------------
   Name: OS_BinSemCreate

   Purpose: Creates a binary semaphore with initial value specified by
            sem_initial_value. sem_id will be 
            returned to the caller
            
   Returns: OS_INVALID_POINTER if sen name or sem_id are NULL
            OS_ERR_NO_FREE_IDS if all of the semaphore ids are taken
            OS_ERR_NAME_TAKEN if this is already the name of a binary semaphore
            OS_SEM_FAILURE if the OS call failed
            OS_SUCCESS if success
            

   Notes: options is an unused parameter 
---------------------------------------------------------------------------------------*/
Result_T OS_BinSemCreate (
        uint32 *sem_id, 
        uint32 sem_initial_value,
        uint32 options)
{
   
    uint32 possible_semid;
    char SemName [OS_MAX_API_NAME];

    UNUSED(options);

    if (sem_id == NULL )
    {
        return OS_INVALID_POINTER;
    }
    
    /* Check Parameters */

    pthread_mutex_lock(&OS_bin_sem_table_mut);  

    for (possible_semid = 0; possible_semid < OS_MAX_BIN_SEMAPHORES; possible_semid++)
    {
        if (OS_bin_sem_table[possible_semid].free == TRUE)    
            break;
    }
    

    if((possible_semid >= OS_MAX_BIN_SEMAPHORES) ||  
       (OS_bin_sem_table[possible_semid].free != TRUE))
    {    pthread_mutex_unlock(&OS_bin_sem_table_mut);
        return OS_ERR_NO_FREE_IDS;
    }
    
    /* Set the ID to be taken so another task doesn't try to grab it */
    OS_bin_sem_table[possible_semid].free = FALSE;

    pthread_mutex_unlock(&OS_bin_sem_table_mut);

   /*
   ** Create semaphore
   */

   errno = 0;
    
   sprintf(SemName,"OS_BinSemName%lu",possible_semid);

   /* unlink the name first, to make sure it doesn't exist anywhere*/
   sem_unlink(SemName);
   
   OS_bin_sem_table[possible_semid].id = sem_open(SemName,O_CREAT, 666, sem_initial_value);
   if(OS_bin_sem_table[possible_semid].id == (sem_t*) SEM_FAILED )
   {        
      /* Since the call failed, set the free flag back to true */
      pthread_mutex_lock(&OS_bin_sem_table_mut);        
      OS_bin_sem_table[possible_semid].free = TRUE;
      pthread_mutex_unlock(&OS_bin_sem_table_mut);
      printf("Error Creating semaphore in OS_BinSemCreate! errno = %d\n", errno); 
      return OS_ERROR;
   }

            
    *sem_id = possible_semid;

    pthread_mutex_lock(&OS_bin_sem_table_mut);  

    OS_bin_sem_table[*sem_id].free = FALSE;
    OS_bin_sem_table[*sem_id].creator = OS_TaskGetId();

    pthread_mutex_unlock(&OS_bin_sem_table_mut);

	return OS_SUCCESS;
}/* end OS_BinSemCreate */

/*--------------------------------------------------------------------------------------
     Name: OS_BinSemDelete

    Purpose: Deletes the specified Binary Semaphore.

    Returns: OS_ERR_INVALID_ID if the id passed in is not a valid binary semaphore
             OS_SEM_FAILURE the OS call failed
             OS_SUCCESS if success
    
    Notes: Since we can't delete a semaphore which is currently locked by some task 
           (as it may ber crucial to completing the task), the semaphore must be full to
           allow deletion.
---------------------------------------------------------------------------------------*/

Result_T OS_BinSemDelete (uint32 sem_id)
{
#if defined (CONFIG_OS_STATIC_RESOURCE_ALLOCATION)
    return OS_NOT_SUPPORTED;
#else
    /* Check to see if this sem_id is valid */
    if (sem_id >= OS_MAX_BIN_SEMAPHORES || OS_bin_sem_table[sem_id].free == TRUE)
        return OS_ERR_INVALID_ID;
    

       
    if (sem_close( (OS_bin_sem_table[sem_id].id)) != 0) /* 0 = success */ 
    {
        return OS_SEM_FAILURE;
    }

    
    pthread_mutex_lock(&OS_bin_sem_table_mut);  
   
    OS_bin_sem_table[sem_id].free = TRUE;
    OS_bin_sem_table[sem_id].creator = UNINITIALIZED;

    pthread_mutex_unlock(&OS_bin_sem_table_mut);
   
    return OS_SUCCESS;
#endif

}/* end OS_BinSemDelete */


/*---------------------------------------------------------------------------------------
    Name: OS_BinSemGive

    Purpose: The function  unlocks the semaphore referenced by sem_id by performing
             a semaphore unlock operation on that semaphore.If the semaphore value 
             resulting from this operation is positive, then no threads were blocked             waiting for the semaphore to become unlocked; the semaphore value is
             simply incremented for this semaphore.

    
    Returns: OS_SEM_FAILURE the semaphore was not previously  initialized or is not
             in the array of semaphores defined by the system
             OS_ERR_INVALID_ID if the id passed in is not a binary semaphore
             OS_SUCCESS if success
                
---------------------------------------------------------------------------------------*/

Result_T OS_BinSemGive ( uint32 sem_id )
{
    uint32 ret_val ;
    int32    ret;
   
    /* Check Parameters */

    if(sem_id >= OS_MAX_BIN_SEMAPHORES || OS_bin_sem_table[sem_id].free == TRUE)
        return OS_ERR_INVALID_ID;
    
    ret = sem_post(OS_bin_sem_table[sem_id].id);
    
    if ( ret != 0 )
    {
       ret_val = OS_SEM_FAILURE;
    }
    else
    {
       ret_val = OS_SUCCESS ;
    }
    
    return ret_val;
}/* end OS_BinSemGive */

/*---------------------------------------------------------------------------------------
    Name: OS_BinSemFlush

    Purpose: The function unblocks all tasks pending on the specified semaphore. However,
             this function does not change the state of the semaphore.

    
    Returns: OS_SEM_FAILURE the semaphore was not previously  initialized or is not
             in the array of semaphores defined by the system
             OS_ERR_INVALID_ID if the id passed in is not a binary semaphore
             OS_SUCCESS if success

---------------------------------------------------------------------------------------*/
Result_T OS_BinSemFlush (uint32 sem_id)
{
    uint32 ret_val ;
    int32    ret = 0;
    int i;

    /* Check Parameters */

    if(sem_id >= OS_MAX_BIN_SEMAPHORES || OS_bin_sem_table[sem_id].free == TRUE)
        return OS_ERR_INVALID_ID;
    
    for (i = 0; i < OS_MAX_TASKS; i++)
    {
         sem_post(OS_bin_sem_table[sem_id].id);
    }
    
    
    if ( ret != 0 )
    {
       ret_val = OS_SEM_FAILURE;
    }
    else
    {
       ret_val = OS_SUCCESS ;
    }
    
    return ret_val;

}/* end OS_BinSemFlush */

/*---------------------------------------------------------------------------------------
    Name:    OS_BinSemTake

    Purpose: The locks the semaphore referenced by sem_id by performing a 
             semaphore lock operation on that semaphore.If the semaphore value 
             is currently zero, then the calling thread shall not return from 
             the call until it either locks the semaphore or the call is 
             interrupted by a signal.

    Return:  OS_ERR_INVALID_ID the Id passed in is not a valid binary semaphore
             OS_SEM_FAILURE if the OS call failed
             OS_SUCCESS if success
             
----------------------------------------------------------------------------------------*/

Result_T OS_BinSemTake ( uint32 sem_id )
{
    uint32 ret_val ;
    int    ret;
    
    if(sem_id >= OS_MAX_BIN_SEMAPHORES  || OS_bin_sem_table[sem_id].free == TRUE)
        return OS_ERR_INVALID_ID;
    
    ret = sem_wait(OS_bin_sem_table[sem_id].id);
    
    if ( ret == 0 )
    {
       ret_val = OS_SUCCESS;
    }
    else
    {
       ret_val = OS_SEM_FAILURE;
    }
    
    return ret_val;
}/* end OS_BinSemTake */

Result_T OS_BinSemTryTake (uint32 sem_id)
{
    /* Check Parameters */

    if(sem_id >= OS_MAX_BIN_SEMAPHORES  || OS_bin_sem_table[sem_id].free == TRUE)
        return OS_ERR_INVALID_ID;

    /* Note to self: Check out sem wait in the manual */
    if ( sem_trywait( (OS_bin_sem_table[sem_id].id) ) != 0 )
        return OS_SEM_FAILURE;
    else
        return OS_SUCCESS;

}/* end OS_BinSemTryTake */

/*---------------------------------------------------------------------------------------
    Name: OS_BinSemTimedWait
    
    Purpose: The function locks the semaphore referenced by sem_id . However,
             if the semaphore cannot be locked without waiting for another process
             or thread to unlock the semaphore , this wait shall be terminated when 
             the specified timeout ,msecs, expires.

    Returns: OS_SEM_TIMEOUT if semaphore was not relinquished in time
             OS_SUCCESS if success
             OS_SEM_FAILURE the semaphore was not previously initialized or is not
             in the array of semaphores defined by the system
             OS_ERR_INVALID_ID if the ID passed in is not a valid semaphore ID

                            

NOTE: sem_timedwait is not implemented in OS X POSIX. The code that would be required is
left #if 0 'd out for when sem_timewait is implemented. For now, however, a rough 
apporoximation by sem_trywait is used instead.
----------------------------------------------------------------------------------------*/

Result_T OS_BinSemTimedWait ( uint32 sem_id, uint32 msecs )
{
    uint32           ret_val ;
    struct timespec  temp_timespec ;
    int timeloop;

    
    if( (sem_id >= OS_MAX_BIN_SEMAPHORES) || (OS_bin_sem_table[sem_id].free == TRUE) )
        return OS_ERR_INVALID_ID;   


    /*
    ** Compute an absolute time for the delay
    */
    ret_val = OS_CompAbsDelayedTime( msecs , &temp_timespec) ;
    
    /* try it this way */

    for (timeloop = msecs; timeloop >0; timeloop -= 100)
    {
    	int status = -1;
        if( (status = sem_trywait(OS_bin_sem_table[sem_id].id)) == -1 && errno == EAGAIN)
        {
            /* sleep for 100 msecs */
            usleep(100*1000);
        }
    
        else
        {   /* something besides the sem being taken made it fail */
//            if(sem_trywait(OS_bin_sem_table[sem_id].id) == -1)
        	/* 
        	 * The value of the semaphore is not left whether the call
        	 * successed and the semaphore was locke, so that another
        	 * sem_trywait would try to lock the sempahore twice...and
        	 * the routine would fail.
        	 */
        	if( status == -1 )
                return OS_SEM_FAILURE;
        
            /* took the sem successfully */
            else
                return OS_SUCCESS;
        }
    }
    return OS_SEM_TIMEOUT;
}

/*---------------------------------------------------------------------------------------
    Name: OS_BinSemGetInfo

    Purpose: This function will pass back a pointer to structure that contains 
             all of the relevant info( name and creator) about the specified binary
             semaphore.
             
    Returns: OS_ERR_INVALID_ID if the id passed in is not a valid semaphore 
             OS_INVALID_POINTER if the bin_prop pointer is null
             OS_SUCCESS if success
---------------------------------------------------------------------------------------*/

Result_T OS_BinSemGetInfo (uint32 sem_id, OS_bin_sem_prop_t *bin_prop)  
{
    /* Check to see that the id given is valid */
    
    if (sem_id >= OS_MAX_BIN_SEMAPHORES || OS_bin_sem_table[sem_id].free == TRUE)
        return OS_ERR_INVALID_ID;

    if (bin_prop == NULL)
        return OS_INVALID_POINTER;

    /* put the info into the stucture */
    pthread_mutex_lock(&OS_bin_sem_table_mut);  

    bin_prop ->creator =    OS_bin_sem_table[sem_id].creator;
    
    pthread_mutex_unlock(&OS_bin_sem_table_mut);


    return OS_SUCCESS;
    
} /* end OS_BinSemGetInfo */

/*---------------------------------------------------------------------------------------
   Name: OS_CountSemCreate

   Purpose: Creates a counting semaphore with initial value specified by
            sem_initial_value. sem_id will be 
            returned to the caller
            
   Returns: OS_INVALID_POINTER if sen name or sem_id are NULL
            OS_ERR_NO_FREE_IDS if all of the semaphore ids are taken
            OS_SEM_FAILURE if the OS call failed
            OS_SUCCESS if success
            

   Notes: options is an unused parameter 
---------------------------------------------------------------------------------------*/
Result_T OS_CountSemCreate (
        uint32 *sem_id, 
        uint32 sem_initial_value,
        uint32 options)
{
    uint32 possible_semid;
    char SemName [OS_MAX_API_NAME];

    UNUSED(options);
    

    if (sem_id == NULL )
        return OS_INVALID_POINTER;
    
    /* Check Parameters */

    pthread_mutex_lock(&OS_count_sem_table_mut);  

    for (possible_semid = 0; possible_semid < OS_MAX_COUNT_SEMAPHORES; possible_semid++)
    {
        if (OS_count_sem_table[possible_semid].free == TRUE)    
            break;
    }
    

    if((possible_semid >= OS_MAX_COUNT_SEMAPHORES) ||  
       (OS_count_sem_table[possible_semid].free != TRUE))
    {
        pthread_mutex_unlock(&OS_count_sem_table_mut);
        return OS_ERR_NO_FREE_IDS;
    }

    /* set the ID to taken so no other task can grab it */
    OS_count_sem_table[possible_semid].free = FALSE; 
    pthread_mutex_unlock(&OS_count_sem_table_mut);

   /*
   ** Create semaphore
   */

   errno = 0;
    
   sprintf(SemName,"OS_CountSemName%lu",possible_semid);

   /* unlink the name first, to make sure it doesn't exist anywhere*/
   sem_unlink(SemName);
   
   OS_count_sem_table[possible_semid].id = sem_open(SemName,O_CREAT, 666, sem_initial_value);
   if(OS_count_sem_table[possible_semid].id == (sem_t *)SEM_FAILED )
   {
       /* Since the call failed, set it the free flag back to true */
      pthread_mutex_lock(&OS_count_sem_table_mut); 
      OS_count_sem_table[possible_semid].free = TRUE;
      pthread_mutex_unlock(&OS_count_sem_table_mut); 
      printf("Error Creating semaphore in OS_CountSemCreate! errno = %d\n", errno);
      return OS_ERROR;
   }
            
    *sem_id = possible_semid;

    pthread_mutex_lock(&OS_count_sem_table_mut);  

    OS_count_sem_table[*sem_id].free = FALSE;
    OS_count_sem_table[*sem_id].creator = OS_TaskGetId();

    pthread_mutex_unlock(&OS_count_sem_table_mut);

   return OS_SUCCESS;
}/* end OS_CountSemCreate */

/*--------------------------------------------------------------------------------------
     Name: OS_CountSemDelete

    Purpose: Deletes the specified Countary Semaphore.

    Returns: OS_ERR_INVALID_ID if the id passed in is not a valid counting semaphore
             OS_SEM_FAILURE the OS call failed
             OS_SUCCESS if success
    
    Notes: Since we can't delete a semaphore which is currently locked by some task 
           (as it may ber crucial to completing the task), the semaphore must be full to
           allow deletion.
---------------------------------------------------------------------------------------*/

Result_T OS_CountSemDelete (uint32 sem_id)
{
#if defined (CONFIG_OS_STATIC_RESOURCE_ALLOCATION)
    return OS_NOT_SUPPORTED;
#else
    /* Check to see if this sem_id is valid */
    if (sem_id >= OS_MAX_COUNT_SEMAPHORES || OS_count_sem_table[sem_id].free == TRUE)
        return OS_ERR_INVALID_ID;
    

       
    if (sem_close( (OS_count_sem_table[sem_id].id)) != 0) /* 0 = success */ 
    {
        return OS_SEM_FAILURE;
    }

    /* Remove the Id from the table, and its name, so that it cannot be found again */
    
    pthread_mutex_lock(&OS_count_sem_table_mut);  
   
    OS_count_sem_table[sem_id].free = TRUE;
    OS_count_sem_table[sem_id].creator = UNINITIALIZED;

    pthread_mutex_unlock(&OS_count_sem_table_mut);
   
    return OS_SUCCESS;
#endif

}/* end OS_CountSemDelete */


/*---------------------------------------------------------------------------------------
    Name: OS_CountSemGive

    Purpose: The function  unlocks the semaphore referenced by sem_id by performing
             a semaphore unlock operation on that semaphore.If the semaphore value 
             resulting from this operation is positive, then no threads were blocked             waiting for the semaphore to become unlocked; the semaphore value is
             simply incremented for this semaphore.

    
    Returns: OS_SEM_FAILURE the semaphore was not previously  initialized or is not
             in the array of semaphores defined by the system
             OS_ERR_INVALID_ID if the id passed in is not a counting semaphore
             OS_SUCCESS if success
                
---------------------------------------------------------------------------------------*/

Result_T OS_CountSemGive ( uint32 sem_id )
{
    uint32 ret_val ;
    int32    ret;
    
    /* Check Parameters */

    if(sem_id >= OS_MAX_COUNT_SEMAPHORES || OS_count_sem_table[sem_id].free == TRUE)
        return OS_ERR_INVALID_ID;
    
    ret = sem_post(OS_count_sem_table[sem_id].id);
    
    if ( ret != 0 )
    {
       ret_val = OS_SEM_FAILURE;
    }
    else
    {
       ret_val = OS_SUCCESS ;
    }
    
    return ret_val;
}/* end OS_CountSemGive */

/*---------------------------------------------------------------------------------------
    Name:    OS_CountSemTake

    Purpose: The locks the semaphore referenced by sem_id by performing a 
             semaphore lock operation on that semaphore.If the semaphore value 
             is currently zero, then the calling thread shall not return from 
             the call until it either locks the semaphore or the call is 
             interrupted by a signal.

    Return:  OS_ERR_INVALID_ID the Id passed in is not a valid counting semaphore
             OS_SEM_FAILURE if the OS call failed
             OS_SUCCESS if success
             
----------------------------------------------------------------------------------------*/

Result_T OS_CountSemTake ( uint32 sem_id )
{
    uint32 ret_val ;
    int    ret;
    
    if(sem_id >= OS_MAX_COUNT_SEMAPHORES  || OS_count_sem_table[sem_id].free == TRUE)
        return OS_ERR_INVALID_ID;
    
    ret = sem_wait(OS_count_sem_table[sem_id].id);
    
    if ( ret == 0 )
    {
       ret_val = OS_SUCCESS;
    }
    else
    {
       ret_val = OS_SEM_FAILURE;
    }
    
    return ret_val;
}/* end OS_CountSemTake */

/*---------------------------------------------------------------------------------------
    Name: OS_CountSemTimedWait
    
    Purpose: The function locks the semaphore referenced by sem_id . However,
             if the semaphore cannot be locked without waiting for another process
             or thread to unlock the semaphore , this wait shall be terminated when 
             the specified timeout ,msecs, expires.

    Returns: OS_SEM_TIMEOUT if semaphore was not relinquished in time
             OS_SUCCESS if success
             OS_SEM_FAILURE the semaphore was not previously initialized or is not
             in the array of semaphores defined by the system
             OS_ERR_INVALID_ID if the ID passed in is not a valid semaphore ID

                            

NOTE: sem_timedwait is not implemented in OS X POSIX. The code that would be required is
left #if 0 'd out for when sem_timewait is implemented. For now, however, a rough 
apporoximation by sem_trywait is used instead.
----------------------------------------------------------------------------------------*/

Result_T OS_CountSemTimedWait ( uint32 sem_id, uint32 msecs )
{
    uint32           ret_val ;
    struct timespec  temp_timespec ;
    int timeloop;

    
    if( (sem_id >= OS_MAX_COUNT_SEMAPHORES) || (OS_count_sem_table[sem_id].free == TRUE) )
        return OS_ERR_INVALID_ID;   


    /*
    ** Compute an absolute time for the delay
    */
    ret_val = OS_CompAbsDelayedTime( msecs , &temp_timespec) ;
    
    /* try it this way */
    timeloop = (msecs == 0) ? 100 : msecs;
    for (timeloop; timeloop >0; timeloop -= 100)
    {
    	int status = -1;
        if ((status = sem_trywait(OS_count_sem_table[sem_id].id)) == -1 && errno == EAGAIN)
        {
            /* sleep for 100 msecs */
            usleep(100*1000);
        }
    
        else
        {   /* something besides the sem being taken made it fail */
        	/* 
        	 * The value of the semaphore is not left whether the call
        	 * successed and the semaphore was locke, so that another
        	 * sem_trywait would try to lock the sempahore twice...and
        	 * the routine would fail.
        	 */
//            if(sem_trywait(&(OS_count_sem_table[sem_id].id)) == -1)
        	if(status == -1)
                return OS_SEM_FAILURE;
        
            /* took the sem successfully */
            else
                return OS_SUCCESS;
        }
    }
    return OS_SEM_TIMEOUT;
}

/*---------------------------------------------------------------------------------------
    Name: OS_CountSemGetInfo

    Purpose: This function will pass back a pointer to structure that contains 
             all of the relevant info( name and creator) about the specified counting
             semaphore.
             
    Returns: OS_ERR_INVALID_ID if the id passed in is not a valid semaphore 
             OS_INVALID_POINTER if the count_prop pointer is null
             OS_SUCCESS if success
---------------------------------------------------------------------------------------*/

Result_T OS_CountSemGetInfo (uint32 sem_id, OS_count_sem_prop_t *count_prop)  
{
    /* Check to see that the id given is valid */
    
    if (sem_id >= OS_MAX_COUNT_SEMAPHORES || OS_count_sem_table[sem_id].free == TRUE)
        return OS_ERR_INVALID_ID;

    if (count_prop == NULL)
        return OS_INVALID_POINTER;

    /* put the info into the stucture */
    pthread_mutex_lock(&OS_count_sem_table_mut);  

    count_prop ->creator =    OS_count_sem_table[sem_id].creator;
    
    pthread_mutex_unlock(&OS_count_sem_table_mut);


    return OS_SUCCESS;
    
} /* end OS_CountSemGetInfo */
/****************************************************************************************
                                  MUTEX API
****************************************************************************************/

/*---------------------------------------------------------------------------------------
    Name: OS_MutSemCreate

    Purpose: Creates a mutex semaphore initially full.

    Returns: OS_INVALID_POINTER if sem_id is NULL
             OS_ERR_NO_FREE_IDS if there are no more free mutex Ids
             OS_SEM_FAILURE if the OS call failed
             OS_SUCCESS if success
    
    Notes: the options parameter is not used in this implementation

---------------------------------------------------------------------------------------*/
Result_T OS_MutSemCreate (
        uint32 *sem_id, 
        uint32 options)
{
    int                 return_code;
    int                 mutex_init_attr_status;
    pthread_mutexattr_t mutex_attr ;    
    uint32              possible_semid;

    UNUSED(options);

    /* Check Parameters */

    if (sem_id == NULL )
        return OS_INVALID_POINTER;
    
    pthread_mutex_lock(&OS_mut_sem_table_mut);  

    for (possible_semid = 0; possible_semid < OS_MAX_MUTEXES; possible_semid++)
    {
        if (OS_mut_sem_table[possible_semid].free == TRUE)    
            break;
    }

    if( (possible_semid == OS_MAX_MUTEXES) || 
        (OS_mut_sem_table[possible_semid].free != TRUE) )
    {
        pthread_mutex_unlock(&OS_mut_sem_table_mut);
        return OS_ERR_NO_FREE_IDS;
    }
   
    /* Set the free flag to false to make sure no other task grabs it */

    OS_mut_sem_table[possible_semid].free = FALSE;
    pthread_mutex_unlock(&OS_mut_sem_table_mut);

    /* 
    ** initialize the attribute with default values 
    */
    mutex_init_attr_status = pthread_mutexattr_init( &mutex_attr) ; 
    /* Linux does not support ? mutex_setprotocol_status = pthread_mutexattr_setprotocol(&mutex_attr,PTHREAD_PRIO_INHERIT) ; */

    /* 
    ** create the mutex 
    ** upon successful initialization, the state of the mutex becomes initialized and ulocked 
    */
    return_code =  pthread_mutex_init((pthread_mutex_t *) &OS_mut_sem_table[possible_semid].id,&mutex_attr); 
    if ( return_code != 0 )
    {
        /* Since the call failed, set free back to true */
        pthread_mutex_lock(&OS_mut_sem_table_mut);
        OS_mut_sem_table[possible_semid].free = TRUE;
        pthread_mutex_unlock(&OS_mut_sem_table_mut);
        
        printf("Error: Mutex could not be created. ID = %lu\n",possible_semid);
        return OS_ERROR;
    }
    else
    {
       /*
       ** Mark mutex as initialized
       */
/*     printf("Mutex created, mutex_id = %d \n" ,possible_semid) ;*/
       
    *sem_id = possible_semid;
    
    pthread_mutex_lock(&OS_mut_sem_table_mut);  

    OS_mut_sem_table[*sem_id].free = FALSE;
    OS_mut_sem_table[*sem_id].creator = OS_TaskGetId();
    

    pthread_mutex_unlock(&OS_mut_sem_table_mut);

       return OS_SUCCESS;
    }

}/* end OS_MutexSemCreate */


/*--------------------------------------------------------------------------------------
     Name: OS_MutSemDelete

    Purpose: Deletes the specified Mutex Semaphore.
    
    Returns: OS_ERR_INVALID_ID if the id passed in is not a valid mutex
             OS_SEM_FAILURE if the OS call failed
             OS_SUCCESS if success

    Notes: The mutex must be full to take it, so we have to check for fullness

---------------------------------------------------------------------------------------*/

Result_T OS_MutSemDelete (uint32 sem_id)
{
#if defined (CONFIG_OS_STATIC_RESOURCE_ALLOCATION)
    return OS_NOT_SUPPORTED;
#else
    int status=-1;
    /* Check to see if this sem_id is valid   */
    if (sem_id >= OS_MAX_MUTEXES || OS_mut_sem_table[sem_id].free == TRUE)
        return OS_ERR_INVALID_ID;

    status = pthread_mutex_destroy( &(OS_mut_sem_table[sem_id].id)); /* 0 = success */   
    
    if( status != 0)
        return OS_SEM_FAILURE;
    /* Delete its presence in the table */
   
    pthread_mutex_lock(&OS_mut_sem_table_mut);  

    OS_mut_sem_table[sem_id].free = TRUE;
    OS_mut_sem_table[sem_id].creator = UNINITIALIZED;
    
    pthread_mutex_unlock(&OS_mut_sem_table_mut);

    
    return OS_SUCCESS;

#endif

}/* end OS_MutSemDelete */

/*---------------------------------------------------------------------------------------
    Name: OS_MutSemGive

    Purpose: The function releases the mutex object referenced by sem_id.The 
             manner in which a mutex is released is dependent upon the mutex's type 
             attribute.  If there are threads blocked on the mutex object referenced by 
             mutex when this function is called, resulting in the mutex becoming 
             available, the scheduling policy shall determine which thread shall 
             acquire the mutex.

    Returns: OS_SUCCESS if success
             OS_SEM_FAILURE if the semaphore was not previously  initialized 
             OS_ERR_INVALID_ID if the id passed in is not a valid mutex

---------------------------------------------------------------------------------------*/

Result_T OS_MutSemGive ( uint32 sem_id )
{
    uint32 ret_val ;

    /* Check Parameters */

    if(sem_id >= OS_MAX_MUTEXES || OS_mut_sem_table[sem_id].free == TRUE)
        return OS_ERR_INVALID_ID;

    /*
    ** Unlock the mutex
    */
    if(pthread_mutex_unlock(&(OS_mut_sem_table[sem_id].id)))
    {
        ret_val = OS_SEM_FAILURE ;
    }
    else
    {
        ret_val = OS_SUCCESS ;
    }
    
    return ret_val;
} /* end OS_MutSemGive */


/*---------------------------------------------------------------------------------------
    Name: OS_MutSemTake

    Purpose: The mutex object referenced by sem_id shall be locked by calling this
             function. If the mutex is already locked, the calling thread shall
             block until the mutex becomes available. This operation shall return
             with the mutex object referenced by mutex in the locked state with the              calling thread as its owner.

    Returns: OS_SUCCESS if success
             OS_SEM_FAILURE if the semaphore was not previously initialized or is 
             not in the array of semaphores defined by the system
             OS_ERR_INVALID_ID the id passed in is not a valid mutex
---------------------------------------------------------------------------------------*/
Result_T OS_MutSemTake ( uint32 sem_id )
{
    int ret_val ;

    /* 
    ** Check Parameters
    */  
   if(sem_id >= OS_MAX_MUTEXES || OS_mut_sem_table[sem_id].free == TRUE)
        return OS_ERR_INVALID_ID;
 
    /*
    ** Lock the mutex
    */
    if( pthread_mutex_lock(&(OS_mut_sem_table[sem_id].id) ))
    {
        ret_val = OS_SEM_FAILURE ;
    }
    else
    {
        ret_val = OS_SUCCESS ;
    }
    
    return ret_val;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  OS_MutSemTryTake
 *  Description:  This function tries to catch a semaphore.
 *  Parameters:
 *      - sem_id:   Semaphore identifier
 *  Return:
 *      OS_SUCCESS when the semaphore is catched
 *      OS_ERR_INVALID_ID when the semaphore id is erroneous
 *      OS_SEM_FAILURE when there is an error trying the get the semaphore
 * =====================================================================================
 */
Result_T OS_MutSemTryTake (uint32 sem_id)
{
    /* Check Parameters */
    if(sem_id >= OS_MAX_BIN_SEMAPHORES  || OS_bin_sem_table[sem_id].free == TRUE)
        return OS_ERR_INVALID_ID;

    /* Note to self: Check out sem wait in the manual */
    if ( pthread_mutex_trylock(&(OS_mut_sem_table[sem_id].id)) != 0)
        return OS_SEM_FAILURE;
    else
        return OS_SUCCESS;
}

/*---------------------------------------------------------------------------------------
    Name: OS_MutnSemGetInfo

    Purpose: This function will pass back a pointer to structure that contains 
             all of the relevant info( name and creator) about the specified mutex
             semaphore.
             
    Returns: OS_ERR_INVALID_ID if the id passed in is not a valid semaphore 
             OS_INVALID_POINTER if the mut_prop pointer is null
             OS_SUCCESS if success
---------------------------------------------------------------------------------------*/

Result_T OS_MutSemGetInfo (uint32 sem_id, OS_mut_sem_prop_t *mut_prop)  
{
    /* Check to see that the id given is valid */
    
    if (sem_id >= OS_MAX_MUTEXES || OS_mut_sem_table[sem_id].free == TRUE)
        return OS_ERR_INVALID_ID;

    if (mut_prop == NULL)
        return OS_INVALID_POINTER;
    
    /* put the info into the stucture */    
    
    pthread_mutex_lock(&OS_mut_sem_table_mut);  

    mut_prop -> creator =   OS_mut_sem_table[sem_id].creator;

    pthread_mutex_unlock(&OS_mut_sem_table_mut);


    
    return OS_SUCCESS;
    
} /* end OS_BinSemGetInfo */


/****************************************************************************************
                                    INFO API
****************************************************************************************/

/*---------------------------------------------------------------------------------------
** Name: OS_IntAttachHandler
**
** Purpose:
**      The call associates a specified C routine to a specified interrupt   
**      number.Upon occurring of the InterruptNumber , the InerruptHandler 
**      routine will be called and passed the parameter. 
**
** Assumptions and Notes:
**
** Parameters:
**      InterruptNumber : The Interrupt Number that will cause the start of
**                        the ISR
**      InerruptHandler : The ISR associatd with this interrupt
**      parameter :The parameter that is passed to the ISR
**
** Global Inputs: None
**
** Global Outputs: None
**
**
** Return Values: 
**  OS_SUCCESSOS_INVALID_INT_NUM, OS_INVALID_POINTER
---------------------------------------------------------------------------------------*/

Result_T OS_IntAttachHandler( uint32 InterruptNumber,
            void * InterruptHandler , int32 parameter ) 
{
    UNUSED(InterruptNumber);
    UNUSED(InterruptHandler);
    UNUSED(parameter);

    return(OS_UNIMPLEMENTED) ;
}
                                                                    
  /*---------------------------------------------------------------------------------------
** Name: OS_IntUnlock
** Purpose:
**      Enable the interrupts. 
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
**      OS_SUCCESS
---------------------------------------------------------------------------------------*/
Result_T OS_IntUnlock (int32 IntLevel)
{
    UNUSED(IntLevel);

    return(OS_UNIMPLEMENTED) ;
}

/*---------------------------------------------------------------------------------------
** Name: OS_Intlock
** Purpose:
**      Disable the interrupts. 
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
**      OS_SUCCESS
---------------------------------------------------------------------------------------*/
Result_T OS_IntLock ( void ) 
{

    return(OS_UNIMPLEMENTED) ;
}
/*---------------------------------------------------------------------------------------
** Name: OS_IntEnable
** Purpose:
**      Enables interrupts through Level 
**
** Assumptions and Notes:
**
** Parameters:
**              Level - the interrupts to enable
**
** Global Inputs: None
**
** Global Outputs: None
**
**
** Return Values: 
**      OS_SUCCESS
---------------------------------------------------------------------------------------*/
Result_T OS_IntEnable(int32 Level)
{
    UNUSED(Level);
    return OS_UNIMPLEMENTED;

}

/*---------------------------------------------------------------------------------------
** Name: OS_IntDisable
** Purpose:
**      Disables interrupts through Level 
**
** Assumptions and Notes:
**
** Parameters:
**              Level - the interrupts to disable
**
** Global Inputs: None
**
** Global Outputs: None
**
**
** Return Values: 
**      OS_SUCCESS
---------------------------------------------------------------------------------------*/

Result_T OS_IntDisable(int32 Level)
{
    UNUSED(Level);
    return OS_UNIMPLEMENTED;
}


/*---------------------------------------------------------------------------------------
** Name: OS_Tick2Micros
**
** Purpose:
** This function returns the duration of a system tick in micro seconds.
**
** Assumptions and Notes:
**
** Parameters: None
**
** Global Inputs: None
**
** Global Outputs: None
**
**
**
** Return Values: duration of a system tick in micro seconds
---------------------------------------------------------------------------------------*/
uint32 OS_Tick2Micros (void)
{
   /*
   ** In an OSX or Linux system, what is the number of ticks per second ?
   ** and how do we find out ?
   */
    
   /*return(10000);*/
   return(1000000/(CLOCKS_PER_SEC));
}



/*---------------------------------------------------------------------------------------
** Name: OS_Milli2Ticks
**
** Purpose:
** This function accepts a time interval in milli_seconds, as an input and 
** returns the tick equivalent  for this time period. The tick value is 
**  rounded up.
**
** Assumptions and Notes:
**
** Parameters:
**      milli_seconds : the time interval ,in milli second , to be translated
**                      to ticks
**
** Global Inputs: None
**
** Global Outputs: None
**
**
** Return Values: the number of ticks rounded up.
---------------------------------------------------------------------------------------*/

uint32 OS_Milli2Ticks(uint32 milli_seconds)
{
    uint32 num_of_ticks,tick_duration_usec ;

    tick_duration_usec = OS_Tick2Micros() ;

    num_of_ticks = 
        ( (milli_seconds * 1000) + tick_duration_usec -1 ) / tick_duration_usec ;

    return(num_of_ticks) ; 

}
/*---------------------------------------------------------------------------------------
 * Name: OS_GetLocalTime
 * 
 * Purpose: This functions get the local time of the machine its on
 * ------------------------------------------------------------------------------------*/

Result_T OS_GetLocalTime(OS_time_t *time_struct)
{
/* the code that is #if 0'd out below is what we actually want to use. However 
 * clock_gettime doesn;t seem to be implemented in linux right now. The code that is
 * being used instead will return the clock of the system its in. Hopefully that will
 * be good enough for this port for now. */
    
    struct timeval tv;
    int Status;
    int32 ReturnCode;

   if (time_struct == NULL)
      return OS_INVALID_POINTER;

    Status = gettimeofday(&tv, NULL);
    time_struct-> seconds = tv.tv_sec;
    time_struct-> microsecs = tv.tv_usec;
    
    if (Status == 0)
    {
        ReturnCode = OS_SUCCESS;
    }
    else
    {
        ReturnCode = OS_ERROR;
    }
    
    return ReturnCode;

#if 0  
   int status;
   struct  timespec  time;

   if (time_struct == NULL)
      return OS_INVALID_POINTER;
   
    /*status = clock_gettime(CLOCK_REALTIME, &time);*/
    if (status != 0)
        return OS_ERROR;

   time_struct -> seconds = time.tv_sec;
   time_struct -> microsecs = time.tv_nsec / 1000;

    return OS_SUCCESS;

#endif


}/* end OS_GetLocalTime */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  OS_GetTicksSinceBoot()
 *  Description:  
 * =====================================================================================
 */
Result_T OS_GetTicksSinceBoot(OS_ticks_t *ticks)
{
    return OS_UNIMPLEMENTED;
}


/*---------------------------------------------------------------------------------------
 * Name: OS_SetLocalTime
 * 
 * Purpose: This functions set the local time of the machine its on
 * ------------------------------------------------------------------------------------*/

Result_T OS_SetLocalTime(OS_time_t *time_struct)
{
    struct timeval tv;
    int Status;
    int32 ReturnCode;

   if (time_struct == NULL)
      return OS_INVALID_POINTER;


    tv.tv_sec = time_struct -> seconds;
    tv.tv_usec = time_struct -> microsecs;

    Status = settimeofday(&tv, NULL);

    if (Status == 0)
    {
        ReturnCode = OS_SUCCESS;
    }
    else
    {
        ReturnCode = OS_ERROR;
    }
    
    return ReturnCode;

} /*end OS_SetLocalTime */
/*---------------------------------------------------------------------------------------
** Name: OS_SetMask
** Purpose:
**      Set the masking register to mask and unmask interrupts 
**
** Assumptions and Notes:
**
** Parameters:
**      MaskSetting :the value to be written into the mask register
**
** Global Inputs: None
**
** Global Outputs: None
**
**
** Return Values: 
**      OS_SUCCESS
---------------------------------------------------------------------------------------*/
Result_T OS_SetMask ( uint32 MaskSetting ) 
{
    UNUSED(MaskSetting);
    return(OS_UNIMPLEMENTED) ;
}

/*--------------------------------------------------------------------------------------
** Name: OS_GetMask
** Purpose:
**      Read and report the setting of the cpu mask register.
**
** Assumptions and Notes:
**
** Parameters:
**      MaskSettingPtr : pointer to a location where the function store the
**                               reading of the cpu mask register.
**
** Global Inputs: None
**
** Global Outputs: None
**
**
** Return Values: 
**      OS_SUCCESS
---------------------------------------------------------------------------------------*/
Result_T OS_GetMask ( uint32 * MaskSettingPtr ) 
{
    UNUSED(MaskSettingPtr);
    return(OS_UNIMPLEMENTED) ;
}

/*---------------------------------------------------------------------------------------
** Name: OS_CompAbsDelayedTime
**
** Purpose:
** This function accept time interval, milli_second, as an input and 
** computes the absolute time at which this time interval will expire. 
** The absolute time is programmed into a struct.
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
** Return Values: OS_SUCCESS, 
---------------------------------------------------------------------------------------*/
LOCAL uint32  OS_CompAbsDelayedTime( uint32 milli_second , struct timespec * tm)
{

    /* 
    ** get the current time 
    */
    /* Note: this is broken at the moment! */
    /*clock_gettime( CLOCK_REALTIME,  tm ); */
    
    /* Using gettimeofday instead of clock_gettime because clock_gettime is not
     * implemented in the linux posix */
    struct timeval tv;

    gettimeofday(&tv, NULL);
    tm->tv_sec = tv.tv_sec;
    tm->tv_nsec = tv.tv_usec * 1000;



    
    /* add the delay to the current time */
    tm->tv_sec  += (time_t) (milli_second / 1000) ;
    /* convert residue ( milli seconds)  to nano second */
    tm->tv_nsec +=  (milli_second % 1000) * 1000000 ;
    
    if(tm->tv_nsec > 999999999 )
    {
        tm->tv_nsec -= 1000000000 ;
        tm->tv_sec ++ ;
    }
    
    return(OS_SUCCESS) ;    
}
/* ---------------------------------------------------------------------------
 * Name: OS_printf 
 * 
 * Purpose: This function abstracts out the printf type statements. This is 
 *          useful for using OS- specific thats that will allow non-polled
 *          print statements for the real time systems. 
 ---------------------------------------------------------------------------*/
void OS_printf( const char *String, ...)
{
    va_list     ptr;
    char msg_buffer [OS_BUFFER_SIZE];
    
    va_start(ptr,String);
    vsnprintf(&msg_buffer[0], (size_t)OS_BUFFER_SIZE, String, ptr);
    va_end(ptr);
    
    msg_buffer[OS_BUFFER_SIZE -1] = '\0';
    printf("%s", &msg_buffer[0]);
    
}/* end OS_printf*/

/* ---------------------------------------------------------------------------
 * Name: OS_ThreadKillHandler
 * 
 * Purpose: This function allows for a task to be deleted when OS_TaskDelete
 * is called  
----------------------------------------------------------------------------*/

LOCAL void    OS_ThreadKillHandler(int sig)
{
    UNUSED(sig);
    pthread_exit(NULL);

}/*end OS_ThreadKillHandler */


