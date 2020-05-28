#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "mintool.h"


int main(int argc, char *argv[]){
  
	int opt = 0;

	while ((opt = getopt(argc, argv, "v:p:s")) != -1){

		printf("This is opt: %d, %d\n", opt, optind);

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
		else if (opt == 'v'){
			printf("V-case\n");
		}
		else{
			printf("NO-case\n");
			return 0;
		}
	}

  return 0;
}
