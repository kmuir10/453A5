#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "mintool.h"

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

  if(img == NULL){
    perror("fopen\n");
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
  
  inode inod = findFile(img, sb, suBlockLoc, "usr/src/include/stdio.h");
  return 0;
}

