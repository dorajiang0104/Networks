// Server side implementation of UDP client-server model 
#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h>
#include "packet_1.h"

#define MAXLINE 1024
#define PACKET_NO 5

void generateResponse(res_packet *res, data_packet packet, char* seg_nos);

// Driver code 
int main(int argc, char *argv[]) { 
    int sockfd, portno;
    struct sockaddr_in servaddr, cliaddr; 
    socklen_t len = sizeof(cliaddr); //len is value/resuslt 
    data_packet packet;
    res_packet res;
    size_t data_size = sizeof(data_packet);
    size_t res_size = sizeof(res_packet);
    char seg_nos[256]; 

    if (argc < 2) {
        fprintf(stderr,"ERROR, no port provided\n");
        exit(1);
    }

    portno = atoi(argv[1]);
    
    // Creating socket file descriptor
    //Initialization
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { 
        perror("socket creation failed"); 
        exit(EXIT_FAILURE); 
    } 
    
    memset(&servaddr, 0, sizeof(servaddr)); 
    memset(&cliaddr, 0, sizeof(cliaddr)); 
    
    // Filling server information 
    servaddr.sin_family = AF_INET; // IPv4 
    servaddr.sin_addr.s_addr = INADDR_ANY; 
    servaddr.sin_port = htons(portno); 
    //binding
    // Bind the socket with the server address 
    if ( bind(sockfd, (const struct sockaddr *)&servaddr, 
            sizeof(servaddr)) < 0 ) 
    { 
        perror("bind failed"); 
        exit(EXIT_FAILURE); 
    }

    for (;;) {
        int n;
        // res_packet response;

        n = recvfrom(sockfd, &packet, data_size, MSG_WAITALL, ( struct sockaddr *) &cliaddr, &len); 

        if (n > 0) {
            printf("from Client payload # %i : %s\n",  packet.seg_no, packet.payload);
        }
        generateResponse(&res, packet, seg_nos);

        //send err/ack packet
        sendto(sockfd, &res, sizeof(res_packet), 0, (const struct sockaddr *) &cliaddr, len);
    }
    
    return 0; 
} 

void generateResponse(res_packet *res, data_packet packet, char* seg_nos) {
    //response type: ack or reject
    short response = ACK_PACKET;

    char last_seg = seg_nos[(unsigned int) packet.client_id];
    if (last_seg == PACKET_NO) { //last request finished
        seg_nos[(unsigned int) packet.client_id] = 0;
        last_seg = 0;
    }
    short rej_sub_code = 0;
    //1235
    if (last_seg + 1 < packet.seg_no) {
        response = REJECT_PACKET;
        rej_sub_code = OUT_OF_SEQ;
    }
    //end id not ffff
    if (packet.end_id != (short)END_ID) {
        response = REJECT_PACKET;
        rej_sub_code = END_ID_MISSING;
    }
    //length not payload length
    if (strlen(packet.payload) + 1 != (unsigned int)packet.length) {
        response = REJECT_PACKET;
        rej_sub_code = LEN_MISMATCH;
    }
    
    //1233
    if (last_seg + 1 > packet.seg_no) {
        response = REJECT_PACKET;
        rej_sub_code = DUPLICATE_PACKET;
    }

    if (response == (short) REJECT_PACKET) {
        res->start_id = 0XFFFF;
        res->client_id = packet.client_id;
        res->type = REJECT_PACKET;
        res->rej_sub_code = rej_sub_code;
        res->recv_seg_no= packet.seg_no;
        res->end_id = 0XFFFF;
    } else {
        res->start_id = 0XFFFF;
        res->client_id = packet.client_id;
        res->type = ACK_PACKET;
        res->rej_sub_code = 0;
        res->recv_seg_no= packet.seg_no;
        res->end_id = 0XFFFF;

        // set current data to last
        seg_nos[(unsigned int) packet.client_id]++;
    }
} 
