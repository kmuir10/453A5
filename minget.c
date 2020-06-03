#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "mintool.h"

static args read_input(int argc, char *argv[]);

int main(int argc, char *argv[]){
  char *env = getenv("MY_DEBUG");
  char b510, b511;
  partent pt[4];
  int sptLoc = 0;
  sublock sb;
  eprintf("%d minget\n", 0);

  args a = read_input(argc, argv);

  /* open the image */
  FILE *img = fopen(a.image, "r");

  if(img == NULL){
    perror("fopen\n");
    exit(EXIT_FAILURE);
  }
  else{
    printf("Such file\n");
    printf("v_flag: %d\n", a.v_flag);
    printf("p_flag: %i\n", a.p_flag);
    printf("s_flag: %i\n", a.s_flag);
    printf("partition: %i\n", a.ptn);
    printf("subpartition: %i\n", a.sptn);
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
  if(a.p_flag){
    getPtable(img, pt, 0);
  }
  /* stpLoc isn't updated properly yet, fix read_input first to get that from args */
  if(a.s_flag){
    getPtable(img, pt, sptLoc);
  }

  /* get the superblock */
  getSublock(img, &sb, 0);
  eprintf("magic number: %x\n", sb.magic);
  
  inode inod = findFile(img, sb, 0, a.filepath);
  printf("%d\n", inod.size);
  return 0;
}

void bad_args(){
      printf("usage: minget [ -v ] [ -p num [ -s num ] ] imagefile path "
             "[ destination ]\n");
      printf("Options\n");
      printf("-p part --- select partition for filesystem (default: none)\n");
      printf("-s sub --- select subpartition for filesystem (default: none)\n");
      printf("-h help --- print usage information and exit\n");
      printf("-v verbose --- increase verbosity level\n");
      exit(EXIT_FAILURE);
}

args read_input(int argc, char *argv[]){
  args a;
  if (argc < 2){
    bad_args();
  }
  else{
    a = parse_flags(argc, argv);
    if (a.has_flags == 1){
      a.image = argv[optind++];
      a.filepath = argv[optind++];
      a.dest = (optind >= argc) ? NULL : argv[optind];
    }
    else{
      a.image = argv[1];
      a.filepath = argv[2];
    }

  }
  return a;
}
