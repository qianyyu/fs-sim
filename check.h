#include <stdio.h>
#include <stdint.h>

using namespace std;

bool condition1(Super_block sb);
bool condition2(Super_block sb);
bool condition3(Super_block sb);
bool condition4(Super_block sb);
bool condition5(Super_block sb);
bool condition6(Super_block sb);
bool check_all(Super_block sb, char *disk_name);
bitset<128> ToBits_free_block_list(unsigned char* byte);
bitset<40> ToBits_name(unsigned char* byte);