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
int portno;
struct stat f_state;

struct file_inf my_fi;

int main (int argc, char *argv[])
{
    /* Check input argument */
    if (argc < 4) {
        fprintf(stderr, "ERROR, usage:\n\t %s <ip> <port> <file>\n", argv[0]);
        exit(1);
    }


    // portno = string_to_int(argv[2]);
    portno = atoi(argv[2]);

    // file_name = argv[3]
    clean_fi(&my_fi);
    strcat(my_fi.file_name, argv[3]);

    // Open file & Check file
    FILE *file;
    file = fopen(my_fi.file_name, "rb");
    if (file == NULL) {
        fprintf(stderr, "ERROR, open %s fail\n", argv[3]);
        exit(1);
    }

    // Get file imformation
    stat(my_fi.file_name, &f_state);

    // Store size of file
    my_fi.file_size = f_state.st_size;

    // Read file
    char buffer[my_fi.file_size+1];
    bzero(buffer, sizeof(buffer));
    fread(buffer, my_fi.file_size, 1, file);


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

    /* Disable loopback so you do not receive your own datagrams.
    {
    char loopch = 0;
    if(setsockopt(sd, IPPROTO_IP, IP_MULTICAST_LOOP, (char *)&loopch, sizeof(loopch)) < 0)
    {
    perror("Setting IP_MULTICAST_LOOP error");
    close(sd);
    exit(1);
    }
    else
    printf("Disabling the loopback...OK.\n");
    }
    */

    /* Set local interface for outbound multicast datagrams. */
    /* The IP address specified must be associated with a local, */
    /* multicast capable interface. */
    localInterface.s_addr = inet_addr(argv[1]);
    if(setsockopt(sd, IPPROTO_IP, IP_MULTICAST_IF, (char *)&localInterface, sizeof(localInterface)) < 0) {
        perror("Setting local interface error");
        exit(1);
    } else
        printf("Setting the local interface...OK\n");
    
    
    /* Send a message to the multicast group specified by the*/
    /* groupSock sockaddr structure. */
    
    /*int datalen = p_len;*/
    if(sendto(sd, &my_fi, sizeof(my_fi), 0, (struct sockaddr*)&groupSock, sizeof(groupSock)) < 0) {
        perror("Sending datagram message error");
    } else
        printf("Sending datagram message...OK\n");

    /* Try the re-read from the socket if the loopback is not disable
    if(read(sd, databuf, datalen) < 0) {
        perror("Reading datagram message error\n");
        close(sd);
        exit(1);
    } else {
        printf("Reading datagram message from client...OK\n");
        printf("The message is: %s\n", databuf);
    }*/
    

    // Spilt file to p_len byte
    int p_num;
    if (f_state.st_size%p_len == 0){
        p_num = f_state.st_size/p_len;
    } else { 
        p_num = f_state.st_size/p_len+1; 
    }

    // Send file
    //char p_buffer[p_len+lpi];
    struct packet_inf p_buffer;
    for(int i = 0; i < p_num; i++){
        clean_pi(&p_buffer);
        //bzero(p_buffer, p_len+lpi);
        //sprintf(p_buffer, "%d", i); 
        
        // Store index in packet
        p_buffer.index = i;

        // send the last packet whose size different with each other
        if(i == p_num-1 && my_fi.file_size % p_len != 0){
            for(int j = 0; j < my_fi.file_size % p_len; j++){
                p_buffer.content[j] = buffer[i*p_len+j];
            }
            if(sendto(sd, &p_buffer, sizeof(p_buffer), 0, (struct sockaddr*)&groupSock, sizeof(groupSock)) < 0) {
                perror("Sending datagram message error");
            } 
            
        } else {
            for(int j = 0; j < p_len; j++){
                p_buffer.content[j] = buffer[i*p_len+j];
            }
            if(sendto(sd, &p_buffer, sizeof(p_buffer), 0, (struct sockaddr*)&groupSock, sizeof(groupSock)) < 0) {
                perror("Sending datagram message error");
            } //else
                //printf("Sending datagram message...OK\n");

        }
        
    }
    





    return 0;
}
