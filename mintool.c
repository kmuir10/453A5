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

void getNextInode(FILE *img, inode *inod, char *nextFile, int32_t ptLoc, int32_t zSize){
}

inode findFile(FILE *img, sublock sb, int32_t ptLoc, char *path){
  inode inod;
  int32_t inodesLoc = 0, zSize = 0;
  char *input = malloc(sizeof(char) * strlen(path));
  strcpy(input, path);
  char *tok = strtok(input, "/");
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

  printf("%s\n", tok);
  zSize = sb.blocksize << sb.log_zone_size;
  while((tok = strtok(NULL, "/")) != NULL){
    printf("%s\n", tok);
    getNextInode(img, &inod, tok, ptLoc, zSize);
  }
  printf("%s\n", path);
  free(input);
  return inod;
}

/*Reads the terminal files*/
void read_input(int argc, char *argv[], 
  char image_buffer[100], char *filepath_buffer, 
  int *v_flag, int *p_flag, int *s_flag,
  int *num_of_partitions, int *num_of_sub_partitions){

  int has_flags = 0;

  if (!strcmp(argv[0], "./minls")){
    int opt = 0;
    long ptn = UNSPEC, sptn = UNSPEC;

    if (argc == 1){
      printf("usage: minls [ -v ] [ -p num [ -s num ] ] imagefile [ path ]\n");
      printf("Options\n");
      printf("-p part --- select partition for filesystem (default: none)\n");
      printf("-s sub --- select subpartition for filesystem (default: none)\n");
      printf("-h help --- print usage information and exit\n");
      printf("-v verbose --- increase verbosity level\n");
    }
    else{
      while ((opt = getopt(argc, argv, "vps")) != -1){
        printf("This is opt: %d, %d\n", opt, optind);
        switch(opt){
          case('p'):
            ptn = strtol(argv[optind], NULL, 10);
            *num_of_partitions = strtol(argv[optind], NULL, 10);
            printf("P-case, id: %ld\n", ptn);
            *p_flag = 1;
            break;
          case('s'):
            sptn = strtol(argv[optind], NULL, 10);
            *num_of_sub_partitions = strtol(argv[optind], NULL, 10);
            printf("S-case, id: %ld\n", sptn);
            *s_flag = 1;
            break;
          case('v'):
            printf("V-case\n");
            *v_flag = 1;
            break;
          default:
            printf("NO-case\n");
        }
      }

      printf("Image: %s\n", argv[(optind + 1) + 1]);
      strcpy(image_buffer, argv[(optind + 1) + 1]);

      if (argv[optind + 1 + 2] != NULL){
        printf("Filepath: %s\n", argv[(optind + 1) + 2]);
        filepath_buffer = malloc(sizeof(char) * strlen(argv[optind+1+2]));
        strcpy(filepath_buffer, argv[(optind + 1) + 2]);
      }
    }
  }
  else if (!strcmp(argv[0], "./minget")){
    int opt = 0;
    long ptn = UNSPEC, sptn = UNSPEC;

    if (argc == 1){
      printf("usage: minget [ -v ] [ -p part [ -s subpart ] ] imagefile srcpath [ dstpath ]\n");
      printf("Options\n");
      printf("-p part --- select partition for filesystem (default: none)\n");
      printf("-s sub --- select subpartition for filesystem (default: none)\n");
      printf("-h help --- print usage information and exit\n");
      printf("-v verbose --- increase verbosity level\n");
    }
    else if (argc == 2){
      perror("Invalid number of arguments\n");
      exit(EXIT_FAILURE);
    }
    else{
      while ((opt = getopt(argc, argv, "vps")) != -1){
        printf("This is opt: %d, %d\n", opt, optind);
        has_flags = 1;
        switch(opt){
          case('p'):
            ptn = strtol(argv[optind], NULL, 10);
            *num_of_partitions = strtol(argv[optind], NULL, 10);
            printf("P-case, id: %ld\n", ptn);
            *p_flag = 1;
            break;
          case('s'):
            sptn = strtol(argv[optind], NULL, 10);
            *num_of_sub_partitions = strtol(argv[optind], NULL, 10);
            printf("S-case, id: %ld\n", sptn);
            *s_flag = 1;
            break;
          case('v'):
            printf("V-case\n");
            *v_flag = 1;
            break;
          default:
            printf("NO-case\n");
        }
      }

      if (has_flags == 1){
        printf("Image: %s\n", argv[(optind + 1) + 1]);
        strcpy(image_buffer, argv[(optind + 1) + 1]);

        printf("Filepath: %s\n", argv[(optind + 1) + 2]);
        filepath_buffer = malloc(sizeof(char) * strlen(argv[optind+1+2]));
        strcpy(filepath_buffer, argv[(optind + 1) + 2]);
      }
      else{
        printf("Image: %s\n", argv[1]);
        strcpy(image_buffer, argv[1]);

        printf("Filepath: %s\n", argv[2]);
        filepath_buffer = malloc(sizeof(char) * strlen(argv[2]));
        strcpy(filepath_buffer, argv[2]);
      }

      if(s_flag && !p_flag){
        fprintf(stderr, "Must have partition to have subpartition\n");
        exit(EXIT_FAILURE);
      }
    }
  }

  
}
