#ifndef MK_H
#define MK_H


typedef unsigned int u32;
typedef unsigned short u16;
typedef unsigned char u8;


#define max_LOC_range 65536
#define max_TEMP_range 125
#define min_TEMP_range -70

#define TEMP_hdr 0xA1
#define LOC_hdr 0xA2
#define STT_hdr 0xA3

#define TEMP_ln 3
#define LOC_ln 67
#define STT_ln 3

#pragma pack (push, 1)
struct A1_tmp{
    u8 hdr;
    char tmp;
    u8 crc;        
};

struct A2_loc{
    u8 hdr;
    u16 loc[32];
    u16 crc;        
};

struct A3_stt{
    u8 hdr;
    u8 stt;
    u8 crc;
};
#pragma pack ( pop)







#endif