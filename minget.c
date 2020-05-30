#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "mintool.h"

int v_flag = 0;
int p_flag = 0;
int s_flag = 0;
char image_buffer[100];
char filepath_buffer[100];
FILE *image_fd;
int has_flags = 0;

int main(int argc, char *argv[]){
  char *env = getenv("MY_DEBUG");
  char b510, b511;
  size_t sz = 0;
  partent pt[4];
  int sptLoc = 0;
  sublock sb;
  int suBlockLoc = SBLOCK_ADDR;
  eprintf("%d minget\n", 0);

  /* open the image */
  read_input(argc, argv);

  printf("Image Path: %s\n", image_buffer);

  FILE *img = fopen(image_buffer, "r");

  if(img == NULL){
    perror("fopen\n");
    exit(EXIT_FAILURE);
  }
  eprintf("%d file opened\n", 1);

  /* check the signature */
  if(fseek(img, 510, SEEK_SET) != 0){
    perror("fseek");
    exit(EXIT_FAILURE);
  }
  eprintf("%d seeked\n", 2);
  b510 = fgetc(img);
  b511 = fgetc(img);
  printf("%d, %d\n", (int8_t)b510, (int8_t)b511);

  /* go to the partition directory */
  if(fseek(img, 0x1BE, SEEK_SET) != 0){
    perror("fseek");
    exit(EXIT_FAILURE);
  }

  /* get the first entry if the user asked for it*/
  if(p_flag){
    getPtable(img, pt, 0);
  }
  /* stpLoc isn't updated properly yet, fix read_input first to get that from args */
  if(s_flag){
    getPtable(img, pt, sptLoc);
  }

  /* get the superblock */
  getSublock(img, &sb, 0);
  eprintf("magic number: %x\n", sb.magic);
  
  inode inod = findFile(img, sb, 0, "usr/src/include/stdio.h");
  return 0;
}

void read_input(int argc, char *argv[]){
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
          printf("P-case, id: %ld\n", ptn);
          p_flag = 1;
          break;
        case('s'):
          sptn = strtol(argv[optind], NULL, 10);
          printf("S-case, id: %ld\n", sptn);
          s_flag = 1;
          break;
        case('v'):
          printf("V-case\n");
          v_flag = 1;
          break;
        default:
          printf("NO-case\n");
      }
    }

    if (has_flags == 1){
      printf("Image: %s\n", argv[(optind + 1) + 1]);
      strcpy(image_buffer, argv[(optind + 1) + 1]);

      printf("Filepath: %s\n", argv[(optind + 1) + 2]);
      strcpy(filepath_buffer, argv[(optind + 1) + 2]);
    }
    else{
      printf("Image: %s\n", argv[1]);
      strcpy(image_buffer, argv[1]);

      printf("Filepath: %s\n", argv[2]);
      strcpy(filepath_buffer, argv[2]);
    }

    if(s_flag && !p_flag){
      fprintf(stderr, "Must have partition to have subpartition\n");
      exit(EXIT_FAILURE);
    }
  }
}
