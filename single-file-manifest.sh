#!/bin/bash

if [ $# -ne 1 ]; then
    echo "Usage:"
    echo "  $0 <file>"
    exit 1
fi

FILE="$1"

if [ ! -f "$FILE" ]; then
    echo "File not found: $FILE"
    exit 1
fi

SIZE=$(stat -c %s "$FILE")
SHA=$(sha256sum "$FILE" | awk '{print $1}')
NAME=$(basename "$FILE")

cat <<EOF
{
    "path":"$NAME",
    "size":$SIZE,
    "sha256":"$SHA",
    "executable":false
}
EOF
