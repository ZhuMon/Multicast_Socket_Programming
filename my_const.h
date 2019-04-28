#include <string.h>

#ifndef MY_CONST_H
#define MY_CONST_H
#define p_len 1024 // length of packet

struct file_inf{
    char file_name[256];
    long int file_size;
    _Bool fec; // whether use fec
};

struct packet_inf{
    int index;
    char content[p_len];
    char prev[p_len]; // store content of prev packet
    char next[p_len]; // store content of next packet
};

void clean_fi(struct file_inf *fi){
    memset(fi -> file_name, 0, sizeof(fi -> file_name));
    memset(&fi -> file_size, 0, sizeof(&fi -> file_size));
    memset(&fi -> fec, 0, sizeof(&fi -> fec));
}

void clean_pi(struct packet_inf *pi){
    memset(pi -> content, 0, sizeof(pi -> content));
    memset(&pi -> index, 0, sizeof(&pi -> index));
    memset(pi -> prev, 0, sizeof(pi -> prev));
    memset(pi -> next, 0, sizeof(pi -> next));
}
#endif
