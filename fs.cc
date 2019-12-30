#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>


// String function
#include <string.h>
#include <fstream>


// C++ IO
#include <iostream>
#include <algorithm>


// Open system call:
#include <sys/types.h>

#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

//for the open() permission flags
#include <sys/stat.h>

// Data Structure:
#include <vector>
#include <string>
#include <set>
#include <cstring>

// My Library
#include "FileSystem.h"

using namespace std;

int global = 1;

/* Reference:
        CMPUT 379 Assignment1(Lab TAs and/or Instructor) */
vector<string> tokenize(const string &str, const char *delim) {
    char* cstr = new char[str.size() + 1];
    strcpy(cstr, str.c_str());

    char* tokenized_string = strtok(cstr, delim);

    vector<string> tokens;
    while (tokenized_string != NULL){
        tokens.push_back(string(tokenized_string));
        tokenized_string = strtok(NULL, delim);
    }
    delete[] cstr;
    return tokens;
}



int main(int argc, char* argv[]){
    if(argc != 2){
        cerr << "Usage: ./fs <input_file>"<< endl;
        exit(1);
    }
    const char *input_file_name = *(argv+1);
    ifstream myfile (input_file_name);
    if (! myfile.is_open()){
        cerr << "Error: file is not open "<<endl;
        exit(1);
    }
    vector<string> tokens;
    string command;
    int line=0;

    while(!myfile.eof()){
        line++;
        getline(myfile,command);
        tokens = tokenize(command," ");
        if(tokens.size()==0)
            continue;
        char tag = command[0];

        switch(tag){
            // Mount the file system residing on the disk
            case 'M':{
                if(tokens.size()!=2){
                    cerr << "Command Error: "<< input_file_name <<", " << line << endl;
                    continue;
                }
                string disk_name = tokens.at(1);
                const char *name = disk_name.c_str();
                fs_mount((char*)name); 
                break;
            }

            // Create file
            case 'C':{
                if(tokens.size()!=3){
                    cerr << "Command Error: "<< input_file_name <<", " << line << endl;
                    continue;
                }               
                string file_name = tokens.at(1);
                if(file_name.size()>5){
                    cerr << "Command Error: "<< input_file_name <<", " << line << endl;
                    continue;
                }
                int file_size = atoi(tokens.at(2).c_str());
                if(file_size>127 || file_size <0){
                    cerr << "Command Error: "<< input_file_name <<", " << line << endl;
                    continue;
                }
                fs_create((char*)file_name.c_str(),file_size); 
                break;
            }

            // Delete file
            case 'D':{
                if(tokens.size()!=2){
                    cerr << "Command Error: "<< input_file_name <<", " << line << endl;
                    continue;
                }
                string file_name = tokens.at(1);
                if(file_name.size()>5){
                    cerr << "Command Error: "<< input_file_name <<", " << line << endl;
                    continue;
                }
                fs_delete((char*)file_name.c_str()); 
                break;
            }
            // Read file
            case 'R':{
                if(tokens.size()!=3){
                    cerr << "Command Error: "<< input_file_name <<", " << line << endl;
                    continue;
                }
                string file_name = tokens.at(1);
                if(file_name.size()>5){
                    cerr << "Command Error: "<< input_file_name <<", " << line << endl;
                    continue;
                }
                int block_num = atoi(tokens.at(2).c_str());
                if(block_num>127 || block_num <0){
                    cerr << "Command Error: "<< input_file_name <<", " << line << endl;
                    continue;
                }
                fs_read((char*)file_name.c_str(),block_num); 
                break;
            }
            // Write file
            case 'W': {
                if(tokens.size()!=3){
                    cerr << "Command Error: "<< input_file_name <<", " << line << endl;
                    continue;
                }
                string file_name = tokens.at(1);
                if(file_name.size()>5){
                    cerr << "Command Error: "<< input_file_name <<", " << line << endl;
                    continue;
                }
                int block_num = atoi(tokens.at(2).c_str());
                if(block_num>127 || block_num <0){
                    cerr << "Command Error: "<< input_file_name <<", " << line << endl;
                    continue;
                }
                fs_write((char*)file_name.c_str(),block_num); 
                break;
            }
            // Update Buffer
            case 'B':{
                if(tokens.size() == 1){
                    cerr << "Command Error: "<< input_file_name <<", " << line << endl;
                    continue;
                }
                auto end = command.begin()+1;
                while((*end)==' ')
                    end++;
                command.erase(command.begin(),end);
                if(command.size()>1024){
                    cerr << "Command Error: "<< input_file_name <<", " << line << endl;
                    continue;
                }
                fs_buff((char*)command.c_str()); 
                break;
            }
            // list files
            case 'L':{
                if(tokens.size()!=1){
                    cerr << "Command Error: "<< input_file_name <<", " << line << endl;
                    continue;
                }
                fs_ls();
                break;
            }
            // Changing file size
            case 'E':{ 
                if(tokens.size()!=3){
                    cerr << "Command Error: "<< input_file_name <<", " << line << endl;
                    continue;
                }
                string name = tokens.at(1);
                if(name.size()>5){
                    cerr << "Command Error: "<< input_file_name <<", " << line << endl;
                    continue;
                }
                int new_size =  atoi(tokens.at(2).c_str());
                if(new_size>127 || new_size <0){
                    cerr << "Command Error: "<< input_file_name <<", " << line << endl;
                    continue;
                }
                fs_resize((char*)(name.c_str()),new_size); 
                break;
            }
            // Defragment the disk
            case 'O':{ 
                if(tokens.size()!=1){
                    cerr << "Command Error: "<< input_file_name <<", " << line << endl;
                    continue;
                }
                fs_defrag(); 
                break;
            }
            // Change the current working directory
            case 'Y':{
                if(tokens.size() != 2){
                    cerr << "Command Error: "<< input_file_name <<", " << line << endl;
                    continue;
                }
                string dir_name = tokens.at(1);
                if(dir_name.size()>5){
                    cerr << "Command Error: "<< input_file_name <<", " << line << endl;
                    continue;
                }
                fs_cd((char*)dir_name.c_str()); 
                break;
            }
            default:{
                cerr << "Command Error: "<< input_file_name <<", " << line << endl;
                break;
            }
        }
        tokens.clear();
    }
    myfile.close();
    fs_free();
    return 0;
}
