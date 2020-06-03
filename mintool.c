#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "mintool.h"


int32_t swend32(int32_t i){
  return (((i&0xFF)<<24)|((i&0xFF00)<<8)|
      ((i&0xFF0000)>>8)|((i&0xFF000000)>>24));
}

int16_t swend16(int16_t i){
  return (((i&0xFF)<<8)|((i&0xFF00)>>8));
}

int32_t zsize(sublock sblk){
  return sblk.blocksize << sblk.log_zone_size;
}

/* get partition table from img into pt, which should be a 4 element array of 
 * partition table entries */
void getPtable(FILE *img, partent *pt, int ptStart){
  uint16_t sig = 0;

  /* the partition is ptStart sectors away from the start of the file
   * multiply by SECTOR_SZ to convert from sectors to bytes, and add the offset
   * PTABLE_ADDR (0x1BE) to get the partition table itself */

  int ptableLoc = ptStart * SECTOR_SZ + PTABLE_ADDR;
  if(fseek(img, ptableLoc, SEEK_SET) < 0){
    perror("fseek");
    exit(EXIT_FAILURE);
  }

  /* read all four partition table entries into pt, totaling 64 bytes */
  if(fread(pt, sizeof(struct partent), PTABLE_SZ, img) < PTABLE_SZ){
    perror("fread");
    exit(EXIT_FAILURE);
  }

  /* while we're at it, check the signature */
  if(fread(&sig, sizeof(uint16_t), 1, img) < 1){
    perror("fread");
    exit(EXIT_FAILURE);
  }
  printf("signature: %x\n", sig);

  /*If not a minix type, stop*/
  if (pt -> type != MINIX_PTYPE){
    printf("type number: %d\n", pt -> type);
    printf("Not of type MINIX_PTYPE\n");
    exit(EXIT_FAILURE);
  }
}

void getSublock(FILE *img, sublock *sb, int ptStart){
  int sbLoc = ptStart + SBLOCK_ADDR;
  printf("sbLoc:%d\n", sbLoc);
  if(fseek(img, sbLoc, SEEK_SET) < 0){
    perror("fseek");
    exit(EXIT_FAILURE);
  }

  if(fread(sb, sizeof(sublock), 1, img) < 1){
    perror("fread");
    exit(EXIT_FAILURE);
  }
}

void getNextInode(FILE *img, inode *inod, char *nextFile, int32_t ptLoc, int32_t zSize){
}

loader prep_ldr(sublock sb, int32_t pt_loc){
  loader ldr;
  ldr.inod = malloc(sizeof(inode));
  ldr.current_zone = 0;
  ldr.z_size = sb.blocksize << sb.log_zone_size;
  ldr.i1.z_idx = 0;
  ldr.i1.zones = malloc(ldr.z_size);
  ldr.i2.z_idx = 0;
  ldr.i2.zones = malloc(ldr.z_size);
  ldr.contents = malloc(ldr.z_size);
  ldr.inodes_loc = pt_loc + (2 + sb.i_blocks + sb.z_blocks) * sb.blocksize;
  
  if(!ldr.contents || !ldr.i1.zones || !ldr.i2.zones || !ldr.inod){
    perror("malloc");
    exit(EXIT_FAILURE);
  }
  return ldr;
}

inode *search_dir(FILE *img, char *tok, loader ldr){
  return NULL;
}

void findFile(FILE *img, char *path, loader ldr){
  char *tokenized = malloc(strlen(path));
  char *tok;
  printf("bleh\n");
  if(!tokenized){
    perror("malloc");
    exit(EXIT_FAILURE);
  }
  tok = strtok(tokenized, "/");
  while((tok = strtok(NULL, "/")) != NULL){
    if(search_dir(img, tok, ldr) == NULL){
      printf("File not found\n");
      exit(EXIT_FAILURE);
    }
  }

  if(fseek(img, ldr.inodes_loc, SEEK_SET) < 0){
    perror("fseek");
    exit(EXIT_FAILURE);
  }

  if(fread(ldr.inod, sizeof(inode), 1, img) < 1){
    perror("fread");
    exit(EXIT_FAILURE);
  }
  free(tokenized);
}

void findRoot(FILE *img, loader ldr){
  if(fseek(img, ldr.inodes_loc, SEEK_SET) < 0){
    perror("fseek");
    exit(EXIT_FAILURE);
  }

  if(fread(ldr.inod, sizeof(inode), 1, img) < 1){
    perror("fread");
    exit(EXIT_FAILURE);
  }

  if(!(ldr.inod->mode & DIRECTORY)){
    printf("Failed to find root directory\n");
    exit(EXIT_FAILURE);
  }
}

args parse_flags(int argc, char *argv[]){
  int opt = 0;
  args a = {0, 0, 0, 0, 0, 0, NULL, NULL, NULL};
  while ((opt = getopt(argc, argv, "vps")) != -1){
    a.has_flags = 1;
    switch(opt){
      case('p'):
        a.pt = (int)strtol(argv[optind], NULL, 10);
        a.p_flag = 1;
        optind++;
        break;
      case('s'):
        a.spt = (int)strtol(argv[optind], NULL, 10);
        a.s_flag = 1;
        optind++;
        break;
      case('v'):
        a.v_flag = 1;
        break;
      default:
        exit(EXIT_FAILURE);
    }
  }
  if(a.s_flag && !a.p_flag){
    fprintf(stderr, "Must have partition to have subpartition\n");
    exit(EXIT_FAILURE);
  }
  return a;
}

void seek_read_inode(FILE *img, indir_zone idr, loader ldr, void *tgt){
  int next_addr = ldr.inodes_loc + (idr.zones[idr.z_idx] - 1) * sizeof(inode);
  if(fseek(img, next_addr, SEEK_SET) < 0){
    perror("fseek");
    exit(EXIT_FAILURE);
  }
  if(fread(tgt, ldr.z_size, 1, img) < 1){
    perror("fread");
    exit(EXIT_FAILURE);
  }
}

void get_next_indirect(loader ldr, FILE *img){
  while (ldr.current_zone * ldr.z_size < ldr.inod -> size){

    /* Loop through indirect zone contents */
    while (ldr.i1.z_idx < ldr.z_size / sizeof(int32_t)){
      ldr.i1.z_idx++;
      if (ldr.i1.zones[ldr.i1.z_idx] == 0){
        ldr.current_zone++;
      }
      else{
        seek_read_inode(img, ldr.i1, ldr, (void *)ldr.contents);
        return;
      }
    }

    /* Loop through double indirect zone contents */
    while (ldr.i2.z_idx < ldr.z_size / sizeof(int32_t)){
      ldr.i2.z_idx++;
      if (ldr.i2.zones[ldr.i2.z_idx] == 0){
        ldr.current_zone += ldr.z_size;
      }
      else{
        seek_read_inode(img, ldr.i2, ldr, (void *)ldr.i1.zones);
        ldr.i1.z_idx = 0;
      }
    }
  }
  return;
}

void get_next_zone(loader ldr, FILE *img){
  if (ldr.current_zone < DIRECT_ZONES){
    if(fseek(img, ldr.inodes_loc, SEEK_SET) < 0){
      perror("fseek");
      exit(EXIT_FAILURE);
    }
    if(fread(ldr.contents, sizeof(inode), 1, img) < 1){
      perror("fread");
      return;
    }
  }
  else{
    get_next_indirect(ldr, img);
  }
}

void get_permission(inode* i){

  char permissions[10] = "----------";

  if (i -> mode & FILE_TYPE_MASK){
    permissions[0] = '-';
  }
  else if (i -> mode & REGULAR_FILE_MASK){
    permissions[0] = '-';
  }
  else if (i -> mode & DIRECTORY_MASK){
    permissions[0] = 'd';
  }
  else if (i -> mode & OWNER_READ_MASK){
    permissions[1] = 'r';
  }
  else if (i -> mode & OWNER_WRITE_MASK){
    permissions[2] = 'w';
  }
  else if (i -> mode & OWNER_EXECUTE_MASK){
    permissions[3] = 'x';
  }
  else if (i -> mode & GROUP_EXECUTE_MASK){
    permissions[4] = 'r';
  }
  else if (i -> mode & GROUP_EXECUTE_MASK){
    permissions[5] = 'w';
  }
  else if (i -> mode & GROUP_EXECUTE_MASK){
    permissions[6] = 'x';
  }
  else if (i -> mode & OTHER_EXECUTE_MASK){
    permissions[7] = 'r';
  }
  else if (i -> mode & OTHER_EXECUTE_MASK){
    permissions[8] = 'w';
  }
  else if (i -> mode & OTHER_EXECUTE_MASK){
    permissions[9] = 'x';
  }

  printf("Permissions: %s\n", permissions);
}
