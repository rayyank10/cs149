#include <fcntl.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
//#include <cstdlib>
#include "bigbag.h"
#include <unistd.h>
#include <string.h>
#include <stdlib.h>


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
    if( argc == 2) {
        fd = open(argv[1], O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
        if (fd == -1) {
            perror(argv[1]);
            exit(3);
        }
        else{
            file_base = mmap(0, BIGBAG_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        }
    }else if (strcmp(argv[1],"-t") == 0  && argc == 3){    //check to see if we want to enter -t mode
        fd = open(argv[2], O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
        if (fd == -1) {
            perror(argv[2]);
            exit(3);
        }
        else {
            file_base = mmap(0, BIGBAG_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
        }

    }
    else
    {
        printf("USAGE -t filename %s %s \n", argv[0],argv[0]);
        exit(2);
    }
    if (file_base == MAP_FAILED) {
        perror("mmap");
        return 3;
    }

    // Header is now the file
    struct bigbag_hdr_s *hdr = file_base;
    struct bigbag_entry_s *first_entry = file_base + hdr->first_free;

    //entry_addr(hdr,hdr->first_free)->next = 0;


    struct stat stat;
    fstat(fd, &stat);
    //if the file does not exist create a new file
    if(stat.st_size != BIGBAG_SIZE)
    {
        ftruncate(fd,BIGBAG_SIZE);
    }
    //initialize the values
    int newFile = 0;
    if(stat.st_size == 0)
    {
        newFile =1;
    }
    if(newFile)
    {
        hdr->magic = 0xA99B15C5;
        hdr->first_free = 12;
        hdr->first_element = 12;

        first_entry->next = 0;
        first_entry->entry_magic = BIGBAG_FREE_ENTRY_MAGIC;
        first_entry->entry_len = BIGBAG_SIZE-MIN_ENTRY_SIZE-12;
    }


    size_t n;
    char *line = NULL;
    //set up the I/O
    while (getline(&line, &n, stdin) > 0) {
        // remove the newline
        size_t endi = strlen(line) - 1;
        if (line[endi] == '\n') {
            line[endi] = '\0';
        }

        //if the user enters l list out al entries
        if (line[0] == 'l' ) {
            struct bigbag_entry_s *entry;
            int offset = hdr->first_element;
            while (offset + sizeof(*entry) < stat.st_size) {
                entry = entry_addr(hdr, offset);

                if (entry == NULL) {
                    break;
                }
                if (entry->entry_magic == BIGBAG_USED_ENTRY_MAGIC) {
                    printf("%s\n", entry->str);
                }

                offset = entry->next;
            }
        }
            //if user enters a set up adding the of the entry
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

                printf("added %s \n", str);
            }
            else {
                printf("out of space");
            }
        }
            //if user enters c check to see if entry is available
        else if (line[0] == 'c')
        {
            struct bigbag_entry_s *ctEntry;
            struct bigbag_entry_s *ltEntry;

            ctEntry = entry_addr(hdr,hdr->first_element);

            char *str = &line[2];
            int check;
            while (ctEntry->next != 0)
            {
                if(strcmp(str, ctEntry->str)==0){
                    check = 0;
                    printf("This string is here \n");
                    break;
                }
                else{
                    check =1;
                }
                ctEntry = entry_addr(hdr,ctEntry->next);

            }
            if(check ==1)
            {
                printf("not here \n");
            }

        }
            // if an invalid command is added
        else
        {

            printf("%s not used correctly", line);
            printf("\n Possible Commands \n a:string_to_add \n c:string_to_check \n l \n");
        }

    }
}
