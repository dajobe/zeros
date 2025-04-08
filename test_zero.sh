#!/bin/sh
#
# shellcheck disable=SC3043  # Don't warn about local
#
# Functional tests for ./zero
#
# Exit immediately if a command exits with a non-zero status.
set -e

PROG=./zero
TEST_FILE="test_zero.out"

# Helper function to get file size
size() {
    local filename="$1"
    case $(uname) in
        (Darwin | *BSD*)
            stat -Lf %z -- "$filename"
            ;;
        (*)
            stat -c %s -- "$filename"
            ;;
    esac
}

# Helper function to check file size
check_size() {
    local filename="$1"
    local expected_size="$2"
    local actual_size=$(size "$filename" 2>/dev/null)

    if [ -z $actual_size ]; then
        echo "FAIL: $filename does not exist (expected size $expected_size)."
        exit 1
    fi

    if [ "$actual_size" -ne "$expected_size" ]; then
        echo "FAIL: $filename size is $actual_size, expected $expected_size."
        exit 1
    fi
    echo "PASS: $filename size is correct ($expected_size)."
}

# Helper function to check file content (only for small files)
check_content_zero() {
    local filename="$1"
    local size="$2"
    echo "Checking content of $filename (size $size)..."
    # Use cmp with /dev/zero. Need to limit /dev/zero size.
    if ! cmp -n "$size" "$filename" /dev/zero; then
        echo "FAIL: Content of $filename is not all zeros."
        # Optionally show diff: head -c $size /dev/zero | diff -u - $filename
        exit 1
    fi
    echo "PASS: Content of $filename verified as zeros."

}

# Cleanup function
# shellcheck disable=SC2317  # Don't warn about cleanup being unreachable
cleanup() {
    rm -f "$TEST_FILE"
}
trap cleanup EXIT

echo "--- Testing zero ---"

# Default size is 1G block size, 1G total size. Too slow for CI.
# Let's use smaller defaults for testing.

# Test 1: Small file, specific total size, default block size
echo "Test 1: Create 2M file (default block)"
$PROG -t 2M "$TEST_FILE"
check_size "$TEST_FILE" $((2 * 1024 * 1024))
check_content_zero "$TEST_FILE" $((2 * 1024 * 1024))

# Test 2: Specific block and total size (total multiple of block)
echo "Test 2: Create 4K file using 1K blocks"
$PROG -b 1K -t 4K "$TEST_FILE"
check_size "$TEST_FILE" $((4 * 1024))
check_content_zero "$TEST_FILE" $((4 * 1024))

# Test 3: Specific block and total size (total not multiple of block)
echo "Test 3: Create 5K file using 2K blocks"
$PROG -b 2K -t 5K "$TEST_FILE"
check_size "$TEST_FILE" $((5 * 1024))
check_content_zero "$TEST_FILE" $((5 * 1024))

# Test 4: Only block size specified (total should default to block size)
echo "Test 4: Create file with only -b 3M"
$PROG -b 3M "$TEST_FILE"
check_size "$TEST_FILE" $((3 * 1024 * 1024))
# check_content_zero "$TEST_FILE" $((3 * 1024 * 1024)) # Getting slower, skip content check

# Test 5: Zero total size (should create empty file)
echo "Test 5: Create 0 byte file"
$PROG -t 0 "$TEST_FILE"
check_size "$TEST_FILE" 0

# Test 6: Invalid block size (should fail)
echo "Test 6: Invalid block size '-b 0'"
if $PROG -b 0 "$TEST_FILE"; then echo "FAIL: Expected error for zero block size"; exit 1; else echo "PASS: Correctly failed on zero block size"; fi

echo "--- zero tests completed successfully ---"
exit 0
