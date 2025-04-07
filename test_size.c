#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h> /* For LLONG_MAX */
#include "size.h"

/* Simple assertion macro */
#define ASSERT_EQ(test_name, expected, actual) \
    if ((expected) != (actual)) { \
        fprintf(stderr, "FAIL: %s: Expected %lld, got %lld\n", \
                test_name, (long long)(expected), (long long)(actual)); \
        exit(1); \
    } else { \
        /* printf("PASS: %s\n", test_name); */ \
    }

#define ASSERT_LT(test_name, val1, val2) \
    if (!((val1) < (val2))) { \
        fprintf(stderr, "FAIL: %s: Expected %lld < %lld\n", \
                test_name, (long long)(val1), (long long)(val2)); \
        exit(1); \
    } else { \
        /* printf("PASS: %s\n", test_name); */ \
    }

int main() {
    long long result;
    long long tiny_max = 1500;

    printf("Running size parsing tests...\n");

    /* Test basic numbers */
    result = parse_size("1024", 0);
    ASSERT_EQ("Basic number", 1024, result);

    result = parse_size("0", 0);
    ASSERT_EQ("Zero", 0, result);

    result = parse_size("1", 0);
    ASSERT_EQ("One", 1, result);

    /* Test suffixes (case-insensitive) */
    result = parse_size("1k", 0);
    ASSERT_EQ("Suffix k lower", 1LL << 10, result);

    result = parse_size("1K", 0);
    ASSERT_EQ("Suffix K upper", 1LL << 10, result);

    result = parse_size("2m", 0);
    ASSERT_EQ("Suffix m lower", 2LL << 20, result);

    result = parse_size("2M", 0);
    ASSERT_EQ("Suffix M upper", 2LL << 20, result);

    result = parse_size("3g", 0);
    ASSERT_EQ("Suffix g lower", 3LL << 30, result);

    result = parse_size("3G", 0);
    ASSERT_EQ("Suffix G upper", 3LL << 30, result);

    result = parse_size("4t", 0);
    ASSERT_EQ("Suffix t lower", 4LL << 40, result);

    result = parse_size("4T", 0);
    ASSERT_EQ("Suffix T upper", 4LL << 40, result);

    result = parse_size("5p", 0);
    ASSERT_EQ("Suffix p lower", 5LL << 50, result);

    result = parse_size("5P", 0);
    ASSERT_EQ("Suffix P upper", 5LL << 50, result);

    /* Test max_size constraint */
    result = parse_size("2K", tiny_max); /* 2048 > 1500 */
    ASSERT_LT("Exceeds max_size (suffix)", result, 0);

    result = parse_size("1600", tiny_max); /* 1600 > 1500 */
    ASSERT_LT("Exceeds max_size (no suffix)", result, 0);

    result = parse_size("1K", tiny_max); /* 1024 < 1500 */
    ASSERT_EQ("Within max_size (suffix)", 1024, result);

    result = parse_size("1400", tiny_max); /* 1400 < 1500 */
    ASSERT_EQ("Within max_size (no suffix)", 1400, result);

    /* Test maximum value without explicit max_size (should use LLONG_MAX) */
    /* Find largest power of 2 suffix */
#if LLONG_MAX >> 60 > 0
    /* We can likely test 'P' */
    char max_suffix_val[30];
    sprintf(max_suffix_val, "%lldP", LLONG_MAX >> 50);
    result = parse_size(max_suffix_val, 0);
    ASSERT_EQ("Max value with P suffix", (LLONG_MAX >> 50) << 50, result);

    sprintf(max_suffix_val, "%lldP", (LLONG_MAX >> 50) + 1 );
    result = parse_size(max_suffix_val, 0);
    ASSERT_LT("Overflow value with P suffix", result, 0);
#else
    /* Test with 'T' as likely max suffix */
    char max_suffix_val[30];
    sprintf(max_suffix_val, "%lldT", LLONG_MAX >> 40);
    result = parse_size(max_suffix_val, 0);
    ASSERT_EQ("Max value with T suffix", (LLONG_MAX >> 40) << 40, result);

    sprintf(max_suffix_val, "%lldT", (LLONG_MAX >> 40) + 1 );
    result = parse_size(max_suffix_val, 0);
    ASSERT_LT("Overflow value with T suffix", result, 0);
#endif

    /* Arbitrary leading space is ok for strtoll */    
    result = parse_size(" 10G", 0);
    ASSERT_EQ("Leading space", 10LL<<30, result);

    /* Test invalid inputs */
    result = parse_size("-1", 0);
    ASSERT_LT("Negative number", result, 0);

    result = parse_size("abc", 0);
    ASSERT_LT("Non-numeric", result, 0);

    result = parse_size("10X", 0);
    ASSERT_LT("Invalid suffix", result, 0);

    result = parse_size("10Gabc", 0);
    ASSERT_LT("Suffix with extra chars", result, 0);

    /* Space before suffix is invalid */    
    result = parse_size("10 G", 0);
    ASSERT_LT("Space before suffix", result, 0);

    result = parse_size("", 0);
    ASSERT_LT("Empty string", result, 0);

    /* Test large number string without suffix potentially exceeding LLONG_MAX */
    /* This string represents LLONG_MAX + 1 */
    char too_large[40];
    sprintf(too_large, "%lld", LLONG_MAX / 10); /* Most digits */
    strcat(too_large, "99"); /* Add digits to definitely overflow */
    /* Note: strtoll itself will catch this overflow and set errno */
    result = parse_size(too_large, 0);
    ASSERT_LT("Number exceeds LLONG_MAX", result, 0);

    printf("All size parsing tests passed.\n");
    return 0;
}
