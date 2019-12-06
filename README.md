# Assignment#3 - fs-sim
fs-sim(aka CMPUT 379 Assignment 3) is a program in C++ which simulates a trivial file system. 
This program mounts a file system that resides on a virtual disk (i.e., a file acting as the disk)
and checks if it is consistent. It subsequently reads commands from an input file and executes them 
by simulating the file system operations.

### Usage
```
make
./create_fs <disk_name>
./fs <your_input_file> > stdout 2>stderr
```


### Design Choices
- I used a unordered_map (called folder) to describe the structure of my file system. The map is a mapping between uint8_t and vector<uint8_t>. The key is the ID(in uint8_t) of the parent directory. The value is a list of ID(in uint8_t) of files and/or directories. Using this data structure, it is much easier to implement fs_ls(), fs_cd() and check_same_name()
- Also, We need to use superblock a lot in this assignment. Thus, I decide to use a global superblock, so I do not need to read from disk file to get the superblock. 



### Used System Calls
- `pread()`
- `pwrite()`

### Used Global Variable
- Super_block *gsb
- gfiledesc
- disk_name
- current_working_directory
- folder
- buffer



### Testing Choices
- Before I get started, I aware that this assignment will be quite long. Thus, I want to make sure that every function
I wrote is correct.
So I don't need to worry about the correctness of other functions when I am writing a new function.
- When I complete a function, I want to test them immediately. Thus, I try to write `fs_mount()` and `fs_create()` first.
 (If I write some functions like `fs_read()` or `fs_delete()` at the begining, I cannot test them without  
 `fs_mount()` and `fs_create()` )
- Next, I try to implement `fs_resize()`, `fs_delete()` and `fs_defrag()`. All of those functions are related to superblock a lot. So I print every change of superblock to stdout, and It will be very clear if I get something wrong. In this process, I also wrote  some testing function `print_list()` and `print_inode()` to print every bit of superblock.
- The remaining parts are easier. I did some simple unit test to test remaining functions.
- After I finish all of functions, I wrote a script to run all of the test cases provided by TAs. (Script is located in my Makefile.)
- After I passed all of test cases, I go to read description and rubric again, making sure that I did not miss something. 



### The files I have uploaded:
- fs.cc 
- FileSystem.cc
- FileSystem.h 
- check.cc 
- check.h
- readme.md
- Makefile



### Information Sources
- [C++ Reference](http://www.cplusplus.com/reference/stl/) 
- [Discussion Forum](https://eclass.srv.ualberta.ca/mod/forum/view.php?id=3735393) (Classmates, Lab TAs and Instructor)
- [Lab9](https://eclass.srv.ualberta.ca/pluginfile.php/5476760/mod_resource/content/1/379%20Lab%2010.pdf)  (Lab TAs and/or Instructor)
- tokenize function: CMPUT 379 Assignment1  (Lab TAs and/or Instructor)
- Starter Code, test cases and consistency test cases  (Lab TAs and/or Instructor)
