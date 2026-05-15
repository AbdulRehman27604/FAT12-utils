#include "fat12.h"
#include "dir.h"

#include <stdio.h>
#include <sys/stat.h>

int main(int argc, char *argv[]) {
    DirPos pos;
    char linux_name[PATH_MAX];
    char fat_name[13];
    struct stat st;
    FILE *src = NULL;
    long entry_offset;
    uint16_t first_cluster;
    int ok = 1;

    /* program needs disk image name and target path */
    if (argc != 3) {
        printf("Usage: %s <input file> <filename>\n", argv[0]);
        return 1;
    }

    /* open the FAT12 disk image */
    if (!open_image(argv[1])) {
        return 1;
    }

    /* find which directory inside the image the file should go into */
    if (!get_target_dir(argv[2], &pos, linux_name)) {
        printf("The directory not found.\n");
        close_image();
        return 1;
    }

    /* open the source file from current Linux directory */
    src = fopen(linux_name, "rb");
    if (src == NULL) {
        printf("File not found.\n");
        close_image();
        return 1;
    }

    /* get file information like size and modification time */
    if (stat(linux_name, &st) != 0) {
        printf("File not found.\n");
        fclose(src);
        close_image();
        return 1;
    }

    /* convert file name into FAT 8.3 format */
    if (!make_fat_name(linux_name, fat_name, sizeof(fat_name))) {
        printf("File not found.\n");
        fclose(src);
        close_image();
        return 1;
    }

    /* check whether disk image has enough free clusters */
    {
        uint32_t cluster_bytes =
            (uint32_t)boot.bytes_per_sector * boot.sectors_per_cluster;
        uint32_t needed =
            (st.st_size == 0) ? 1
            : (uint32_t)((st.st_size + cluster_bytes - 1) / cluster_bytes);
        uint32_t free_clusters = get_free_bytes() / cluster_bytes;

        if (free_clusters < needed) {
            printf("No enough free space in the disk image.\n");
            fclose(src);
            close_image();
            return 1;
        }
    }

    /* do not overwrite if a file with same name already exists */
    if (file_exists_in_dir(&pos, fat_name)) {
        printf("File already exists.\n");
        fclose(src);
        close_image();
        return 1;
    }

    /* find an empty directory entry slot */
    if (pos.root) {
        if (!find_free_root_slot(&entry_offset)) {
            printf("No enough free space in the disk image.\n");
            fclose(src);
            close_image();
            return 1;
        }
    } else {
        if (!find_free_slot_in_cluster(pos.cluster, &entry_offset)) {
            printf("No enough free space in the disk image.\n");
            fclose(src);
            close_image();
            return 1;
        }
    }

    /* allocate clusters for the new file data */
    if (!alloc_clusters((uint32_t)st.st_size, &first_cluster)) {
        printf("No enough free space in the disk image.\n");
        fclose(src);
        close_image();
        return 1;
    }

    /* write the file contents into those clusters */
    if (!write_file_clusters(src, first_cluster, (uint32_t)st.st_size)) {
        free_cluster_chain(first_cluster);
        ok = 0;
    }

    /* write the new directory entry into root or subdirectory */
    if (ok && !write_dir_entry(entry_offset, linux_name, first_cluster,
                               (uint32_t)st.st_size, st.st_mtime)) {
        free_cluster_chain(first_cluster);
        ok = 0;
    }

    /* clean up before exiting */
    fclose(src);
    close_image();

    return ok ? 0 : 1;
}