#include<iostream>
#include<string.h>
#include<getopt.h>
#include "httpserver.hpp"

#define IF_DEBUG_PRINTF 0

void Usage(std::string proc)
{
   std::cout << "Usage: " << proc << "--ip  " << " --port " 
   << " --number-thread "<< " --proxy(option) "<< std::endl;
}

int main(int argc,char* argv[])
{
   std::string ip;
   int port = -1;
   int number_thread = -1;

   int opt = -1;
   
   const char* short_options = "abc";
   const struct option long_options[] =
   {
      {"ip",  1, NULL, 'i'},
      {"port",1, NULL, 'p'},
      {"number-thread",1, NULL, 'n'},
      {"proxy",2,NULL,'o'},
      {0, 0, 0, 0}
   };
   if(argc < 4)
   {
   	  Usage(argv[0]);
   	  exit(1);
   }
   else{
      while((opt = getopt_long(argc,argv,short_options,long_options,NULL)) != -1)
      {
         switch (opt) {
         	case 'i':
                ip = optarg;
         	    break;
         	case 'p':
         	    port = atoi(optarg);
         	    break;
         	case 'n':
         	    number_thread = atoi(optarg);
         	default:
         	    break;
         }
      }
   }
   if(IF_DEBUG_PRINTF)
   	  std::cout<< "ip: "<< ip <<" port: "<< port <<" number-thread: "<< number_thread << std::endl;
   //atoi函数把字符串转换成整型数
   HttpServer* ser = new HttpServer(ip,port,number_thread);
   ser->InitServer();
   ser->Start();
   return 0;
}