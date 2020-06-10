#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "mintool.h"

void *safe_malloc(int32_t size){
  void *res = malloc(size);
  if(res == NULL){
    perror("malloc");
    exit(EXIT_FAILURE);
  }
  return res;
}

/* get partition table from img into pt, which should be a 4 element array of 
 * partition table entries */
void getPtable(FILE *img, partent *pt, int ptStart){
  uint16_t sig = 0;

  /* the partition is ptStart sectors away from the start of the file
   * multiply by SECTOR_SZ to convert from sectors to bytes, and add the offset
   * PTABLE_ADDR (0x1BE) to get the partition table itself */

  int ptableLoc = ptStart * SECTOR_SZ + PTABLE_ADDR;
  if(fseek(img, ptableLoc, SEEK_SET) < 0){
    perror("fseek");
    exit(EXIT_FAILURE);
  }

  /* read all four partition table entries into pt, totaling 64 bytes */
  if(fread(pt, sizeof(struct partent), PTABLE_SZ, img) < PTABLE_SZ){
    perror("fread");
    exit(EXIT_FAILURE);
  }

  /* while we're at it, check the signature */
  if(fread(&sig, sizeof(uint16_t), 1, img) < 1){
    perror("fread");
    exit(EXIT_FAILURE);
  }

  if(sig != PTABLE_SIG){
    printf("Partition table invalid - signature: %d\n", sig);
    exit(EXIT_FAILURE);
  }

  /*If not a minix type, stop*/
  if (pt -> type != MINIX_PTYPE){
    printf("type number: %d\n", pt -> type);
    printf("Not of type MINIX_PTYPE\n");
    exit(EXIT_FAILURE);
  }
}

void getSublock(args a, FILE *img, sublock *sb, int ptStart){
  int sbLoc = ptStart + SBLOCK_ADDR;
  if(fseek(img, sbLoc, SEEK_SET) < 0){
    perror("fseek");
    exit(EXIT_FAILURE);
  }
  if(fread(sb, sizeof(sublock), 1, img) < 1){
    perror("fread");
    exit(EXIT_FAILURE);
  }
  if(a.v_flag){
    print_sb(*sb);
  }
}

/* the one and only function for creating and initializing a loader struct */
loader *prep_ldr(sublock sb, int32_t pt_loc){
  loader *ldr = safe_malloc(sizeof(loader));
  ldr->inod = safe_malloc(sizeof(inode));
  ldr->current_zone = 0;
  ldr->z_size = sb.blocksize << sb.log_zone_size;
  ldr->i1.z_idx = 0;
  ldr->i1.zones = safe_malloc(ldr->z_size);
  ldr->i2.z_idx = 0;
  ldr->i2.zones = safe_malloc(ldr->z_size);
  ldr->contents = safe_malloc(ldr->z_size);
  ldr->inodes_loc = pt_loc + (2 + sb.i_blocks + sb.z_blocks) * sb.blocksize;
  ldr->pt_loc = pt_loc;
  ldr->all_loaded = 0;
  ldr->found = 0;
  ldr->empty_count = 0;
  return ldr;
}

void read_zone(FILE *img, int32_t addr, loader *ldr, void *tgt){
  if(fseek(img, addr, SEEK_SET) < 0){
    perror("fseek");
    exit(EXIT_FAILURE);
  }
  if(fread(tgt, ldr->z_size, 1, img) < 1){
    perror("fread");
    exit(EXIT_FAILURE);
  }
}

/* Specifically for updating the inode in the loader struct. Must be called
 * before searching through or printing the contents of the associated file */
void load_ldr_inode(FILE *img, loader *ldr, uint32_t inode_num){
  int32_t addr = ldr->inodes_loc + (inode_num - 1)  * sizeof(inode);
  if(fseek(img, addr, SEEK_SET) < 0){
    perror("fseek");
    exit(EXIT_FAILURE);
  }
  if(fread(ldr->inod, sizeof(inode), 1, img) < 1){
    perror("fread a");
    exit(EXIT_FAILURE);
  }
  read_zone(img, ldr->inod->indirect * ldr->z_size, ldr, ldr->i1.zones);
  read_zone(img, ldr->inod->two_indirect * ldr->z_size, ldr, ldr->i2.zones);
  ldr->current_zone = ldr->all_loaded = ldr->found = 0;
}

/* Gets a zone from an indirect zone determined by the loader */
void get_next_indirect(loader *ldr, FILE *img){
  int addr;
  ldr->empty_count = 0;

  /* loop through whole file or until non-hole zone is found */
  while (ldr->current_zone * ldr->z_size < ldr->inod -> size){

    /* Loop through indirect zone contents */
    while (ldr->i1.z_idx < ldr->z_size / sizeof(int32_t)){

      /* if not a hole */
      if (ldr->i1.zones[ldr->i1.z_idx] != 0){
        addr = ldr->pt_loc + ldr->i1.zones[ldr->i1.z_idx] * ldr->z_size;
        read_zone(img, addr, ldr, (void *)ldr->contents);
        ldr->i1.z_idx++;
        ldr->current_zone++;
        return;
      }

      /* else continue searching and increment blank zones to print */
      ldr->empty_count++;
      ldr->i1.z_idx++;
      ldr->current_zone++;
    }

    /* Loop through double indirect zone contents and load next indirect zone
     * into the loader struct*/
    while (ldr->i2.z_idx < ldr->z_size / sizeof(int32_t)){
      if (ldr->i2.zones[ldr->i2.z_idx] != 0){
        addr = ldr->pt_loc + ldr->i2.zones[ldr->i2.z_idx] * ldr->z_size;
        read_zone(img, addr, ldr, (void *)ldr->i1.zones);
        ldr->i1.z_idx = 0;
        break;
      }
      ldr->empty_count += ldr->z_size / sizeof(int32_t);
      ldr->current_zone += ldr->z_size / sizeof(int32_t);
      ldr->i2.z_idx++;
    }
  }
  ldr->all_loaded = 1;
}

/* loads the next non-hole into the loader struct, increments empty hole
 * counter if holes are found. These will be printed elsewhere in the
 * case of minget */
void get_next_zone(loader *ldr, FILE *img){
  int addr;
  if (ldr->current_zone < DIRECT_ZONES){

    /* Calculate address of non-hole zone and read it in */
    if(ldr->inod->zone[ldr->current_zone] != 0){
      addr = ldr->pt_loc + ldr->inod->zone[ldr->current_zone] * ldr->z_size;
      read_zone(img, addr, ldr, (void *)ldr->contents);
    }
    else if(ldr->current_zone * ldr->z_size < ldr->inod->size){
      ldr->empty_count++;
    }
    ldr->current_zone++;
  }

  /* direct zones have run out */
  else{
    get_next_indirect(ldr, img);
  }

  /* signal when file all file contents are fully loaded */
  if(ldr->current_zone * ldr->z_size > ldr->inod->size){
    ldr->all_loaded = 1;
  }
}

/* look through a zone for a dirent with filename == tok 
 * and return its inode number */
uint32_t search_zone(FILE *img, char *tok, loader *ldr){
  int i;
  dirent *entries = (dirent *)ldr->contents;
  for(i = 0; i < ldr->z_size / sizeof(dirent); i++){
    if(entries[i].inode == 0){
      continue;
    }
    if(strcmp(tok, entries[i].name) == 0){
      ldr->found = 1;
      return entries[i].inode;
    }
  }
  return 0;
}

/* Gets the first zone of a directory, treats it as an array of directory
 * entries, and then returns the inode number of a particular index */
uint32_t dirent_inode_val(FILE *img, loader *ldr, int entry){
    get_next_zone(ldr, img);
    dirent *entries = (dirent *)ldr->contents;
    return entries[entry].inode;
}

uint32_t search_dir(FILE *img, char *tok, loader *ldr){
  uint32_t inode_num;
  ldr->found = 0;

  /* Special cases, present and parent directory */
  if(strcmp(tok, ".") == 0){
    return dirent_inode_val(img, ldr, 0);
  }
  if(strcmp(tok, "..") == 0){
    return dirent_inode_val(img, ldr, 1);
  }

  if(!(ldr->inod->mode & DIRECTORY)){
    fprintf(stderr, "This isn't a directory\n");
    exit(EXIT_FAILURE);
  }

  while(!ldr->all_loaded){
    get_next_zone(ldr, img);
    if((inode_num = search_zone(img, tok, ldr))){
      break;
    }
  }
  if(!ldr->found){
    printf("File not found\n");
    exit(EXIT_FAILURE);
  }
  return inode_num;
}

/* copies the filepath and splits it into individual filenames. Searches the
 * current directory for a token, then loads that directory until the search
 * either fails or the last token is found. The function returns with the
 * loader ready to read the contents of that file/directory */
void findFile(args a, FILE *img, loader *ldr){
  uint32_t inode_num;
  char *tokenized = safe_malloc(strlen(a.filepath));
  strcpy(tokenized, a.filepath);
  char *tok = strtok(tokenized, "/");
  do{
    inode_num = search_dir(img, tok, ldr);
    load_ldr_inode(img, ldr, inode_num);
  }while((tok = strtok(NULL, "/")) != NULL);
  if(a.v_flag){
    print_inode(ldr);
  }
  free(tokenized);
}

void findRoot(FILE *img, loader *ldr){
  load_ldr_inode(img, ldr, 1);
  if(!(ldr->inod->mode & DIRECTORY)){
    printf("Failed to find root directory\n");
    exit(EXIT_FAILURE);
  }
}

/* creates and fills an args struct with partition, subpartition, and verbose
 * flag information. optind is incremented after -p and -s because numbers
 * are expected after each flag */
args parse_flags(int argc, char *argv[]){
  int opt = 0;
  args a = {0, 0, 0, 0, 0, 0, NULL, NULL, NULL};
  while ((opt = getopt(argc, argv, "vps")) != -1){
    a.has_flags = 1;
    switch(opt){
      case('p'):
        a.pt = (int)strtol(argv[optind], NULL, 10);
        a.p_flag = 1;
        optind++;
        break;
      case('s'):
        a.spt = (int)strtol(argv[optind], NULL, 10);
        a.s_flag = 1;
        optind++;
        break;
      case('v'):
        a.v_flag = 1;
        break;
      default:
        exit(EXIT_FAILURE);
    }
  }
  if(a.s_flag && !a.p_flag){
    fprintf(stderr, "Must have partition to have subpartition\n");
    exit(EXIT_FAILURE);
  }
  return a;
}

FILE *open_image(char *image_path){
  FILE *img = fopen(image_path, "r");
  if(img == NULL){
    perror("fopen");
    exit(EXIT_FAILURE);
  }
  return img;
}

/* Always called, whether a destination is specified or not. /dev/stdout
 * is the default destination if none is provided */
FILE *open_dest(char *dest_path){
  FILE *dest;
  if(dest_path != NULL){
    dest = fopen(dest_path, "w");
    if(dest == NULL){
      perror("fopen");
      exit(EXIT_FAILURE);
    }
  }
  else{
    dest = fopen("/dev/stdout", "w");
  }
  return dest;
}

/* loads and prints partition and subpartition information depending on
 * what is specified by the values set in args a */
int find_pt(args a, FILE *img){
  partent pt_table[4];
  int pt_loc;

  if(a.p_flag){
    getPtable(img, pt_table, 0);
    pt_loc = pt_table[a.pt].lFirst * SECTOR_SZ;
    if(a.v_flag){
      print_pt(pt_table);
    }
  }

  if(a.s_flag){
    getPtable(img, pt_table, pt_table[a.pt].lFirst);
    pt_loc = pt_table[a.spt].lFirst * SECTOR_SZ;
    if(a.v_flag){
      print_pt(pt_table);
    }
  }
  return pt_loc;
}

void print_pt(partent pt_table[4]){
	fprintf(stderr, "\nPartition:\n");
	fprintf(stderr, "  bootind %11u\n", pt_table->bootind);
	fprintf(stderr, "  start_head %11u\n", pt_table->start_head);
 	fprintf(stderr, "  start_sec %11u\n", pt_table->start_sec);
 	fprintf(stderr, "  start_cyl %11u\n", pt_table->start_cyl);
 	fprintf(stderr, "  type %11u\n", pt_table->type);
 	fprintf(stderr, "  end_head %11u\n", pt_table->end_head);
 	fprintf(stderr, "  end_sec %11u\n", pt_table->end_sec);
 	fprintf(stderr, "  end_cyl %11u\n", pt_table->end_cyl);
 	fprintf(stderr, "  First %11u\n", pt_table->lFirst);
 	fprintf(stderr, "  size %11u\n", pt_table->size);
}

void print_sb(sublock sb){
	fprintf(stderr, "\nSuperblock Contents:\n");
 	fprintf(stderr, "Stored Fields:\n");
 	fprintf(stderr, "  ninodes %12u\n", sb.ninodes);
 	fprintf(stderr, "  i_blocks %11u\n", sb.i_blocks);
 	fprintf(stderr, "  z_blocks %11u\n", sb.z_blocks);
 	fprintf(stderr, "  firstdata %10u\n", sb.firstdata);
 	fprintf(stderr, "  log_zone_size %6u (zone size: %d)\n", 
    sb.log_zone_size, sb.blocksize << sb.log_zone_size);
 	fprintf(stderr, "  max_file %11u\n", sb.max_file);
 	fprintf(stderr, "  magic %#14x\n", sb.magic);
 	fprintf(stderr, "  zones %14u\n", sb.zones);
 	fprintf(stderr, "  blocksize %10u\n", sb.blocksize);
 	fprintf(stderr, "  subversion %9u\n", sb.subversion);

  fprintf(stderr, "Computed Fields:\n");
  fprintf(stderr, "  version%13d\n", MINIX_VERSION);
  fprintf(stderr, "  firstImap%11d\n", FIRSTIMAP);
  fprintf(stderr, "  firstZmap%11d\n", 
    FIRSTIMAP + sb.i_blocks);
  fprintf(stderr, "  firstIblock%9d\n", 
    FIRSTIMAP + sb.i_blocks + sb.z_blocks);
  fprintf(stderr, "  zonesize%12d\n", 
    sb.blocksize << sb.log_zone_size);
  fprintf(stderr, "  ptrs_per_zone%7lu\n", 
    (sb.blocksize << sb.log_zone_size) / sizeof(uint32_t));
  fprintf(stderr, "  ino_per_block%7d\n", (int)
    sb.blocksize / INODE_SZ);
  fprintf(stderr, "  wrongended%10d\n", 0);
  fprintf(stderr, "  fileent_size%8d\n", DIRENT_SZ);
  fprintf(stderr, "  max_filename%8d\n", MAX_FILENAME_SZ);
  fprintf(stderr, "  ent_per_zone%8d\n", 
    sb.blocksize / DIRENT_SZ);
}

void print_inode(loader *ldr){
  char perm[PERMISSION_SIZE + 1];
  int i;
  get_permission(ldr->inod, perm);
	fprintf(stderr, "\nFile inode:\n");
 	fprintf(stderr, "  unsigned short mode %#14x\t(%s)\n", 
    ldr->inod->mode, perm);
 	fprintf(stderr, "  unsigned short links %13u\n", ldr->inod->links);
 	fprintf(stderr, "  unsigned short uid %15u\n", ldr->inod->uid);
 	fprintf(stderr, "  unsigned short gid %15u\n", ldr->inod->gid);
 	fprintf(stderr, "  uint32_t  size %14u\n", ldr->inod->size);
 	fprintf(stderr, "  uint32_t  atime %13u --- ", ldr->inod->atime);
 	get_time(ldr->inod->atime);
 	fprintf(stderr, "  uint32_t  mtime %13u --- ", ldr->inod->mtime);
 	get_time(ldr->inod->mtime);
 	fprintf(stderr, "  uint32_t  ctime %13u --- ", ldr->inod->ctime);
 	get_time(ldr->inod->ctime);
 	fprintf(stderr, "\n  Direct zones:\n");
 	for (i = 0; i < DIRECT_ZONES; i++){
 	  fprintf(stderr, "\t      zone[%i]   =%11u\n", i, 
      ldr->inod->zone[i]);
 	}
  fprintf(stderr, "   uint32_t indirect   =%11u\n", ldr->inod->indirect);
  fprintf(stderr, "   uint32_t double   =%13u\n", ldr->inod->two_indirect);
}

void get_permission(inode* i, char* perm){
  char permissions[PERMISSION_SIZE + 1];
  memset(permissions, '-', PERMISSION_SIZE);
  permissions[PERMISSION_SIZE] = 0;

  if (i->mode & DIRECTORY_MASK){
    permissions[DIRECTORY_INDEX] = 'd';
  }
  if (i->mode & OWNER_READ_MASK){
    permissions[OWNER_READ] = 'r';
  }
  if (i->mode & OWNER_WRITE_MASK){
    permissions[OWNER_WRITE] = 'w';
  }
  if (i->mode & OWNER_EXECUTE_MASK){
    permissions[OWNER_EXECUTE] = 'x';
  }
  if (i->mode & GROUP_READ_MASK){
    permissions[GROUP_READ] = 'r';
  }
  if (i->mode & GROUP_WRITE_MASK){
    permissions[GROUP_WRITE] = 'w';
  }
  if (i->mode & GROUP_EXECUTE_MASK){
    permissions[GROUP_EXECUTE] = 'x';
  }
  if (i->mode & OTHER_READ_MASK){
    permissions[OTHER_READ] = 'r';
  }
  if (i->mode & OTHER_WRITE_MASK){
    permissions[OTHER_WRITE] = 'w';
  }
  if (i->mode & OTHER_EXECUTE_MASK){
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

