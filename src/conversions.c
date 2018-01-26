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
    if (str == NULL) {
        return 0;
    }

    unsigned long num = 0;

    for (int i = strlen(str) - 1; i >= 0; i--) {
        unsigned long temp = (str[i] - '0');

        for (int j = strlen(str) - 1; j > i; j--) {
            temp *= 10;
        }
        num += temp;
    }
    return num;
}


// TODO(AnTiZ): evaluate replacement with stdint types
unsigned long long atoull(const char* const str) {
    if (str == NULL) {
        return 0;
    }

    unsigned long long num = 0;

    for (int i = strlen(str) - 1; i >= 0; i--) {
        unsigned long long temp = (str[i] - '0');

        for (int j = strlen(str) - 1; j > i; j--) {
            temp *= 10;
        }
        num += temp;
    }
    return num;
}
