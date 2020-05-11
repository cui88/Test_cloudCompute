#!/bin/bash
echo "Welcome to advance test!"
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
        cp -f index.html ./$line1/Lab2
        cd $line1/Lab2
        screen -d -m -S $line1
        sleep 1
        make
        if [ $? -eq 0 ];then
            if [ -e ./httpserver ];then
                chmod 777 ./httpserver
            fi
            screen -S $line1 -p 0 -X stuff "./httpserver --ip 127.0.0.1 --port 8888 --number-thread 8 --proxy https://www.baidu.com ^M"
            sleep 1
            cd ..
            cd ..
            stdbuf -i 0 -o 0 -e 0 curl -S -X GET http://127.0.0.1:8888/index.html >./judge_proxy/$line1 &
            sleep 2;kill %1
            cnt1=`grep baidu.com ./judge_proxy/$line1 | wc -l`
            echo "proxy:$cnt1"
            if (($cnt1 > 0));then
                echo "advanced：$line test proxy is right!" >> ./GitHub_URL_proxy_right.txt
            else
                echo "basic：$line test proxy is error!" >> ./GitHub_URL_proxy_error.txt
                screen -S $line1 -p 0 -X stuff "^C"
                bash ./test_basic_2.sh $line1
                continue
            fi         
            screen -S $line1 -p 0 -X stuff "^C"
            cd $line1/Lab2
            screen -S $line1 -p 0 -X stuff "./httpserver --ip 127.0.0.1 --port 8888 --number-thread 8 ^M" 
            sleep 1
            cd ..
            cd ..
            stdbuf -i 0 -o 0 -e 0 nc 127.0.0.1 8888 < input > ./judge_pipeline/$line1 &
            sleep 2;
            kill %1
            cnt2=`egrep 200 ./judge_pipeline/$line1 | wc -l`
            echo "pipeline:$cnt2"
            if (($cnt2 > 1));then
                echo "advanced：$line test more TCP  connction is right!" >> ./GitHub_URL_pipeline_right.txt
            else
                echo "advanced：$line test more TCP  connction is error!" >> ./GitHub_URL_pipeline_error.txt
            fi
            screen -S $line1 -p 0 -X stuff "^C"
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
done < GitHub_URL.txt