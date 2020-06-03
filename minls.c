#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "mintool.h"

/*Minls lists a file or directory on the given filesystem image. If the optional
path argument is ommitted it defaults to the root directory.*/

void bad_args();
args read_input(int argc, char *argv[]);

int main(int argc, char *argv[]){  
  
  args a = read_input(argc, argv);

  /* open the image */
  FILE *img = fopen(a.image, "r");

  if (img == NULL){
    perror("No such file\n");
    exit(EXIT_FAILURE);
  }
  else{
    printf("Such file\n");
    printf("v_flag: %d\n", a.v_flag);
    printf("p_flag: %i\n", a.p_flag);
    printf("s_flag: %i\n", a.s_flag);
    printf("num_of_partitions: %i\n", a.ptn);
    printf("num_of_sub_partitions: %i\n", a.sptn);
  }


  /*Get Partition Table*/

  /*Get Superblock contents*/

  /*Get File Inode contents*/

  return 0;
}

args read_input(int argc, char *argv[]){
  args a;
  if (argc == 1){
    bad_args();
  }
  else{
    a = parse_flags(argc, argv);
    a.image = argv[optind++];
    if (optind < argc){
      a.filepath = argv[optind];
    }
  }
  return a;
}

void bad_args(){
      printf("usage: minls [ -v ] [ -p num [ -s num ] ] imagefile [ path ]\n");
      printf("Options\n");
      printf("-p part --- select partition for filesystem (default: none)\n");
      printf("-s sub --- select subpartition for filesystem (default: none)\n");
      printf("-h help --- print usage information and exit\n");
      printf("-v verbose --- increase verbosity level\n");
      exit(EXIT_FAILURE);
}


