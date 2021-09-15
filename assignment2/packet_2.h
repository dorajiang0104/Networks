#ifndef PACKET_2_H
#define PACKET_2_H
// subscribe payload
// |_technology(1 byte)_|_Source Subscriber No(8 bytes)_|
typedef struct sub_payload {
    char technology;
    double phone;
} sub_payload;

// DATA
typedef struct data_packet {
    short start_id;
    char client_id; // use char since it is 1 byte data
    short type;
    char seg_no;
    char length; // same as client_id
    char technology;
    double phone;
    short end_id;
} data_packet;

#define START_ID 0XFFFF
#define END_ID 0XFFFF
#define CLIENT_ID_MAX 0XFF //256
#define DATA_LENGTH 0XFF //256

// packet type
#define ACC_PER 0XFFF8
#define NOT_PAID 0XFFF9
#define NOT_EXIST 0XFFFA
#define ACC_OK 0XFFFB

#define SUB_MAX 0XFFFFFFFF //4294967295
#endif