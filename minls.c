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

int v_flag = 0;
int p_flag = 0;
int s_flag = 0;
char image_buffer[100];
char filepath_buffer[100];
int is_root_dir = 0;
FILE *image_fd;

int main(int argc, char *argv[]){  
  
  read_input(argc, argv);

  image_fd = NULL;
  image_fd = fopen(image_buffer, "r");

  printf("filepath_buffer: %s\n", image_buffer);

  if (image_fd == NULL){
    perror("No such file\n");
  }
  else{
    printf("Such file\n");
  }

  /*Get Partition Table*/

  /*Get Superblock contents*/

  /*Get File Inode contents*/

  return 0;
}

void read_input(int argc, char *argv[]){
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

    printf("Image: %s\n", argv[(optind + 1) + 1]);
    strcpy(image_buffer, argv[(optind + 1) + 1]);

    if (argv[optind + 1 + 2] != NULL){
      printf("Filepath: %s\n", argv[(optind + 1) + 2]);
      strcpy(filepath_buffer, argv[(optind + 1) + 2]);
    }
    else{
      is_root_dir = 1;
    }
  }

  
}
