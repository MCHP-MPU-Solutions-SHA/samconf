#include <stdio.h>
#include "reg_api.h"

/*
 * Following modules' register description could not be found in xml files
 * Here we generate register description for these modules manually
 */

/* Special Function Registers */
reg SFR_11066H[]=
{
  {"DDRCFG", 0x00000004, RW},
  {"OHCIICR", 0x00000010, RW},
  {"OHCIISR", 0x00000014, RW},
  {"SECURE", 0x00000028, RW},
  {"UTMICKTRIM", 0x00000030, RW},
	{"UTMIHSTRIM", 0x00000034, RW},
	{"UTMIFSTRIM", 0x00000038, RW},
	{"UTMISWAP", 0x0000003C, RW},
	{"CAN", 0x00000048, RW},
	{"SN0", 0x0000004C, R},
	{"SN1", 0x00000050, R},
	{"AICREDIR", 0x00000054, RW},
	{"L2CC_HRAMC", 0x00000058, RW},
	{"I2SCLKSEL", 0x00000090, RW},
	{"QSPICLK_REG", 0x00000094, RW},
  {NULL}
};

/* Software Modem Device */
reg SMD_11027A[]=
{
  {"DRIVE", 0x0000000C, RW},
  {NULL}
};

