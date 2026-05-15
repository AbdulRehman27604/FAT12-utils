#include "fat12.h"
#include "dir.h"

#include <stdio.h>

int main(int argc, char *argv[]) {
    int ok;

    /* program expects exactly one disk image argument */
    if (argc != 2) {
        printf("Usage: %s <input file>\n", argv[0]);
        return 1;
    }

    /* open the FAT12 disk image */
    if (!open_image(argv[1])) {
        return 1;
    }

    /* list root directory and all subdirectories */
    ok = list_root();

    /* close the disk image before exiting */
    close_image();

    return ok ? 0 : 1;
}