#!/bin/bash

echo "============================"
echo "Sound card play test"
echo "============================"

play_cards=()
index=0
audio_file=/rp_test/audio/test.wav

while read -r card; do
    echo "$index : $card"
    let index++
    play_cards+=("$card")
done < <(aplay -l | grep card | awk '{print $3}')

#echo "${play_cards[@]}"

read -p "Select sound card to play audio file: " card_index

if [[ ! $card_index =~ ^[0-9]+$ ]]; then
    echo "Error: Input must be a number."
    exit 1
fi

if (( card_index < 0 || card_index >= index )); then
    echo "Error: Invalid index. Please select a valid sound card."
    exit 1
fi

echo "Select card: ${play_cards[card_index]}"
echo "aplay -Dplughw:${play_cards[card_index]},0 $audio_file"
aplay -Dplughw:${play_cards[card_index]},0 $audio_file
