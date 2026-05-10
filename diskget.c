#include "fat12.h"
#include "dir.h"

#include <stdio.h>

int main(int argc, char *argv[]) {
    DirEntry entry;
    const char *out_name;

    /* program needs disk image name and file name */
    if (argc != 3) {
        printf("Usage: %s <input file> <filename>\n", argv[0]);
        return 1;
    }

    /* open the FAT12 disk image */
    if (!open_image(argv[1])) {
        return 1;
    }

    /* diskget only searches for the file in the root directory */
    if (!find_root_file(argv[2], &entry)) {
        printf("File not found.\n");
        close_image();
        return 1;
    }

    /* output file is created in current Linux directory */
    out_name = get_base_name(argv[2]);

    /* copy file contents from FAT12 image to Linux file */
    if (!copy_out_file(&entry, out_name)) {
        close_image();
        return 1;
    }

    /* optional success message for testing */
    printf("File copied successfully.\n");

    /* close image before exiting */
    close_image();
    return 0;
}