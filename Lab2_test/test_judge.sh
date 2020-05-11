#!/bin/bash
echo "Welcome to judge test!"
fromdos file_name.txt
flag_0=0 
while read line || [[ -n ${line} ]];
do
    flag_1=1
    line1=${line%/*}
    line1=${line1##*/}
    echo $line1
    if [ $flag_0 == 1 ];then
        bash ./listen_screen.sh $file_name
        flag_0=1
    fi
    file_name=$line1

    if [ -e $line1/Lab2 ];then
        cd $line1/Lab2
        screen -d -m -S $line1
        sleep 1
        make
        if [ $? -eq 0 ];then
            if [ -e ./httpserver ];then
                chmod 777 ./httpserver
            fi
           # cp -f index.html ./$line1/Lab2
            
            screen -S $line1 -p 0 -X stuff "./httpserver --ip 127.0.0.1 --port 8888 --number-thread 8 --proxy https://www.baidu.com ^M"
            sleep 1
            cd ..
            cd ..
            curl -S -X GET http://127.0.0.1:8888/index.html >./judge_proxy/$line1
            if [ -s ./judge_proxy/$line1 ];then
                bash ./test_basic.sh $line1
                continue
            fi
            # file1=./judge_proxy/$line1
            # file2=./answer/judge_proxy
            # if [ -f $file1 ] && [ -f $file2 ];then
            #     diff -a -b -B -w -i -y $file1 $file2 
            #     if [ $? != 0 ];then
            #        echo "advanced：$line test_proxy is error!" >> ./GitHub_URL_proxy_error.txt
            #     else
            #        echo "$line" >> ./GitHub_URL_proxy_right.txt
            #     fi
            # else
            #     echo "$file1 or $file2 does not exist,please check filename." >> ./GitHub_URL_proxy_error.txt
            # fi            
            screen -S $line1 -p 0 -X stuff "^C"
            cd $line1/Lab2
            screen -S $line1 -p 0 -X stuff "./httpserver --ip 127.0.0.1 --port 8888 --number-thread 8 ^M"
            sleep 1
            cd ..
            cd ..
            wrk -t 1 -c 1 --script=test.lua --latency http://127.0.0.1:8888  > ./judge_pipeline/$line1
            screen -S $line1 -p 0 -X stuff "^C"

            file3=./judge_pipeline/$line1
            fromdos $file3
            if [ -f $file3 ];then
            	line4=$(head -12 $file3 | tail -1)
		    error_info="errors"
                diff -a -B $file1 $file2 > /dev/null
                if [[ $line4 =~ $error_info ]];then
            	   echo "advanced：$line test_pipeline is error!" >> ./GitHub_URL_pipeline_error.txt
                else
            	   echo "$line" >> ./GitHub_URL_pipeline_right.txt
                fi
            else
             	echo "$file3 does not exist,please check filename." >> ./GitHub_URL_pipeline_error.txt
            fi
        else 
        	cd ..
        	cd ..
            echo "make:$line1 make faild!" >>./error.log
        fi
    else 
        echo "file：$line1/Lab2 can not find!" >>./error.log
    fi
    bash ./listen_screen.sh $line1
    echo $line1 finish
    echo $line1 finish
done < advance_filename_test.txt
