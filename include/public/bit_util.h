#ifndef _BIT__UTIL_H_
#define _BIT__UTIL_H_

#define BIT_AT( x, y) ((x)>>(y) & 0x00000001)
#define BIT_SET( x, y) ((x) |= (0x00000001 << (y)))
#define BIT_CLEAR( x, y) ((x) &= ~(0x00000001 << (y)))

// Utility macros

#define SET_FIELD_BYTE( buf, off, data)	(((buf)[(off)] = (uint8_t)(data)))

#define SET_FIELD_WORD( buf, off, data)	SET_FIELD_BYTE( buf, off, data); \
										SET_FIELD_BYTE( (buf)+1, off, (data)>>8);

#define SET_FIELD_DWORD( buf, off, data)	SET_FIELD_BYTE( buf, off, data); \
											SET_FIELD_BYTE( (buf)+1, off, (data)>>8); \
											SET_FIELD_BYTE( (buf)+2, off, (data)>>16); \
											SET_FIELD_BYTE( (buf)+3, off, (data)>>24);
											
#define GET_FIELD_BYTE( buf, off)		(buf)[(off)]

#define GET_FIELD_WORD( buf, off)	(	(uint32_t)GET_FIELD_BYTE( (buf), (off)) 		|		\
										(uint32_t)GET_FIELD_BYTE( (buf), (off)+1)<<8    )

#define GET_FIELD_DWORD( buf, off)	(	(uint32_t)GET_FIELD_BYTE( (buf), (off)) 		|		\
										(uint32_t)GET_FIELD_BYTE( (buf), (off+1))<<8  |		\
										(uint32_t)GET_FIELD_BYTE( (buf), (off+2))<<16 |		\
										(uint32_t)GET_FIELD_BYTE( (buf), (off+3))<<24   )

#endif /*_BIT__UTIL_H_*/
