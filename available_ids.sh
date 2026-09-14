#!/bin/bash
TILES_JSON="assets/tiles/tiles.json"

if [[ ! -f "$TILES_JSON" ]]; then
    echo "Error: file not found: $TILES_JSON"
    exit 1
fi

CHARS=(0 1 2 3 4 5 6 7 8 9 A B C D E F G H I J K L M N O P Q R S T U V W X Y Z)

declare -A USED
while read -r id; do
    USED["$id"]=1
done < <(grep -oE '"id"[[:space:]]*:[[:space:]]*"[0-9A-Z]{2}"' "$TILES_JSON" | grep -oE '[0-9A-Z]{2}"$' | tr -d '"')

AVAILABLE=()
for c0 in "${CHARS[@]}"; do
    for c1 in "${CHARS[@]}"; do
        id="$c0$c1"
        [[ -z "${USED[$id]}" ]] && AVAILABLE+=("$id")
    done
done

WIDTH=$(tput cols 2>/dev/null)
[[ -z "$WIDTH" ]] && WIDTH=80
PER_ROW=$(( WIDTH / 3 ))
[[ "$PER_ROW" -lt 1 ]] && PER_ROW=1

for i in "${!AVAILABLE[@]}"; do
    printf '%s ' "${AVAILABLE[$i]}"
    (( (i + 1) % PER_ROW == 0 )) && echo
done
echo
