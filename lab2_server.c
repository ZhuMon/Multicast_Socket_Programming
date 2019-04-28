/* Send Multicast Datagram code example. */
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "my_const.h"

struct in_addr localInterface;
struct sockaddr_in groupSock;
int sd;
int portno; // port number
struct stat f_state;
FILE *file; // file to transfer
int p_num; // number of packets

struct file_inf my_fi;  // record file information
struct packet_inf p_buffer; // packet to transfer

void pure_transfer(){
    // Read file
    char buffer[my_fi.file_size+1];
    //printf("hi\n");
    bzero(buffer, sizeof(buffer));
    fread(buffer, my_fi.file_size, 1, file);

    /* Send file message to the multicast group specified by the*/
    /* groupSock sockaddr structure. */
    
    if(sendto(sd, &my_fi, sizeof(my_fi), 0, (struct sockaddr*)&groupSock, sizeof(groupSock)) < 0) {
        perror("Sending datagram message error");
    } else
        printf("Sending datagram message...OK\n");


    // Send file
    for(int i = 0; i < p_num; i++){
        clean_pi(&p_buffer);
        
        // Store index in packet
        p_buffer.index = i;

        // send the prev packet whose size is different with each other
        if(i == p_num+1 && my_fi.file_size % p_len != 0){
            
            for(int j = 0; j < my_fi.file_size % p_len; j++){
                p_buffer.content[j] = buffer[i*p_len+j];
            }
            if(sendto(sd, &p_buffer, sizeof(p_buffer), 0, (struct sockaddr*)&groupSock, sizeof(groupSock)) < 0) {
                perror("Sending datagram message error");
            } 
            
        } else {
            //split file to packet
            for(int j = 0; j < p_len; j++){
                p_buffer.content[j] = buffer[i*p_len+j];
            }
            if(sendto(sd, &p_buffer, sizeof(p_buffer), 0, (struct sockaddr*)&groupSock, sizeof(groupSock)) < 0) {
                perror("Sending datagram message error");
            } 

        }
        
    }
}

void fec_transfer(){
    // Read file
    char buffer[my_fi.file_size+1];
    //printf("hi\n");
    bzero(buffer, sizeof(buffer));
    fread(buffer, my_fi.file_size, 1, file);

    /* Send file message to the multicast group specified by the*/
    /* groupSock sockaddr structure. */
    
    if(sendto(sd, &my_fi, sizeof(my_fi), 0, (struct sockaddr*)&groupSock, sizeof(groupSock)) < 0) {
        perror("Sending datagram message error");
    } else
        printf("Sending datagram message...OK\n");


    // first packet which only has redundant data
    clean_pi(&p_buffer);
    p_buffer.index = 0;
    for(int j = 0; j < p_len; j++){
        p_buffer.next[j] = buffer[j];
    }
    if(sendto(sd, &p_buffer, sizeof(p_buffer), 0, (struct sockaddr*)&groupSock, sizeof(groupSock)) < 0) {
        perror("Sending datagram message error");
    }


    // Send the other packets
    for(int i = 0; i <= p_num; i++){
        clean_pi(&p_buffer);
        
        // Store index in packet
        p_buffer.index = i+1;

        if(i == p_num){
            // the prev packet which only has redundant data
            if(my_fi.file_size % p_len != 0){
                for(int j = 0; j < my_fi.file_size % p_len; j++){
                    p_buffer.prev[j] = buffer[(i-1)*p_len+j];
                }
            } else {
                for(int j = 0; j < p_len; j++){
                    p_buffer.prev[j] = buffer[(i-1)*p_len+j];
                }
            }

            if(sendto(sd, &p_buffer, sizeof(p_buffer), 0, (struct sockaddr*)&groupSock, sizeof(groupSock)) < 0) {
                perror("Sending datagram message error");
            }

        } else if(i == p_num-1){
            // the prev packet which has content
            
            //split file to packet
            if(my_fi.file_size % p_len != 0){
                for(int j = 0; j < my_fi.file_size % p_len; j++){
                    p_buffer.content[j] = buffer[i*p_len+j];
                }
                for(int j = 0; j < p_len; j++){
                    p_buffer.prev[j] = buffer[(i-1)*p_len+j];
                }
            } else {
                for(int j = 0; j < p_len; j++){
                    p_buffer.content[j] = buffer[i*p_len+j];
                    p_buffer.prev[j] = buffer[(i-1)*p_len+j];
                }
            }
            if(sendto(sd, &p_buffer, sizeof(p_buffer), 0, (struct sockaddr*)&groupSock, sizeof(groupSock)) < 0) {
                perror("Sending datagram message error");
            } 
            
        } else {
            //split file to packet
            for(int j = 0; j < p_len; j++){
                p_buffer.content[j] = buffer[i*p_len+j];
                if(i>0){
                    p_buffer.prev[j] = buffer[(i-1)*p_len+j];
                }
                p_buffer.next[j] = buffer[(i+1)*p_len+j];
            }
            if(sendto(sd, &p_buffer, sizeof(p_buffer), 0, (struct sockaddr*)&groupSock, sizeof(groupSock)) < 0) {
                perror("Sending datagram message error");
            } 

        }
        
    }
}



int main (int argc, char *argv[])
{
    /* Check input argument */
    if (argc < 4) {
        fprintf(stderr, "ERROR, usage:\n\t %s <ip> <port> <file> (fec)\n", argv[0]);
        exit(1);
    }


    // portno = string_to_int(argv[2]);
    portno = atoi(argv[2]);

    // file_name = argv[3]
    clean_fi(&my_fi);
    strcat(my_fi.file_name, argv[3]);

    // Open file & Check file
    file = fopen(my_fi.file_name, "rb");
    if (file == NULL) {
        fprintf(stderr, "ERROR, open %s fail\n", argv[3]);
        exit(1);
    }

    // Get file imformation
    stat(my_fi.file_name, &f_state);

    // Store size of file
    my_fi.file_size = f_state.st_size;


    // Split file to p_len byte
    if (f_state.st_size%p_len == 0){
        p_num = f_state.st_size/p_len;
    } else { 
        p_num = f_state.st_size/p_len+1; 
    }

    /* Create a datagram socket on which to send. */
    sd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sd < 0) {
        perror("Opening datagram socket error");
        exit(1);
    } else
        printf("Opening the datagram socket...OK.\n");

    /* Initialize the group sockaddr structure with a */
    /* group address of 225.1.1.1 and port 5555. */
    memset((char *) &groupSock, 0, sizeof(groupSock));
    groupSock.sin_family = AF_INET;
    groupSock.sin_addr.s_addr = inet_addr("226.1.1.1");
    groupSock.sin_port = htons(portno);

    /* Set local interface for outbound multicast datagrams. */
    /* The IP address specified must be associated with a local, */
    /* multicast capable interface. */
    localInterface.s_addr = inet_addr(argv[1]);
    if(setsockopt(sd, IPPROTO_IP, IP_MULTICAST_IF, (char *)&localInterface, sizeof(localInterface)) < 0) {
        perror("Setting local interface error");
        exit(1);
    } else
        printf("Setting the local interface...OK\n");
    
    if(argc > 4 && strcmp(argv[4], "fec") == 0){
        my_fi.fec = 1;
        fec_transfer();
    } else if (argc > 4) {
        perror("Please enter \"fec\" or remain blank in argv[4]");
        exit(1);
    } else {
        my_fi.fec = 0;
        pure_transfer();
    }

    return 0;
}
