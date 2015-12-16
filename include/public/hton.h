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

#ifndef __HTON_H__
#define __HTON_H__

#ifdef __cplusplus
extern "C" {
#endif

unsigned int htonl(unsigned int hostlong);
unsigned int ntohl(unsigned int hostlong);

unsigned short htons(unsigned short hostshort);
unsigned short ntohs(unsigned short hostshort);

#ifdef __cplusplus
}
#endif

#endif

