/*
iroffer - An IRC file server using the DCC protocol
Copyright (C) see CONTRIBUTORS

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "conversions.h"

#include "string.h"


unsigned long atoul(const char* str) {
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
unsigned long long atoull(const char* str) {
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
