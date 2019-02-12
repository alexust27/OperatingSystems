#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <errno.h>
#include <stdlib.h>

char prog[] = {
        0x55, 0x48, 0x89,
        0xe5, 0x89, 0x7d,
        0xfc, 0x89, 0x75,
        0xf8, 0x8b,
        0x55,
        0xfc, 0x8b, 0x45,
        0xf8, 0x01, 0xd0,
        0x5d, 0xc3
};//program for +

int x;
int y;
char op;

void make_end(int offset){
    prog[offset] = 0xf8;
    prog[offset + 1] = 0x5d;
    prog[offset + 2] = 0xc3;
}

void change_prog(){
    prog[11] = 0x45;
    prog[19] = 0x00;
    switch(op) {
        case '+' :
            prog[11] = 0x55;
            prog[19] = 0xc3;
            break;
        case '-' :
            prog[13] = 0x2b;
            make_end(15);
            prog[18] = 0x00;
            break;
        case '*':
            prog[13] = 0x0f;
            prog[14] = 0xaf;
            prog[15] = 0x45;
            make_end(16);
            break;
        default:
            prog[13] = 0x99;
            prog[14] = 0xf7;
            prog[15] = 0x7d;
            make_end(16);
            break;
    }
}

void read_nums() {
    char* str = NULL;
    size_t len = 0;
    int sz = (int) getline(&str, &len, stdin);
    char clean_str[sz];
    int j = 0;
    for(int i = 0; i < sz; ++i){
        if(str[i] != ' ' && str[i] != '\n') {
            clean_str[j] = str[i];
            j++;
        }
    }
    x = 0; y = 0;
    int findx = 0;
    char ch;
    for(int i = 0; i < j; ++i) {
        ch = clean_str[i];
        if (findx == 0)
            if(ch >='0' && ch <='9') {
                x = x * 10 + (ch -'0');
            }
            else {
                findx = 1;
                op = ch;
            }
        else {
            if (ch >='0' && ch <='9')
                y = y * 10 + (ch - '0');
        }
    }
}

int main(int argc, char **argv) {

    printf("Please type the expression: \n");

    read_nums();
    change_prog();

    void * mm = mmap(NULL, sizeof(prog), PROT_READ | PROT_WRITE
                                         | PROT_EXEC, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);

    if (mm == MAP_FAILED) {
        perror("map");
        exit(1);
    }

    memcpy(mm, prog, sizeof(prog));

    if (mprotect(mm, sizeof(prog), PROT_READ | PROT_EXEC) == -1) {
        perror("protect");
        exit(1);
    }


    int res = ((int(*)(int, int))mm)(x,y);

    if (munmap(mm, sizeof(prog)) == -1) {
        perror("mmap");
        exit(1);
    }
    printf("%d %c %d = %d\n", x, op, y, res);
   return 0;
}