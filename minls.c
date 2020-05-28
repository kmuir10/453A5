#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "mintool.h"


int main(int argc, char *argv[]){
  
	int opt = getopt(argc, argv, "p:s");

	printf("This is opt: %i\n", opt);

  return 0;
}
