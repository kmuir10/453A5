#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "mintool.h"

int v_flag = 0;
int p_flag = 0;
int s_flag = 0;
char image_buffer[100];
char filepath_buffer[100];
FILE *image_fd;

int main(int argc, char *argv[]){
  char *env = getenv("MY_DEBUG");
  char b510, b511;
  size_t sz = 0;
  partent pt[4];
  eprintf("%d minget\n", 0);

  /* open the image */
  //FILE *img = fopen("/home/chsitu/CPE-453/Asgn5/Professor/Images/HardDisk", "r");
  read_input(argc, argv);
  
  char *image_path = malloc(strlen(filepath_buffer) + 
  	strlen(image_buffer) + 1);
  strcpy(image_path, filepath_buffer);
	strcat(image_path, image_buffer);

  printf("Image Path: %s\n", image_path);

  FILE *img = fopen(image_path, "r");

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

void read_input(int argc, char *argv[]){
  int opt = 0;
  long ptn = UNSPEC, sptn = UNSPEC;

  if (argc == 1){
    printf("usage: minget [ -v ] [ -p part [ -s subpart ] ] imagefile srcpath [ dstpath ]\n");
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

    printf("Filepath: %s\n", argv[(optind + 1) + 2]);
    strcpy(filepath_buffer, argv[(optind + 1) + 2]);
  }
}