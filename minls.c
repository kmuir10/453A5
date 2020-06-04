#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "mintool.h"

/*Minls lists a file or directory on the given filesystem image. If the optional
path argument is ommitted it defaults to the root directory.*/

void bad_args();
args read_input(int argc, char *argv[]);

void get_permission(inode* i, char* perm){

  char permissions[PERMISSION_SIZE + 1];
  memset(permissions, '-', PERMISSION_SIZE);
  permissions[PERMISSION_SIZE] = 0;

  if (i -> mode & DIRECTORY_MASK){
    permissions[DIRECTORY_INDEX] = 'd';
  }
  if (i -> mode & OWNER_READ_MASK){
    permissions[OWNER_READ] = 'r';
  }
  if (i -> mode & OWNER_WRITE_MASK){
    permissions[OWNER_WRITE] = 'w';
  }
  if (i -> mode & OWNER_EXECUTE_MASK){
    permissions[OWNER_EXECUTE] = 'x';
  }
  if (i -> mode & GROUP_READ_MASK){
    permissions[GROUP_READ] = 'r';
  }
  if (i -> mode & GROUP_WRITE_MASK){
    permissions[GROUP_WRITE] = 'w';
  }
  if (i -> mode & GROUP_EXECUTE_MASK){
    permissions[GROUP_EXECUTE] = 'x';
  }
  if (i -> mode & OTHER_READ_MASK){
    permissions[OTHER_READ] = 'r';
  }
  if (i -> mode & OTHER_WRITE_MASK){
    permissions[OTHER_WRITE] = 'w';
  }
  if (i -> mode & OTHER_EXECUTE_MASK){
    permissions[OTHER_EXECUTE] = 'x';
  }

  printf("Permissions: %s\n", permissions);

  strcpy(perm, permissions);
}

int main(int argc, char *argv[]){  
  
  args a = read_input(argc, argv);

  /* open the image */
  FILE *img = fopen(a.image, "r");

  if (img == NULL){
    perror("No such file\n");
    exit(EXIT_FAILURE);
  }
  else{
    printf("Such file\n");
    printf("v_flag: %d\n", a.v_flag);
    printf("p_flag: %i\n", a.p_flag);
    printf("s_flag: %i\n", a.s_flag);
    printf("num_of_partitions: %i\n", a.pt);
    printf("num_of_sub_partitions: %i\n", a.spt);
  }

  /*Get Partition Table*/

  struct partent *pt = safe_malloc(sizeof(partent));
  getPtable(img, pt, a.pt);

  struct sublock *sb = safe_malloc(sizeof(sublock));
  getSublock(img, sb, a.pt);

  struct loader *ldr = safe_malloc(sizeof(loader));
  uint32_t inode_num = 0;
  load_inode(img, ldr, inode_num);
  char perm[PERMISSION_SIZE + 1];
  get_permission(ldr -> inod, perm);

  if (a.v_flag == 1){
  	/*Print the partition, superblock, and inode to stderr*/
  	fprintf(stderr, "Partition:\n");
  	fprintf(stderr, "bootind: %u\n", pt -> bootind);
  	fprintf(stderr, "start_head: %u\n", pt -> start_head);
   	fprintf(stderr, "start_sec: %u\n", pt -> start_sec);
   	fprintf(stderr, "start_cyl: %u\n", pt -> start_cyl);
   	fprintf(stderr, "type: %u\n", pt -> type);
   	fprintf(stderr, "end_head: %u\n", pt -> end_head);
   	fprintf(stderr, "end_sec: %u\n", pt -> end_sec);
   	fprintf(stderr, "end_cyl: %u\n", pt -> end_cyl);
   	fprintf(stderr, "lFirst: %u\n", pt -> lFirst);
   	fprintf(stderr, "size: %u\n", pt -> size);

   	fprintf(stderr, "Superblock:\n");
   	fprintf(stderr, "ninodes: %u\n", sb -> ninodes);
   	fprintf(stderr, "pad1: %u\n", sb -> pad1);
   	fprintf(stderr, "i_blocks: %u\n", sb -> i_blocks);
   	fprintf(stderr, "z_blocks: %u\n", sb -> z_blocks);
   	fprintf(stderr, "firstdata: %u\n", sb -> firstdata);
   	fprintf(stderr, "log_zone_size: %u\n", sb -> log_zone_size);
   	fprintf(stderr, "pad2: %u\n", sb -> pad2);
   	fprintf(stderr, "max_file: %u\n", sb -> max_file);
   	fprintf(stderr, "zones: %u\n", sb -> zones);
   	fprintf(stderr, "magic: %u\n", sb -> magic);
   	fprintf(stderr, "pad3: %u\n", sb -> pad3);
   	fprintf(stderr, "blocksize: %u\n", sb -> blocksize);
   	fprintf(stderr, "subversion: %u\n", sb -> subversion);

   	fprintf(stderr, "Inode:\n");
   	fprintf(stderr, "mode: %s\n", perm);
   	fprintf(stderr, "links: %u\n", ldr -> inod -> links);
   	fprintf(stderr, "uid: %u\n", ldr -> inod -> uid);
   	fprintf(stderr, "gid: %u\n", ldr -> inod -> gid);
   	fprintf(stderr, "size: %u\n", ldr -> inod -> size);
   	fprintf(stderr, "atime: %u\n", ldr -> inod -> atime);
   	fprintf(stderr, "mtime: %u\n", ldr -> inod -> mtime);
   	fprintf(stderr, "ctime: %u\n", ldr -> inod -> ctime);
   	fprintf(stderr, "DIRECT_ZONES: %u\n", DIRECT_ZONES);
   	int i = 0;
   	for (i = 0; i < DIRECT_ZONES; i++){
   		fprintf(stderr, "zone[%i]: %u\n", i, ldr -> inod -> zone[i]);
   	}
    fprintf(stderr, "indirect: %u\n", ldr -> inod -> indirect);
    fprintf(stderr, "two_indirect: %u\n", ldr -> inod -> two_indirect);
    fprintf(stderr, "unused: %u\n", ldr -> inod -> unused);
  }
  

  /*Get Superblock contents*/

  /*Get File Inode contents*/

  return 0;
}

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
  }
  return a;
}

void bad_args(){
      printf("usage: minls [ -v ] [ -p num [ -s num ] ] imagefile [ path ]\n");
      printf("Options\n");
      printf("-p part --- select partition for filesystem (default: none)\n");
      printf("-s sub --- select subpartition for filesystem (default: none)\n");
      printf("-h help --- print usage information and exit\n");
      printf("-v verbose --- increase verbosity level\n");
      exit(EXIT_FAILURE);
}

