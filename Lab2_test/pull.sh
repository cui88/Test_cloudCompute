#! /bin/bash
echo "Congradulate you into the test part!"

cat GitHub_URL.txt |while read line
do
    filename=${line%/*}
    filename=${filename##*/}
	if [ -d ./filename ];then 
		echo Folder already exists
	else
		mkdir -p ./$filename
    fi 
address=${line:19}
echo $address
cd $filename
git init
git remote add origin1 git@github.com:$address
git pull origin1 master

git log  --pretty="%h" --since="2020-03-01" --until="2020-04-29 10:00:00" -n4 >log.txt
cat log.txt
line_log=$(cat log.txt |head -n 1)
git reset --hard $line_log
cd ..
done
