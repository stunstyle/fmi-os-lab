#!/bin/bash

alphabet=$(echo {A..Z} | sed -r 's/ //g')

for i in {1..25};
do
	cypher=$(echo $alphabet | sed -r "s/^.{$i}//g")$(echo $alphabet | sed -r "s/.{$(( 26 - $i ))}$//g")
	current_input=$(cat encrypted | tr $cypher $alphabet)
	echo $current_input | grep "FUEHRER"
	if [[ $? == 0 ]]
	then
		echo "BINGO! FUEHRER was found"
		echo $current_input | tr [A-Z] [a-z] > decoded
	fi
done
