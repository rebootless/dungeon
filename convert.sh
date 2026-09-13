#!/bin/bash

SRC="/assets/legacy"
DST="/assets/tiles"

show_help() {
    cat <<EOF
Usage: $(basename "$0") [OPTIONS]

Convert an image to grayscale.

The input image is read from:
  $SRC

The converted image is saved to:
  $DST

Options:
  -h, --help    Show this help message

Example:
  $(basename "$0")
EOF
}

case "$1" in
    -h|--help)
        show_help
        exit 0
        ;;
esac

read -rp "Image name: " IMAGE

INPUT="$SRC/$IMAGE"
OUTPUT="$DST/$IMAGE"

if [[ ! -f "$INPUT" ]]; then
    echo "Error: file not found: $INPUT"
    exit 1
fi

mkdir -p "$DST"

magick "$INPUT" -colorspace Gray "$OUTPUT"

echo "Done: $OUTPUT"
