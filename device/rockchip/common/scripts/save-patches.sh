#!/bin/bash -e

[ -z "$DEBUG" ] || set -x

[ -d .git ] || exit 1

PATCH_DIR="${1:-$PWD}"
PROJECT="${2:-${PWD#$SDK_DIR/}}"
BASE_COMMIT=$(git log --pretty="%H" -1 "${3:-HEAD}" --)

echo "[$PROJECT] Base commit: $(git log --oneline $BASE_COMMIT -1)"

# Clean
rm -rf "$PATCH_DIR"
mkdir -p "$PATCH_DIR"

# Saving header of apply script
cat << EOF > "$PATCH_DIR/apply-patches.sh"
#!/bin/bash -e

[ -z "\$DEBUG" ] || set -x

PATCH_DIR="\$(dirname "\$(realpath "\$0")")"

cd "\$PATCH_DIR/$(realpath "$PWD" --relative-to="$PATCH_DIR")"

echo "[$PROJECT] Applying patches from \$PATCH_DIR"

git add .
if git diff HEAD -- | grep -q ""; then
	git stash >/dev/null || git reset --hard
fi

EOF
chmod a+x "$PATCH_DIR/apply-patches.sh"

# Check files
git reset &>/dev/null
if git status -s | grep -q ""; then
	if [ "$RK_SAVE_COMMITTED" ]; then
		echo "[$PROJECT] Uncommitted changes ignored:"
		git status -s
	elif [ "$RK_SAVE_TRACKED" ]; then
		if git status -s | grep -q "^?? "; then
			echo "[$PROJECT] Untracked file changes ignored:"
			git status -s | grep "^?? "
		fi
		git add -u
	else
		git add -A
	fi
fi

# Nothing to save
if ! git diff $BASE_COMMIT ${RK_SAVE_COMMITTED:+HEAD --} | grep -q ""; then
	git reset &>/dev/null
	echo "[$PROJECT] No patch to save"

	# Perform a clean checkout
	echo "git checkout $BASE_COMMIT" >> "$PATCH_DIR/apply-patches.sh"
	exit 0
fi

echo "[$PROJECT] Saving patches into $PATCH_DIR"
echo "[$PROJECT] Patches:"

# Saving commits
MERGE_BASE=$(git merge-base HEAD $BASE_COMMIT || true)
if [ -n "$MERGE_BASE" ]; then
	git diff $MERGE_BASE HEAD -- | grep -q "" &&
		git format-patch $MERGE_BASE..HEAD -o "$PATCH_DIR"
else
	# Orphan tree
	git format-patch -$(git log --oneline | wc -l) -o "$PATCH_DIR"
fi

# Saving uncommited changes
if [ -z "$RK_SAVE_COMMITTED" ] && git status -s | grep -q ""; then
	echo "$PATCH_DIR/local.diff"
	git diff --binary HEAD -- > "$PATCH_DIR/local.diff"
fi
git reset &>/dev/null

# Update apply script
cat << EOF >> "$PATCH_DIR/apply-patches.sh"
if [ -n "$MERGE_BASE" ]; then
	echo "Base commit: $(git log --oneline $MERGE_BASE -1)"
	git checkout $MERGE_BASE
else
	git checkout HEAD^ &>/dev/null || true
	git branch -D orphan &>/dev/null || true
	git checkout --orphan orphan >/dev/null
	git add -f .
	git reset --hard
fi

for f in \$(find "\$PATCH_DIR" -name "*.patch" | sort); do
	git am "\$f"
done

if [ -e "\$PATCH_DIR/local.diff" ]; then
	echo "Applying: \$PATCH_DIR/local.diff"
	git apply "\$PATCH_DIR/local.diff"
fi
EOF
