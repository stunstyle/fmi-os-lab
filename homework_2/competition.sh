#!/bin/bash
function check_args() {
if [ ! -d "${1}" -o "${#}" -lt 2 ]; then
	echo "usage: "${0}" <data dir> <participants|outliers|unique|cross_check|bonus>"
	exit 1
fi
}

function print_participants() {
	real_participants=$(find ${1} -type f -exec basename {} \;)
	echo "$real_participants" | sort
}

check_args $@


if [ "${2}" = "participants" ]; then
	print_participants ${@}
elif [ "${2}" = "outliers" ]; then
	real_callsigns=$(find ${1} -type f -exec basename {} \;) # | sed -re 's/^(.)/ \1/g' | sed -re 's/(.)$/\1 /g')
	all_callsigns="$(find ${1} -type f -exec cat {} \; | egrep "^QSO:" |tr '\t' ' ' | tr -s ' ' |  awk '{ print $9 }')"

	all_callsigns=$(echo -e "${all_callsigns}" | sort | uniq)

	while IFS= read -r line; do
	echo -e "${real_callsigns}" | egrep -q "^$(echo ${line} | sed -re 's/(.)/[\1]/g')$"
	if [ $? -ne 0 ]; then
		echo "${line}"
	fi
done < <(echo -e "${all_callsigns}")
elif [ "${2}" = "unique" ]; then
	real_callsigns=$(find ${1} -type f -exec basename {} \; | sort)
	all_callsigns=""
	while IFS= read -r sign; do
		all_callsigns=$all_callsigns'\n'$(egrep "^QSO" ${1}${sign} | awk '{ print $9 }' | sort | uniq) 	
	done < <(printf '%s\n' "$real_callsigns")

all_callsigns=$(echo -e "${all_callsigns}" | sort | uniq -c)
echo "$all_callsigns" | sort -n | awk ' $1 < 4  { print $2 }'

elif [ "${2}" = "cross_check" ]; then
	all_rows=$(find ${1} -type f -exec cat {} \; | fgrep "QSO:" | tr '\t' ' ' | tr -s ' ')
	all_files=$(find ${1} -type f -exec basename {} \;)
        while IFS= read -r line; do
		line=$(echo $line | tr -s ' ')
		curr_callsign=$(echo $line | cut -d ' ' -f6) 
		other_callsign=$(echo $line | cut -d ' ' -f9)
		#		echo $all_files
		fgrep -q " $other_callsign " < <(echo " "$all_files" ")  
		if [ $? -ne 0 ]; then
			echo "${line}" "(${other_callsign} has no report file)"
			continue
		fi
		
		curr_date_time=$(echo $line | cut -d ' ' -f4-5)
		other_dates_times=$(fgrep " $curr_callsign " ${1}${other_callsign} | tr -s ' ' | cut -d ' ' -f4-5)

		fgrep -q "${curr_date_time}" < <(echo "${other_dates_times}")
		if [ $? -ne 0 ]; then
			echo $line
		fi
	done < <(printf '%s\n' "$all_rows")	
elif [ "${2}" = "bonus" ]; then
	all_rows=$(find ${1} -type f -exec cat {} \; | fgrep "QSO:" | tr '\t' ' ' | tr -s ' ')
	all_files=$(find ${1} -type f -exec basename {} \;)
	
	while IFS= read -r line; do
		line=$(echo $line | tr -s ' ' )
		curr_callsign=$(echo $line | cut -d ' ' -f6)
		other_callsign=$(echo $line | cut -d ' ' -f9)

		fgrep -q " $other_callsign " < <(echo " "$all_files" ")
		if [ $? -ne 0 ]; then
			echo "${line}" "(${other_callsign} has no report file)"
			continue
		fi
		
		curr_date_time=$(date -d "$(echo $line | cut -d ' ' -f4-5)" +%s )
		other_dates_times=$(fgrep " $curr_callsign " ${1}${other_callsign} | tr -s ' ' | cut -d ' ' -f4-5)
		flag=0
		while IFS= read -r line2; do
			curr_date=$(date -d "$(echo $line2)" +%s)
			diff=$(( $curr_date_time - ${curr_date:-0} ))
			if (( "${diff#-}" < 181 )); then		
				flag=1	
				break
			fi
		
		done < <(echo "$other_dates_times")
		if [ $flag -eq 1 ]; then
			continue
		else
			echo $line
		fi

	done < <(printf '%s\n' "$all_rows")


else
				
	echo "available: <participants|outliers|unique|cross_check|bonus>"
	exit 2
fi

