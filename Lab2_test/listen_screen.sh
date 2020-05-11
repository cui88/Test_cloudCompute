screen -ls > listen_screen_file.txt
sleep 1
if cat listen_screen_file.txt | grep "$1">/dev/null
then
	screen -S $1 -p 0 -X stuff "^C"
	screen -S $1 -p 0 -X stuff "exit^M"
	echo "screen: $1 exit!"
else
	echo "no $1 find in listen_screen_file.txt" >debug.log
fi
