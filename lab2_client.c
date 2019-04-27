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
char databuf[256];
int portno;


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
    //datalen = sizeof(databuf);
    struct file_inf my_fi;
    clean_fi(&my_fi);
    if(read(sd, &my_fi, sizeof(my_fi)) < 0) {
        perror("Reading datagram message error");
        close(sd);
        exit(1);
    } else {
        printf("Reading datagram message...OK.\n");
        printf("The message from multicast server is: \n%s\n", databuf);
    }
    
   

    // open file to record
    FILE *outfile;
    outfile = fopen(my_fi.file_name, "wb");

    // find number of packets
    int p_num = my_fi.file_size / p_len;
    if(my_fi.file_size%p_len == 0){
        p_num = p_num;
    } else {
        p_num += 1;
    }

    // receive packet
    struct packet_inf p_buffer;
    int get_packet[p_num]; // record whether get the i-th packet
    int p_index_i;
    for(int i = 0; i < p_num; i++){
        clean_pi(&p_buffer);
        
        // send the last packet whose size different with each other
        if(i == p_num - 1 && my_fi.file_size % p_len != 0){

            if(read(sd, &p_buffer, sizeof(p_buffer)) < 0) {
                fprintf(stderr,"Receive error");
                exit(1);
            } else{
                get_packet[p_buffer.index] = 1; // record "get the packet"
                fwrite(p_buffer.content, 1, sizeof(p_buffer.content), outfile);
            }

        } else {
            if(read(sd, &p_buffer, sizeof(p_buffer)) < 0) {
                fprintf(stderr,"Receive error");
                exit(1);
            } else{
                get_packet[p_buffer.index] = 1; // record "get the packet"
                fwrite(p_buffer.content, 1, sizeof(p_buffer.content), outfile);
            }
        }


    }
    int got = 0; // record # of got packets 
    for(int i = 0;i < p_num;i++){
        if(get_packet[i]){
            got++;
        }
    }

    printf("Lose rate: %f\n", (p_num-got)/(float)p_num);
    fclose(outfile);
    close(sd);

    return 0;
}
