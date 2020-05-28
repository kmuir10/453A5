#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "mintool.h"

int main(int argc, char *argv[]){
  char *env = getenv("MY_DEBUG");
  char b510, b511;
  Partn pt = malloc(sizeof(struct partn));
  eprintf("%d minget\n", 0);
  FILE *img = fopen("/home/kmuir/os/assignments/453A5/Images/Partitioned", "r");
  if(img == NULL){
    perror("fopen\n");
    exit(EXIT_FAILURE);
  }
  eprintf("%d file opened\n", 1);
  if(fseek(img, 510, SEEK_SET) != 0){
    perror("fseek");
    exit(EXIT_FAILURE);
  }
  eprintf("%d seeked\n", 2);
  b510 = fgetc(img);
  b511 = fgetc(img);
  printf("%d, %d\n", (int8_t)b510, (int8_t)b511);
  pt->bootind = pt->end_cyl = 1;
  printf("%x %x %x %x %x %x %x %x %x %x", pt->bootind, pt->start_head, pt->start_sec, pt->start_cyl, pt->type, pt->end_head, pt->end_sec, pt->end_cyl, pt->lFirst, pt->size);
  fread(pt, 1, sizeof(struct partn), img);
  printf("%x %x %x %x %x %x %x %x %x %x", pt->bootind, pt->start_head, pt->start_sec, pt->start_cyl, pt->type, pt->end_head, pt->end_sec, pt->end_cyl, pt->lFirst, pt->size);
  return 0;
}
