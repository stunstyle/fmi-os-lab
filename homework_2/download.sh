#!/bin/bash

if [ ! -d "${1}" -o "${#}" -lt 2 ]; then
	echo "usage: "${0}" <dir> <link>"
	exit 1
fi

if [ ! -f /usr/bin/curl ]; then
	echo "curl not found on this system; please install it and run this script again"
	exit 2
fi
html=$(curl http://lzdx.bfra.bg/logs/2017/)
table_index=$(echo "$html" | grep -n "<table" | head -1 | cut -d: -f1) 
all_callsigns=$(echo "$html" | tail -n +"$table_index" | egrep -o '[A-Z0-9]{3,10}'| sort | uniq)

# echo "${all_callsigns}"

while IFS= read -r sign; do
	curl "http://lzdx.bfra.bg/logs/2017/${sign}" > ${1}/${sign}
done < <(echo "$all_callsigns")
