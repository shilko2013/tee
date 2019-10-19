#include <errno.h>
#include <getopt.h>
#include <string.h>
#include <malloc.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>

extern int errno;

const char* ILLEGAL_ARGUMENT_ERROR_MESSAGE = "Illegal argument!\n";
const char* USAGE_MESSAGE = "Usage: tee [-ai] [file...]\n";
const char* INTERRUPT_MESSAGE = "\nProgram was interrupted by SIGINT\n";
const char* FILE_OPEN_ERROR_MESSAGE = "Error in file ";

int* files;
int APPEND_MODE = 0;
int IGNORE_INTERRUPTS = 0;
int ERROR_COUNT = 0;

void handler_interruption() {
    if (!IGNORE_INTERRUPTS) {
        write(2, INTERRUPT_MESSAGE, strlen(INTERRUPT_MESSAGE));
        _exit(ERROR_COUNT);
    }
}

int main(int argc, char *argv[]) {
    signal(SIGINT, handler_interruption);
    int rez = 0;
    while ((rez = getopt(argc, argv, "ai")) != -1) {
        switch (rez) {
            case 'a':
                APPEND_MODE = 1;
                break;
            case 'i':
                IGNORE_INTERRUPTS = 1;
                break;
            case '?':
                write(2, ILLEGAL_ARGUMENT_ERROR_MESSAGE, strlen(ILLEGAL_ARGUMENT_ERROR_MESSAGE));
                write(2, USAGE_MESSAGE, strlen(USAGE_MESSAGE));
                return 1;
        }
    }
    const int FILE_CNT = argc - optind + 1;
    files = malloc(FILE_CNT * sizeof(int));
    files[0] = 1;
    for (int i = optind, current_i = i; current_i < argc; ++i, ++current_i) {
        if (APPEND_MODE)
            files[i - optind + 1] = open(argv[current_i], O_WRONLY | O_APPEND | O_CREAT);
        else
        files[i - optind + 1] = open(argv[current_i], O_WRONLY | O_CREAT | O_TRUNC, 0666);
        if (files[i - optind + 1] < 0) {
            ++ERROR_COUNT;
            --i;
        }
    }
    char input;
    while (read(0, &input, 1) > 0) {
        for (int i = 0; i < FILE_CNT - ERROR_COUNT; ++i)
            write(files[i], &input, 1);
    }
    for (int i = 1; i < FILE_CNT - ERROR_COUNT; ++i)
        close(files[i]);
    free(files);
    return ERROR_COUNT;
}