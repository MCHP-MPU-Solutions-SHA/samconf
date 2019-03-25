#ifndef _REG_API_H_
#define _REG_API_H_

#define R  0x01 // Read 
#define W  0x02 // Write
#define C  0x04 // Clear on read
#define RW R|W

typedef enum {
	INFO_NONE=-1,
	INFO_LIST_BASIC=0,
	INFO_LIST_MORE,
	INFO_DUMP_SAFE,
	INFO_DUMP_FORCE,
} info_id;

#define VERSION_MASK	0x1F // chip ID version mask

#define SPACING_SIZE 2
#define SPACING_STR  "  "
#define SPACING_STR2 "    "

#define MAX_NAME_LEN	20

typedef unsigned char  uchar;
typedef unsigned short ushort;
typedef unsigned int   uint;
typedef unsigned long  ulong;

typedef struct {
	char *name;
	uint mask;
	uint attr;
} bit;

typedef struct {
	char  *name;
	ulong addr;
	uint  access;
	bit   (*bits)[];
} reg;

typedef struct {
	char  *name;
	ulong addr;
	char  *version;
	reg   (*regs)[];
} ip;

typedef struct {
	char  *name;
	int   series;
	ulong cidr_data;
	ulong exid_data;
	ip    (*ips)[];
} chip;

typedef struct {
	char *name;
	char *arch;
  char *family;
	ulong cidr_addr;
	ulong exid_addr;
	ulong cidr_data;
	ulong cidr_mask;
} series;

extern series sam_series[];
extern chip sam_chips[];

int detect_chip(chip (*chips_p)[], series (*series_p)[], int *similar);
int exist_chip(chip (*chips_p)[], char *name);
int exist_module(chip *sam_chip, char *name);
int exist_reigster(ip *sam_ip, char *name);
void list_chips(chip (*chips_p)[], char *similar, int info);
void list_modules(chip *sam_chip, char *similar, int info);
void list_registers(ip *sam_ip, char *similar, int info);
ulong search_register(ip (*ips_p)[], char *ip_name, char *reg_name);
int compare_register(chip *sam_chip, char *file);

#endif /* _REG_API_H_ */
