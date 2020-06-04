#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "mintool.h"

void *safe_malloc(int32_t size){
  void *res = malloc(size);
  if(res == NULL){
    perror("malloc");
    exit(EXIT_FAILURE);
  }
  return res;
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

  if(sig != PTABLE_SIG){
    printf("Partition table invalid - signature: %d\n", sig);
    exit(EXIT_FAILURE);
  }

  /*If not a minix type, stop*/
  if (pt -> type != MINIX_PTYPE){
    printf("type number: %d\n", pt -> type);
    printf("Not of type MINIX_PTYPE\n");
    exit(EXIT_FAILURE);
  }
}

void getSublock(FILE *img, sublock *sb, int ptStart){
  int sbLoc = ptStart + SBLOCK_ADDR;
  if(fseek(img, sbLoc, SEEK_SET) < 0){
    perror("fseek");
    exit(EXIT_FAILURE);
  }

  if(fread(sb, sizeof(sublock), 1, img) < 1){
    perror("fread");
    exit(EXIT_FAILURE);
  }
}

loader *prep_ldr(sublock sb, int32_t pt_loc){
  loader *ldr = safe_malloc(sizeof(loader));
  ldr->inod = safe_malloc(sizeof(inode));
  ldr->current_zone = 0;
  ldr->z_size = sb.blocksize << sb.log_zone_size;
  ldr->i1.z_idx = 0;
  ldr->i1.zones = safe_malloc(ldr->z_size);
  ldr->i2.z_idx = 0;
  ldr->i2.zones = safe_malloc(ldr->z_size);
  ldr->contents = safe_malloc(ldr->z_size);
  ldr->inodes_loc = pt_loc + (2 + sb.i_blocks + sb.z_blocks) * sb.blocksize;
  ldr->pt_loc = pt_loc;
  ldr->all_loaded = 0;
  ldr->found = 0;
  
  return ldr;
}

void read_zone(FILE *img, int32_t addr, loader *ldr, void *tgt){
  if(fseek(img, addr, SEEK_SET) < 0){
    perror("fseek");
    exit(EXIT_FAILURE);
  }
  if(fread(tgt, ldr->z_size, 1, img) < 1){
    perror("fread");
    exit(EXIT_FAILURE);
  }
}

void load_inode(FILE *img, loader *ldr, uint32_t inode_num){
  int32_t addr = ldr->inodes_loc + (inode_num - 1)  * sizeof(inode);
  if(fseek(img, addr, SEEK_SET) < 0){
    perror("fseek");
    exit(EXIT_FAILURE);
  }
  if(fread(ldr->inod, sizeof(inode), 1, img) < 1){
    perror("fread");
    exit(EXIT_FAILURE);
  }
  printf("size: %d\n", ldr->z_size);
  read_zone(img, ldr->inod->indirect * ldr->z_size, ldr, ldr->i1.zones);
  read_zone(img, ldr->inod->two_indirect * ldr->z_size, ldr, ldr->i2.zones);
  ldr->current_zone = ldr->all_loaded = ldr->found = 0;
}

void get_next_indirect(loader *ldr, FILE *img){
  int addr;
  printf("start of indirect\n");
  while (ldr->current_zone * ldr->z_size < ldr->inod -> size){
    printf("start of loop\n");

    /* Loop through indirect zone contents */
    while (ldr->i1.z_idx < ldr->z_size / sizeof(int32_t)){
      if (ldr->i1.zones[ldr->i1.z_idx] != 0){
        printf("non zero direct zone found, loading\n");
        addr = ldr->pt_loc + ldr->i1.zones[ldr->i1.z_idx] * ldr->z_size;
        read_zone(img, addr, ldr, (void *)ldr->contents);
        ldr->i1.z_idx++;
        ldr->current_zone++;
        return;
      }
      printf("found hole\n");
      ldr->i1.z_idx++;
      ldr->current_zone++;
    }

    /* Loop through double indirect zone contents */
    while (ldr->i2.z_idx < ldr->z_size / sizeof(int32_t)){
      printf("looping through double indirect\n");
      if (ldr->i2.zones[ldr->i2.z_idx] != 0){
        printf("non zero indirect zone found, loading\n");
        addr = ldr->pt_loc + ldr->i2.zones[ldr->i2.z_idx] * ldr->z_size;
        read_zone(img, addr, ldr, (void *)ldr->i1.zones);
        ldr->i1.z_idx = 0;
        break;
      }
      ldr->i2.z_idx++;
      printf("big hole found\n");
      ldr->current_zone += ldr->z_size / sizeof(int32_t);
    }
  }
  printf("end of indirect\n");
  ldr->all_loaded = 1;
}

void get_next_zone(loader *ldr, FILE *img){
  int addr;
  if (ldr->current_zone < DIRECT_ZONES){
    addr = ldr->pt_loc + ldr->inod->zone[ldr->current_zone] * ldr->z_size;
    read_zone(img, addr, ldr, (void *)ldr->contents);
    printf("current zone: %d\n", ldr->current_zone);
    ldr->current_zone++;
    if(ldr->current_zone * ldr->z_size > ldr->inod->size){
      ldr->all_loaded = 1;
    }
  }
  else{
    get_next_indirect(ldr, img);
  }
}

uint32_t search_zone(FILE *img, char *tok, loader *ldr){
  int i;
  dirent *entries = (dirent *)ldr->contents;
  for(i = 0; i < ldr->z_size / sizeof(dirent); i++){
    if(strcmp(tok, entries[i].name) == 0){
      ldr->found = 1;
      return entries[i].inode;
    }
  }
  return 0;
}

uint32_t dirent_inode_val(FILE *img, loader *ldr, int entry){
    get_next_zone(ldr, img);
    dirent *entries = (dirent *)ldr->contents;
    return entries[entry].inode;
}

uint32_t search_dir(FILE *img, char *tok, loader *ldr){
  uint32_t inode_num;
  ldr->found = 0;
  if(strcmp(tok, ".") == 0){
    return dirent_inode_val(img, ldr, 0);
  }
  if(strcmp(tok, "..") == 0){
    return dirent_inode_val(img, ldr, 1);
  }
  while(!ldr->all_loaded){
    get_next_zone(ldr, img);
    if((inode_num = search_zone(img, tok, ldr))){
      break;
    }
  }
  if(!ldr->found){
    printf("File not found\n");
    exit(EXIT_FAILURE);
  }
  return inode_num;
}

void findFile(FILE *img, char *path, loader *ldr){
  uint32_t inode_num;
  char *tokenized = safe_malloc(strlen(path));
  strcpy(tokenized, path);
  char *tok = strtok(tokenized, "/");
  do{
    inode_num = search_dir(img, tok, ldr);
    load_inode(img, ldr, inode_num);
  }while((tok = strtok(NULL, "/")) != NULL);
  free(tokenized);
}

void findRoot(FILE *img, loader *ldr){
  load_inode(img, ldr, 1);
  if(!(ldr->inod->mode & DIRECTORY)){
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

void get_permission(inode* i){

  char permissions[PERMISSION_SIZE + 1];
  memset(permissions, '-', PERMISSION_SIZE);
  permissions[PERMISSION_SIZE] = 0;

  if (i -> mode & DIRECTORY_MASK){
    permissions[DIRECTORY_INDEX] = 'd';
  }
  if (i -> mode & OWNER_READ_MASK){
    permissions[OWNER_READ] = 'r';
  }
  if (i -> mode & OWNER_WRITE_MASK){
    permissions[OWNER_WRITE] = 'w';
  }
  if (i -> mode & OWNER_EXECUTE_MASK){
    permissions[OWNER_EXECUTE] = 'x';
  }
  if (i -> mode & GROUP_READ_MASK){
    permissions[GROUP_READ] = 'r';
  }
  if (i -> mode & GROUP_WRITE_MASK){
    permissions[GROUP_WRITE] = 'w';
  }
  if (i -> mode & GROUP_EXECUTE_MASK){
    permissions[GROUP_EXECUTE] = 'x';
  }
  if (i -> mode & OTHER_READ_MASK){
    permissions[OTHER_READ] = 'r';
  }
  if (i -> mode & OTHER_WRITE_MASK){
    permissions[OTHER_WRITE] = 'w';
  }
  if (i -> mode & OTHER_EXECUTE_MASK){
    permissions[OTHER_EXECUTE] = 'x';
  }

  printf("Permissions: %s\n", permissions);
}
