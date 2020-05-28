#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "mintool.h"


int main(int argc, char *argv[]){
  
	int opt = getopt(argc, argv, "p:s");

	printf("This is opt: %i\n", opt);

	if (opt == -1){
		printf("Bad-case\n");
		return 0;
	}
	else if (opt == 'p'){
		printf("P-case\n");
	}
	else if (opt == 's'){
		printf("S-case\n");
	}
	else{
		printf("NO-case\n");
		return 0;
	}

  return 0;
}
