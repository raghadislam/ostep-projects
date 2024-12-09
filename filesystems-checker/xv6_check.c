 #include<stdio.h>
#include<stdlib.h>
#include<sys/mman.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<unistd.h>
#include<fcntl.h>
#include<string.h>
#include "fs.h"

char bits[8] = { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 };
#define BITSET(bitmapblocks, blockaddr) ((*(bitmapblocks + blockaddr / 8)) & (bits[blockaddr % 8]))

typedef struct _image_t {
    struct superblock *sb;
    uint firstDataBlock;
    uint numOfInodeBlocks;
    uint numOfBitmapBlocks;
    char *mmapImage;
    char *inodeBlocks;
    char *bitmapBlocks;
    char *dataBlocks;
} image_t;

int check_inode_type(struct dinode *inode) {
    switch(inode->type) {
        case T_DIR:
        case T_FILE:
        case T_DEV:
            break;
        default:
            return 1;
    }

    return 0;
}

int check_inode_dir_blocks(image_t *image, struct dinode *inode) {
    int idx;
    int blockAddr;
    for (idx = 0; idx < NDIRECT; idx++) {
        blockAddr = inode->addrs[idx];
        if (blockAddr == 0)
            continue;

        if (blockAddr < 0 || blockAddr >= image->sb->size) {
            return 1;
        }
    }
    return 0;
}

int check_inode_indirect_blocks(image_t *image, struct dinode *inode) {
    int blockaddr;
    blockaddr = inode->addrs[NDIRECT];
    uint *indirectblk;
    int i;

    if (blockaddr == 0)
        return 0;

    if (blockaddr < 0 || blockaddr >= image->sb->size) {
        return 1;
    }

    indirectblk = (uint *) (image->mmapImage + blockaddr * BSIZE);
    for (i = 0; i < NINDIRECT; i++, indirectblk++) {
        blockaddr = *(indirectblk);
        if (blockaddr == 0)
            continue;

        if (blockaddr < 0 || blockaddr >= image->sb->size) {
            return 1;
        }
    }
    return 0;
}

int check_dir(image_t *image, struct dinode *inode, int inum) {
    int i, j, parentDirEntry = 0, selfDirEntry = 0;
    uint blockAddr;
    struct dirent *entry;

    for (i = 0; i < NDIRECT; i++) {
        blockAddr = inode->addrs[i];
        if (blockAddr == 0)
            continue;

        entry = (struct dirent *) (image->mmapImage + blockAddr * BSIZE);
        for (j = 0; j < DPB; j++, entry++) {
            if (!selfDirEntry && strcmp(".", entry->name) == 0) {
                selfDirEntry = 1;
                if (entry->inum != inum)
                    return 1;
            }

            if (!parentDirEntry && strcmp("..", entry->name) == 0) {
                parentDirEntry = 1;
                if (inum != 1 && entry->inum == inum)
                    return 1;

                if (inum == 1 && entry->inum != inum)
                    return 1;
            }

            if (parentDirEntry && selfDirEntry)
                break;
        }

        if (parentDirEntry && selfDirEntry)
            break;
    }

    if (!parentDirEntry || !selfDirEntry)
        return 1;

    return 0;
}

int check_bitmap_addr(image_t *image, struct dinode *inode) {
    int i, j;
    uint blockAddr;
    uint *indirect;

    for (i = 0; i < (NDIRECT + 1); i++) {
        blockAddr = inode->addrs[i];
        if (blockAddr == 0)
            continue;

        if (!BITSET(image->bitmapBlocks, blockAddr))
            return 1;

        if (i == NDIRECT) {
            indirect = (uint *) (image->mmapImage + blockAddr * BSIZE);
            for (j = 0; j < NINDIRECT; j++, indirect++) {
                blockAddr = *(indirect);
                if (blockAddr == 0)
                    continue;

                if (!BITSET(image->bitmapBlocks, blockAddr))
                    return 1;
            }
        }
    }
    return 0;
}

int inode_check(image_t *image) {
    struct dinode *inode;
    int i, count_not_allocated = 0;
    inode = (struct dinode *)(image->inodeBlocks);

    for(i = 0; i < image->sb->ninodes; i++, inode++) {
        if (inode->type == 0) {
            count_not_allocated++;
            continue;
        }

        if (check_inode_type(inode) != 0) {
            fprintf(stderr, "ERROR: bad inode.\n");
            return 1;
        }

        if (check_inode_dir_blocks(image, inode) != 0) {
            fprintf(stderr, "ERROR: bad direct address in inode.\n");
            return 1;
        }

        if (check_inode_indirect_blocks(image, inode) != 0) {
            fprintf(stderr, "ERROR: bad indirect address in inode.\n");
            return 1;
        }

        if (i == 1 && (inode->type != T_DIR || check_dir(image, inode, i) != 0)) {
            fprintf(stderr, "ERROR: root directory does not exist.\n");
            return 1;
        }

        if (inode->type == T_DIR && check_dir(image, inode, i) != 0) {
            fprintf(stderr, "ERROR: directory not properly formatted.\n");
            return 1;
        }

        if (check_bitmap_addr(image, inode) != 0) {
            fprintf(stderr, "ERROR: address used by inode but marked free in bitmap.\n");
            return 1;
        }

    }

    return 0;
}

void get_used_data_blocks(image_t *image, struct dinode *inode, int *used) {
    int i, j;
    uint blockAddr;
    uint *indirect;

    for (i = 0; i < (NDIRECT + 1); i++) {
        blockAddr = inode->addrs[i];
        if (blockAddr == 0)
            continue;

        used[blockAddr - image->firstDataBlock] = 1;

        if (i == NDIRECT) {
            indirect = (uint *) (image->mmapImage + blockAddr * BSIZE);
            for (j = 0; j < NINDIRECT; j++, indirect++) {
                blockAddr = *(indirect);
                if (blockAddr == 0)
                    continue;

                used[blockAddr - image->firstDataBlock] = 1;
            }
        }
    }
}

int bitmap_check(image_t *image) {
    struct dinode *inode;
    int i;
    int used_dbs[image->sb->nblocks];
    uint blockaddr;
    memset(used_dbs, 0, image->sb->nblocks * sizeof(int));

    inode = (struct dinode *)(image->inodeBlocks);
    for(i = 0; i < image->sb->ninodes; i++, inode++) {
        if (inode->type == 0)
            continue;

        get_used_data_blocks(image, inode, used_dbs);
    }

    for (i = 0; i < image->sb->nblocks; i++) {
        blockaddr = (uint) (i + image->firstDataBlock);
        if (used_dbs[i] == 0 && BITSET(image->bitmapBlocks, blockaddr)) {
            fprintf(stderr, "ERROR: bitmap marks block in use but it is not in use.\n");
            return 1;
        }
    }

    return 0;
}

void fill_direct_used(image_t *image, struct dinode *inode, uint *add) {
    int i;
    uint blockaddr;

    for (i = 0; i < NDIRECT; i++) {
        blockaddr = inode->addrs[i];
        if (blockaddr == 0)
            continue;

        add[blockaddr - image->firstDataBlock]++;
    }
}

void fill_indirect_used(image_t *image, struct dinode *inode, uint *addr) {
    int i;
    uint *indirect;
    uint blockAdd = inode->addrs[NDIRECT];

    indirect = (uint *) (image->mmapImage + blockAdd * BSIZE);
    for (i = 0; i < NINDIRECT; i++, indirect++) {
        blockAdd = *(indirect);
        if (blockAdd == 0)
            continue;

        addr[blockAdd - image->firstDataBlock]++;
    }
}

int blockAdd_check(image_t *image) {
    struct dinode *inode;
    int i;
    uint duaddrs[image->sb->nblocks];
    memset(duaddrs, 0, sizeof(uint) * image->sb->nblocks);

    uint iuaddrs[image->sb->nblocks];
    memset(iuaddrs, 0, sizeof(uint) * image->sb->nblocks);

    inode = (struct dinode *)(image->inodeBlocks);

    for(i = 0; i < image->sb->ninodes; i++, inode++) {
        if (inode->type == 0)
            continue;

        fill_direct_used(image, inode, duaddrs);
        fill_indirect_used(image, inode, iuaddrs);
    }

    for (i = 0; i < image->sb->nblocks; i++) {
        if (duaddrs[i] > 1) {
            fprintf(stderr, "ERROR: direct address used more than once.\n");
            return 1;
        }

        if (iuaddrs[i] > 1) {
            fprintf(stderr, "ERROR: indirect address used more than once.\n");
            return 1;
        }
    }

    return 0;
}

void traverse(image_t *image, struct dinode *rootinode, int *inodemap) {
    uint blockAddr;
    uint *indirect;
    struct dinode *inode;
    struct dirent *dir;

    int i, j;
    if (rootinode->type == T_DIR) {
        for (i = 0; i < NDIRECT; i++) {
            blockAddr = rootinode->addrs[i];
            if (blockAddr == 0)
                continue;

            dir = (struct dirent *) (image->mmapImage + blockAddr * BSIZE);
            for (j = 0; j < DPB; j++, dir++) {
                if (dir->inum != 0 && strcmp(dir->name, ".") != 0 && strcmp(dir->name, "..") != 0) {
                    inode = ((struct dinode *) (image->inodeBlocks)) + dir->inum;
                    inodemap[dir->inum]++;
                    traverse(image, inode, inodemap);
                }
            }
        }

        blockAddr = rootinode->addrs[NDIRECT];
        if (blockAddr != 0) {
            indirect = (uint *) (image->mmapImage + blockAddr * BSIZE);
            for (i = 0; i < NINDIRECT; i++, indirect++) {
                blockAddr = *indirect;

                if (blockAddr == 0)
                    continue;

                dir = (struct dirent *) (image->mmapImage + blockAddr * BSIZE);

                for (j = 0; j < DPB; j++, dir++) {
                    if (dir->inum != 0 && strcmp(dir->name, ".") != 0 && strcmp(dir->name, "..") != 0) {
                        inode = ((struct dinode *) (image->inodeBlocks)) + dir->inum;
                        inodemap[dir->inum]++;
                        traverse(image, inode, inodemap);
                    }
                }
            }
        }
    }
}

int direct_check(image_t *image) {
    int i;
    int inodemap[image->sb->ninodes];
    memset(inodemap, 0, sizeof(int) * image->sb->ninodes);
    struct dinode *inode, *rootInode;

    inode = (struct dinode *) (image->inodeBlocks);
    rootInode = ++inode;

    inodemap[0]++;
    inodemap[1]++;

    traverse(image, rootInode, inodemap);

    inode++;
    for (i = 2; i < image->sb->ninodes; i++, inode++) {
        if (inode->type != 0 && inodemap[i] == 0) {
            fprintf(stderr, "ERROR: inode marked use but not found in a directory.\n");
            return 1;
        }

        if (inodemap[i] > 0 && inode->type == 0) {
            fprintf(stderr, "ERROR: inode referred to in directory but marked free.\n");
            return 1;
        }

        if (inode->type == T_FILE && inode->nlink != inodemap[i]) {
            fprintf(stderr, "ERROR: bad reference count for file.\n");
            return 1;
        }

        if (inode->type == T_DIR && inodemap[i] > 1) {
            fprintf(stderr, "ERROR: directory appears more than once in file system.\n");
            return 1;
        }
    }
    return 0;
}

int fileSystemChecker(char *filename) {
	int fd = open(filename, O_RDONLY, 0);
    image_t image;
	struct stat st;
	char *mmapImage;
    int rv = 0;

    if (fd == -1) {
		fprintf(stderr, "image not found.\n");
        return 1;
    }
    if (fstat(fd, &st) != 0) {
        fprintf(stderr, "failed to fstat file %s\n", filename);
        return 1;
    }

	mmapImage = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE | MAP_POPULATE, fd, 0);
    image.mmapImage = mmapImage;
	image.sb = (struct superblock *) (mmapImage + BSIZE);
    image.numOfInodeBlocks = (image.sb->ninodes / (IPB)) + 1;
    image.numOfBitmapBlocks =  (image.sb->size / (BPB)) + 1;
    image.inodeBlocks = (char *) (mmapImage + BSIZE * 2);
    image.bitmapBlocks = (char *) (image.inodeBlocks + BSIZE * image.numOfInodeBlocks);
    image.dataBlocks = (char *) (image.bitmapBlocks + BSIZE * image.numOfBitmapBlocks);
    image.firstDataBlock = image.numOfInodeBlocks + image.numOfBitmapBlocks + 2;


    rv = inode_check(&image);
    if (rv != 0)
        goto cleanup;

    rv = bitmap_check(&image);
    if (rv != 0)
        goto cleanup;

    rv = blockAdd_check(&image);
    if (rv != 0)
        goto cleanup;

    rv = direct_check(&image);
    if (rv != 0)
        goto cleanup;

cleanup:
    munmap(mmapImage, st.st_size);
	close(fd);

	return rv;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: xcheck <file_system_image>\n");
        exit(1);
    }

    return fileSystemChecker(argv[1]);
}  
