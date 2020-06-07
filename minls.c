#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "mintool.h"
#include <time.h>

/*Minls lists a file or directory on the given filesystem image. 
If the optional path argument is ommitted it defaults to the 
root directory.*/

void bad_args();
args read_input(int argc, char *argv[]);

int i = 0;

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

  strcpy(perm, permissions);
}

void get_time(uint32_t ti){
	uint32_t rawtime = ti;
 	struct tm * timeinfo;
 	time_t t = rawtime;
 	timeinfo = localtime(&t);
 	asctime(timeinfo);
 	fprintf(stderr, "%s", asctime(timeinfo));
}

int main(int argc, char *argv[]){  
  
  args a = read_input(argc, argv);
  int pt_loc = 0;

  /* open the image */
  FILE *img = fopen(a.image, "r");

  /*
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
  */

  /*Get Partition Table*/

  /* get the first partition table if requested
   * and calculate location of requested partition */
  partent pt_table[4];
  if(a.p_flag){
    getPtable(img, pt_table, 0);
    pt_loc = pt_table[a.pt].lFirst * SECTOR_SZ;
  }

  /* get the subpartition table if requested and calculate location */
  if(a.s_flag){
    getPtable(img, pt_table, pt_table[a.pt].lFirst);
    pt_loc = pt_table[a.spt].lFirst * SECTOR_SZ;
  }

  /*Get Superblock*/

  sublock sb;
  getSublock(img, &sb, pt_loc);

  /*Get Loader*/

  uint32_t inode_num = 1;
  loader *ldr = prep_ldr(sb, pt_loc);
  load_inode(img, ldr, inode_num);
  char perm[PERMISSION_SIZE + 1];
  get_permission(ldr -> inod, perm);

  if (a.v_flag == 1){
  	/*Print the partition, superblock, and inode to stderr*/

  	if (a.p_flag == 1){
  		fprintf(stderr, "\nPartition:\n");
	  	fprintf(stderr, "\tbootind %u\n", pt_table -> bootind);
	  	fprintf(stderr, "\tstart_head %u\n", pt_table -> start_head);
	   	fprintf(stderr, "\tstart_sec %u\n", pt_table -> start_sec);
	   	fprintf(stderr, "\tstart_cyl %u\n", pt_table -> start_cyl);
	   	fprintf(stderr, "\ttype %u\n", pt_table -> type);
	   	fprintf(stderr, "\tend_head %u\n", pt_table -> end_head);
	   	fprintf(stderr, "\tend_sec %u\n", pt_table -> end_sec);
	   	fprintf(stderr, "\tend_cyl %u\n", pt_table -> end_cyl);
	   	fprintf(stderr, "\tlFirst %u\n", pt_table -> lFirst);
	   	fprintf(stderr, "\tsize %u\n", pt_table -> size);
  	}

   	fprintf(stderr, "\nSuperblock:\n");
   	fprintf(stderr, "Stored Fields:\n");
   	fprintf(stderr, "\tninodes: %u\n", sb.ninodes);
   	fprintf(stderr, "\ti_blocks %u\n", sb.i_blocks);
   	fprintf(stderr, "\tz_blocks %u\n", sb.z_blocks);
   	fprintf(stderr, "\tfirstdata %u\n", sb.firstdata);
   	fprintf(stderr, "\tlog_zone_size %u\n", sb.log_zone_size);
   	fprintf(stderr, "\tmax_file %u\n", sb.max_file);
   	fprintf(stderr, "\tmagic 0x%x\n", sb.magic);
   	fprintf(stderr, "\tzones %u\n", sb.zones);
   	fprintf(stderr, "\tblocksize %u\n", sb.blocksize);
   	fprintf(stderr, "\tsubversion %u\n", sb.subversion);

   	fprintf(stderr, "\nFile inode:\n");
   	fprintf(stderr, "\tuint16_t mode 0x%x (%s)\n", 
      ldr -> inod -> mode, perm);
   	fprintf(stderr, "\tuint16_t links %u\n", ldr -> inod -> links);
   	fprintf(stderr, "\tuint16_t uid %u\n", ldr -> inod -> uid);
   	fprintf(stderr, "\tuint16_t gid %u\n", ldr -> inod -> gid);
   	fprintf(stderr, "\tuint32_t size %u\n", ldr -> inod -> size);
   	fprintf(stderr, "\tuint32_t atime %u --- ", ldr -> inod -> atime);
   	get_time(ldr -> inod -> atime);
   	fprintf(stderr, "\tuint32_t mtime %u --- ", ldr -> inod -> mtime);
   	get_time(ldr -> inod -> mtime);
   	fprintf(stderr, "\tuint32_t ctime %u --- ", ldr -> inod -> ctime);
   	get_time(ldr -> inod -> ctime);
   	fprintf(stderr, "\n\tDIRECT_ZONES\n");
   	for (i = 0; i < DIRECT_ZONES; i++){
   	  fprintf(stderr, "\t\tzone[%i] = %u\n", i, ldr -> inod -> zone[i]);
   	}
    fprintf(stderr, "\tuint32_t indirect %u\n", ldr -> inod -> indirect);
    fprintf(stderr, "\tuint32_t double %u\n", ldr -> inod -> two_indirect);
  }
  
  /*Print contents of current filepath*/

  findFile(img, a.filepath, ldr);

  if (ldr -> inod -> mode & DIRECTORY_MASK){
    /*Is a directory*/
    /*print all files info*/

    //search_dir and search zone (refactor both of them)
    dirent *entries = (dirent *)ldr->contents;

    fprintf(stderr, "\n/:\n");
    for(i = 0; i < ldr->z_size / sizeof(dirent); i++){
      if(entries[i].inode != 0){
        load_inode(img, ldr, entries[i].inode);
        get_permission(ldr -> inod, perm);
        fprintf(stderr, "%s %d %s\n", perm, 
          ldr -> inod -> size, entries[i].name);
      }
    }
  }
  else{
    /*Is a file*/
    /*Print mode, size, and filename*/
    get_permission(ldr -> inod, perm);
    printf("%s %d %s\n", perm, ldr -> inod -> size, a.filepath);
  }

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
    else{
    	a.filepath = ".";
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

