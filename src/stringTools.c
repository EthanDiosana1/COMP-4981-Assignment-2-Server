#include "stringTools.h"

#include <regex.h>

int isValidIp(char * ip)
{
    regex_t regex;
    int regex_return;

    regex_return = regcomp(&regex, "", 0);

    if (regex_return != 0)
    {
        perror("RegEx failed to compile.\n");
        return;
    }

    return regex_return;
}
