#!/bin/bash
echo "Welcome to judge test!"
flag_0=0 
cat /dev/null >./basic/result.txt
#min_line=0
while read line || [[ -n ${line} ]];
do
    line1=${line%/*}
    line1=${line1##*/}
    echo $line1
    if [ $flag_0 == 1 ];then
        bash ./listen_screen.sh $file_name
        flag_0=1
    fi
    file_name=$line1

    if [ -e $line1/Lab2 ];then
        screen -d -m -S $line1
        sleep 1
            cd $line1/Lab2
            make
            if [ $? -eq 0 ];then
                cd ..
                cd ..
                screen -S $line1 -p 0 -X stuff "./$line1/Lab2/httpserver --ip 127.0.0.1 --port 8888 --number-thread 5 ^M"

                curl -S -X GET http://127.0.0.1:8888/index.html &>./basic/get_test/correct/$line1
                curl -S -X GET http://127.0.0.1:8888/Empty_dir/index.html &>./basic/get_test/error/$line1
                curl -S -X POST --data 'Name=HNU&ID=CS06142' http://127.0.0.1:8888/Post_show &>./basic/post_test/correct/$line1
                curl -S -X POST --data 'Incompatible_key1=val1&Incompatible_key2=val2' http://127.0.0.1:8888/Post_show &>./basic/post_test/error/$line1
                curl -S -X DELETE http://127.0.0.1:8888/index.html &>./basic/other_method/$line1 

                while read line2 || [[ -n ${line2} ]]; do
                    file1=${line2%#*}/$line1
                    echo $file1
                    file2=${line2#*#}
                    echo $file2
                    if [ -f $file1 ] && [ -f $file2 ];then
                        diff -a -B -b -c $file1 $file2 >>error.log
                        if [ $? != 0 ];then
                           echo "$file1 $file2 is error!" >>./basic/result.txt
                        else
                           echo "$file1 $file2 is right!" >>./basic/result.txt
                        fi
                    else
                        echo "$line1:$file1 or $file2 does not exist,please check filename." >>./basic/error/$line1
                    fi
                done < file_name.txt
                siege -c 10 -r 100  http://127.0.0.1:8888/index.html &>>./basic/siege/get_test_1000/$line1
                siege -c 100 -r 100 http://127.0.0.1:8888/index.html &>>./basic/siege/get_test_10000/$line1
            else
                cd ..
                cd ..
                echo "$line1:make not successful!">>./basic/error/$line1
            fi
    else 
        echo "$line1：$line1/Lab2 can not find!" >>./basic/error/$line1
    fi
    bash ./listen_screen.sh $line1
done < filename_test.txt