#ifndef DEMO_H_
#define DEMO_H_

#include <stdio.h>
#include <unistd.h>
#include <time.h>

#define DEMOMODE 1

#define STARTTIME 630720000
#define NSECDEMO 2592000

#ifdef DEMOMODE
#define DEMORET \
do {    \
    int diff = difftime(time(NULL), STARTTIME);  \
    if( (diff < 0) || (diff > NSECDEMO) )  \
    {   \
        printf("Demo License expired. Contact: Aitor.Viana.Sanchez@esa.int\n");     \
        exit(0);    \
    }   \
}while(0);

#else
#define DEMORET 
#endif

#endif
