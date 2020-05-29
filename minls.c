#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "mintool.h"


int main(int argc, char *argv[]){
  int opt = 0;
  long ptn = UNSPEC, sptn = UNSPEC;

  while ((opt = getopt(argc, argv, "vps")) != -1){
    printf("This is opt: %d, %d\n", opt, optind);
    switch(opt){
      case('p'):
        ptn = strtol(argv[optind], NULL, 10);
        printf("P-case, id: %ld\n", ptn);
        break;
      case('s'):
        sptn = strtol(argv[optind], NULL, 10);
        printf("S-case, id: %ld\n", sptn);
        break;
      case('v'):
        printf("V-case\n");
        break;
      default:
        printf("NO-case\n");
    }
  }

  printf("Image: %s\n", argv[(optind + 1) + 1]);

  if (argv[optind + 1 + 2] != NULL){
    printf("Filepath: %s\n", argv[(optind + 1) + 2]);
  }  
  
  return 0;
}

