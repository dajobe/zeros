#!/bin/sh
#
# Tests for zero progress output (non-TTY stderr)
#
set -e

PROG=./zero
TEST_FILE="test_progress.out"
LOG="test_progress.log"

cleanup() {
    rm -f "$TEST_FILE" "$LOG"
}
trap cleanup EXIT

echo "--- Testing zero progress output ---"

$PROG -t 1M "$TEST_FILE" 2>"$LOG"

grep -q '%' "$LOG" || { echo "FAIL: progress log missing percent"; exit 1; }
grep -qE 'KiB|MiB|GiB' "$LOG" || { echo "FAIL: progress log missing IEC units"; exit 1; }
grep -q 'wrote' "$LOG" || { echo "FAIL: progress log missing summary"; exit 1; }
grep -q 'buffer' "$LOG" || { echo "FAIL: progress log missing buffer size"; exit 1; }
grep -qE 'fallocate zero-range|write loop' "$LOG" || {
    echo "FAIL: progress log missing fill method";
    exit 1;
}

if grep -q 'ETA' "$LOG" && tail -1 "$LOG" | grep -q 'ETA'; then
    echo "FAIL: final summary should not contain ETA";
    exit 1;
fi

echo "PASS: progress output looks correct"

$PROG -q -t 1M "$TEST_FILE" 2>"$LOG"
if grep -q '%' "$LOG"; then
    echo "FAIL: quiet mode should not print progress percent";
    exit 1;
fi
grep -q 'wrote' "$LOG" || { echo "FAIL: quiet mode missing summary"; exit 1; }

echo "PASS: quiet mode suppresses progress"

echo "--- progress tests completed successfully ---"
