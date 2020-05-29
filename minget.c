#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "mintool.h"

int main(int argc, char *argv[]){
  char *env = getenv("MY_DEBUG");
  char b510, b511;
  size_t sz = 0;
  partent pt[4];
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
  sz = fread(pt, sizeof(struct partent), 4, img);
  getPtable(img, pt, 0);
  printf("%x %x %x %x %x %x %x %x %x %x\n", pt[0].bootind, pt[0].start_head, pt[0].start_sec, pt[0].start_cyl, pt[0].type, pt[0].end_head, pt[0].end_sec, pt[0].end_cyl, pt[0].lFirst, pt[0].size);
  printf("%x %x %x %x %x %x %x %x %x %x\n", pt[1].bootind, pt[1].start_head, pt[1].start_sec, pt[1].start_cyl, pt[1].type, pt[1].end_head, pt[1].end_sec, pt[1].end_cyl, pt[1].lFirst, pt[1].size);
  printf("%x %x %x %x %x %x %x %x %x %x\n", pt[2].bootind, pt[2].start_head, pt[2].start_sec, pt[2].start_cyl, pt[2].type, pt[2].end_head, pt[2].end_sec, pt[2].end_cyl, pt[2].lFirst, pt[2].size);
  printf("%x %x %x %x %x %x %x %x %x %x\n", pt[3].bootind, pt[3].start_head, pt[3].start_sec, pt[3].start_cyl, pt[3].type, pt[3].end_head, pt[3].end_sec, pt[3].end_cyl, pt[3].lFirst, pt[3].size);

  fseek(img, pt[0].lFirst * 512 + 0x1BE, SEEK_SET);
  sz = fread(pt, sizeof(struct partent), 4, img);
  eprintf("bytes read: %d\n", (int)(sz * sizeof(struct partent)));
  printf("%x %x %x %x %x %x %x %x %x %x\n", pt[0].bootind, pt[0].start_head, pt[0].start_sec, pt[0].start_cyl, pt[0].type, pt[0].end_head, pt[0].end_sec, pt[0].end_cyl, pt[0].lFirst, pt[0].size);
  printf("%x %x %x %x %x %x %x %x %x %x\n", pt[1].bootind, pt[1].start_head, pt[1].start_sec, pt[1].start_cyl, pt[1].type, pt[1].end_head, pt[1].end_sec, pt[1].end_cyl, pt[1].lFirst, pt[1].size);
  printf("%x %x %x %x %x %x %x %x %x %x\n", pt[2].bootind, pt[2].start_head, pt[2].start_sec, pt[2].start_cyl, pt[2].type, pt[2].end_head, pt[2].end_sec, pt[2].end_cyl, pt[2].lFirst, pt[2].size);
  printf("%x %x %x %x %x %x %x %x %x %x\n", pt[3].bootind, pt[3].start_head, pt[3].start_sec, pt[3].start_cyl, pt[3].type, pt[3].end_head, pt[3].end_sec, pt[3].end_cyl, pt[3].lFirst, pt[3].size);
  return 0;
}
