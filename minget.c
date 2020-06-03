#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "mintool.h"

args read_input(int argc, char *argv[]);

int main(int argc, char *argv[]){
  char *env = getenv("MY_DEBUG");
  partent pt_table[4];
  int ptLoc = 0;
  sublock sb;
  eprintf("%d minget\n", 0);

  args a = read_input(argc, argv);

  /* open the image */
  FILE *img = fopen(a.image, "r");

  if(img == NULL){
    perror("fopen\n");
    exit(EXIT_FAILURE);
  }

  /* get the first partition table if requested
   * and calculate location of requested partition */
  if(a.p_flag){
    getPtable(img, pt_table, 0);
    ptLoc = pt_table[a.pt].lFirst * SECTOR_SZ;
  }

  /* sptLoc isn't updated properly yet, fix read_input first to get that from args */
  if(a.s_flag){
    getPtable(img, pt_table, pt_table[a.pt].lFirst);
    ptLoc = pt_table[a.spt].lFirst * SECTOR_SZ;
  }

  /* get the superblock */
  getSublock(img, &sb, ptLoc);
  eprintf("magic number: %x\n", sb.magic);
  
  loader ldr = prep_ldr(sb, ptLoc);
  findRoot(img, ldr);
  
  
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
