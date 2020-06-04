#ifndef MINTOOL_H
#define MINTOOL_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include "debug.h"

typedef struct args{
  int v_flag;
  int p_flag;
  int s_flag;
  int pt;
  int spt;
  int has_flags;
  char *image;
  char *filepath;
  char *dest;
}args;

/*Mask Bits*/
#define FILE_TYPE_MASK 0170000
#define REGULAR_FILE_MASK 0100000
#define DIRECTORY_MASK 0040000
#define OWNER_READ_MASK 0000400
#define OWNER_WRITE_MASK 0000200
#define OWNER_EXECUTE_MASK 0000100
#define GROUP_READ_MASK 0000040
#define GROUP_WRITE_MASK 0000020
#define GROUP_EXECUTE_MASK 0000010
#define OTHER_READ_MASK 0000004
#define OTHER_WRITE_MASK 0000002
#define OTHER_EXECUTE_MASK 0000001

#define PTABLE_ADDR 0x1BE
#define SBLOCK_ADDR 1024
#define MINIX_PTYPE 0x81
#define SIG_510     0x55
#define SIG_511     0xAA
#define MAGIC       0x4D5A
#define MAGIC_R     0x5A4D
#define SECTOR_SZ   512
#define INODE_SZ    64
#define DIRENT_SZ   64
#define DECIMAL     10
#define UNSPEC      -1
#define PTABLE_SZ   4

/* partition struct */
typedef struct partent{
  uint8_t bootind;
  uint8_t start_head;
  uint8_t start_sec;
  uint8_t start_cyl;
  uint8_t type; /*if not 81, stop*/
  uint8_t end_head;
  uint8_t end_sec;
  uint8_t end_cyl;
  uint32_t lFirst;
  uint32_t size;
}partent;

/* superblock struct */
typedef struct sublock{
  uint32_t ninodes;
  uint16_t pad1;
  int16_t i_blocks;
  int16_t z_blocks;
  uint16_t firstdata;
  int16_t log_zone_size;
  int16_t pad2;
  uint32_t max_file;
  uint32_t zones;
  int16_t magic;
  int16_t pad3;
  uint16_t blocksize;
  uint8_t subversion;
}sublock;

#define DIRECT_ZONES  7
#define REGULAR_FILE  0100000
#define DIRECTORY     040000

typedef struct inode{
  uint16_t mode;
  uint16_t links;
  uint16_t uid;
  uint16_t gid;
  uint32_t size;
  int32_t atime;
  int32_t mtime;
  int32_t ctime;
  uint32_t zone[DIRECT_ZONES];
  uint32_t indirect;
  uint32_t two_indirect;
  uint32_t unused;
}inode;

/* directory entry */
typedef struct dirent{
  uint32_t inode;
  unsigned char name[60];
}dirent;

/* swap endianness */
int32_t swend32(int32_t i);
int16_t swend16(int16_t i);

/* calculate zone size */
int32_t zsize(sublock sblk);

/* get partition table */
void getPtable(FILE *img, partent *pt, int ptStart);

/* copy inode with name nextFile into inod */
void getNextInode(FILE *img, inode *inod, char *nextFile, 
  int32_t ptLoc, int32_t zSize);

/* get superblock */
void getSublock(FILE *img, sublock *sb, int ptStart);

typedef struct indir_zone{
  uint32_t *zones;
  uint32_t z_idx;
}indir_zone;

typedef struct loader{
  char *contents;
  inode *inod;
  int32_t current_zone;
  indir_zone i1;
  indir_zone i2;
  int32_t z_size;
  int32_t inodes_loc;
  int32_t pt_loc;
  int all_loaded;
}loader;

/* Get the root inode of a filesystem */
void findRoot(FILE *img, loader *ldr);

/* Get an inode from a filepath and superblock */
void findFile(FILE *img, char *path, loader *ldr);

/* parse v, p, and s options */
args parse_flags(int argc, char *argv[]);

void load_zone(FILE *img, void *buf, int32_t zSize);

loader *prep_ldr(sublock sb, int32_t pt_loc);

#endif

