while read line2 || [[ -n ${line2} ]];
do
	line3=${line2%:*}
	line4=${line2##*:}
	echo $line3 $line4
	$line3
done  < file_name.txt
