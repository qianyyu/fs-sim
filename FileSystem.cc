#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

// String function
#include <string.h>

// C++ IO
#include <iostream>

// Open system call:
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

//for the open() permission flags
#include <sys/stat.h>
#include <sys/mount.h>
// Data Structure:
#include <vector>
#include <string>
#include <set>
#include <cstring>
#include <bitset> 
#include <unordered_map>

// My Library
#include "FileSystem.h"
#include "check.h"
#include "helper.h"

using namespace std;

/** 
 * -------------------------------------------------------------------------------------
 * Assignment:           3
 * Due Date:             December 6, 2019
 * Name:                 Qian Yu
 * CCID:                 qian6
 * Student ID:           1546189
 * Instructor:           Omid Ardakanian
 * Lab Section:          Tuesday 1700 - 1950
 * -------------------------------------------------------------------------------------
 */


/** 
 * -------------------------------------------------------------------------------------
 * Implemented Functions:
 * - void fs_mount(char *name)
 * - void fs_create(char name[5], int size)
 * - void fs_delete(char name[5])
 * - void fs_read(char name[5], int block_num)
 * - void fs_write(char name[5], int block_num)
 * - void fs_buff(uint8_t buff[1024])
 * - void fs_ls(void)
 * - void fs_resize(char name[5], int new_size)
 * - void fs_cd(char name[5])
 * - void fs_defrag(void)
 * 
 * To Do:
 * 
 * -------------------------------------------------------------------------------------
 */


Super_block *gsb = nullptr;
int   gfiledesc;
char  disk_name[100];
uint8_t current_working_directory = 127;
unordered_map<uint8_t,vector<uint8_t>> folder; // key: Folder ID, Value: List of File/Directory Inode ID
uint8_t buffer[1024];

// Last Modified: Nov 15
char* strip_space(char *str){
    char *end,*left,*right;
    int len;
    left = str;
    end = str + strlen(str) - 1;
    right = end;
 
    while(left<=end   && *left == ' ')
        left++;
    while(right>=left && *right == ' ')
        right--;

    len = (right < left) ? 0:(right-left)+1;
    left[len] = '\0';
    return left;
}



void printbincharpad(char c){
    for (int i = 7; i >= 0; --i){
        putchar( (c & (1 << i)) ? '1' : '0' );
    }
}


/*
void unique_name(char *str){
    set<string> name_list;
    for (int i = 16; i < 1024; i+=8){
        int index = i;
        string name = "";
        while(str[index]!='\0' && (index-i)<=5){
            name += str[index];
            index++;
        }
        name_list.insert(name);
    }
}
*/

bool check_same_name(char* name){
    for (int i=0; i <= 125 ; ++i){
        Inode inode = (gsb->inode)[i];
        bitset<40> bit_name_inode = ToBits_name((unsigned char* )inode.name);
        bitset<40> bit_name_add   = ToBits_name((unsigned char* )name);

        bitset<8> parent_inode(inode.dir_parent);
        parent_inode.reset(7);
        bitset<8> parent(current_working_directory);

        if(bit_name_add == bit_name_inode && parent_inode == parent)
            return false;
    }
    return true;
}


Super_block *convert_to_superblock(char *str){
    Super_block *superblock = new Super_block;
    strncpy(superblock->free_block_list,str,16);
    int index = 0;
    for (int i = 16; i < 1024; i+=8){
        Inode *this_inode = (superblock->inode)+index;
        index++;
        this_inode->name[0] = str[i];
        this_inode->name[1] = str[i+1];
        this_inode->name[2] = str[i+2];
        this_inode->name[3] = str[i+3];
        this_inode->name[4] = str[i+4];

        this_inode->used_size   = (uint8_t) (str[i+5]);

        this_inode->start_block = (uint8_t) (str[i+6]);

        this_inode->dir_parent  = (uint8_t) (str[i+7]);
    }
    return superblock;
}



int check_available_blocks(int size){
    bitset<128> space_list = ToBits_free_block_list((unsigned char* )gsb->free_block_list);

    int cur_size = 0;
    for (int i = 127; i >=0; --i){
        if(!space_list.test(i)){
            cur_size++;
        }
        else
            cur_size = 0;
        if(cur_size == size)
            return 128 - i -size;
    }
    return -1;
}


void update_bitset_to_space_list(bitset<128> space_list){

    for (int i = 0; i <=15; i++){
        bitset<8> space_list2((space_list << (8*i)).to_string());
        unsigned char c = static_cast<unsigned char>( space_list2.to_ulong() );
        gsb->free_block_list[i] = c;
    }
    // for (int i = 0; i < 16; ++i){
    //     printbincharpad(gsb->free_block_list[i]);
    //     cout << endl;
    // }    
    // cout <<" --------------------------" << endl;


}

bool check_increasing(int left, int right, bitset<128>*b){
    bitset<128> space_list = *b;

    bool in_use = false;
    for (int i = 127-left; i > (127-right) ; --i){
        if(space_list.test(i)){
            in_use = true;
            break;
        }
    }
    if(in_use)
        return false;
    else{
        for (int i = 127-left; i > (127-right) ; --i){
            space_list.set(i);
        }
        update_bitset_to_space_list(space_list);
        return true;
    }
}




void print_list(){
    cout <<" --------------------------" << endl;
    for (int i = 0; i < 16; ++i){
         printbincharpad(gsb->free_block_list[i]);
         cout << endl;
     }    
    cout <<" --------------------------" << endl;   
}

void print_inode(int i){
     printbincharpad(gsb->inode[i].name[0]);
     printbincharpad(gsb->inode[i].name[1]);
     printbincharpad(gsb->inode[i].name[2]);
     printbincharpad(gsb->inode[i].name[3]);
     printbincharpad(gsb->inode[i].name[4]);
     printbincharpad(gsb->inode[i].used_size);
     printbincharpad(gsb->inode[i].start_block);
     printbincharpad(gsb->inode[i].dir_parent);
}

void update(){
    char buf[1024];
    strncpy(buf,gsb->free_block_list,16);
    int index = 0;
    for (int i = 16; i < 1024; i+=8){
        Inode inode = gsb->inode[index++];
        buf[i]   = inode.name[0];
        buf[i+1] = inode.name[1];
        buf[i+2] = inode.name[2];
        buf[i+3] = inode.name[3];
        buf[i+4] = inode.name[4];

        buf[i+5] = (char) inode.used_size;
        buf[i+6] = (char) inode.start_block;
        buf[i+7] = (char) inode.dir_parent;
    }
    pwrite(gfiledesc, buf, 1024,0);

}


void init_map(){
    uint8_t current_directory = 127;
    vector<uint8_t> directory_list;
    vector<uint8_t> list;          // file + directory
    while(true){
        for (int i = 0; i <= 125; i++){
            Inode inode = gsb->inode[i];
            bool in_use = inode.used_size>=128?true:false;
            if(!in_use)
                continue;
            uint8_t index_parent = inode.dir_parent;
            bool is_dir = inode.dir_parent>=128?true:false;
            if(is_dir)
                index_parent -= 128;
            
            if(index_parent == current_directory){
                list.push_back((uint8_t)i);
                // cout << "add a file" << endl;
                if(is_dir)
                    directory_list.push_back((uint8_t)i);
            }
        }
        folder[current_directory] = list;
        list.clear();
        if(directory_list.empty())
            break;
        current_directory = directory_list.back();
        directory_list.pop_back();
    }
}


char* id_to_name(uint8_t id){
    return gsb->inode[(unsigned)id].name;
}

// Mounts the file system residing on the virtual disk with the specified name. 
// The mounting process involves loading the superblock of the file system,
// We say a file system is incosistent when an aribitrary number of bits 
// in its superblock are changed accidentally.
void fs_mount(char *new_disk_name){  
    char* name = strip_space(new_disk_name);
    int filedesc = open(name,O_RDWR, S_IRWXU | S_IRWXG | S_IRWXO);
    char buf[1024];
    ssize_t bytes;

    // Failure
    if(filedesc<0){
        // File does not exist
        if(errno == ENOENT){
            cerr << "Error: Cannot find disk " << name << endl;
        }
        // Other errors:
        else{
            perror("");
        }
        return;
    }

    // SuperBlock
    bytes = pread(filedesc,buf,1024,0);
    Super_block *sb = convert_to_superblock(buf);

    // for (int i = 0; i < 16; ++i){
    //    printbincharpad(sb.free_block_list[i]);
    //    cout << endl;
    // }    
    
    if(check_all(*sb,new_disk_name)){
        gsb = sb;
        gfiledesc = filedesc;
        strcpy(disk_name,new_disk_name);
        current_working_directory = 127;
        init_map();
    }
}

/**
 * Creates a new file or directory in the current working directory with the given name and the given number of blocks
 * and store the attributes in the first available inode.
 * A size of zero means that the user is creating a directory.
 * If no inode is availablee, you must print the following error to stderr.
 */
void fs_create(char name[5], int size){
    if(gsb == nullptr){
        cerr << "Error: No file system is mounted" << endl;
        return;
    }
    // cout << "Create start" << endl;
    if(!check_same_name(name)){
        cerr << "Error: File or directory "<< name << " already exists" << endl;
        return;
    }
    if(size > 127 || size < 0){
        cerr << "Error: Cannot allocate "<< size << " on " << disk_name << endl;
        return;
    }

    Inode* inode;

    int i = 0;
    for (  ; i <= 125; ++i){
        Inode *in = &(gsb->inode[i]);
        bitset<8> used_size(in->used_size);

       // The inode is in use, go to next inode
        if(used_size.test(7)){
            // There is no available inode
            if(i==125){
                cerr << "Error: Superblock in disk " << disk_name << " is full, cannot create " << name << endl;
                return;
            }
            continue;
        }
        inode = in;
        break;
    }
    // It is a directory:
    if(size == 0){
        strncpy(inode->name, name, 5);
        inode->used_size   = (uint8_t)128;  // 128 = 1000 0000
        inode->start_block = (uint8_t)  0;  //   0 = 0000 0000
        // 1 means directory,
        inode->dir_parent  = (uint8_t) 128+current_working_directory;  // 128 = 1000 0000
    }
    else{
        // update free_block_list and start_index
        int start_index;
        if((start_index = check_available_blocks(size))!=-1){
            inode->start_block     =  start_index;
            bitset<128> space_list =  ToBits_free_block_list((unsigned char* )gsb->free_block_list);

            for (int i = 127 - start_index; i > 127 - start_index - size; --i){
                space_list.set(i);
            }
            update_bitset_to_space_list(space_list);
        }
        else{
            cerr << "Error: Cannot allocate "<< size <<" on "<< disk_name << endl;
            return;
        }
        uint8_t asize      =  (uint8_t)  size + 128;
        // update size
        inode->used_size   = (uint8_t) asize;
        strncpy(inode->name, name, 5);
        // update dir_parent
        inode->dir_parent  =  current_working_directory;
    }
    update();
    init_map();
}


// Deletes the file or directory with the given name in the current working directory. If the name represents a
// directory, your program should recursively delete all files and directories within this directory. For every file
// or directory that is deleted, you must zero out the corresponding inode and data block. Do not shift other
// inodes or file data blocks after deletion.
void fs_delete(char name[5]){
    if(gsb == nullptr){
        cerr << "Error: No file system is mounted" << endl;
        return;
    }
    char name2[6];
    strncpy(name2,name,5);
    name2[5] = 0;

    vector<uint8_t> current_folder = folder[current_working_directory];

    bool unfind = true;
    for(auto element:current_folder){
        char my_name[6];
        Inode *inode = &(gsb->inode[element]);

        strncpy(my_name,(inode->name),5);
        my_name[5] = 0;

        if(strcmp(my_name,name2) == 0){
            char temp_buff[1024];
            memset(temp_buff,0,1024);

            int start_block = inode->start_block;
            int size = (inode->used_size) - 128;
            bool is_dir = (inode->dir_parent) >= 128?true:false;
            if(is_dir){
                vector<uint8_t> second_folder = folder[element];
                uint8_t temp_working_directory = current_working_directory;
                current_working_directory = element;
                for(auto second_element:second_folder){
                    fs_delete(gsb->inode[second_element].name);
                }
                current_working_directory = temp_working_directory;
            }

            bitset<128> space_list = ToBits_free_block_list((unsigned char* )gsb->free_block_list);
            for (int i = start_block; i < start_block + size; ++i){
                pwrite(gfiledesc, temp_buff, 1024, 1024*i);
                int list_index = 127 - i;
                space_list.reset(list_index);
            }
            (inode->name)[0] = 0;
            (inode->name)[1] = 0;
            (inode->name)[2] = 0;
            (inode->name)[3] = 0;
            (inode->name)[4] = 0;
            inode->used_size = 0;
            inode->start_block = 0;
            inode->dir_parent = 0;
            

            update_bitset_to_space_list(space_list);
            update();
            unfind = false;
            break;
        }
    }
    if(unfind){
        cerr << "Error: File or directory "<< name2 << " does not exist"<< endl;
    }
    update();
    init_map();
}


// Opens the file with the given name and reads the block num-th block of the file into the buffer. If no such
// file exists or the given name corresponds to a directory under the current working directory, 
void fs_read(char name[5], int block_num){
    if(gsb == nullptr){
        cerr << "Error: No file system is mounted" << endl;
        return;
    }

    // Error Handling
    if(block_num < 0){
        cerr << "Error: "<< name <<" does not have block "<< block_num << endl;
        return;
    }


    // Convert name to c_string
    char name2[6];
    strncpy(name2,name,5);
    name2[5] = 0;

    vector<uint8_t> current_folder = folder[current_working_directory];
    bool unfind = true;
    for(auto element:current_folder){
        // Convert name to c_string
        char my_name[6];
        strncpy(my_name,id_to_name(element),5);
        my_name[5] = 0;
        Inode inode = gsb->inode[element];


        if(strcmp(my_name,name2) == 0 && ((unsigned)inode.dir_parent)<128 ){
            // Error Handling
            int size = ((unsigned)inode.used_size) - 128;
            if(block_num >= size){
                cerr << "Error: "<< name2 <<" does not have block "<< block_num << endl;
                return;
            }
            // Normal Routine
            unfind = false;
            int index = (int) inode.start_block;
            index += block_num;

            pread(gfiledesc, buffer, 1024, 1024*index);
            break;
        }
    }

    // Error Handling
    if(unfind){
        cerr << "Error: File "<< name2 << " does not exist"<< endl;
        return;
    }
}
void fs_write(char name[5], int block_num){
    if(gsb == nullptr){
        cerr << "Error: No file system is mounted" << endl;
        return;
    }
    // Error Handling
    if(block_num < 0){
        cerr << "Error: "<< name <<" does not have block "<< block_num << endl;
        return;
    }

    // Convert name to c_string
    char name2[6];
    strncpy(name2,name,5);
    name2[5] = 0;

    vector<uint8_t> current_folder = folder[current_working_directory];
    bool unfind = true;
    for(auto element:current_folder){
        // Convert name to c_string
        char my_name[6];
        strncpy(my_name,id_to_name(element),5);
        my_name[5] = 0;
        Inode inode = gsb->inode[element];

        if(strcmp(my_name,name2) == 0 && (((unsigned)inode.dir_parent)<128)){
            // Error Handling
            int size = ((unsigned)inode.used_size) - 128;
            if(block_num >= size){
                cerr << "Error: "<< name2 <<" does not have block "<< block_num << endl;
                return;
            }

            // Normal Routine
            unfind = false;
            int index = (int)inode.start_block;
            index += block_num;
            pwrite(gfiledesc, buffer, 1024, 1024*index);
            break;
        }
    }

    // Error Handling
    if(unfind){
        cerr << "Error: File "<< name2 << " does not exist"<< endl;
        return;
    }
}

void fs_buff(char buff[1024]){
    if(gsb == nullptr){
        cerr << "Error: No file system is mounted" << endl;
        return;
    }
    memset(buffer,0,1024);

    for (int i = 0; i < strlen(buff); ++i){
        buffer[i] = buff[i];
    }
}


void fs_ls(void){
    if(gsb == nullptr){
        cerr << "Error: No file system is mounted" << endl;
        return;
    }
    vector<uint8_t>current_folder =  folder[current_working_directory];
    int current_size = current_folder.size()+2; 
    int parent_size;
    if(current_working_directory == 127)
        parent_size = current_size;
    else{
        Inode inode = gsb->inode[current_working_directory];
        uint8_t parent_index = inode.dir_parent - 128;
        vector<uint8_t>parent_folder = folder[parent_index];
        parent_size = parent_folder.size()+2;
    }
    char name[5];
    name[0] = '.';
    name[1] = '\0';
    printf("%-5s %3d\n", name, current_size);
    name[1] = '.';
    name[2] = '\0';
    printf("%-5s %3d\n", name, parent_size);
    name[0] = 0;
    name[1] = 0;

    for(auto element:current_folder){
        Inode inode = gsb->inode[element];
        uint8_t parent = inode.dir_parent;
        strncpy(name,inode.name,5);
        bool is_dir = parent>=128?true:false;

        if(is_dir){
            vector<uint8_t>child_folder =  folder[element];
            int folder_size = child_folder.size() + 2;
            printf("%-5.5s %3d\n", name, folder_size);
        }
        else{
            uint8_t file_size = inode.used_size;
            file_size -= 128;
            printf("%-5.5s %3d KB\n", name, (unsigned)file_size);
        }
    }

}



void fs_resize(char name[5], int new_size){
    if(gsb == nullptr){
        cerr << "Error: No file system is mounted" << endl;
        return;
    }
    char name2[6];
    strncpy(name2,name,5);
    name2[5] = 0;

    vector<uint8_t> current_folder = folder[current_working_directory];

    bool unfind = true;
    for(auto element:current_folder){
        char my_name[6];
        strncpy(my_name, id_to_name(element),5);
        my_name[5] = 0;
        Inode *inode = &(gsb->inode[element]);

        // Not a directory
        if(strcmp(my_name,name) == 0 && ((unsigned)inode->dir_parent)<128){
            unfind = false;
            int old_size = (int)inode->used_size;
            old_size -= 128;
            int start_index = (int) inode->start_block;
            bitset<128> space_list = ToBits_free_block_list((unsigned char* )gsb->free_block_list);

            // If increasing.
            if(new_size > old_size){
                // if there are space in the following block
                // left: start_index + old_size
                // right: start_index + old_size + new_size 
                if(check_increasing(start_index+old_size,start_index+new_size,&space_list)){
                    ;
                }
                // if there are not enough space in the following block
                // rebase the inode
                else{
                    int new_index;
                    if((new_index = check_available_blocks(new_size)) != -1){
                        inode->start_block     =  new_index;
                    }
                    else{
                        cerr << "Error: File "<<name2<<" cannot expand to size " << new_size<< endl;
                        return;
                    }
                    int block_index_left  = start_index;
                    int block_index_right = new_index;
                    int index2 = 127 - new_index;
                    for (int i = 127 - start_index; i > 127 - start_index - old_size; --i){
                        space_list.reset(i);
                        space_list.set(index2--);
                        char zero_buff[1024];
                        memset(zero_buff,0,1024);
                        char temp_buff[1024];
                        pread(gfiledesc,temp_buff,1024,1024*block_index_left++);
                        pwrite(gfiledesc,zero_buff,1024,1024*block_index_left++);
                        pwrite(gfiledesc,temp_buff,1024,1024*block_index_right++);
                    }

                    for (int i = 127 - new_index - old_size; i > 127 -  new_index - new_size; --i){
                        space_list.set(i);
                    }
                }

            }
            // If decreasing
            else if(new_size < old_size){
                int index = start_index+new_size;
                for (int i = 127 - start_index - new_size; i > 127 - start_index - old_size; --i){
                        char temp_buff[1024];
                        memset(temp_buff,0,1024);
                        pwrite(gfiledesc,temp_buff,1024,1024*index++);
                        space_list.reset(i);
                    }
            }
            // If equal
            else
                return;
            update_bitset_to_space_list(space_list);
            (inode->used_size)+=(new_size-old_size);
            update();
            break;
        }
    }

    if(unfind){
        cerr << "Error: File "<< name2 <<" does not exist" << endl;
    }
}




void move_forward(int new_block,int old_block,bitset<128>* space_list){
    Inode *inode = nullptr;
    for (int i = 0; i <= 125; ++i){
        int old_start_block = gsb->inode[(uint8_t)i].start_block;
        if( old_start_block == old_block){
            inode  = &(gsb->inode[(uint8_t)i]);
            break;
        }
    }
    inode->start_block = new_block;
    int size = inode->used_size - 128;
    char zero_buff[1024];
    memset(zero_buff,0,1024);
    for (int i = old_block; i < old_block + size; ++i){
        char temp_buff[1024];
        int old_space_list_index = 127 - i;
        int new_space_list_index = 127 - new_block;

        pread(gfiledesc, temp_buff, 1024, 1024*i);
        pwrite(gfiledesc, zero_buff, 1024, 1024*i);
        (*space_list).reset(old_space_list_index);

        pwrite(gfiledesc, temp_buff, 1024, 1024*new_block++);
        (*space_list).set(new_space_list_index);
    }

}




/*
 * Re-organizes the file blocks such that there is no free block between the used blocks
 * and between the superblock and the used blocks
 * To this end, starting with the file that has the smallest start block
 * the start block of every file must be moved over to the smallest numbered data block that is free
*/
void fs_defrag(void){
    if(gsb == nullptr){
        cerr << "Error: No file system is mounted" << endl;
        return;
    }
    bitset<128> space_list = ToBits_free_block_list((unsigned char* )gsb->free_block_list);
    bool free_space = false;
    int new_index;
    for (int i = 127; i >= 0; --i){
        if(!free_space && !space_list.test(i)){
            free_space = true;
            new_index = 127 - i;
        }
        if(free_space && space_list.test(i)){
            move_forward(new_index, 127-i ,&space_list);
            free_space = false;
            i=127;
        }
    }
    update_bitset_to_space_list(space_list);
    update();
    return;
}




/*
 * Changes the current working directory to a directory with the specified name
 * in the current working directory. This directory can be ., .., or any directory 
 * the user created on the disk. If the specified directory does
 * You can assume that the given name has no slash at the end.
*/
void fs_cd(char name[5]){
    if(gsb == nullptr){
        cerr << "Error: No file system is mounted" << endl;
        return;
    }
    char name2[6];
    strncpy(name2,name,5);
    name2[5] = 0;

    vector<uint8_t> current_folder = folder[current_working_directory];

    // cd . does not change anything
    if(strcmp(".",name) == 0){
        return;
    }
    // If you are in root directory, cd .. does not change anything
    if(strcmp("..",name) == 0 && current_working_directory == 127){
        return;
    }

    // cd .. <==> Go to parent directory
    if(strcmp("..",name) == 0){
        Inode inode = gsb->inode[current_working_directory];
        uint8_t parent_index = inode.dir_parent-128;
        current_working_directory = parent_index;
        return;
    }

    bool unfind = true;
    for(auto element:current_folder){
        char my_name[6];
        strncpy(my_name, id_to_name(element),5);
        my_name[5] = 0; 
        if(strcmp(my_name,name) == 0){
            // Normal Routine
            unfind = false;
            current_working_directory = element;
            break;
        }
    }

    // Error Handling
    // not exist in the current working directory (i.e., the name does not exist or it points to a file rather than a
    // directory), print the following error message to stderr:
    //          Error: Directory <directory name> does not exist
    if(unfind){
        cerr <<"Error: Directory " << name << " does not exist"<< endl;
        return;
    }
}


