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

using namespace std;




/* Helper Functions: -------------------------------------------------------------------------------------*/

// Converting a 16-byes char array to a 128-bit bitset
bitset<128> ToBits_free_block_list(unsigned char* byte)
{   
    bitset<128> list(0x0);
    for(int i=0;i<=15;i++){
        bitset<128> element(byte[i]);
        list = list | element;
        if(i!=15)
            list <<= 8;
    }
    return list;
}


// Converting a 5-byte char array to a 40-bit bitset
bitset<40> ToBits_name(unsigned char* byte)
{   
    bitset<40> list(0x0);
    for(int i=0;i<=4;i++){
        bitset<40> element(byte[i]);
        list = list | element;
        if(i!=4)
            list <<= 8;
    }
    return list;
}


/* Helper Functions: -------------------------------------------------------------------------------------*/


// Blocks that are marked free in the free-space list cannot be 
// allocated to any file. Similarly, blocks marked in use in the 
// free-space list must be allocated to exactly one file.
bool condition1(Super_block sb){
    unsigned char free_block_list[16];
    strncpy((char*)free_block_list, sb.free_block_list, 16);
    bitset<128> free_list = ToBits_free_block_list(free_block_list);

    // cout << "free_list: " << free_list << endl;
    bitset<128> new_free_list(0x0);
    new_free_list.set(127);
    // cout << "new_free_list: " << new_free_list << endl;

    // make sure that the super block is in-use.
    if(!free_list.test(127)){
        return false;
    }

    for (int i = 0; i <= 125; ++i){
        Inode inode = sb.inode[i];
        int used_size = inode.used_size;
        bool state = (used_size >= 128)? true :false;


        /*
        cout << (unsigned)inode.used_size << endl;
        cout << (unsigned)inode.start_block << endl;
        cout << (unsigned)inode.dir_parent << endl;

        cout << "---------" << endl;
        */
        

        // If it is free, skip the inode.
        if(state){
            uint8_t size = used_size-128;
            uint8_t start_index = inode.start_block;
            for (int j = start_index; j < start_index+size; ++j){
                int index = 127-j;
                if(new_free_list.test(index)){
                    return false;
                }
                new_free_list.set(index);
                /*
                int byte_index = i/8;
                int bit_index = i%8;
                int bit = (free_block_list[byte_index] & (1 << (7-it_index)))? 1 : 0;
                if(bit == 0){
                    return false;
                } 
                */
            }
        }
    }

    if(new_free_list == free_list)
        return true;
    else{
        return false;
    }
}


// The name of every file/directory must be unique in each directory.
// May not stable
bool condition2(Super_block sb){
    vector<bitset<40>> name_list;
    unordered_map<uint8_t, vector<bitset<40>> > umap;
    for (int i = 0; i <= 125; ++i){
        Inode inode = sb.inode[i];

        bool in_use = inode.used_size >= 128? true:false;
        bool is_dir = inode.dir_parent>= 128?true:false;


        if(in_use){
            uint8_t parent_index;
            if(is_dir)
                parent_index = (unsigned)inode.dir_parent-128;
            else
                parent_index = (unsigned)inode.dir_parent;
            if(umap.find(parent_index) == umap.end()){
                name_list.clear();
                name_list.push_back(ToBits_name((unsigned char* )inode.name));
                char name[5];
                name[0] = '.';
                name[1] = 0;
                name_list.push_back(ToBits_name((unsigned char* )name));
                name[1] = '.';
                name[2] = 0;
                name_list.push_back(ToBits_name((unsigned char* )name));
                umap[parent_index] = name_list;
            }
            else{
                auto it = find(umap[parent_index].begin(),umap[parent_index].end(),ToBits_name((unsigned char* )inode.name));
                if(it!= umap[parent_index].end()){
                    return false;
                }
                umap[parent_index].push_back(ToBits_name((unsigned char*)(inode.name)));
            }
        }
    }
    return true;
}



// If the state of an inode is free, all bits in this inode must be zero. Otherwise, 
// the name attribute stored in the inode must have at least one bit that is not zero.
bool condition3(Super_block sb){
    for (int i = 0; i <= 125; ++i){
        Inode inode = sb.inode[i];

        bool cond_name = ((unsigned)inode.name[0] == 0);
        cond_name &= ((unsigned)inode.name[1] == 0);
        cond_name &= ((unsigned)inode.name[2] == 0);
        cond_name &= ((unsigned)inode.name[3] == 0);
        cond_name &= ((unsigned)inode.name[4] == 0);

        bool cond_size = (inode.used_size == 0);
        bool cond_start = (inode.start_block == 0);
        bool cond_parent = (inode.dir_parent == 0);

        bool state =  inode.used_size>=128?true:false;
        // It is free
        if(!state){
            if(cond_name && cond_size && cond_start && cond_parent ){
                // cout << i << " failed" << endl;
                // cout <<"bit_name: "<< bit_name.to_string()<< endl;
                // cout <<"used_size: "<< used_size.to_string()<< endl;
                // cout <<"dir_parent: "<< dir_parent.to_string()<< endl;
                ;
            }
            else{
                return false;
            }
        }
        // It is in-use
        else{
            if(cond_name){
                // cout << i << " failed" << endl;
                // cout << bit_name.to_string()<< endl;
                return false;
            }
        }
    }
    return true;
}

// The start block of every inode that is marked as a file must have a value between 1 and 127
// inclusive.
bool condition4(Super_block sb){
    for (int i = 0; i <= 125; ++i){
        Inode inode = sb.inode[i];

        bool in_use = inode.used_size >= 128? true:false;
        bool is_file = inode.dir_parent<128?true:false;
        int start_block = (unsigned)inode.start_block;

        // Check if it is a file:
        // bsize.test(7) ==> It is in use
        // (!bdir.test(7)) ==> It pertains to a file not a directory
        if(in_use && is_file){
            if(start_block < 1 || start_block > 127)
                return false;
        }
    }
    return true;
}


// The _size_ and _start block_ of an inode that is marked as a directory must be zero.
bool condition5(Super_block sb){
    for (int i = 0; i <= 125; ++i){
        Inode inode = sb.inode[i];

        bool in_use = inode.used_size >= 128? true:false;
        bool is_dir = inode.dir_parent >= 128?true:false;
        int start_block = (unsigned)inode.start_block;

        if(in_use && is_dir){
            if ((((unsigned)inode.used_size)!=128) || (start_block != 0))
                return false;
        }
    }
    return true;
}


// For every inode, the index of its parent inode cannot be 126. Moreover, 
// if the index of the parent inode is between 0 and 125 inclusive, then 
// the parent inode must be in use and marked as a directory.
bool condition6(Super_block sb){
    for (int i = 0; i <= 125; ++i){
        Inode inode = sb.inode[i];

        bitset<8> used_size(inode.used_size);
        if(!used_size.test(7))
            continue;
        bitset<8> start_block(inode.start_block);
        bitset<8> bdir(inode.dir_parent);
        uint8_t dir_parent = inode.dir_parent;

        bool is_dir = bdir.test(7);

        if(is_dir)
            dir_parent -= 128;

        if(dir_parent==126)
            return false;

        if(dir_parent==127)
            continue;

        // Check the parent inode must be in use and marked as a directory.
        Inode parent_inode = sb.inode[dir_parent];
        bitset<8> parent_size(parent_inode.used_size);
        bitset<8> parent_dir(parent_inode.dir_parent);

        // If it is not in use, return false:
        if(!parent_size.test(7))
            return false;

        // If it is not directory, return false:
        if(!parent_dir.test(7))
            return false;
    }
    return true;
}



bool check_all(Super_block sb, char *disk_name){
    if(!condition4(sb)){
        cerr << "Error: File system in " << disk_name << " is inconsistent (error code: 4)" << endl;
        return false;
    }

    if(!condition3(sb)){
        cerr << "Error: File system in " << disk_name << " is inconsistent (error code: 3)" << endl;
        return false;
    }

    if(!condition5(sb)){
        cerr << "Error: File system in " << disk_name << " is inconsistent (error code: 5)" << endl;
        return false;
    }
    
    if(!condition2(sb)){
        cerr << "Error: File system in " << disk_name << " is inconsistent (error code: 2)" << endl;
        return false;
    }
    
    if(!condition6(sb)){
        cerr << "Error: File system in " << disk_name << " is inconsistent (error code: 6)" << endl;
        return false;
    }
    if(!condition1(sb)){
        cerr << "Error: File system in " << disk_name << " is inconsistent (error code: 1)" << endl;
        return false;
    }
    return true;
}

