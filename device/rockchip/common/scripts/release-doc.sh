#!/bin/bash -e

SCRIPTS_DIR="$(dirname "$(realpath "$BASH_SOURCE")")"
DEVICE_DIR="$(realpath "$SCRIPTS_DIR/../../")"
SDK_DIR="$(realpath "$DEVICE_DIR/../../")"
CHIPS_DIR="$DEVICE_DIR/.chips"

choose_chip()
{
	CHIP_ARRAY=( $(ls "$CHIPS_DIR") )
	CHIP_ARRAY_LEN=${#CHIP_ARRAY[@]}
	echo "Pick a chip:"
	echo ""

	echo ${CHIP_ARRAY[@]} | xargs -n 1 | sed "=" | sed "N;s/\n/. /"

	local INDEX
	read -p "Which would you like? [1]: " INDEX
	INDEX=$((${INDEX:-1} - 1))
	CHIP="${CHIP_ARRAY[$INDEX]}"
}

CHIP=$1
if [ -z "$CHIP" -o ! -e "$CHIPS_DIR/$CHIP" ]; then
	choose_chip
	[ "$CHIP" ] || exit 1
fi

DOC_DIR="$2"
if [ -z "$DOC_DIR" ]; then
	for d in $(find "$SDK_DIR/docs" -name Socs); do
		"$0" "$CHIP" "$d"
	done
	exit 0
fi

SOC_DIR=$(echo $CHIP | tr '[:lower:]' '[:upper:]')
if [ ! -d "$DOC_DIR/$SOC_DIR" ]; then
	echo "There's no doc for $CHIP in $DOC_DIR"
	exit 0
fi

echo "Releasing docs for $CHIP in $DOC_DIR"

cd "$DOC_DIR"

ORIG_COMMIT=$(git log --oneline -1 | cut -d' ' -f1)

COMMIT_MSG=$(mktemp)
cat << EOF > $COMMIT_MSG
Release $CHIP - $(date +%Y-%m-%d)

Based on:
$(git log -1 --format="%h %s")
EOF

git add -f .
git stash &>/dev/null

# Drop other docs
DOCS="$(ls)"
mv "$SOC_DIR"/* .
rm -rf $DOCS

# Create new branch
git branch -D $CHIP &>/dev/null || true
git checkout --orphan $CHIP &>/dev/null
git reset &>/dev/null
git add .
git commit -s -F $COMMIT_MSG &>/dev/null

# Recover
git checkout $ORIG_COMMIT &>/dev/null
cd "$SDK_DIR"
