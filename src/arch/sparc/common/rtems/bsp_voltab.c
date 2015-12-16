/**
 *  \file   bsp_voltab.c
 *  \brief  This file includes the mount table information to implement the
 *  filesystem capabilities over RTEMS
 *
 *  Detailed description starts here.
 *
 *  \author  Nicholas Yanchik / GSFC code 582
 *  \author  Aitor Viana Sanchez (avs), Aitor.Viana.Sanchez@esa.int
 *
 *  \internal
 *    Created:  28/01/08
 *   Revision:  $Id: doxygen.templates.example,v 1.4 2007/08/02 14:35:24 mehner Exp $
 *   Compiler:  gcc/g++
 *    Company:  European Space Agency (ESA-ESTEC)
 *  Copyright:  Copyright (c) 2008, Aitor Viana Sanchez
 *
 *  This source code is released for free distribution under the terms of the
 *  GNU General Public License as published by the Free Software Foundation.
 * =====================================================================================
 */


/*-----------------------------------------------------------------------------
 *  Changelog:
 *  28/01/08 18:47:06
 *  The rtems_ramdisk_configuration_size must be declared as size_t type when
 *  using the GCC version greater than the 3.0.
 *  Also the swapout_task_priority variable must be defined.
 *-----------------------------------------------------------------------------*/
/****************************************************************************************
                                    INCLUDE FILES
****************************************************************************************/
#if 0
#include "osal/osapi.h"

#if defined(CONFIG_OS_ENABLE_FILESYSTEM)

#include <rtems/score/cpuopts.h>

#ifndef RTEMS_VERSION_LATER_THAN
#define RTEMS_VERSION_LATER_THAN(ma,mi,re) \
	(    __RTEMS_MAJOR__  > (ma)	\
	 || (__RTEMS_MAJOR__ == (ma) && __RTEMS_MINOR__  > (mi))	\
	 || (__RTEMS_MAJOR__ == (ma) && __RTEMS_MINOR__ == (mi) && __RTEMS_REVISION__ > (re)) \
    )
#endif
#ifndef RTEMS_VERSION_ATLEAST
#define RTEMS_VERSION_ATLEAST(ma,mi,re) \
	(    __RTEMS_MAJOR__  > (ma)	\
	|| (__RTEMS_MAJOR__ == (ma) && __RTEMS_MINOR__  > (mi))	\
	|| (__RTEMS_MAJOR__ == (ma) && __RTEMS_MINOR__ == (mi) && __RTEMS_REVISION__ >= (re)) \
	)
#endif

#define MB                      0x00100000
#define FS_OFFSET               (32 * MB)

#define FS_BLOCK_SIZE           512
#define FS_START_ADDRESS        (0x40000000 + FS_OFFSET)
#define FS_SIZE_MB              (10*MB)                 
#define FS_N_BLOCKS             (FS_SIZE_MB/FS_BLOCK_SIZE)

#define FS_START_BUF_ADDRESS    (FS_START_BUF_ADDRESS + FS_SIZE_MB)
#define FS_BUF_SIZE             (1*MB)
#define FS_BUF_N_BLOCKS         (FS_BUF_SIZE/FS_BLOCK_SIZE)

#include <rtems/bdbuf.h>
#include <rtems/ramdisk.h>
#include <rtems/fsmount.h>

#if (__GNUC__ > 3)
#include <rtems/dosfs.h>
#else
#include <dosfs.h>
#endif


/*-----------------------------------------------------------------------------
 *  Volume table structure.
 *
 *  The Free field must be set to FALSE if we want to use the filesystem
 *  involved.
 *-----------------------------------------------------------------------------*/
OS_VolumeInfo_t OS_VolumeTable [NUM_TABLE_ENTRIES] =
{
    /* Dev Name  Phys Dev  Vol Type        Volatile?  Free?     IsMounted? Volname  MountPt BlockSz */
//    {"/dev/ramdisk0",   "/", FS_BASED,        TRUE,      TRUE,     FALSE,     " ",      " ",     0        },
    {"unused",   "unused", FS_BASED,        TRUE,      TRUE,     FALSE,     " ",      " ",     0        },
    {"unused",   "unused", FS_BASED,        TRUE,      TRUE,     FALSE,     " ",      " ",     0        },
    {"unused",   "unused", FS_BASED,        TRUE,      TRUE,     FALSE,     " ",      " ",     0        },
    {"unused",   "unused", FS_BASED,        TRUE,      TRUE,     FALSE,     " ",      " ",     0        }
};


/*-----------------------------------------------------------------------------
 *  RTEMS file system information
 *-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
 *  This structure is the RTEMS mount table information. This info. is related
 *  with the OS_VolumeTable to get the device name and the mount point.
 *-----------------------------------------------------------------------------*/
fstab_t OS_MountTable[NUM_TABLE_ENTRIES] = 
{
    {OS_VolumeTable[0].DeviceName, OS_VolumeTable[0].MountPoint, &msdos_ops, RTEMS_FILESYSTEM_READ_WRITE, FSMOUNT_MNT_OK | FSMOUNT_MNTPNT_CRTERR | FSMOUNT_MNT_FAILED, 0 },
    {OS_VolumeTable[1].DeviceName, OS_VolumeTable[1].MountPoint, &msdos_ops, RTEMS_FILESYSTEM_READ_WRITE, FSMOUNT_MNT_OK | FSMOUNT_MNTPNT_CRTERR | FSMOUNT_MNT_FAILED, 0 },
    {OS_VolumeTable[2].DeviceName, OS_VolumeTable[2].MountPoint, &msdos_ops, RTEMS_FILESYSTEM_READ_WRITE, FSMOUNT_MNT_OK | FSMOUNT_MNTPNT_CRTERR | FSMOUNT_MNT_FAILED, 0 },
    {OS_VolumeTable[3].DeviceName, OS_VolumeTable[3].MountPoint, &msdos_ops, RTEMS_FILESYSTEM_READ_WRITE, FSMOUNT_MNT_OK | FSMOUNT_MNTPNT_CRTERR | FSMOUNT_MNT_FAILED, 0 }
};

rtems_bdbuf_config rtems_bdbuf_configuration[] = {
    //  BlockSize BlockNum    Address
    {FS_BLOCK_SIZE,         FS_BUF_N_BLOCKS,          (char*)0x40300000}
};

rtems_ramdisk_config rtems_ramdisk_configuration[NUM_TABLE_ENTRIES] = {
    {
        block_size: FS_BLOCK_SIZE,
        block_num: FS_N_BLOCKS,
        location: (void*)FS_START_ADDRESS
    },
    {0,         0,          (void*)0x0},
    {0,         0,          (void*)0x0},
    {0,         0,          (void*)0x0},
};

#if ( RTEMS_VERSION_LATER_THAN(4, 6, 5) )
rtems_task_priority swapout_task_priority = 15;
size_t rtems_ramdisk_configuration_size = 
#else
int rtems_ramdisk_configuration_size = 
#endif
    ( sizeof(rtems_ramdisk_configuration)/sizeof(rtems_ramdisk_configuration[0])  );
int rtems_bdbuf_configuration_size = 
    ( sizeof(rtems_bdbuf_configuration)/sizeof(rtems_bdbuf_configuration[0]) );

#endif
#endif

