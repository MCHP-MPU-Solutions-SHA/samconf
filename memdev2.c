#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>

#define FATAL do { fprintf(stderr, "Error at line %d, file %s (%d) [%s]\n", \
  __LINE__, __FILE__, errno, strerror(errno)); exit(1); } while(0)
 
// define for access type
#define RD    0x01 // read
#define WR    0x02 // write
#define BYTE  0x10 // byte
#define HALF  0x20 // halt
#define WORD  0x40 // word

#define MAP_SIZE 4096UL
#define MAP_MASK (MAP_SIZE - 1)

ulong memdev2(ulong address, ulong data, int access, int info)
{
	int fd;
	void *map_base, *virt_addr;
	ulong result = 0;

	if ((fd = open("/dev/mem", O_RDWR | O_SYNC)) == -1) FATAL;
	if (info) {
		printf("/dev/mem opened.\n"); 
		fflush(stdout);
	}

	/* Map one page */
	map_base = mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, address & ~MAP_MASK);
	if (map_base == (void *) -1) FATAL;
	if (info) {
		printf("Memory mapped at address %p.\n", map_base); 
		fflush(stdout);
	}

	virt_addr = map_base + (address & MAP_MASK);
	if (access & WR) {
		switch (access & 0xF0) {
			case BYTE:
				*((unsigned char *) virt_addr) = data;
				result = *((unsigned char *) virt_addr);
				break;
			case HALF:
				*((unsigned short *) virt_addr) = data;
				result = *((unsigned short *) virt_addr);
				break;
			case WORD:
			default:
				*((unsigned long *) virt_addr) = data;
				result = *((unsigned long *) virt_addr);
				break;
		}
		if (info) {
			printf("Written 0x%lX; readback 0x%lX\n", data, result); 
			fflush(stdout);
		}
	} else { // default read
		switch (access & 0xF0) {
			case BYTE:
				result = *((unsigned char *) virt_addr);
				break;
			case HALF:
				result = *((unsigned short *) virt_addr);
				break;
			case WORD:
			default:
				result = *((unsigned long *) virt_addr);
				break;
		}
		if (info) {
			printf("Value at address 0x%lX (%p): 0x%lX\n", address, virt_addr, result); 
			fflush(stdout);
		}
	}

	if (munmap(map_base, MAP_SIZE) == -1) FATAL;
		close(fd);

	return result;
}

