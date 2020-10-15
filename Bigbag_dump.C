#include <fcntl.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include "bigbag.h"

// Returns the address of the entry
struct bigbag_entry_s *entry_addr(void *hdr, uint32_t offset) {
    if (offset == 0) return NULL;
    return (struct bigbag_entry_s *)((char*)hdr + offset);
}

// Returns the offset of the entry
uint32_t entry_offset(void *hdr, void *entry) {
    return (uint32_t)((uint64_t)entry - (uint64_t)hdr);
}


int main(int argc, char **argv) {

    // Opens the file
    int fd = open(argv[1], O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        perror(argv[1]);
        return 2;
    }

    // Maps the file
    void *file_base = mmap(0, BIGBAG_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (file_base == MAP_FAILED) {
        perror("mmap");
        return 3;
    }

    // Header is now the file
    struct bigbag_hdr_s *hdr = file_base;

    // Stat gives you size
    struct stat stat;
    fstat(fd, &stat);

    // This part begins listing out all the entries in the file 
    printf("size = %ld\n", stat.st_size);
    printf("magic = %08x\n", htonl(hdr->magic));
    printf("first_free = %d\n", hdr->first_free);
    printf("first_element = %d\n", hdr->first_element);

    struct bigbag_entry_s *entry;
    int offset = sizeof(*hdr); // Offset is choosing the entries 

    while (offset + sizeof(*entry) < stat.st_size) {
        entry = entry_addr(hdr, offset); // Changing the entry to look at

        if (entry == NULL) {
            printf("bad entry at offset %d\n", offset);
            break;
        }
        printf("----------------\n");
        printf("entry offset: %d\n", offset); // Location of data
        printf("entry magic: %x\n", (int)entry->entry_magic); // Magic is either 0xFA or 0xDA, meaning free space or used space
        printf("entry len: %d\n", entry->entry_len); // Length of data
        printf("entry next offset: %d\n", entry->next); // Next entry, notice how it isn't printing by next.
        if (entry->entry_magic == BIGBAG_USED_ENTRY_MAGIC) { // Only print string if it is a used space
            printf("entry data: %s\n", entry->str);
        }

        offset += sizeof(*entry) + entry->entry_len; // Offset is being reassigned here, try changing it to something else
    }
}
