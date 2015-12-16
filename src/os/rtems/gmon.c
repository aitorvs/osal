/**
 *  \file   gmon.c
 *  \brief  This file implements the gprof stub on the LEON2/3 over RTEMS.
 *
 *  The file implements the mcoung function responsible for recording the
 *  call-graph table.
 *  It also implements the functionality of the profil() function (see man(2)
 *  profil) by replacing the clock isr and wrapping the real Clock_isr() RTEMS
 *  function.
 *
 *  \author  Aitor Viana Sanchez (avs), Aitor.Viana.Sanchez@esa.int
 *
 *  \internal
 *    Created:  05/17/2010
 *   Revision:  $Id: gmon.c 1.4 05/17/2010 avs Exp $
 *   Compiler:  gcc/g++
 *    Company:  European Space Agency (ESA-ESTEC)
 *  Copyright:  Copyright (c) 2010, Aitor Viana Sanchez
 *
 *  This source code is released for free distribution under the terms of the
 *  GNU General Public License as published by the Free Software Foundation.
 * =====================================================================================
 */

/*-
 * Copyright (c) 1991, 1998 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. [rescinded 22 July 1999]
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <public/osal_config.h>

#if (CONFIG_OS_PROFIL_ENABLE == 1)
static char sccsid[] = "@(#)gmon.c	1.0 (E.S.A) 10/05/2010";

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

#if (CONFIG_RTEMS == 1 )
#   include <rtems.h>
#endif

#include "gmon.h"

void _mcount() __attribute__((weak, alias("mcount")));

/*  These variables are defined in the linkcmd linker script and point to the
 *  start and end of the text area
 */
extern unsigned int _endtext, text_start;
extern rtems_isr Clock_isr(rtems_vector_number v);

extern int gmon_initialize_link(void);
extern void gmon_write_link(char *buf, uint32_t dim);

/*
 *	froms is actually a bunch of unsigned shorts indexing tos
 */
static int		profiling = 3;
static long		tolimit = 0;
static unsigned long	s_textsize = 0;

static unsigned short	*froms;
static struct tostruct	*tos = 0;
static char		*s_lowpc = 0;
static char		*s_highpc = 0;

static int	ssiz;
static int	s_profil_sz = 0;
static char	*sbuf = 0;
static char *s_profil = 0;

#define	MSG "No space for profiling buffer(s)\n"

#define SAVE_L1( _pc_ )  \
    do{ \
        asm volatile( "mov %%l1, %0" : "=r" (_pc_) : "0" (_pc_) );    \
    }while(0)

static void moncontrol( int mode );
static rtems_isr profile_clock_isr(rtems_vector_number vector);

static void moncontrol( int mode )
{
    if (mode) {
        /* start */
        s_profil = sbuf + sizeof(struct phdr);
        s_profil_sz = ssiz - sizeof(struct phdr);
        profiling = 0;
    } else {
        /* stop */
        s_profil = 0;
        s_profil_sz = 0;
        profiling = 3;
    }
}

static void _mcleanup()
{
    int			fromindex;
    int			endfrom;
    char		*frompc;
    int			toindex;
    struct rawarc	rawarc;
    int ret = -1;

    moncontrol(0);
    set_vector( Clock_isr, 0x18, 1 );
    ret = gmon_initialize_link();
    if( ret == 0 )
    {
        gmon_write_link( (char*)sbuf , (uint32_t)ssiz );
    }

    fprintf( stderr , "[mcleanup] sbuf 0x%x ssiz %d\n" , (unsigned int)sbuf , (unsigned int)ssiz );
    endfrom = s_textsize / (HASHFRACTION * sizeof(*froms));
    for ( fromindex = 0 ; fromindex < endfrom ; fromindex++ ) {
        if ( froms[fromindex] == 0 ) {
            continue;
        }
        frompc = s_lowpc + (fromindex * HASHFRACTION * sizeof(*froms));
        for (toindex=froms[fromindex]; toindex!=0; toindex=tos[toindex].link) 
        {
            fprintf( stderr ,
                    "[mcleanup] frompc 0x%x selfpc 0x%x count %d\n" ,
                    (unsigned int)frompc , 
                    (unsigned int)tos[toindex].selfpc , 
                    (unsigned int)tos[toindex].count );
            rawarc.raw_frompc = (unsigned long) frompc;
            rawarc.raw_selfpc = (unsigned long) tos[toindex].selfpc;
            rawarc.raw_count = tos[toindex].count;
            if( ret == 0 )
            {
            gmon_write_link( (char*)&rawarc , (uint32_t)sizeof(rawarc));
            }
        }
    }

    free(sbuf);
    free(froms);
    free(tos);
}

void monstartup(char *lowpc, char *highpc)
{
    int			monsize;
    char		*buffer;

    printf("%s\n", sccsid);
    /*
     *	round lowpc and highpc to multiples of the density we're using
     *	so the rest of the scaling (here and in gprof) stays in ints.
     */
    lowpc = (char *)
        ROUNDDOWN((unsigned) lowpc, HISTFRACTION*sizeof(HISTCOUNTER));
    s_lowpc = lowpc;
    highpc = (char *)
        ROUNDUP((unsigned) highpc, HISTFRACTION*sizeof(HISTCOUNTER));
    s_highpc = highpc;
    s_textsize = highpc - lowpc;
    monsize = (s_textsize / HISTFRACTION) + sizeof(struct phdr);
    buffer = (char*)malloc( monsize );
    if ( buffer == (char *) -1 ) 
    {
        printf("%s\n", MSG);
        return;
    }
    bzero( buffer, monsize );

    froms = (unsigned short *) malloc( s_textsize / HASHFRACTION );
    if ( froms == (unsigned short *) -1 ) 
    {
        printf("%s\n", MSG);
        froms = 0;
        return;
    }
    bzero(froms, (s_textsize/HASHFRACTION) );

    tolimit = s_textsize * ARCDENSITY / 100;
    if ( tolimit < MINARCS ) {
        tolimit = MINARCS;
    } else if ( tolimit > 65534 ) {
        tolimit = 65534;
    }
    tos = (struct tostruct *) malloc( tolimit * sizeof( struct tostruct ) );
    if ( tos == (struct tostruct *) -1 ) 
    {
        printf("%s\n", MSG);
        froms = 0;
        tos = 0;
        return;
    }
    else
        bzero( tos, tolimit*sizeof(struct tostruct) );

    tos[0].link = 0;
    sbuf = buffer;
    ssiz = monsize;
    ( (struct phdr *) sbuf ) -> lpc = lowpc;
    ( (struct phdr *) sbuf ) -> hpc = highpc;
    ( (struct phdr *) sbuf ) -> ncnt = ssiz;
    monsize -= sizeof(struct phdr);
    if ( monsize <= 0 )
        return;

    printf("PC histrogram scale = %d\n", HISTFRACTION);

    /*  Register the new clock driver   */
    set_vector( profile_clock_isr, 0x18, 1 );
    atexit( _mcleanup );
    moncontrol(1);
}

void mcount()
{
    static char			*selfpc;
    static unsigned short		*frompcindex;
    static struct tostruct	*top;
    static struct tostruct	*prevtop;
    static long			toindex;
    static int first_call = 1;

    if( first_call )
    {
        monstartup((char*)&text_start, (char*)&_endtext);
        first_call = 0;
    }

    /*
     *	find the return address for mcount,
     *	and the return address for mcount's caller.
     */

    /* selfpc = pc pushed by mcount call.
       This identifies the function that was just entered.  */
    selfpc = (void *) __builtin_return_address (0);
    /* frompcindex = pc in preceding frame.
       This identifies the caller of the function just entered.  */
    frompcindex = (void *) __builtin_return_address (1);

    /*
     *	check that we are profiling
     *	and that we aren't recursively invoked.
     */
    if (profiling) return;
    profiling++;
    /*
     *	check that frompcindex is a reasonable pc value.
     *	for example:	signal catchers get called from the stack,
     *			not from text space.  too bad.
     */
    frompcindex = (unsigned short *) ((long) frompcindex - (long) s_lowpc);
    if ((unsigned long) frompcindex > s_textsize) {
        goto done;
    }
    frompcindex =
        &froms[((long) frompcindex) / (HASHFRACTION * sizeof(*froms))];
    toindex = *frompcindex;
    if (toindex == 0) {
        /*
         *	first time traversing this arc
         */
        toindex = ++tos[0].link;
        if (toindex >= tolimit) {
            goto overflow;
        }
        *frompcindex = toindex;
        top = &tos[toindex];
        top->selfpc = selfpc;
        top->count = 1;
        top->link = 0;
        goto done;
    }
    top = &tos[toindex];
    if (top->selfpc == selfpc) {
        /*
         *	arc at front of chain; usual case.
         */
        top->count++;
        goto done;
    }
    /*
     *	have to go looking down chain for it.
     *	top points to what we are looking at,
     *	prevtop points to previous top.
     *	we know it is not at the head of the chain.
     */
    for (; /* goto done */; ) {
        if (top->link == 0) {
            /*
             *	top is end of the chain and none of the chain
             *	had top->selfpc == selfpc.
             *	so we allocate a new tostruct
             *	and link it to the head of the chain.
             */
            toindex = ++tos[0].link;
            if (toindex >= tolimit) {
                goto overflow;
            }
            top = &tos[toindex];
            top->selfpc = selfpc;
            top->count = 1;
            top->link = *frompcindex;
            *frompcindex = toindex;
            goto done;
        }
        /*
         *	otherwise, check the next arc on the chain.
         */
        prevtop = top;
        top = &tos[top->link];
        if (top->selfpc == selfpc) {
            /*
             *	there it is.
             *	increment its count
             *	move it to the head of the chain.
             */
            top->count++;
            toindex = prevtop->link;
            prevtop->link = top->link;
            top->link = *frompcindex;
            *frompcindex = toindex;
            goto done;
        }

    }
done:
    profiling--;
    return;		/* normal return restores saved registers */

overflow:
    profiling++; /* halt further profiling */
    fprintf( stderr, "%s: tos overflow", __func__ );
    return;
}

extern rtems_status_code __real_Clock_isr();

static volatile uint32_t pc = 0;
rtems_isr __wrap_Clock_isr()
{
    if( s_profil && s_profil_sz )
    {
       /*   This identifies the function that was just entered.  */
//        SAVE_L1(pc);
        if( pc )
        {
            pc -= (int)s_lowpc;
            pc = (pc >> HISTFRACTION_LOG2);
            if( pc < s_profil_sz )  s_profil[pc]++;
        }
    }
    pc = 0;

    __real_Clock_isr();
    
}

static rtems_isr profile_clock_isr(rtems_vector_number vector)
{
    SAVE_L1(pc);
    Clock_isr(vector);
    
}

#endif

