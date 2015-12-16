/**
 *  \file   gmon_serial.c
 *  \brief  This file implements the services needed for dumping the profiling
 *  data using a serial link.
 *
 *  \author  Aitor Viana Sanchez (avs), Aitor.Viana.Sanchez@esa.int
 *
 *  \internal
 *    Created:  05/20/2010
 *   Revision:  $Id: gmon_serial.c 1.4 05/20/2010 avs Exp $
 *   Compiler:  gcc/g++
 *    Company:  European Space Agency (ESA-ESTEC)
 *  Copyright:  Copyright (c) 2010, Aitor Viana Sanchez
 *
 *  This source code is released for free distribution under the terms of the
 *  GNU General Public License as published by the Free Software Foundation.
 * =====================================================================================
 */

#include <public/osal_config.h>

#if (CONFIG_OS_PROFILE_OVER_SERIAL == 1)

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <errno.h>
#include <sys/systm.h>

#define SERIAL_VERBOSE_MODE   1
#define BAUDRATE B57600

/******************************************************
 *         Private global variables declaration
 ******************************************************/

/**
 * the serial connection file descriptor
 **/
static int serialFD = -1;



/******************************************************
 *            Function implementation
 ******************************************************/

int gmon_initialize_serial()
{
  struct termios oldtio, newtio;
  serialFD = open( "/dev/console_b" , O_RDWR | O_NOCTTY);
  if( serialFD < 0 )
  {
#if(SERIAL_VERBOSE_MODE == 1)
    printk("can't open new device! error = %d" , serialFD);
#endif
    return -1;
  }
  
  tcgetattr(serialFD , &oldtio);
  bzero(&newtio, sizeof(newtio));
  
  newtio.c_cflag = BAUDRATE | CRTSCTS | CS8 | CLOCAL | CREAD;
  
  newtio.c_iflag = IGNPAR | ICRNL;
  
  newtio.c_oflag  = 0;
  newtio.c_lflag  = 0;
  
  newtio.c_cc[VTIME] = 0;
  newtio.c_cc[VMIN]  = 1;
  
  tcflush(serialFD , TCIFLUSH);
  tcsetattr(serialFD , TCSANOW , &newtio);
  
  return 0;
}

void gmon_write_serial(uint8_t vector[] , unsigned int dim)
{
  write(serialFD , vector , dim);
}

#endif /* TIMELINE_USE_SERIAL_CABLE */
