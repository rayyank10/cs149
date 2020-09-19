#include <stdio.h>
#include <unistd.h>

int countSetBits(int n) {
    //intialize a count variable
    int count = 0;
    //while n is greater than zero use & on n-1 and set that value to n
    while (n > 0) {
        n &= (n - 1);
        //add one to count
        count = count + 1;
    }
    return count;
}

int main(int argc, char **argv) {
    int Bitcount(char *file);

    int bit = 0;
    int totalbits = 0;
    int pid;
    int fds[2];
    pipe(fds);
    if (argc >= 3) {
        for (int i = 1; i < argc; i++) {
            pid = fork();
            if (pid == 0) {
                totalbits += Bitcount(argv[i]);
                write(fds[1], &totalbits, sizeof(totalbits));
                return 0;
            } else if (pid < 0) {
                printf("Fork Failed");
                _exit(3);
            }
        }
        // Parent process
        for (int i = 1; i < argc; i++) {
            read(fds[0], &totalbits, sizeof(totalbits));
            bit += totalbits;
        }
        printf("%i\n", bit);
        return 0;
    }
        else if (argc <= 2) {
            printf("USAGE: %s  filename \n", argv[1]);
            printf("%s %s \n", argv[0], argv[1]);
            printf("$ echo $?\n");
            _exit(1);
    }
}

int Bitcount(char* file){

    char *filename = file;
    FILE *fh;
    fh = fopen(filename, "r");
    if(!fh){
        perror(file);
        printf("$ echo $?\n");
        _exit(2);
    }
    //initialize the ch variable and the ab variable
    int ch = fgetc(fh);
    int bit = 0;
    //while ch doesnt reach the end of file keep getting each integer and counting the set bits and add them to the bit variable
    while (ch != EOF) {
        bit = bit + countSetBits(ch);
        ch = fgetc(fh);
    }
    fclose(fh);
    return bit;
}
