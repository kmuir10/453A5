#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "mintool.h"

args read_input(int argc, char *argv[]);

int min(int a, int b){
  return a < b ? a : b;
}

int main(int argc, char *argv[]){
  char *env = getenv("MY_DEBUG");
  partent pt_table[4];
  int ptLoc = 0;
  sublock sb;
  eprintf("%d minget\n", 0);

  args a = read_input(argc, argv);

  /* open the image */
  FILE *dest;
  FILE *img = fopen(a.image, "r");

  if(img == NULL){
    perror("fopen\n");
    exit(EXIT_FAILURE);
  }

  if(a.dest != NULL){
    dest = fopen(a.dest, "w");
    if(dest == NULL){
      perror("fopen");
      exit(EXIT_FAILURE);
    }
  }
  else{
    dest = fopen("/dev/stdout", "w");
  }

  /* get the first partition table if requested
   * and calculate location of requested partition */
  if(a.p_flag){
    getPtable(img, pt_table, 0);
    ptLoc = pt_table[a.pt].lFirst * SECTOR_SZ;
  }

  /* get the subpartition table if requested and calculate location */
  if(a.s_flag){
    getPtable(img, pt_table, pt_table[a.pt].lFirst);
    ptLoc = pt_table[a.spt].lFirst * SECTOR_SZ;
  }

  /* get the superblock */
  getSublock(img, &sb, ptLoc);
  eprintf("magic number: %x\n", sb.magic);
  
  loader *ldr = prep_ldr(sb, ptLoc);

  char *empty = safe_malloc(ldr->z_size);
  memset(empty, 0, ldr->z_size);

  findRoot(img, ldr);
  findFile(img, a.filepath, ldr);
  int left_to_print = ldr->inod->size, to_print_now;
  if(ldr->inod->mode & DIRECTORY_MASK){
    fprintf(stderr, "%s is a directory\n", a.filepath);
    exit(EXIT_FAILURE);
  }
  if((ldr->inod->mode & FILE_MASK) != REGULAR_FILE){
    fprintf(stderr, "%s is not a regular file\n", a.filepath);
    exit(EXIT_FAILURE);
  }
  while(!ldr->all_loaded){
    get_next_zone(ldr, img);
    int i;
    if(ldr->empty_count > 0){
      for(i = 0; i < ldr->empty_count; i++){
        fwrite(empty, sizeof(char), ldr->z_size, dest);
        left_to_print -= ldr->z_size;
      }
      ldr->empty_count = 0;
    }
    else{
      to_print_now = min(left_to_print, ldr->z_size);
      fwrite(ldr->contents, sizeof(char), to_print_now, dest);
      left_to_print -= to_print_now;
    }
  }
  
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
  if (argc < 3){
    bad_args();
  }
  else{
    a = parse_flags(argc, argv);
    if (a.has_flags == 1){
      if(optind >= argc - 1){
        bad_args();
      }
      a.image = argv[optind++];
      a.filepath = argv[optind++];
      a.dest = (optind >= argc) ? NULL : argv[optind];
    }
    else{
      a.image = argv[1];
      a.filepath = argv[2];
      a.dest = (argc == 4) ? argv[3] : NULL;
    }

  }
  return a;
}
