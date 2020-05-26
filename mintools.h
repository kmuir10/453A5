#define eprintf(format,...);{\
    if(env){\
      fprintf(stderr,format,__VA_ARGS__);\
    }\
  }
  
struct superblock{
	/* on disk. These fields and orientation are non–negotiable */
	uint32_t ninodes; /* number of inodes in this filesystem */
	uint16_t pad1; /* make things line up properly */
	int16_t i_blocks; /* # of blocks used by inode bit map */
	int16_t z_blocks; /* # of blocks used by zone bit map */
	uint16_t firstdata; /* number of first data zone */
	int16_t log_zone_size; /* log2 of blocks per zone */
	int16_t pad2; /* make things line up again */
	uint32_t max file; /* maximum file size */
	uint32_t zones; /* number of zones on disk */
	int16_t magic; /* magic number */
	int16_t pad3; /* make things line up again */
	uint16_t blocksize; /* block size in bytes */
	uint8_t subversion; /* filesystem sub–version */
}