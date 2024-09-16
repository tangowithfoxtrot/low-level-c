/* execve.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    if (argc > 3) {
        fprintf(stderr, "Usage: %s <file-to-exec> <argument>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char *newargv[] = {argv[1], argv[2], NULL};
    char *newenviron[] = {"MYVAR1=value1", "MYVAR2=value2", NULL};

    execve(argv[1], newargv, newenviron);
    exit(EXIT_FAILURE);
}
