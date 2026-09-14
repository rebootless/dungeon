#!/bin/bash
SRC="assets/legacy"
DST="assets/tiles"

mkdir -p "$DST"

for INPUT in "$SRC"/*.png; do
    [[ -f "$INPUT" ]] || continue
    IMAGE=$(basename "$INPUT")
    OUTPUT="$DST/$IMAGE"
    magick "$INPUT" -colorspace Gray "$OUTPUT"
    echo "Done: $OUTPUT"
done
