#include <sys/mman.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <execinfo.h>
#include <setjmp.h>
#include <stdint.h>
#include <sys/ucontext.h>


void error(char *msg) {
    perror(msg);
    exit(1);
}

jmp_buf jb;

void mem_sig(int signum) {
    longjmp(jb, 1);
}

const char *regToStr(int reg) {
    switch (reg) {
        case 0:
            return "REG_R8";
        case 1:
            return "REG_R9";
        case 2:
            return "REG_R10";
        case 3:
            return "REG_R11";
        case 4:
            return "REG_R12";
        case 5:
            return "REG_R13";
        case 6:
            return "REG_R14";
        case 7:
            return "REG_R15";
        case 8:
            return "REG_RDI";
        case 9:
            return "REG_RSI";
        case 10:
            return "REG_RBP";
        case 11:
            return "REG_RBX";
        case 12:
            return "REG_RDX";
        case 13:
            return "REG_RAX";
        case 14:
            return "REG_RCX";
        case 15:
            return "REG_RSP";
        default:
            return "!";
    }
}


void handler(int signum, siginfo_t *inf, void *uc_void) {
    ucontext_t *uc = (ucontext_t *) uc_void;

    char *message = malloc(100 * sizeof(char));
    for (int i = 0; i < 16; ++i) {
        printf("%-15s\t0x%x\n", regToStr(i), (unsigned int) uc->uc_mcontext.gregs[i]);
    }

    printf(message, "\n\naddress is 0x%p\n", inf->si_addr);

    printf("____MEM___DUMP____\n");


    uint64_t addr = (uint64_t) inf->si_addr;
    for (uint64_t i = addr - 20; i < addr + 20; ++i) {
        struct sigaction act;

        memset(&act, 0, sizeof(act));

        act.sa_flags = SA_NODEFER;
        act.sa_handler = mem_sig;

        if (sigaction(SIGSEGV, &act, (struct sigaction *) NULL)) {
            error("sigaction");
        }

        if (setjmp(jb) == 0) {
            if (i == addr)
                printf("->");
            printf("%d ", *(char *) i);
        } else {
            printf("!");
        }
    }
    printf("\n");

    exit(2);

}

void make_sf() {
    char *x = "asf;k;asasdfa asdf";
    x[2] = 10;
//    char *a = (char *) mmap(0, 2, PROT_NONE, MAP_ANONYMOUS | MAP_PRIVATE, 0, 0);
//    a[0] = 0;
}


int main() {
    struct sigaction action;

    memset(&action, 0, sizeof(action));

    action.sa_flags = SA_SIGINFO | SA_NODEFER;
    action.sa_sigaction = handler;

    if (sigaction(SIGSEGV, &action, NULL) != 0) {
        error("sigaction");
    }
    make_sf();

    return 0;
}
