#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "mintool.h"

/*Minls lists a file or directory on the given filesystem image. 
If the optional path argument is ommitted it defaults to the 
root directory.*/

void bad_args();
args read_input(int argc, char *argv[]);

/*Calculates the address of the inode i_num and loads from that 
  address into inod. Can be used without modifying the ldr's inode*/
void load_dirent_inode(FILE *img, loader *ldr, inode *inod, uint32_t i_num){
  int32_t addr = ldr->inodes_loc + (i_num - 1)  * sizeof(inode);
  if(fseek(img, addr, SEEK_SET) < 0){
    perror("fseek");
    exit(EXIT_FAILURE);
  }
  if(fread(inod, sizeof(inode), 1, img) < 1){
    perror("fread a");
    exit(EXIT_FAILURE);
  }
}

/*Reads the arguments from the terminal and set flags, etc.*/
args read_input(int argc, char *argv[]){
  args a;
  if (argc == 1){
    bad_args();
  }
  else{
    a = parse_flags(argc, argv);
    a.image = argv[optind++];
    if (optind < argc){
      a.filepath = argv[optind];
    }
    else{
    	a.filepath = ".";
    }
  }
  return a;
}

/*If user calls only ./minls, we print these usages*/
void bad_args(){
  printf("usage: minls [ -v ] [ -p num [ -s num ] ] imagefile [ path ]\n");
  printf("Options\n");
  printf("-p part --- select partition for filesystem (default: none)\n");
  printf("-s sub --- select subpartition for filesystem (default: none)\n");
  printf("-h help --- print usage information and exit\n");
  printf("-v verbose --- increase verbosity level\n");
  exit(EXIT_FAILURE);
}

/*Print all contents in a filepath*/
void print_file_contents(loader* ldr, args a, FILE *img,
	char perm[PERMISSION_SIZE + 1]){

	int i;
	if (ldr->inod->mode & DIRECTORY_MASK){
    /*Filepath is a directory*/

    dirent *entries = (dirent *)ldr->contents;

    if (strcmp(a.filepath, ".") != 0){
      printf("%s:\n", a.filepath);
    }
    else{
      printf("/:\n");
    }

    /*Load each zone contents and printed it out*/
    inode dir_inode;

    while(!ldr->all_loaded){
      get_next_zone(ldr, img);
      for(i = 0; i < ldr->z_size / sizeof(dirent); i++){
        if(entries[i].inode != 0){
          load_dirent_inode(img, ldr, &dir_inode, entries[i].inode);
          get_permission(&dir_inode, perm);
          printf("%s %9d %s\n", perm, 
            dir_inode.size, entries[i].name);
        }
      }
    }
  }
  else{
    /*Filepath is a file*/
    get_permission(ldr->inod, perm);
    printf("%s %9d %9s\n", perm, ldr->inod->size, a.filepath);
  }
}

int main(int argc, char *argv[]){  
  args a = read_input(argc, argv);
  FILE *img = open_image(a.image);
  int pt_loc = find_pt(a, img);
  sublock sb;
  getSublock(a, img, &sb, pt_loc);

  uint32_t inode_num = 1;
  loader *ldr = prep_ldr(sb, pt_loc);
  load_ldr_inode(img, ldr, inode_num);
  char perm[PERMISSION_SIZE + 1];

  /*Print contents of current filepath*/
  findFile(a, img, ldr);
  print_file_contents(ldr, a, img, perm);

  return 0;
}

