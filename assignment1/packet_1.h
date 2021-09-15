#ifndef PACKET_1_H
#define PACKET_1_H

// DATA
typedef struct data_packet {
    short start_id;
    char client_id; // use char since it is 1 byte data
    short type;
    char seg_no;
    char length; // same as client_id
    char payload[255];
    short end_id;
} data_packet;

// we don't know if what package server will response
// merge the ack package and reject package into one
// depense on if response package having reject sub code.
typedef struct res_packet {
    short start_id;
    char client_id;
    short type;
    short rej_sub_code;
    char recv_seg_no;
    short end_id;
} res_packet;

#define START_ID 0XFFFF
#define END_ID 0XFFFF
#define CLIENT_ID_MAX 0XFF //255
#define DATA_LENGTH 0XFF //255

// packet type
#define DATA_PACKET 0XFFF1
#define ACK_PACKET 0XFFF2
#define REJECT_PACKET 0XFFF3

// sub code in reject package
#define OUT_OF_SEQ 0XFFF4
#define LEN_MISMATCH 0XFFF5
#define END_ID_MISSING 0XFFF6
#define DUPLICATE_PACKET 0XFFF7

#endif
