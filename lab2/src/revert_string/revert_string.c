#include <string.h>

#include "revert_string.h"

void RevertString(char *str)
{
    size_t len = strlen(str);

    for (size_t i = 0; i < len / 2; i++)
    {
        char tmp = str[i];

        str[i] = str[len - 1 - i];
        str[len - 1 - i] = tmp;
    }
}
