#!/bin/bash
echo "Welcome to basic test!" 
line1=$1
echo $line1
bash ./listen_screen.sh $line1
cd ./$line1/Lab2
screen -d -m -S $line1
sleep 1
screen -S $line1 -p 0 -X stuff "./httpserver --ip 127.0.0.1 --port 8888 --number-thread 5 ^M"
sleep 1
cd ..
cd ..
curl -i -X GET http://127.0.0.1:8888/index.html &>./basic/get_test/correct/$line1
curl -i -X GET http://127.0.0.1:8888/Empty_dir/index.html &>./basic/get_test/error/$line1
curl -i -X POST --data 'Name=HNU&ID=CS06142' http://127.0.0.1:8888/Post_show &>./basic/post_test/correct/$line1
curl -i -X POST --data 'Incompatible_key1=val1&Incompatible_key2=val2' http://127.0.0.1:8888/Post_show &>./basic/post_test/error/$line1
curl -i -X DELETE http://127.0.0.1:8888/index.html &>./basic/other_method/$line1 

while read line2 || [[ -n ${line2} ]]; do
    cmp1=${line2%#*}/$line1
    cmp2=${line2#*#}
    echo "$cmp1 grep $cmp2"
    cnt=`grep $cmp2 $cmp1 |wc -l`
    if [ -e cmp1 ];then
        if (($cnt > 0));then
           echo "$file1 $file2 is right!" >>./basic/result.txt 
        else
           echo "$file1 $file2 is error!" >>./basic/result.txt
        fi
    else
        echo "$line1:$cmp1 does not exist,please check filename." >>./basic/error/$line1
    fi
done < file_name.txt
stdbuf -i 0 -o 0 -e 0 siege -c 10 -r 100  http://127.0.0.1:8888/index.html &>>./basic/siege/get_test_1000/$line1 &
sleep 2;kill %1
stdbuf -i 0 -o 0 -e 0 siege -c 100 -r 100 http://127.0.0.1:8888/index.html &>>./basic/siege/get_test_10000/$line1 &
sleep 4;kill %1
cd ./$line1/Lab2
screen -S $line1 -p 0 -X stuff "^C"
cd ..
cd ..
bash ./listen_screen.sh $line1