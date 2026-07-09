#!/usr/bin/env bash

set -e

COMPACT=false

if [[ "$1" == "--compact" ]]; then
    COMPACT=true
    shift
fi

if [ $# -ne 2 ]; then
    echo "Usage:"
    echo "  $0 [--compact] <deploy_directory> <output.json>"
    exit 1
fi

DEPLOY_DIR="$(realpath "$1")"
OUTPUT="$2"

FILES=()

while IFS= read -r -d '' FILE; do
    FILES+=("$FILE")
done < <(find "$DEPLOY_DIR" -type f -print0 | sort -z)

{
if $COMPACT; then
    printf '{"files":['
else
    printf '{\n'
    printf '  "files": [\n'
fi

FIRST=true

for FILE in "${FILES[@]}"; do

    REL="${FILE#$DEPLOY_DIR/}"

    SIZE=$(stat -c%s "$FILE")
    SHA=$(sha256sum "$FILE" | cut -d' ' -f1)

    EXEC=false
    [[ "$REL" == *.exe ]] && EXEC=true

    if ! $FIRST; then
        printf ','
        if ! $COMPACT; then
            printf '\n'
        fi
    fi

    FIRST=false

    if $COMPACT; then
        printf '{"path":"%s","size":%s,"executable":%s,"sha256":"%s"}' \
            "$REL" "$SIZE" "$EXEC" "$SHA"
    else
        printf '    {\n'
        printf '      "path": "%s",\n' "$REL"
        printf '      "size": %s,\n' "$SIZE"
        printf '      "executable": %s,\n' "$EXEC"
        printf '      "sha256": "%s"\n' "$SHA"
        printf '    }'
    fi

done

if $COMPACT; then
    printf ']}'
else
    printf '\n'
    printf '  ]\n'
    printf '}\n'
fi

} > "$OUTPUT"

echo "Manifest written to $OUTPUT"
