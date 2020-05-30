#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "mintool.h"

int main(int argc, char *argv[]){
  char *env = getenv("MY_DEBUG");
  char b510, b511;
  size_t sz = 0;
  partent pt[4];
  sublock sb;
  eprintf("%d minget\n", 0);

  /* open the image */
  FILE *img = fopen("/home/kmuir/os/assignments/453A5/Images/HardDisk", "r");
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

  /* get the first entry */
  getPtable(img, pt, 0);

  /* get the superblock */
  getSublock(img, &sb, 0);
  printf("%d\n", sb.magic);
  return 0;
}
