#!/bin/bash
#New folder judgment pipelineing function
if [ -d ./judge_pipeline ];then
	echo Folder already exists
else
	mkdir -p ./judge_pipeline
fi
#New folder judgment proxy function
if [ -d ./judge_proxy ];then
   echo Folder already exists
else
	mkdir -p ./judge_proxy
fi
#New folder test request
if [ -d ./basic ];then
   echo Folder already exists
else
	mkdir -p ./basic
fi
#New folder test get-request
if [ -d ./basic/get_test ];then
   echo Folder already exists
else
	mkdir -p ./basic/get_test
fi
if [ -d ./basic/get_test/correct ];then
   echo Folder already exists
else
	mkdir -p ./basic/get_test/correct
fi
if [ -d ./basic/get_test/error ];then
   echo Folder already exists
else
	mkdir -p ./basic/get_test/error
fi
#New folder test post-request
if [ -d ./basic/post_test ];then
   echo Folder already exists
else
	mkdir -p ./basic/post_test
fi
if [ -d ./basic/post_test/correct ];then
   echo Folder already exists
else
	mkdir -p ./basic/post_test/correct
fi
if [ -d ./basic/post_test/error_1 ];then
   echo Folder already exists
else
	mkdir -p ./basic/post_test/error_1
fi
if [ -d ./basic/post_test/error_2 ];then
   echo Folder already exists
else
	mkdir -p ./basic/post_test/error_2
fi
#New folder test other-method-request
if [ -d ./basic/other_method ];then
   echo Folder already exists
else
	mkdir -p ./basic/other_method
fi
#New folder as a reference answer
if [ -d ./answer_file ];then
   echo Folder already exists
else
	mkdir -p ./answer_file
fi

