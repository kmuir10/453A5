#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "mintool.h"


int main(int argc, char *argv[]){
  
	int opt = 0;
	char *ptr;

	while ((opt = getopt(argc, argv, "vps")) != -1){

		printf("This is opt: %d, %d\n", opt, optind);

		if (opt == -1){
			printf("Bad-case\n");
			return 0;
		}
		else if (opt == 'p'){
			printf("P-case, id: %lu\n", 
				strtol(argv[optind], &ptr, 10));
		}
		else if (opt == 's'){
			printf("S-case, id: %lu\n", 
				strtol(argv[optind], &ptr, 10));
		}
		else if (opt == 'v'){
			printf("V-case, id: %lu\n", 
				strtol(argv[optind], &ptr, 10));
		}
		else{
			printf("NO-case\n");
			return 0;
		}
	}

  return 0;
}
