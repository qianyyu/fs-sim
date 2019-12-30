#include <stdio.h>
#include <stdint.h>





/**
 * @param: used_size:
 *		1. first bit <-> inode's index
 *		2. 2-8th bits <-> file size[0 - 127]
 * @param: starter_block:
 *		1. the index of the first block[-255]
 * @param: dir_parent:
 *		1. 1st bit    <->    inode's type [1 = pertains directory, 0 = pertains to a file]
 *  	2. other bits <->    index of the parent inode in the super block[0-125]
*/
typedef struct {
	char name[5];        // Name of the file or directory
	uint8_t used_size;   // Inode state and the size of the file or directory
	uint8_t start_block; // Index of the start file block
	uint8_t dir_parent;  // Inode mode and the index of the parent inode
} Inode;



// A super block has 128 bolcks, Size: 128kb
// The first block[1kb] contains: 1. free-space list[128 bits = 16 bytes]
//								  2. 126 index nodes	 
typedef struct {
	char free_block_list[16];
	Inode inode[126];
} Super_block;

void fs_mount(char *new_disk_name);
void fs_create(char name[5], int size);
void fs_delete(char name[5]);
void fs_read(char name[5], int block_num);
void fs_write(char name[5], int block_num);
void fs_buff(char buff[1024]);
void fs_ls(void);
void fs_resize(char name[5], int new_size);
void fs_defrag(void);
void fs_cd(char name[5]);
void fs_free();


// debug
void print_list();
void print_inode(int i);
