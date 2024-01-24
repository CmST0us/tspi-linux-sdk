#!/bin/bash
# SPDX-License-Identifier: GPL-2.0
#
#  merge_config.sh - Takes a list of config fragment values, and merges
#  them one by one. Provides warnings on overridden values, and specified
#  values that did not make it to the resulting .config file (due to missed
#  dependencies or config symbol removal).
#
#  Portions reused from kconf_check and generate_cfg:
#  http://git.yoctoproject.org/cgit/cgit.cgi/yocto-kernel-tools/tree/tools/kconf_check
#  http://git.yoctoproject.org/cgit/cgit.cgi/yocto-kernel-tools/tree/tools/generate_cfg
#
#  Copyright (c) 2009-2010 Wind River Systems, Inc.
#  Copyright 2011 Linaro
#
#  Based on buildroot(2022.08.x)/support/kconfig/merge_config.sh
#  Copyright (c) 2022 Jeffy Chen <jeffy.chen@rock-chips.com>
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License version 2 as
#  published by the Free Software Foundation.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
#  See the GNU General Public License for more details.

set -e

clean_up() {
	rm -f $TMP_FILE
	rm -f $TMP_MERGE_FILE
}

usage() {
	echo "Usage: $0 [OPTIONS] [CONFIG [...]]"
	echo "  -h    display this help text"
	echo "  -m    only merge the fragments, do not execute the make command"
	echo "  -n    use allnoconfig instead of alldefconfig"
	echo "  -r    list redundant entries when merging fragments"
	echo "  -O    dir to put generated output files.  Consider setting \$KCONFIG_CONFIG instead."
}

RUNMAKE=true
ALLTARGET=alldefconfig
WARNREDUN=false
OUTPUT=.

while true; do
	case $1 in
	"-n")
		ALLTARGET=allnoconfig
		shift
		continue
		;;
	"-m")
		RUNMAKE=false
		shift
		continue
		;;
	"-h")
		usage
		exit
		;;
	"-r")
		WARNREDUN=true
		shift
		continue
		;;
	"-O")
		if [ -d $2 ];then
			OUTPUT=$(echo $2 | sed 's/\/*$//')
		else
			echo "output directory $2 does not exist" 1>&2
			exit 1
		fi
		shift 2
		continue
		;;
	*)
		break
		;;
	esac
done

if [ "$#" -lt 1 ] ; then
	usage
	exit
fi

if [ -z "$KCONFIG_CONFIG" ]; then
	if [ "$OUTPUT" != . ]; then
		KCONFIG_CONFIG=$(readlink -m -- "$OUTPUT/.config")
	else
		KCONFIG_CONFIG=.config
	fi
fi

INITFILE=$1
shift;

if [ ! -r "$INITFILE" ]; then
	echo "The base file '$INITFILE' does not exist.  Exit." >&2
	exit 1
fi

MERGE_LIST=$*
SED_CONFIG_EXP1="s/^\([a-zA-Z0-9_]*\)+\{0,1\}=.*/\1/p"
SED_CONFIG_EXP2="s/^# \([a-zA-Z0-9_]*\) is not set$/\1/p"
SED_CONFIG_EXP3="s/^# \([a-zA-Z0-9_]*\) is reset to default$/\1/p"

SED_CLEAR_EXP1="s/^\([a-zA-Z0-9_]*\)+=/\1=/" # Append
SED_CLEAR_EXP2="/^# \([a-zA-Z0-9_]*\) is reset to default$/d" # Reset

TMP_FILE=$(mktemp ./.tmp.config.XXXXXXXXXX)
TMP_MERGE_FILE=$(mktemp ./.merge_tmp.config.XXXXXXXXXX)

echo "Using $INITFILE as base"

trap clean_up EXIT

cat $INITFILE | sed -e "$SED_CLEAR_EXP1" -e "$SED_CLEAR_EXP2" >> $TMP_FILE

# Merge files, printing warnings on overridden values
for MERGE_FILE in $MERGE_LIST ; do
	echo "Merging $MERGE_FILE"
	if [ ! -r "$MERGE_FILE" ]; then
		echo "The merge file '$MERGE_FILE' does not exist.  Exit." >&2
		exit 1
	fi
	CFG_LIST=$(sed -n -e "$SED_CONFIG_EXP1" -e "$SED_CONFIG_EXP2" \
		-e "$SED_CONFIG_EXP3" $MERGE_FILE)

	cat $MERGE_FILE > $TMP_MERGE_FILE
	for CFG in $CFG_LIST ; do
		grep -q -w $CFG $TMP_FILE || continue
		PREV_VAL=$(grep -w $CFG $TMP_FILE)
		ORIG_NEW_VAL=$(grep -w $CFG $TMP_MERGE_FILE)

		if [ "$ORIG_NEW_VAL" = "# $CFG is reset to default" ]; then
			# Reset
			NEW_VAL=
		elif [ "${ORIG_NEW_VAL%=*}" = "$CFG+" ]; then
			# Append
			if echo "$PREV_VAL" | grep -q "^#"; then
				# Replace
				NEW_VAL=${ORIG_NEW_VAL/+=/=}
			else
				ORIG_NEW_STR=$(echo "$ORIG_NEW_VAL" | \
					sed "s/^.*=\"\(.*\)\"/\1/")
				PREV_STR=$(echo "$PREV_VAL" | \
					sed "s/^.*=\"\(.*\)\"/\1/")

				NEW_STR=
				for s in $ORIG_NEW_STR; do
					echo "$PREV_STR" | xargs -n 1 | \
						grep "^$s$" >/dev/null || \
						NEW_STR="$NEW_STR $s"
				done

				if [ -z "$NEW_STR" ]; then
					# Skip
					NEW_VAL="$PREV_VAL"
				else
					# Append string value
					NEW_VAL="${PREV_VAL%\"} ${NEW_STR# }\""
				fi

				# Update fragment
				sed -i "s#\<$CFG\>.*#$NEW_VAL#" $TMP_MERGE_FILE
			fi
		else
			# Normal replace
			NEW_VAL=$ORIG_NEW_VAL
		fi

		if [ "$PREV_VAL" != "$NEW_VAL" ] ; then
			echo "Value of $CFG is redefined by $MERGE_FILE:"
			echo -e "Previous value:\t$PREV_VAL"
			if [ "$NEW_VAL" != "$ORIG_NEW_VAL" ]; then
				echo -e "Modify value:\t$ORIG_NEW_VAL"
			fi
			echo -e "New value:\t$NEW_VAL"
			echo
		elif [ "$WARNREDUN" = "true" ]; then
			echo "Value of $CFG is redundant by $MERGE_FILE:"
		fi
		sed -i "/\<$CFG[ =]/d" $TMP_FILE
	done
	cat $TMP_MERGE_FILE | sed -e "$SED_CLEAR_EXP1" \
		-e "$SED_CLEAR_EXP2" >> $TMP_FILE
done

if [ "$RUNMAKE" = "false" ]; then
	cp -T -- "$TMP_FILE" "$KCONFIG_CONFIG"
	echo "#"
	echo "# merged configuration written to $KCONFIG_CONFIG (needs make)"
	echo "#"
	exit
fi

# If we have an output dir, setup the O= argument, otherwise leave
# it blank, since O=. will create an unnecessary ./source softlink
OUTPUT_ARG=""
if [ "$OUTPUT" != "." ] ; then
	OUTPUT_ARG="O=$OUTPUT"
fi


# Use the merged file as the starting point for:
# alldefconfig: Fills in any missing symbols with Kconfig default
# allnoconfig: Fills in any missing symbols with # CONFIG_* is not set
make KCONFIG_ALLCONFIG=$TMP_FILE $OUTPUT_ARG $ALLTARGET


# Check all specified config values took (might have missed-dependency issues)
for CFG in $(sed -n -e "$SED_CONFIG_EXP1" -e "$SED_CONFIG_EXP2" $TMP_FILE); do

	REQUESTED_VAL=$(grep -w -e "$CFG" $TMP_FILE)
	ACTUAL_VAL=$(grep -w -e "$CFG" "$KCONFIG_CONFIG" || true)
	if [ "$REQUESTED_VAL" != "$ACTUAL_VAL" ] ; then
		echo "Value requested for $CFG not in final .config"
		echo "Requested value:  $REQUESTED_VAL"
		echo "Actual value:     $ACTUAL_VAL"
		echo ""
	fi
done
