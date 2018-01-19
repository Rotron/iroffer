/**
 * Implementation of conversion functions
 * @file
 * @copyright see CONTRIBUTORS
 * @license
 * This file is licensed under the GPLv3+ as found in the LICENSE file.
 */

#include "conversions.h"

#include <string.h>


unsigned long atoul(const char* const str) {
    unsigned long num, temp;
    int i, j;
    if (str == NULL) {
        return 0;
    }

    num = 0;

    for (i = strlen(str) - 1; i >= 0; i--) {
        temp = (str[i] - '0');
        for (j = strlen(str) - 1; j > i; j--) {
            temp *= 10;
        }
        num += temp;
    }
    return num;
}


// TODO(AnTiZ): evaluate replacement with stdint types
unsigned long long atoull(const char* const str) {
    unsigned long long num, temp;
    int i, j;
    if (str == NULL) {
        return 0;
    }

    num = 0;

    for (i = strlen(str) - 1; i >= 0; i--) {
        temp = (str[i] - '0');
        for (j = strlen(str) - 1; j > i; j--) {
            temp *= 10;
        }
        num += temp;
    }
    return num;
}
