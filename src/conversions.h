/**
 * Declaration of conversion functions
 * @file
 * @copyright see CONTRIBUTORS
 * @license
 * This file is licensed under the GPLv3+ as found in the LICENSE file.
 */

#ifndef IROFFER_CONVERSIONS_H
#define IROFFER_CONVERSIONS_H

/**
 * Convert a string of digits into an unsigned long
 *
 * @param str String to be converted; may be NULL.
 * @return The converted number; 0 if str is NULL.
 */
unsigned long atoul(const char* str);

/**
 * Convert a string of digits into an unsigned long long
 *
 * @param str String to be converted; may be NULL.
 * @return The converted number; 0 if str is NULL.
 */
unsigned long long atoull(const char* str);

#endif // IROFFER_CONVERSIONS_H
