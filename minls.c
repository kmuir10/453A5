#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "mintool.h"


int main(int argc, char *argv[]){
  int opt = 0;
  while ((opt = getopt(argc, argv, "v:ps")) != -1){
    printf("This is opt: %d, %d\n", opt, optind);
    switch(opt){
      case('p'):
        printf("P-case\n");
        break;
      case('s'):
        printf("S-case\n");
        break;
      case('v'):
        printf("V-case\n");
        break;
      default:
        printf("NO-case\n");
    }
  }
  return 0;
}

