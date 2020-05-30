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
    perror("Not of MINIX_PTYPE");
    exit(EXIT_FAILURE);
  }
  else{
    printf("Success\n");
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

inode findFile(FILE *img, sublock sb, int32_t ptLoc, char *path){
  inode inod;
  int32_t inodNum = 0, inodesLoc = 0;
  printf("ninodes: %d, i_blocks: %d, z_blocks %d, log_zone_size %d, zones %d, magic %d, blocksize %d\n", sb.ninodes, sb.i_blocks, sb.z_blocks, sb.log_zone_size, sb.zones, sb.magic, sb.blocksize);

  /* start of partition + boot + super + i_block + z_block gets first inode */

  inodesLoc = ptLoc + (2 + sb.i_blocks + sb.z_blocks) * sb.blocksize;
  printf("inodesLoc: 0x%x\n", inodesLoc);
  if(fseek(img, inodesLoc, SEEK_SET) < 0){
    perror("fseek");
    exit(EXIT_FAILURE);
  }

  if(fread(&inod, sizeof(inode), 1, img) < 1){
    perror("fread");
    exit(EXIT_FAILURE);
  }

  if(inod.mode & REGULAR_FILE){
    printf("Regular file\n");
  }
  if(inod.mode & DIRECTORY){
    printf("Directory\n");
  }
  printf("mode:%x\n", inod.mode);

  return inod;
}
