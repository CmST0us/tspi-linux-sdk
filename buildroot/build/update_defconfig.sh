#!/bin/bash
#
# Usage:
# update_defconfig.sh <defconfig> [output dir]

set -e

# Usage: savedefconfig <output defconfig> [input defconfig]
savedefconfig()
{
	# Save original .config
	gzip -fk "$CONFIG" &>/dev/null || true

	[ -z "$2" ] || \
		"$SCRIPT_DIR/parse_defconfig.sh" "$2" "$CONFIG" > /dev/null

	echo "BR2_DEFCONFIG=\"$1\"" >> "$CONFIG"
	HOSTCC=gcc make O="$OUTPUT_DIR" savedefconfig >/dev/null

	# Restore original .config
	gunzip -fk "$CONFIG.gz" &>/dev/null || true
}

BOARD="$(basename "${1%_defconfig}")"
[ -n "$BOARD" ] || exit 1

SCRIPT_DIR="$(realpath "$(dirname "$0")")"
BUILDROOT_DIR="$(dirname "$SCRIPT_DIR")"
OUTPUT_DIR="${2:-$BUILDROOT_DIR/output/$BOARD}"
cd "$BUILDROOT_DIR"

DEFCONFIG="configs/${BOARD}_defconfig"
[ -r "$DEFCONFIG" ] || exit 1

echo "Updating defconfig: $DEFCONFIG"

CONFIG="$OUTPUT_DIR/.config"
ORIG_DEFCONFIG="$OUTPUT_DIR/.defconfig"
BASE_DEFCONFIG="$OUTPUT_DIR/.base_defconfig"
NEW_DEFCONFIG="$OUTPUT_DIR/.new_defconfig"
FRAGMENT="$OUTPUT_DIR/.fragment"

# Generate original defconfig
if [ ! -r "$CONFIG" ]; then
	mkdir -p "$OUTPUT_DIR"
	savedefconfig "$ORIG_DEFCONFIG" "$DEFCONFIG"
else
	savedefconfig "$ORIG_DEFCONFIG"
fi
echo "Original defconfig saved to $ORIG_DEFCONFIG"

# Generate base defconfig
grep "^#include " $DEFCONFIG > "$FRAGMENT" || true
savedefconfig "$BASE_DEFCONFIG" "$FRAGMENT"
echo "Base defconfig saved to $BASE_DEFCONFIG"

# Update defconfig fragment
SED_CONFIG_EXP1="s/^\(<\|>\) \([a-zA-Z0-9_]*\)=.*/\2/p"
SED_CONFIG_EXP2="s/^\(<\|>\) # \([a-zA-Z0-9_]*\) is not set$/\2/p"
SED_STRING_EXP="s/^.*=\"\(.*\)\"/\1/p"

CFG_LIST=$(diff "$ORIG_DEFCONFIG" "$BASE_DEFCONFIG" | \
	sed -n -e "$SED_CONFIG_EXP1" -e "$SED_CONFIG_EXP2" | sort | uniq)

for CFG in $CFG_LIST ; do
	BASE_VAL=$(grep -w $CFG "$BASE_DEFCONFIG" || true)
	ORIG_NEW_VAL=$(grep -w $CFG "$ORIG_DEFCONFIG" || true)

	if [ -z "$ORIG_NEW_VAL" ]; then
		# Reset to default
		NEW_VAL="# $CFG is reset to default"
	else
		# Replace
		NEW_VAL="$ORIG_NEW_VAL"

		if grep -q "^$CFG+=" "$DEFCONFIG"; then
			BASE_STR=$(echo "$BASE_VAL" | sed -n "$SED_STRING_EXP")
			NEW_STR=$(echo "$NEW_VAL" | sed -n "$SED_STRING_EXP")
			if [ -n "$BASE_STR" -a -n "$NEW_VAL" ]; then
				# Try to extract additional strings
				for s in $BASE_STR; do
					NEW_STR=$(echo "$NEW_STR" | \
						xargs -n 1 | grep -v "^$s$" | \
						xargs)
				done

				NEW_VAL="${NEW_STR:+$CFG+=\"$NEW_STR\"}"
			fi
		fi
	fi

	echo "$NEW_VAL" >> $FRAGMENT

	echo "Value of $CFG applied to fragment:"
	echo -e "Base value:\t$BASE_VAL"
	if [ -n "$ORIG_NEW_VAL" -a "$NEW_VAL" != "$ORIG_NEW_VAL" ]; then
		echo -e "New value:\t$ORIG_NEW_VAL"
	fi
	echo -e "Final value:\t$NEW_VAL"
	echo
done

# Update defconfig fragment for dependency changes
savedefconfig "$NEW_DEFCONFIG" "$FRAGMENT"

CFG_LIST=$(diff "$ORIG_DEFCONFIG" "$NEW_DEFCONFIG" | \
	sed -n -e "$SED_CONFIG_EXP1" -e "$SED_CONFIG_EXP2" | sort | uniq)
for CFG in $CFG_LIST ; do
	grep -wq $CFG "$FRAGMENT" || \
		echo "# $CFG is reset to default" >> $FRAGMENT
done

cat $FRAGMENT > $DEFCONFIG

# Verify defconfig fragment
savedefconfig "$NEW_DEFCONFIG" "$DEFCONFIG"
if diff "$ORIG_DEFCONFIG" "$NEW_DEFCONFIG" | grep ""; then
	echo "Configs unmatched, might be something wrong."
	exit 1
fi

# Strip unneeded config resets
TEMP_FILE=$(mktemp)
for CFG in $(grep " is reset to default$" "$DEFCONFIG") ; do
	grep -wv "$CFG" "$DEFCONFIG" > $TEMP_FILE
	savedefconfig "$NEW_DEFCONFIG" $TEMP_FILE
	if ! diff "$ORIG_DEFCONFIG" "$NEW_DEFCONFIG" | grep -q ""; then
		cat $TEMP_FILE > "$DEFCONFIG"
	fi
done

echo "Done updating $DEFCONFIG."
