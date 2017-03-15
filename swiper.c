/*
 *
 * swiper.c - wipe specified block-device (or file) with randomised blocks of data.
 *
 * No 2 blocks written will be the same and they should compress poorly - to prevent
 * compression/de-dupe impacting amount of data written to disk.
 *
 *
*/

#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <fcntl.h>

#define random rand
#define srandom srand

#define BLOCK_SIZE 4096

char* readable_fs(double, char *);

char buffer[BLOCK_SIZE];
char size[100];
char written[100];
char* devname;

int main(int argc, char *argv[])
{
    int opterr = 0;
    int sync_flag = 0;
    int i, c;
    long bk, byte, limit;
    FILE* fd;

    while ((c = getopt (argc, argv, "sl:d:")) != -1)
        switch (c) {
            case 'l':
                limit = atoi(optarg);;
                break;
            case 's':
                sync_flag = 1;
                break;
            case 'd':
                devname = optarg;
                break;
            case '?':
                if (optopt == 'c')
                    fprintf (stderr,"Option -%c requires an argument.\n",optopt);
                else if (isprint (optopt))
                    fprintf (stderr, "Unknown option `-%c'.\n",optopt);
                else
                    fprintf (stderr,"Unknown option character `\\x%x'.\n",optopt);
                    return 1;
            default:
                abort();
        }

    if (argc<5) {
        printf("\n\tUsage: %s -d device -l number [-s]\n\n",argv[0]);
        printf("\t\t-d : Block-level device to write to.\n");
        printf("\t\t-l : Number of %d byte blocks to write.\n",BLOCK_SIZE);
        printf("\t\t-s : Flag to force synchronous writes.\n\n");
        return 1;
    }

    srandom(time(NULL));
    for (i=0; i<BLOCK_SIZE;i++) {
        unsigned char r=random() % 256;
        buffer[i]=r;
    }

    printf ("\n\tAbout to open %s and write %d x %d blocks (%s).\n\n\t**** This is destructive!!! ****\n\n\tContinue [y|n]?  ",\
        devname,limit,BLOCK_SIZE,readable_fs((float)limit*BLOCK_SIZE,size));
    if (tolower(getchar())!='y') return 0;
    printf("\n");

    fd = open(devname, O_RDWR);//(sync_flag?O_RDWR|O_SYNC:O_RDWR));
    if(NULL == fd)
    {
        printf("\n fopen() Error!!!\n");
        return 1;
    }

    for (i=0; i<limit; i++) {
        write(fd, buffer, BLOCK_SIZE);
        buffer[i % BLOCK_SIZE]=random()%256;
        if (i%10000==0) {
                printf("\r\tWritten %s / %s                    ",readable_fs((float)i*BLOCK_SIZE,written),size);
        }
    }

// readable_fs function - http://programanddesign.com/cpp/human-readable-file-size-in-c/
char* readable_fs(double size, char *buf) {
    int i = 0;
    const char* units[] = {"B", "kB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB"};
    while (size >= 1024) {
        size /= 1024;
        i++;
    }
    sprintf(buf, "%.*f %s", i, size, units[i]);
    return buf;
}
