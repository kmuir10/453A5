#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "mintool.h"

args read_input(int argc, char *argv[]);
void print_file(FILE *img, loader *ldr, args a, FILE *dest);

int min(int a, int b){
  return a < b ? a : b;
}

int main(int argc, char *argv[]){
  args a = read_input(argc, argv);
  FILE *dest = open_dest(a.dest);
  FILE *img = open_image(a.image);
  int ptLoc = find_pt(a, img);
  sublock sb;
  getSublock(a, img, &sb, ptLoc);
  loader *ldr = prep_ldr(sb, ptLoc);
  findRoot(img, ldr);
  findFile(a, img, ldr);
  print_file(img, ldr, a, dest);
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

void print_file(FILE *img, loader *ldr, args a, FILE *dest){
  int left_to_print = ldr->inod->size, to_print_now;
  char *empty = safe_malloc(ldr->z_size);           /* for "printing" holes */
  memset(empty, 0, ldr->z_size);

  /* minget can only get a regular file */
  if(ldr->inod->mode & DIRECTORY_MASK){
    fprintf(stderr, "%s is a directory\n", a.filepath);
    exit(EXIT_FAILURE);
  }
  if((ldr->inod->mode & FILE_MASK) != REGULAR_FILE){
    fprintf(stderr, "%s is not a regular file\n", a.filepath);
    exit(EXIT_FAILURE);
  }

  /* print zone by zone */
  while(!ldr->all_loaded){
    get_next_zone(ldr, img);
    int i;
    if(ldr->empty_count > 0){

      /* print as many holes as were found using the empty array */
      for(i = 0; i < ldr->empty_count; i++){
        fwrite(empty, sizeof(char), ldr->z_size, dest);
        left_to_print -= ldr->z_size;
      }
      ldr->empty_count = 0;
    }
    else{

      /* only print as many characters as needed. Do not print any null */
      to_print_now = min(left_to_print, ldr->z_size);
      fwrite(ldr->contents, sizeof(char), to_print_now, dest);
      left_to_print -= to_print_now;
    }
  }
}

args read_input(int argc, char *argv[]){
  args a;
  if (argc < 3){
    bad_args();
  }
  else{
    a = parse_flags(argc, argv);
    if (a.has_flags == 1){

      /* must have at least two more arguments left */
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
