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

int main(int argc, char *argv[]){  
  
  /* open the image */
  int v_flag = 0;
  int p_flag = 0;
  int s_flag = 0;
  char image_buffer[100];
  char filepath_buffer[100];
  int num_of_partitions = 0;
  int num_of_sub_partitions = 0;
  read_input(argc, argv, image_buffer, filepath_buffer,
    &v_flag, &p_flag, &s_flag, &num_of_partitions, &num_of_sub_partitions);

  FILE *img = fopen(image_buffer, "r");

  if (img == NULL){
    perror("No such file\n");
    exit(EXIT_FAILURE);
  }
  else{
    printf("Such file\n");
    printf("v_flag: %d\n", v_flag);
    printf("p_flag: %i\n", p_flag);
    printf("s_flag: %i\n", s_flag);
    printf("num_of_partitions: %i\n", num_of_partitions);
    printf("num_of_sub_partitions: %i\n", num_of_sub_partitions);
  }


  /*Get Partition Table*/

  /*Get Superblock contents*/

  /*Get File Inode contents*/

  return 0;
}

