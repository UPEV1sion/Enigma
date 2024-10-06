#!/bin/bash

START_LETTER="A"

for i in {1..26}; do
	STARTLETTER_REPEATED_SIX_TIMES=$(echo "$START_LETTER$START_LETTER$START_LETTER$START_LETTER$START_LETTER$START_LETTER")
	./build/enigma -e M3 --rotor-one 1 --rotor-two 2 --rotor-three 3 -ro AAA -rp AAA -rf B -pt $STARTLETTER_REPEATED_SIX_TIMES
	START_LETTER=$(echo "$START_LETTER" | tr "A-Z" "B-ZA")
done
