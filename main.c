#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

static struct termios oldattr, newattr;
static char target[4 * 12 + 1];

__attribute__((constructor)) static void prologue(void) {
    tcgetattr(STDIN_FILENO, &oldattr);
    newattr = oldattr;
    newattr.c_lflag &= ~ICANON;
    newattr.c_lflag &= ~ECHO;
    newattr.c_cc[VMIN] = 1;
    newattr.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, &newattr);
}

__attribute__((destructor)) static void epilogue(void) {
    tcsetattr(STDIN_FILENO, TCSANOW, &oldattr);
}

static void usage(FILE *out) {
    fprintf(out, "Usage: pdigit [-l | -s | -r]\n");
}

enum mode {
    MODE_LOW,
    MODE_SHIFT,
    MODE_RANDOM,
};

static enum mode get_mode(int argc, char **argv) {
    if (argc == 1) {
        return MODE_LOW;
    } else if (argc == 2) {
        if (strcmp(argv[1], "-l") == 0) {
            return MODE_LOW;
        } else if (strcmp(argv[1], "-s") == 0) {
            return MODE_SHIFT;
        } else if (strcmp(argv[1], "-r") == 0) {
            return MODE_RANDOM;
        } else if (strcmp(argv[1], "--help") == 0) {
            usage(stdout);
            exit(0);
        } else if (strcmp(argv[1], "-h") == 0) {
            usage(stdout);
            exit(0);
        } else {
            usage(stderr);
            exit(1);
        }
    } else {
        usage(stderr);
        exit(1);
    }
}

static void gen_target(enum mode mode) {
    static const char *pool = "1234567890-=!@#$%^&*()_+";
    switch (mode) {
        case MODE_LOW: {
            strcpy(target, "1111222233334444555566667777888899990000----====");
            return;
        }
        case MODE_SHIFT: {
            strcpy(target, "!!!!@@@@####$$$$%%%%^^^^&&&&****(((())))____++++");
            return;
        }
        case MODE_RANDOM: {
            for (int i = 0; i < sizeof(target); i++) {
                target[i] = pool[random() % strlen(pool)];
            }
            return;
        }
        default:
            // unreachable
            return;
    }
}

int main(int argc, char **argv) {
    srand(time(NULL));
    enum mode mode = get_mode(argc, argv);
    for (int iter = 1;; ++iter) {
        clock_t start = clock();
        int mistype = 0;

        gen_target(mode);

        printf("%d> %s\n", iter, target);
        printf("%d< ", iter);

        int pos = 0;
        while (true) {
            char c = getchar();
            if (target[pos] == c) {
                putchar(c);
                ++pos;
            } else if (c == 'q') {
                goto finish;
            } else {
                ++mistype;
            }

            if (strlen(target) <= pos) {
                break;
            }
        }
        putchar('\n');

        clock_t end = clock();
        printf("mistype: %d\n", mistype);
        printf("elapsed: %f\n", ((double)(end - start)) / CLOCKS_PER_SEC);
    }
finish:
    puts("have a good typing!\n");
    return 0;
}
