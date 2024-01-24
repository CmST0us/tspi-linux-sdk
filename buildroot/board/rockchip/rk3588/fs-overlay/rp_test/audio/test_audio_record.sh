#!/bin/bash

echo "============================"
echo "Sound card record test"
echo "============================"

record_cards=()
index=0
sample_rate=48000
channel=2
format="S16_LE"
record_time=5

while read -r card; do
    echo "$index : $card"
    let index++
    record_cards+=("$card")
done < <(arecord -l | grep card | awk '{print $3}')

#echo "${record_cards[@]}"

read -p "Select sound card to record audio: " card_index

if [[ ! $card_index =~ ^[0-9]+$ ]]; then
    echo "Error: Input must be a number."
    exit 1
fi

if (( card_index < 0 || card_index >= index )); then
    echo "Error: Invalid index. Please select a valid sound card."
    exit 1
fi

audio_file="/rp_test/audio/test_${record_cards[card_index]}_record.wav"

echo =====================================================
echo "Select card: ${record_cards[card_index]}"
echo "Sample rate: $sample_rate"
echo "Channel: $channel"
echo "Sample format: $format"
echo "Output file: $audio_file"
echo "Record time: ${record_time}s"
echo =====================================================

echo "arecord -Dplughw:${record_cards[card_index]},0 -c $channel -r $sample_rate -f  $format -d $record_time $audio_file"
arecord -Dplughw:${record_cards[card_index]},0 -c $channel -r $sample_rate -f $format -d $record_time $audio_file

echo "Record finished, play audio file..."
aplay $audio_file
