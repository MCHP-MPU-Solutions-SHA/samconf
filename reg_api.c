#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "reg_api.h"
#include "memdev2.h"

int max_length(char **str, size_t step)
{
	int max = 0;
	int len;

	while (*str != NULL) {
		len = strlen(*str);
		if (len > max)
			max = len;
		str = (char **)((unsigned long)str + step);
	}

	return max;
}

char *access_str(uint access)
{
	static char str[4];

	str[0] = '\0';
	if (access & R)
		strcat(str, "R");
	if (access & W)
		strcat(str, "W");
	if (access & C)
		strcat(str, "C");

	return str;
}

#if 0
void dump(chip (*chips_p)[])
{
	int i, j, k;
	ip (*ips_p)[];
	reg (*regs_p)[];

	i = 0;
	while ((*chips_p)[i].name != NULL) {
		printf("chip:%s\n", (*chips_p)[i].name);

		if ((*chips_p)[i].ips != NULL) {
			ips_p = (*chips_p)[i].ips;
			j = 0;
			while ((*ips_p)[j].name != NULL) {
				printf("\t0x%08lx %s\n", \
					(*ips_p)[j].addr, (*ips_p)[j].name);

				if ((*ips_p)[j].regs != NULL) {
					regs_p = (*ips_p)[j].regs;
					k = 0;
					while ((*regs_p)[k].name != NULL) {
						printf("\t\t0x%lx\t%s\n", \
							(*regs_p)[k].addr, (*regs_p)[k].name);
						k++;
					}
				}
				j++;
			}
		}
		i++;
	}

}
#endif

int detect_chip(chip (*chips_p)[], series (*series_p)[], int *similar)
{
	int i, j;
	ulong cidr = 0, exid = 0;

	for (i = 0; (*series_p)[i].name != NULL; i++) {
		cidr = memdev2((*series_p)[i].cidr_addr, 0, R|WORD, 0);
		exid = memdev2((*series_p)[i].exid_addr, 0, R|WORD, 0);
		if ((cidr & ~VERSION_MASK) == (*series_p)[i].cidr_data) {
			for (j = 0; (*chips_p)[j].name != NULL; j++) {
				if ((*chips_p)[j].series == i) {
					// cidr and exid all matched, chip detected
					if (exid == (*chips_p)[j].exid_data) {
						return j;
					} else { // store chip id which in same series 
						if ((similar != NULL) && (*similar == -1))
							*similar = j;
					}
				}
			}
		}
	}

	return -1;
}

int exist_chip(chip (*chips_p)[], char *name)
{
	int i;

	for (i = 0; (*chips_p)[i].name != NULL; i++)
		if (strcasecmp((*chips_p)[i].name, name) == 0)
			return i;

	return -1;
}

int exist_module(chip *sam_chip, char *name)
{
	int i;

	for (i = 0; (*sam_chip->ips)[i].name != NULL; i++)
		if (strcasecmp((*sam_chip->ips)[i].name, name) == 0)
			return i;

	return -1;
}

int exist_register(ip *sam_ip, char *name)
{
	int i;

	for (i = 0; (*sam_ip->regs)[i].name != NULL; i++)
		if (strcasecmp((*sam_ip->regs)[i].name, name) == 0)
			return i;

	return -1;
}

void list_chips(chip (*chips_p)[], char *similar, int info)
{
	int i;
	int longest = max_length((char **)(*chips_p), sizeof(chip));

	for (i = 0; (*chips_p)[i].name != NULL; i++) {
		if (info == INFO_LIST_BASIC) { // TBD, follow ls
			if ((similar == NULL) || \
				(strncasecmp((*chips_p)[i].name, similar, strlen(similar)) == 0))
				printf("%s\n", (*chips_p)[i].name);
		} else if (info == INFO_LIST_MORE) {
			if ((similar == NULL) || \
				(strncasecmp((*chips_p)[i].name, similar, strlen(similar)) == 0))
				printf("%-*s0x%0*lX%s0x%0*lX\n", \
					longest + SPACING_SIZE, (*chips_p)[i].name, \
					(int)sizeof(ulong)*2, (*chips_p)[i].cidr_data, \
					SPACING_STR, \
					(int)sizeof(ulong)*2, (*chips_p)[i].exid_data);
		}
	}
}

void list_modules(chip *sam_chip, char *similar, int info)
{
	int i;
	int longest = max_length((char **)(*sam_chip->ips), sizeof(ip));

	for (i = 0; (*sam_chip->ips)[i].name != NULL; i++) {
		if (info == INFO_LIST_BASIC) { // TBD, follow ls
			if ((similar == NULL) || \
				(strncasecmp((*sam_chip->ips)[i].name, similar, strlen(similar)) == 0))
				printf("%s%s\n", SPACING_STR, (*sam_chip->ips)[i].name);
		} else if (info == INFO_LIST_MORE) {
			if ((similar == NULL) || \
				(strncasecmp((*sam_chip->ips)[i].name, similar, strlen(similar)) == 0))
				printf("%s%-*s0x%0*lX%s%s\n", SPACING_STR, longest + SPACING_SIZE, \
					(*sam_chip->ips)[i].name, (int)sizeof(ulong)*2, \
					(*sam_chip->ips)[i].addr, SPACING_STR, (*sam_chip->ips)[i].version);
		}
	}
}

void list_registers(ip *sam_ip, char *similar, info_id info)
{
	int i;
	int longest = max_length((char **)(*sam_ip->regs), sizeof(reg));

	for (i = 0; (*sam_ip->regs)[i].name != NULL; i++) {
		if (info == INFO_LIST_BASIC) { // TBD, follow ls
			if ((similar == NULL) || \
				(strncasecmp((*sam_ip->regs)[i].name, similar, strlen(similar)) == 0))
				printf("%s%s_%s\n", SPACING_STR2, sam_ip->name, \
					(*sam_ip->regs)[i].name);
		} else if (info == INFO_LIST_MORE) {
			if ((similar == NULL) || \
				(strncasecmp((*sam_ip->regs)[i].name, similar, strlen(similar)) == 0))
				printf("%s%s_%-*s0x%0*lX%s%s\n", SPACING_STR2, sam_ip->name, \
					longest + SPACING_SIZE, (*sam_ip->regs)[i].name, \
					(int)sizeof(ulong)*2, (*sam_ip->regs)[i].addr + sam_ip->addr, \
					SPACING_STR, access_str((*sam_ip->regs)[i].access));
		} else if ((info == INFO_DUMP_SAFE) || (info == INFO_DUMP_FORCE)) {
			if ((similar == NULL) || \
				(strncasecmp((*sam_ip->regs)[i].name, similar, strlen(similar)) == 0)) {
				if ((info == INFO_DUMP_FORCE) || !((*sam_ip->regs)[i].access & C))
					printf("%s%s_%-*s0x%0*lX%s0x%0*lX\n", SPACING_STR2, sam_ip->name, \
						longest + SPACING_SIZE, (*sam_ip->regs)[i].name, \
						(int)sizeof(ulong)*2, (*sam_ip->regs)[i].addr + sam_ip->addr, \
						SPACING_STR, \
						(int)sizeof(ulong)*2, \
						memdev2((*sam_ip->regs)[i].addr + sam_ip->addr, 0, R|WORD, 0));
			}
		}
	}
}

ulong search_register(ip (*ips_p)[], char *ip_name, char *reg_name)
{
	int i, j;

	if ((ip_name == NULL) || (reg_name == NULL))
		return 0;

	for (i = 0; (*ips_p)[i].name != NULL; i++)
		if (strcasecmp((*ips_p)[i].name, ip_name) == 0)
			for (j = 0; (*((*ips_p)[i].regs))[j].name != NULL; j++)
				if (strcasecmp((*((*ips_p)[i].regs))[j].name, reg_name) == 0)
					return (*ips_p)[i].addr + (*((*ips_p)[i].regs))[j].addr;

	/* Assuming that the peripheral register address cannot be zero */
	return 0;
}

int compare_register(chip *sam_chip, char *file)
{
	FILE *fp;
	int data_error=0;
	int mod_id, reg_id;
  ulong reg_addr, reg_data, data;
	char *mod_p, *ver_p, *reg_p;
	char buf[MAX_NAME_LEN+1];

	fp = fopen(file, "r");
	if (fp == NULL) {
		printf("ERROR: %s() fail to open file %s\n", __func__, file);
		return -1;
	}

	// First read chip name line
	if (fgets(buf, MAX_NAME_LEN, fp) == NULL) {
		printf("ERROR: %s() fail to read file %s\n", __func__, file);
    goto ERROR;
	}
	// Strip last '\n'
	if (buf[strlen(buf)-1] == '\n')
		buf[strlen(buf)-1] = '\0';
	// Check chip name line
	if (strcmp(sam_chip->name, buf) != 0) {
		printf("ERROR: %s() chip mismatch, current is %s\n", __func__, sam_chip->name);
		goto ERROR;
	}
	printf("%s\t --> matched\n", buf);

	// Second read module name line
	if (fgets(buf, MAX_NAME_LEN, fp) == NULL) {
    printf("ERROR: %s() fail to read file %s\n", __func__, file);
    goto ERROR;
  }
  // Remove the '\n' bebind
  if (buf[strlen(buf)-1] == '\n')
    buf[strlen(buf)-1] = '\0';
	// Get module and version
	mod_p = buf;
	while (*mod_p == ' ')
		mod_p++;
	ver_p = strchr(mod_p, '_');
	if (ver_p == NULL) {
		printf("ERROR: %s() wrong module name format: %s\n", __func__, buf);
		goto ERROR;
	}
	*ver_p = '\0';
	ver_p++;
  // Check module name line
	mod_id = exist_module(sam_chip, mod_p);
	if (mod_id < 0) {
		printf("ERROR: %s() module \"%s\" does not exist\n", __func__, mod_p);
		goto ERROR;
	}
	if (strcmp((*sam_chip->ips)[mod_id].version, ver_p) != 0) {
		printf("ERROR: %s() module version mismatched\n", __func__);
    goto ERROR;
	}
	ver_p[-1] = '_';
	printf("%s\t --> matched\n", buf);

	while (!feof(fp)) {
		if (fscanf(fp, "%s%lx%lx\n", buf, &reg_addr, &reg_data) != 3) {
			printf("ERROR: %s() scan register information\n", __func__);
			goto ERROR;
		}
		mod_p = buf;
		while (*mod_p == ' ')
    mod_p++;
		reg_p = strchr(mod_p, '_');
		if (reg_p == NULL) {
    	printf("ERROR: %s() wrong register name format: %s\n", __func__, buf);
    	goto ERROR;
  	}
		*reg_p = '\0';
		reg_p++;

		if (strcmp((*sam_chip->ips)[mod_id].name, mod_p) != 0) {
			reg_p[-1] = '_';
			printf("ERROR: %s() wrong register name %s\n", __func__, buf);
			goto ERROR;
		}
		reg_id = exist_register(&(*sam_chip->ips)[mod_id], reg_p);
		if (reg_id < 0) {
			printf("ERROR: %s() register \"%s_%s\" does not exist\n", __func__, \
      	mod_p, reg_p);
			goto ERROR;
		}

		if (reg_addr != (*sam_chip->ips)[mod_id].addr + (*((*sam_chip->ips)[mod_id].regs))[reg_id].addr) {
			printf("ERROR: %s() wrong register address %s_%s\n", __func__, \
        mod_p, reg_p);
			goto ERROR;
		}

		data = memdev2(reg_addr, 0, R|WORD, 0);
		if (reg_data != data) {
			printf("    %s_%s\t --> diff  in chip:0x%0*lX  in file:0x%0*lX\n", mod_p, reg_p, \
				(int)sizeof(ulong)*2, data, (int)sizeof(ulong)*2, reg_data);
			data_error = 1;
		}
	}

	if (!data_error)
		printf("    Registers\t --> all data matched\n");

	fclose(fp);
	return 0;

ERROR:
	return -1;
}

