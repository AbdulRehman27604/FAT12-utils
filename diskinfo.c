#include "fat12.h"
#include "dir.h"

#include <stdio.h>
#include <stdint.h>

int main(int argc, char *argv[]) {
    char os_name[9];
    char label[20];
    uint32_t total_size;
    uint32_t free_size;
    int file_count;

    /* program expects exactly one disk image argument */
    if (argc != 2) {
        printf("Usage: %s <input file>\n", argv[0]);
        return 1;
    }

    /* open the FAT12 disk image */
    if (!open_image(argv[1])) {
        return 1;
    }

    /* get OS name and disk label from the image */
    trim_str(os_name, boot.oem_name, 8);
    get_disk_label(label, sizeof(label));

    /* calculate total size, free size, and total file count */
    total_size = disk_sectors * boot.bytes_per_sector;
    free_size = get_free_bytes();
    file_count = count_root_files();

    /* stop if file counting failed */
    if (file_count < 0) {
        fprintf(stderr, "Error counting files\n");
        close_image();
        return 1;
    }

    /* print disk information in required format */
    printf("OS Name: %s\n", os_name);
    printf("Label of the disk: %s\n", label);
    printf("Total size of the disk: %u bytes\n", total_size);
    printf("Free size of the disk: %u bytes\n", free_size);
    printf("==============\n");
    printf("The number of files in the disk: %d\n", file_count);
    printf("=============\n");
    printf("Number of FAT copies: %u\n", boot.fat_count);
    printf("Sectors per FAT: %u\n", boot.sectors_per_fat);

    /* close the image before exiting */
    close_image();
    return 0;
}