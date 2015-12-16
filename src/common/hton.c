/********************************************************************
 * << demoSAT demostrator for RASTA >>
 * 
 * Developed by:
 * 
 * Aitor Viana Sanchez
 * ESA-ESTEC
 * Noordwijk, The Netherlands
 * Aitor.Viana.Sanchez@ESA.int
 *******************************************************************/

#include <public/hton.h>

static const int i = 1;
#define is_bigendian() ( (*(char*)&i) == 0 )

unsigned int htonl(unsigned int hostlong) 
{
	if(is_bigendian())
	{
		return hostlong;	
	}
	else
	{
		return (hostlong>>24) | ((hostlong&0xff0000)>>8) |
			  ((hostlong&0xff00)<<8) | (hostlong<<24);	
	}
}

unsigned short htons(unsigned short hostshort) 
{
	if(is_bigendian())
	{
		return hostshort;	
	}
	else
	{
		return ((hostshort>>8)&0xff) | (hostshort<<8);		
	}
}

unsigned short ntohs(unsigned short hostshort) __attribute__((weak,alias("htons")));
unsigned int ntohl(unsigned int hostlong) __attribute__((weak,alias("htonl")));

