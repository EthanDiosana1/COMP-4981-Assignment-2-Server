#ifndef ARGUMENTS_H
#define ARGUMENTS_H

struct arguments {
    const int argc;
    const char *argv[];
};

void print_arguments(struct arguments arguments);

#endif
