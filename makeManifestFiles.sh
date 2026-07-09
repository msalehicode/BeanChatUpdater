#!/usr/bin/env bash

set -e

if [ $# -ne 2 ]; then
    echo "Usage:"
    echo "  $0 <deploy_directory> <output.json>"
    exit 1
fi

DEPLOY_DIR="$1"
OUTPUT="$2"

echo -n '{"files":[' > "$OUTPUT"

FIRST=true

find "$DEPLOY_DIR" -type f | sort | while read -r FILE; do

    REL="${FILE#$DEPLOY_DIR/}"

    SIZE=$(stat -c%s "$FILE")

    SHA=$(sha256sum "$FILE" | cut -d' ' -f1)

    EXEC=false
    if [[ "$REL" == *.exe ]]; then
        EXEC=true
    fi

    if [ "$FIRST" = true ]; then
        FIRST=false
    else
        echo -n "," >> "$OUTPUT"
    fi

    printf \
'{"path":"%s","size":%s,"executable":%s,"sha256":"%s"}' \
"$REL" \
"$SIZE" \
"$EXEC" \
"$SHA" >> "$OUTPUT"

done

echo ']}' >> "$OUTPUT"

echo "Manifest written to $OUTPUT"
