#include <stdio.h> 
#include <stdlib.h>
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h>
#include "subscriber.h"
#include "packet_2.h"

#define DBSIZE 3 //edit when insert/delete new numbers in database;

subscriber* loadDatabase(char* filename);
void generateResponse(data_packet *res, data_packet packet, subscriber* data);
  
int main(int argc, char *argv[]) { 
    // load database
    
    char* filename = "Verification_Database.txt"; 
    subscriber* data = malloc(DBSIZE * sizeof(subscriber));
    
    data = loadDatabase(filename);
    
    //start server
    int sockfd, portno;
    struct sockaddr_in servaddr, cliaddr; 
    data_packet packet, res;
    socklen_t len = sizeof(cliaddr); //len is value/resuslt 
    size_t data_size = sizeof(data_packet);
    
    if (argc < 2) {
        fprintf(stderr,"ERROR, no port provided\n");
        exit(1);
    }
    
    portno = atoi(argv[1]);
    
    // Creating socket file descriptor 
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
    
    // Bind the socket with the server address 
    if ( bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0 ) { 
        perror("bind failed"); 
        exit(EXIT_FAILURE); 
    }
    
    
    for (;;) {
        int n;
        
        printf("waiting for message from client\n");
        n = recvfrom(sockfd, &packet, data_size, 0, ( struct sockaddr *) &cliaddr, &len); 
        if (n > 0) {
            printf("message received: number: \'%.0lf.\', technology: %dG\n", packet.phone, packet.technology);
        }

        generateResponse(&res, packet, data);

        //send err/ack packet
        sendto(sockfd, &res, data_size, 0, (const struct sockaddr *) &cliaddr, len);
    }
    
    return 0; 
}

subscriber* loadDatabase(char* filename) {
    FILE *fptr; 
    char c;
    subscriber* data = malloc(DBSIZE * sizeof(subscriber));
    
    // Open file 
    fptr = fopen(filename, "r"); 
    if (fptr == NULL) { 
        printf("Cannot open file \n"); 
        exit(0); 
    }
    int key_index = 0;
    int i = 0;
    
    char x[11];
    

    while (fscanf(fptr, " %10s", x) == 1) {     // assumes no data exceeds length of 10 
        if (key_index %3 == 0) {
            data[i].phone = strtod(x, NULL);
        }

        if (key_index %3 == 1) {
            data[i].technology = atoi(x);
        }

        if (key_index %3 == 2) {
            data[i].paid = x[0] - '0';
            i++;
        }

        key_index++;
    }
    

    fclose(fptr);
    printf("data base loaded.\n");

    return data;
}

void generateResponse(data_packet *res, data_packet packet, subscriber* data) {
    short response_type = NOT_EXIST;
    
    for (int i = 0; i < DBSIZE; i++) {
        if (data[i].phone == packet.phone && data[i].technology == packet.technology) {
            response_type = NOT_PAID;
            if (data[i].paid) {
                response_type = ACC_OK;
            }
        }
    }

    res->start_id = START_ID;
    res->client_id = packet.client_id;
    res->seg_no = packet.seg_no;
    res->technology = packet.technology;
    res->phone = packet.phone;
    res->type = response_type;
    res->length = sizeof(char) + sizeof(double);// technology + phone
    res->end_id = END_ID;

    return;
}
