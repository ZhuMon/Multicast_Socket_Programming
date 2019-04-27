#include <string.h>

#ifndef MY_CONST_H
#define MY_CONST_H
#define p_len 1024 // length of packet

struct file_inf{
    char file_name[256];
    long int file_size;
};

struct packet_inf{
    int index;
    char content[p_len];
};

void clean_fi(struct file_inf *fi){
    memset(fi -> file_name, 0, sizeof(fi -> file_name));
    memset(&fi-> file_size, 0, sizeof(&fi -> file_size));
}

void clean_pi(struct packet_inf *pi){
    memset(pi -> content, 0, sizeof(pi -> content));
    memset(&pi-> index, 0, sizeof(&pi -> index));
}
#endif
