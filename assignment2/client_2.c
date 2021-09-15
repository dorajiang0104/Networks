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
#include "packet_2.h"

#define MAXLINE 1024 
#define PACKAGE_NO 5

int digits_only(const char *s);
int getPhoneNum(sub_payload* numbers);
int getClientId();
data_packet* generateDataPacket(int client_id, sub_payload* numbers, int count);


// Driver code 
int main(int argc, char *argv[]) { 
    int sockfd, portno;
    data_packet* packets; 
    data_packet response;
    struct sockaddr_in servaddr;
    socklen_t sockaddrsize = sizeof(servaddr); 
    if (argc < 2) {
        fprintf(stderr,"ERROR, no port provided\n");
        exit(1);
    }

    portno = atoi(argv[1]);
    
    //numbers is used to save different input max 5: sub_payload : number & techonology
    sub_payload* numbers = malloc(PACKAGE_NO * sizeof(sub_payload));

    int client_id = getClientId();
    int count = getPhoneNum(&numbers[0]); //passing numbers array by reference
    packets = generateDataPacket(client_id, numbers, count);

    // Creating socket file descriptor 
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { 
        perror("socket creation failed"); 
        exit(EXIT_FAILURE); 
    } 

    memset(&servaddr, 0, sizeof(servaddr)); 

    // Filling server information 
    servaddr.sin_family = AF_INET; 
    servaddr.sin_port = htons(portno); 
    servaddr.sin_addr.s_addr = INADDR_ANY;
    
    struct timeval tv;
    tv.tv_sec = 3;
    tv.tv_usec = 0;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);

    for (int i = 0; i < count; i++) {
        int n, attempt;
        socklen_t len;

        attempt = 0;


        while (attempt < 3) {
            if (attempt > 0) {
                printf("message no response due to unknown issue, try again.\n");
            }
            
            sendto(sockfd, &packets[i], sizeof(data_packet), 0, (const struct sockaddr *) &servaddr, sockaddrsize);
            printf("message sent: number: \'%.0lf.\', technology: %dG", packets[i].phone, packets[i].technology);
            if (attempt > 0) {
                printf("attempt time: %d. \n", attempt + 1);
            } else {
                 printf("\n");
            }
            n = recvfrom(sockfd, &response, sizeof(data_packet), MSG_WAITALL, (struct sockaddr *) &servaddr, &len); 
            if (n > 0) {
                printf("%.0lf\n", response.phone);
                if(response.type == (short)ACC_OK) {
                    printf("Server: Access Permitted.\n");
                    break;
                } else if(response.type == (short)NOT_PAID) {
                    printf("Server: Phone number %.0lf not paid\n", packets[i].phone);
                    break;
                } else if(response.type == (short)NOT_EXIST) {
                    printf("Server: Phone number %.0lf not exist or can not find a phone number match the technology(%dG)\n", packets[i].phone, packets[i].technology);
                    break;
                } else {
                    printf("Server: unknown error\n");
                    exit(1);
                }

                printf("\n");

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

int digits_only(const char *s) {
    while (*s) {
        if (isdigit(*s++) == 0) return 0;
    }
    return 1;
}

int getClientId() {
    int client_id = 0;
    char input[3];
    printf("Please input your clinet ID(0 - 255): ");
    for (;;) {
        scanf("%s", input);
        int client_id = atoi(input);
        if (client_id >= 0 && client_id <= 255 && digits_only(input) == 1) {
            return client_id;
        }
        printf("Invalid client ID, should be integer from 0 - 255 \n");
    }
}

int getPhoneNum(sub_payload* numbers) {
    int count = 0;
    while (count < (int)PACKAGE_NO) {
        char* input = malloc(11 * sizeof(char));
        printf("Please input your 10 digit Phone Number (0000000000 - 4294967295): \n");
        for (;;) {
            scanf("%s", input);
            if (strlen(input) == 10 && digits_only(input) == 1 && strtod(input, NULL) <= (double) SUB_MAX) {
                numbers[count].phone = strtod(input, NULL);
                break;
            }
            printf("Invalid phone number, needs to be 10 digit numbers and less than 4294967295 \n");
        }

        char tech;
        printf("Please selected technology:\n");
        printf("  [0] 2G\n");
        printf("  [1] 3G\n");
        printf("  [2] 4G\n");
        printf("  [3] 5G\n");
        for(;;) {
            scanf(" %c", &tech);
            while(getchar() != '\n');
            if (tech == '0') {
                numbers[count].technology = 2; //2g
                break;
            }
            if (tech == '1') {
                numbers[count].technology = 3; //3g
                break;
            }
            if (tech == '2') {
                numbers[count].technology = 4; //4g
                break;
            }
            if (tech == '3') {
                numbers[count].technology = 5; //5g
                break;
            }
        }
        count++;
        char yn;

        while (count < (int)PACKAGE_NO) {
            printf("Do you want to input another number? %d input and %d maximum. (Y/N) ", count, PACKAGE_NO);
            scanf(" %c", &yn);
            while(getchar() != '\n');
            if (yn == 'Y' || yn == 'y') {
                break;
            } else if (yn == 'N' || yn == 'n') {
                printf("total numbers: %d\n", count);
                for (int i = 0; i < count; i++) {
                    printf("  %.0lf\n", numbers[i].phone);
                }
                return count;
            }
        }
        
    }

    printf("total numbers: %d\n", count);
    for (int i = 0; i < count; i++) {
        printf("  %.0lf\n", numbers[i].phone);
    }

    return count;
}


data_packet* generateDataPacket(int client_id, sub_payload* numbers, int count) {
    data_packet *packets = malloc(sizeof(data_packet) * count);
    for (int i = 0; i < count; i++) {
        packets[i].start_id = START_ID;
        packets[i].client_id = (char) client_id;
        packets[i].type = ACC_PER;
        packets[i].seg_no = (char) i + 1;
        packets[i].technology = numbers[i].technology;
        packets[i].phone = numbers[i].phone;
        packets[i].length = sizeof(char) + sizeof(double);// technology + phone
        packets[i].end_id = END_ID;
    }

    return packets;
}
