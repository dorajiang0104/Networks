// Client side implementation of UDP client-server model 
#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h>
#include <ctype.h>
#include "packet_1.h"

#define MAXLINE 1024    //buffer max size
#define PACKAGE_NO 5    //given 5 packages

int digits_only(const char *s);
int getClientId();
int ifModify(int modify_again);
int selectPacket(); //return segment no
int selectField(data_packet packet);    //which field in the packet need to be modified
data_packet modifyChar(data_packet packet, int field);  //client id, segment no, length
data_packet* generateDataPackages(int client_id);

int main(int argc, char *argv[]) { 
    int sockfd, portno;
    struct sockaddr_in servaddr; 
    socklen_t sockaddrsize = sizeof(servaddr);
    data_packet* packets;   //client packets to send
    res_packet response;    //response from server
    int client_id;          //user input

    if (argc < 2) {
        fprintf(stderr,"ERROR, no port provided\n");
        exit(1);
    }

    portno = atoi(argv[1]);

    // Creating socket file descriptor
    // socket initialization
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { 
        perror("socket creation failed"); 
        exit(EXIT_FAILURE); 
    } 

    memset(&servaddr, 0, sizeof(servaddr)); 

    // Filling server information 
    servaddr.sin_family = AF_INET; //ip
    servaddr.sin_port = htons(portno); 
    servaddr.sin_addr.s_addr = INADDR_ANY;  // 0.0.0.0 local host

    client_id =  getClientId();
    packets = generateDataPackages(client_id); // set client id get packages
    // modify packets
    int modify_again = 0;
    data_packet packet;
    for (;;) {
        // send packages directly or modify packages
        if (ifModify(modify_again)) {
            // which package you want to modify
            
            int i = selectPacket();
            if (i >= 0 && i < PACKAGE_NO) {
                packet = packets[i];
            }

            int field = selectField(packet);

            switch(field){
                case 1:
                    // modify client id
                    printf("Modify Client ID\n");
                    packets[i] = modifyChar(packet , field);
                    break;
                case 2:
                    // modify seg no
                    printf("Modify Segment Number\n");
                    packets[i] = modifyChar(packet , field);
                    break;
                case 3:
                    // modify length
                    printf("Modify payload length\n");
                    packets[i] = modifyChar(packet , field);
                case 4:
                    // modify length
                    printf("Modify end identifier\n");
                    packets[i].end_id = 0; 
                    break;
                default:
                    printf("Unknown field\n");
                    break;
            }
        // which filed you want to modify
        } else {
            break;
        }
        modify_again = 1;
    }
    
    // 102-106 used for setting up timeout value
    struct timeval tv;
    tv.tv_sec = 3;
    tv.tv_usec = 0;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
    
    //line 108 - 160 is used for 'Procedure' in the requirement
    //sending / receiving packets
    for (int i; i < PACKAGE_NO; i++) {
        int n, attempt;
        socklen_t len;
        
        //a retry counter
        attempt = 0;

        while (attempt < 3) {
            if (attempt > 0) {
                printf("message no response due to unknown issue, try again.\n");
            }

            sendto(sockfd, &packets[i], sizeof(data_packet), 0, (const struct sockaddr *) &servaddr, sockaddrsize); 
            printf("message sent: \'%s.\'", packets[i].payload);
            if (attempt > 0) {
                printf("attempt time: %d. \n", attempt + 1);
            } else {
                 printf("\n");
            }

            n = recvfrom(sockfd, &response, sizeof(res_packet), MSG_WAITALL, (struct sockaddr *) &servaddr, &len); 
            if (n > 0) {
                if(response.type == (short)ACK_PACKET) { //ACK received!
                    printf("Server: ACK. Packet %d acknowledged.\n", i);
                    break;
                }

                if (response.type == (short)REJECT_PACKET) {
                    if (response.rej_sub_code == (short) OUT_OF_SEQ) {
                        printf("Server Error: out of sequence number.\n");
                    } else if (response.rej_sub_code == (short) LEN_MISMATCH) {
                        printf("Server Error: payload length is not equals to length field.\n");
                    } else if (response.rej_sub_code == (short) END_ID_MISSING) {
                        printf("Server Error: missing identifier.\n");
                    } else if (response.rej_sub_code == (short) DUPLICATE_PACKET) {
                        printf("Server Error: duplicate packet.\n");
                    } else {
                        printf("Server Error: Unknown.\n");
                    }
                    exit(-1);
                }
            } else {
                attempt++;
            }

            if (attempt == 3) {
                printf("server no response, exit..\n\n");
                exit(1);
            }
        }
        
    }
    

    close(sockfd); 
    return 0; 
} 


int digits_only(const char *s){
    while (*s) {
        if (isdigit(*s++) == 0) return 0;
    }
    return 1;
}

int getClientId() {
    int client_id = 0;
    char input[3];
    printf("Please input your clinet ID(0 - 255): \n");
    for (;;) {
        scanf("%s", input);
        int client_id = atoi(input);
        if (client_id >= 0 && client_id <= 255 && digits_only(input) == 1) {
            return client_id;
        }
        printf("Invalid client ID, should be integer from 0 - 255 \n");
    }
}

int ifModify(int modify_again) {
    char input;
    for(;;) {
        if (modify_again) {
            printf("Is there any other packets you want to modify?(Y/N)\n");
        } else {
            printf("Do you want to modify any packets?(Y/N)\n");
        }
        
        scanf(" %c", &input);
        while(getchar() != '\n');
        if (input == 'Y' || input == 'y') {
            return 1;
        } else if (input == 'N' || input == 'n') {
            return 0;
        }
    }
    

}

int selectPacket() {
    int p = 0;
    char input[3]; //even though we set the package number to 5, but the maximum # of packages that protocal allowed is 256
    for (;;) {
        printf("Which packet you want to modify?(1 - %d) \n", PACKAGE_NO);
        scanf("%s", input);
        p = atoi(input);
        if (p > 0 && p <= PACKAGE_NO && digits_only(input) == 1) {
            return p - 1;
        }
        printf("Invalid package ID, should be integer from 0 - %d \n", PACKAGE_NO);
    }
    return (-1);
}

int selectField(data_packet packet) {
    printf("Selected Packet: \n");
    printf("[1] Client ID: %d\n", (unsigned int) packet.client_id);
    printf("[2] Segment No.: %d\n", (unsigned int) packet.seg_no);
    printf("[3] Payload length: %d\n", (unsigned int) packet.length);
    printf("[4] End Identifier: %04x\n", (unsigned int) packet.end_id);
    printf("Choose the index to edit the packet field.(1 - 4)\n");

    char input;
    for(;;) {
        scanf(" %c", &input);
        while(getchar() != '\n');
        if (input == '1' || input == '2' || input == '3' || input == '4') {
            return (unsigned int) input - '0';
        }
    }
    return (-1);
}

data_packet modifyChar(data_packet packet, int field) {
    printf("Input new value: (char)\n");
    char input[3];
    for (;;) {
        scanf("%s", input);
        int f = atoi(input);
        if (f >= 0 && f <= 255 && digits_only(input) == 1) {
            if (field == 1) {
                packet.client_id = f;
            }

            if (field == 2) {
                packet.seg_no = f;
            }

            if (field == 3) {
                packet.length = f;
            }
            return packet;
        }
        printf("Invalid value, should be integer from 0 - 255 \n");
    }
}


data_packet* generateDataPackages(int client_id) {
    data_packet *packets = malloc(sizeof(data_packet) * PACKAGE_NO);
    for (int i = 0; i < PACKAGE_NO; i++) {
        packets[i].start_id = START_ID;
        packets[i].client_id = (char) client_id;
        packets[i].type = DATA_PACKET;
        packets[i].seg_no = (char) i + 1;
        char buffer[MAXLINE];
        sprintf(buffer, "This is pacakge #%d sent by Client %d.", i + 1, client_id);
        strncpy(packets[i].payload, buffer, 255);
        packets[i].length = strlen(packets[i].payload) + 1; // include \0
        packets[i].end_id = END_ID;
    }

    return packets;
}

