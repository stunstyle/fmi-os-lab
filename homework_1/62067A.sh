#!/bin/bash

IFS=$' '
for morse_char in $(cat secret_message); do
	work=$(mktemp)
	echo $morse_char > $work
	sed -ri 's/(.)/[\1]/g' $work

	edited_morse_char=$(cat $work)
	egrep " ${edited_morse_char}$" morse | cut -d' ' -f1 | tr [A-Z] [a-z] >> encrypted
done

cat encrypted | tr -d '\n' > encrypted2
echo "" >> encrypted2
rm encrypted
mv encrypted2 encrypted
