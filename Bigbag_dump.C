#include <fcntl.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <cstring>
#include <cstdlib>
#include "bigbag.h"
#include <limits.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>


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
    void *base;

    // Opens the file
    int fd = open(argv[1], O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        perror(argv[1]);
        return 2;
    }
    //base = mmap(NULL, 1024*1024, PROT_WRITE | PROT_READ, MAP_SHARED, fd, 0);

    // Maps the file
    void *file_base = mmap(0, BIGBAG_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (file_base == MAP_FAILED) {
        perror("mmap");
        return 3;
    }
    void *end;

    // Header is now the file
    struct bigbag_hdr_s *hdr = static_cast<bigbag_hdr_s *>(file_base);
    struct bigbag_entry_s *first_entry;
    struct stat stat;
    fstat(fd, &stat);
    size_t n;
    char *line = NULL;

    while (getline(&line, &n, stdin) > 0) {
        // remove the newline
        size_t endi = strlen(line) - 1;
        if (line[endi] == '\n') {
            line[endi] = '\0';
        }
        switch (line[0]) {

            case 'l': {

                // This part begins listing out all the entries in the file
                printf("size = %ld\n", stat.st_size);
                printf("magic = %08x\n", htonl(hdr->magic));
                printf("first_free = %d\n", hdr->first_free);
                printf("first_element = %d\n", hdr->first_element);

                struct bigbag_entry_s *entry;
                int offset = hdr->first_element; // Offset is choosing the entries
                while (offset + sizeof(*entry) < stat.st_size) {
                    entry = entry_addr(hdr, offset); // Changing the entry to look at

                    if (entry == NULL) {
                        printf("bad entry at offset %d\n", offset);
                        break;
                    }
                    printf("----------------\n");
                    printf("entry offset: %d\n", offset); // Location of data
                    printf("entry magic: %x\n",
                           (int) entry->entry_magic); // Magic is either 0xFA or 0xDA, meaning free space or used space
                    printf("entry len: %d\n", entry->entry_len); // Length of data
                    printf("entry next offset: %d\n", entry->next); // Next entry, notice how it isn't printing by next.
                    if (entry->entry_magic == BIGBAG_USED_ENTRY_MAGIC) { // Only print string if it is a used space
                        printf("entry data: %s\n", entry->str);
                    }

                    offset = entry->next;
                }
                break;
            }
            case 'a': {
                struct bigbag_entry_s *bestFitEntry;
                int canFit = 0;
                char *str = &line[1];
                int offset = hdr->first_free;
                bestFitEntry= entry_addr(hdr, offset);
               printf(str);
                strcpy(bestFitEntry->str,str);


            }
        }

    }

}
