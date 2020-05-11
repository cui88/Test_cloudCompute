#ifndef __HTTPSERVER_HTTP
#define __HTTPSERVER_HTTP

#include<iostream>
#include<pthread.h>
#include<signal.h>
#include<unistd.h>
#include "threadpool.hpp"
#include "protocolutil.hpp"
class HttpServer{
private:
	int listen_sock;
	int port;
	std::string ip;
    int num_thread;
public:
	HttpServer(std::string ip_,int port_,int num_thread_):ip(ip_),port(port_),num_thread(num_thread_),listen_sock(-1)
	{}

	void InitServer()
	{
       listen_sock = SocketApi::Socket();
       SocketApi::Bind(listen_sock,port);
       SocketApi::Listen(listen_sock);
	}

	void Start()
	{
	   while(1)
	   {
            std::string peer_ip;
            int peer_port;
            int sock = SocketApi::Accept(listen_sock,peer_ip,peer_port);
            if(sock>= 0)
            {
            	std::cout << peer_ip << " : " << peer_port <<std::endl;
            	Task t(sock,Entry::HandlerRequest);
            	singleton st(num_thread);
            	st.GetInstance()->PushTask(t);
            }
	   }

	}

	~HttpServer()
	{
		if(listen_sock >= 0)
			close(listen_sock);
	}
};


#endif