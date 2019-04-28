/* Receiver/client multicast Datagram example. */
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "my_const.h"

struct sockaddr_in localSock;
struct ip_mreq group;
int sd;
int datalen;
int portno;
FILE *outfile; // the output file
int p_num;     // expect received packets

struct file_inf my_fi;      // imformation of file
struct packet_inf p_buffer; // buffer of received packet

void pure_transfer(){
    int get_packet[p_num]; // record whether get the i-th packet
    char buffer[my_fi.file_size]; // to reconstruct file
    memset(buffer, 0, sizeof(buffer));

    while(p_buffer.index < p_num-1){
        clean_pi(&p_buffer);

        if(read(sd, &p_buffer, sizeof(p_buffer)) < 0) {
            fprintf(stderr,"Receive error");
            exit(1);
        } else{
            get_packet[p_buffer.index] = 1; // record "get the packet"
            
            // reconstruct file
            for(int i = 0;i < sizeof(p_buffer.content); i++){
                buffer[p_buffer.index * p_len + i] = p_buffer.content[i];
            }

        }
    }

    fwrite(buffer, 1, sizeof(buffer), outfile);

    int got = 0; // record # of got packets 
    for(int i = 0;i < p_num;i++){
        if(get_packet[i]){
            got++;
        } else {
            printf("%d ", i);
        }
    }
    printf("\n");

    printf("Lose rate: %f%%\n", ((p_num-got)/(float)p_num)*100);
}

void fec_transfer(){
    // redundant data will have 2 more packets
    _Bool get_packet[p_num+2]; // record whether get the i-th packet
    int content[p_num];      // whether record the i-th content
    char buffer[p_num*p_len]; // to reconstruct file //avoid overflow
    memset(content, 0, sizeof(content));
    memset(buffer, 0, sizeof(buffer));

    clean_pi(&p_buffer);
    while(p_buffer.index < p_num+1){
        clean_pi(&p_buffer);

        if(read(sd, &p_buffer, sizeof(p_buffer)) < 0) {
            fprintf(stderr,"Receive error");
            exit(1);
        } else{
            get_packet[p_buffer.index] = 1; // record "get the packet"
            if(p_buffer.index < p_num && content[p_buffer.index] == 0){
                for(int i = 0; i < sizeof(p_buffer.next); i++){
                    buffer[p_buffer.index*p_len+i] = p_buffer.next[i];
                }
                content[p_buffer.index] = 1;
            } 
            if(p_buffer.index > 0 && 
                      p_buffer.index < p_num+1 && 
                      content[p_buffer.index-1] == 0){
            // if get the packet which has content and the content never recorded
                for(int i = 0; i < sizeof(p_buffer.content);i++){
                    buffer[(p_buffer.index-1)*p_len+i] = p_buffer.content[i];
                }
                content[p_buffer.index-1] = 1;
            } 
            if(p_buffer.index > 1 && content[p_buffer.index-2] == 0){
                for(int i = 0; i < sizeof(p_buffer.prev);i++){
                    buffer[(p_buffer.index-2)*p_len+i] = p_buffer.prev[i];
                }
                content[p_buffer.index-2] = 1;
            }

        }
    }

    fwrite(buffer, 1, my_fi.file_size, outfile);
    int got = 0; // record # of got packets 
    int complete = 0;
    for(int i = 0;i < p_num+2;i++){
        if(get_packet[i]){
            got++;
        }
    }
    for(int i = 0; i < p_num; i++){
        if(content[i]){
            complete++;
        }
    }
    printf("Lose rate: %f%%\n", ((p_num+2-got)/(float)(p_num+2))*100);
    printf("Complete rate: %f%%\n", (complete/(float)p_num)*100);
}

int main(int argc, char *argv[])
{
    if(argc < 3){
        fprintf(stderr, "ERROR, usage:\n\t%s <ip> <port>\n", argv[0]);
        exit(1);
    }

    // portno = string_to_int(argv[2])
    portno = atoi(argv[2]);






    /* Create a datagram socket on which to receive. */
    sd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sd < 0) {
        perror("Opening datagram socket error");
        exit(1);
    } else
        printf("Opening datagram socket....OK.\n");

    /* Enable SO_REUSEADDR to allow multiple instances of this */
    /* application to receive copies of the multicast datagrams. */
    {
        int reuse = 1;
        if(setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(reuse)) < 0) {
            perror("Setting SO_REUSEADDR error");
            close(sd);
            exit(1);
        } else
            printf("Setting SO_REUSEADDR...OK.\n");
    }

    /* Bind to the proper port number with the IP address */
    /* specified as INADDR_ANY. */
    memset((char *) &localSock, 0, sizeof(localSock));
    localSock.sin_family = AF_INET;
    localSock.sin_port = htons(portno);
    localSock.sin_addr.s_addr = INADDR_ANY;
    if(bind(sd, (struct sockaddr*)&localSock, sizeof(localSock))) {
        perror("Binding datagram socket error");
        close(sd);
        exit(1);
    } else
        printf("Binding datagram socket...OK.\n");

    /* Join the multicast group 226.1.1.1 on the local 203.106.93.94 */
    /* interface. Note that this IP_ADD_MEMBERSHIP option must be */
    /* called for each local interface over which the multicast */
    /* datagrams are to be received. */
    group.imr_multiaddr.s_addr = inet_addr("226.1.1.1");
    group.imr_interface.s_addr = inet_addr(argv[1]);
    if(setsockopt(sd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&group, sizeof(group)) < 0) {
        perror("Adding multicast group error");
        close(sd);
        exit(1);
    } else
        printf("Adding multicast group...OK.\n");

    /* Read from the socket. */
    clean_fi(&my_fi);
    if(read(sd, &my_fi, sizeof(my_fi)) < 0) {
        perror("Reading datagram message error");
        close(sd);
        exit(1);
    } else {
        printf("Reading datagram message...OK.\n");
        //printf("The message from multicast server is: \n%s\n", databuf);
    }
    
   

    // open file to record
    outfile = fopen(my_fi.file_name, "wb");

    // find number of packets
    p_num = my_fi.file_size / p_len;
    if(my_fi.file_size%p_len == 0){
        p_num = p_num;
    } else {
        p_num += 1;
    }

    if(my_fi.fec){
        fec_transfer();
    } else {
        pure_transfer();
    }


    fclose(outfile);
    close(sd);

    return 0;
}
