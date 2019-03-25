#ifndef _MEMDEV2_H_
#define _MEMDEV2_H_

// define for access type
#define RD    0x01 // read
#define WR    0x02 // write
#define BYTE  0x10 // byte
#define HALF  0x20 // halt
#define WORD  0x40 // word

ulong memdev2(ulong address, ulong data, int access, int info);

#endif /* _MEMDEV2_H_ */
