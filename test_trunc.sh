#!/bin/sh
#
# shellcheck disable=SC3043  # Don't warn about local
#
# Functional tests for ./trunc
#
# Exit immediately if a command exits with a non-zero status.
set -e

PROG=./trunc
TEST_FILE="test_trunc.img"

# Helper function to check file size
check_size() {
    local filename="$1"
    local expected_size="$2"
    local actual_size

    if ! actual_size=$(stat -f %z "$filename" 2>/dev/null); then
        echo "FAIL: $filename does not exist (expected size $expected_size)."
        exit 1
    fi

    if [ "$actual_size" -ne "$expected_size" ]; then
        echo "FAIL: $filename size is $actual_size, expected $expected_size."
        exit 1
    fi
    echo "PASS: $filename size is correct ($expected_size)."
}

# Cleanup function
# shellcheck disable=SC2317  # Don't warn about cleanup being unreachable
cleanup() {
    rm -f "$TEST_FILE"
}
trap cleanup EXIT

echo "--- Testing trunc ---"

# Test 1: Create a new file with specific size (bytes)
echo "Test 1: Create 1024 byte file"
$PROG "$TEST_FILE" 1024
check_size "$TEST_FILE" 1024

# Test 2: Create a file with K suffix
echo "Test 2: Create 2K file"
$PROG "$TEST_FILE" 2K
check_size "$TEST_FILE" 2048

# Test 3: Truncate existing file to smaller size (M suffix)
echo "Test 3: Truncate to 1M"
$PROG "$TEST_FILE" 1M
check_size "$TEST_FILE" $((1 * 1024 * 1024))

# Test 4: Extend existing file (G suffix) - may be sparse
echo "Test 4: Extend to 1G"
$PROG "$TEST_FILE" 1G
check_size "$TEST_FILE" $((1 * 1024 * 1024 * 1024))

# Test 5: Truncate to zero size
echo "Test 5: Truncate to 0"
$PROG "$TEST_FILE" 0
check_size "$TEST_FILE" 0

# Test 6: Invalid size argument (should fail)
echo "Test 6: Invalid size '1X'"
if $PROG "$TEST_FILE" 1X; then
    echo "FAIL: Expected error for invalid size";
    exit 1;
else
    echo "PASS: Correctly failed on invalid size";
fi

echo "--- trunc tests completed successfully ---"
exit 0
