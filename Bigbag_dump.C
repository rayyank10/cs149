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
   int fd;
    void *file_base;

    if( argc == 2) {                   // here we check if user wants to enter protected mode
        fd = open(argv[1], O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
        if (fd == -1) {
            perror(argv[1]);
            exit(3);
        }
        else{
            ftruncate(fd,BIGBAG_SIZE);
            file_base = mmap(0, BIGBAG_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        }
    }else if (strcmp(argv[1],"-t") == 0  && argc == 3){
        fd = open(argv[2], O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
        if (fd == -1) {
            perror(argv[2]);
            exit(3);
        }
        else {
            ftruncate(fd,BIGBAG_SIZE);
            file_base = mmap(0, BIGBAG_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
        }

    }
    else
    {
        printf("USAGE: %s bos_filename or %s -t bos-filename\n", argv[0],argv[0]);
        exit(2);
    }
  if (file_base == MAP_FAILED) {
        perror("mmap");
        return 3;
    }

    // Header is now the file
    struct bigbag_hdr_s *hdr = static_cast<bigbag_hdr_s *>(file_base);
    struct bigbag_entry_s *first_entry = reinterpret_cast<bigbag_entry_s *>(static_cast<bigbag_hdr_s *>(file_base) +

                                                                            hdr->first_free);
    int newFile = 0;
    if(hdr->first_free == 0)
    {
        newFile =1;
    }
    if(newFile)
    {
        hdr->magic = 0xA99B15C5;
        hdr->first_free = 12;
        hdr->first_element = 12;
    }
    if(newFile)
    {
        first_entry->next = 0;
        first_entry->entry_magic = BIGBAG_FREE_ENTRY_MAGIC;
        first_entry->entry_len = BIGBAG_SIZE-MIN_ENTRY_SIZE-12;
    }
    entry_addr(hdr,hdr->first_free)->next = 0;
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
      if (line[0] == 'l' ) {
                struct bigbag_entry_s *entry;
                int offset = hdr->first_element; // Offset is choosing the entries
                while (offset + sizeof(*entry) < stat.st_size) {
                    entry = entry_addr(hdr, offset); // Changing the entry to look at

                    if (entry == NULL) {
                        printf("bad entry at offset %d\n", offset);
                        break;
                    }
                    if (entry->entry_magic == BIGBAG_USED_ENTRY_MAGIC) { // Only print string if it is a used space
                        printf("%s\n", entry->str);
                    }

                    offset = entry->next;
                }
            }
            else if (line[0] == 'a') {

                struct bigbag_entry_s *bestFitEntry;
                struct bigbag_entry_s *prevEntry;
                struct bigbag_entry_s *currentEntry;

                currentEntry = entry_addr(hdr,hdr->first_element);
                char *str = &line[2];
                int stringSize = strlen(str)+1;
                int offset = hdr->first_free;
              hdr->first_free = hdr->first_free + MIN_ENTRY_SIZE + stringSize;
                struct bigbag_entry_s *freeSpace;
                freeSpace = entry_addr(hdr,hdr->first_free);
                freeSpace->next = 0;
                freeSpace->entry_magic = BIGBAG_FREE_ENTRY_MAGIC;
                bestFitEntry= entry_addr(hdr, offset);
                freeSpace->entry_len = freeSpace->entry_len - stringSize - MIN_ENTRY_SIZE;
                bestFitEntry->entry_magic = BIGBAG_USED_ENTRY_MAGIC;
                bestFitEntry->entry_len = stringSize;
               if(freeSpace->entry_len > stringSize + MIN_ENTRY_SIZE) {
                   strcpy(bestFitEntry->str, str);

                   while (currentEntry->next != 0) {
                       if (strcmp(str, currentEntry->str) < 0) {
                           break;
                       }
                       prevEntry = currentEntry;
                       currentEntry = entry_addr(hdr, currentEntry->next);
                   }
                   if (entry_offset(hdr, currentEntry) != hdr->first_element) {
                       prevEntry->next = entry_offset(hdr, bestFitEntry);
                   } else {
                       hdr->first_element = entry_offset(hdr, bestFitEntry);
                   }
                   bestFitEntry->next = entry_offset(hdr, currentEntry);
                   printf("added %s", str);
                   break;
               }
               else {
                   printf("out of space");
               }
            }
            else if (line[0] == 'c')
            {
                struct bigbag_entry_s *ctEntry;
                ctEntry = entry_addr(hdr,hdr->first_element);
                char *str = &line[2];
                int check;
                while (ctEntry->next != 0)
                {
                    if(strcmp(str, ctEntry->str)==0){
                        printf("This string is already here");
                        check = 0;

                        break;
                    }
                    else{
                        check =1;
                    }
                    ctEntry = entry_addr(hdr,ctEntry->next);

                }
                if(check ==1)
                {
                    printf("not here");
                }
            }
            else
      {

                printf("%s not used correctly", line);
                printf("\n Possible Commands \n a:string_to_add \n c:string_to_check \n l");
      }

        }

    }
