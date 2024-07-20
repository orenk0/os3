//Based on Tirgul06
#include "message_slot.h"    

#include <fcntl.h>      /* open */ 
#include <unistd.h>     /* exit */
#include <sys/ioctl.h>  /* ioctl */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char const *argv[])
{
    int file_desc;
    unsigned int channel_id;
    ssize_t message_len;
    char buffer[BUF_LEN];
  
    if (argc != 3) {
        perror("you need 3 parameters\n");
        exit(1);
    }
    channel_id = (unsigned int)strtoul(argv[2], NULL, 10);
    //opening the device file
    file_desc = open(argv[1], O_RDWR);
    if( file_desc < 0 ) {
        perror("Can't open device file\n");
        exit(1);
    }
    //setting the channel id
    if (ioctl(file_desc, MSG_SLOT_CHANNEL, channel_id) < 0) {
        perror("Can't ioctl with this channel id\n");
        close(file_desc);
        exit(1);
    }
    //reading the message from the device:
    message_len = read(file_desc, buffer, BUF_LEN);
    if (message_len < 0) {
        perror("Can't read the message");
        close(file_desc);
        exit(1);
    }
    //closing
    if (close(file_desc) < 0) {
        perror("Can't close the device file\n");
        exit(1);
    }
    if (write(STDOUT_FILENO, buffer, message_len) != message_len) {
        perror("Can't write to standard output");
        close(file_desc);
        exit(1);
    }
    exit(0);
}
