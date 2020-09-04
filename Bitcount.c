#include <stdio.h>
#include<stdio.h>

//This is an adaption of Brian Kernighan's algorithm

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



int main(int argc, char * argv[]) {

    //open the file

    FILE * fh = fopen(argv[1], "r");

    //if file cannot be opened exit with code 2

    if (fh == NULL) {

        perror(argv[1]);

        exit(2);

    }

    //check to see if there is the right amount of arguments, it has to be 2 becuase the first argument is the program

    if (argc <= 2) {



        //initialize the ch variable and the ab variable

        int ch = fgetc(fh);

        int bit = 0;

        //while ch doesnt reach the end of file keep getting each integer and counting the set bits and add them to the bit variable

        while (ch != EOF) {

            bit = bit + countSetBits(ch);

            ch = fgetc(fh);

        }

        //print the amount of bits

        printf("%d \n", bit);

        //close the file

        fclose(fh);



    }

        //if there are too many arguments print a usage statement and exit with code 1

    else if (argc >= 3) {

        printf("USAGE: %s %s filename \n", argv[1], argv[2]);

        printf("%s %s %s \n", argv[0], argv[1], argv[2]);

        printf("$ echo $?\n");

        exit(1);

    }



}
