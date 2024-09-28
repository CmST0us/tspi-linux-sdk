#!/bin/bash -e

INPUT=$1
OUTPUT=$2
SIZE=$3
BUF=$4

if [ ! -e "$INPUT" ]; then
	echo "ERROR: No input file \"$INPUT\""
	exit 1
fi

if [ "$SIZE" -gt 1024 ]; then
	echo "ERROR: SIZE bigger than 1K"
	exit 1
fi

BIG_END=$[SIZE / 256]
LIT_END=$[SIZE - (BIG_END * 256)]
BIG_END=$(echo "ibase=10;obase=16;$BIG_END" | bc)
LIT_END=$(echo "ibase=10;obase=16;$LIT_END" | bc)

rm -rf "$OUTPUT"
dd if="$INPUT" of="$OUTPUT" bs=1k count=10
echo -en "\x$LIT_END\x$BIG_END" >> "$OUTPUT"
echo -n "$BUF" >> "$OUTPUT"
SKIP=$[10 * 1024 + SIZE + 2]
dd if="$INPUT" of="$OUTPUT" seek=$SKIP skip=$SKIP bs=1
