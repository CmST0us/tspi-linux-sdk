/*************************************************************************
	> File Name: rkboot_control.h
	> Author: jkand.huang
	> Mail: jkand.huang@rock-chips.com
	> Created Time: Wed 09 Jan 2019 02:57:43 PM CST
 ************************************************************************/

#ifndef _RKBOOT_CONTROL_H
#define _RKBOOT_CONTROL_H

#define AB_SLOT_NUM  2

/* Magic for the A/B struct when serialized. */
#define AVB_AB_MAGIC "\0AB0"
#define AVB_AB_MAGIC_LEN 4

/* Versioning for the on-disk A/B metadata - keep in sync with avbtool. */
#define AVB_AB_MAJOR_VERSION 1
#define AVB_AB_MINOR_VERSION 0

/* Size of AvbABData struct. */
#define AVB_AB_DATA_SIZE 32

/* Maximum values for slot data */
#define AVB_AB_MAX_PRIORITY 15
#define AVB_AB_MAX_TRIES_REMAINING 7

#define MISC_OFFSET 2048
#define MISC_OFFSET_CMDLINE 4096
#define MISC_PARTITION_NMAE "/dev/block/by-name/misc"

#define CMDLINE_LENGTH 2048

#define CMD_WIPE_USERDATA "cmd_wipe_userdata"

/*
struct rk_ab {
    unsigned char magic[AB_MAGIC_LEN];
    unsigned int version;
    int last_boot;
    int use_a;
    int use_b;
    int current_boot;
    unsigned char reserved[12];
    unsigned int crc32;
};
*/

typedef struct AvbABSlotData {
    /* Slot priority. Valid values range from 0 to AVB_AB_MAX_PRIORITY,
     * both inclusive with 1 being the lowest and AVB_AB_MAX_PRIORITY
     * being the highest. The special value 0 is used to indicate the
     * slot is unbootable.
     */
    unsigned char priority;//0,14,15

    /* Number of times left attempting to boot this slot ranging from 0
     * to AVB_AB_MAX_TRIES_REMAINING.
     */
    unsigned char tries_remaining;//7--,成功启动，设为0

    /* Non-zero if this slot has booted successfully, 0 otherwise. */
    unsigned char successful_boot;//0,1

    /* Reserved for future use. */
    unsigned char reserved[1];
}AvbABSlotData;

/* Struct used for recording A/B metadata.
 *
 * When serialized, data is stored in network byte-order.
 */
typedef struct AvbABData {
    /* Magic number used for identification - see AVB_AB_MAGIC. */
    unsigned char magic[AVB_AB_MAGIC_LEN];

    /* Version of on-disk struct - see AVB_AB_{MAJOR,MINOR}_VERSION. */
    unsigned char version_major; //AVB_AB_MAJOR_VERSION
    unsigned char version_minor; //AVB_AB_MINOR_VERSION

    /* Padding to ensure |slots| field start eight bytes in. */
    unsigned char reserved1[2];

    /* Per-slot metadata. */
    AvbABSlotData slots[2];

    /* Reserved for future use. */
    unsigned char last_boot;//默认a，上一次成功启动slot的标志位，0-->a，1-->b
    unsigned char reserved2[11];

    /* CRC32 of all 28 bytes preceding this field. */
    unsigned int crc32;
}AvbABData;

int setSlotActivity();
int setSlotSucceed();
int readMisc(struct AvbABData *info);
void display(struct AvbABData info);

bool wipe_userdata(bool auto_reboot);
#endif
