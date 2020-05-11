#ifndef __PROTOCOLUTIL_HPP__
#define __PROTOCOLUTIL_HPP__

#include<iostream>
#include<iostream>
#include<string>
#include<vector>
#include<unordered_map>
#include<sstream>
#include<algorithm>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<sys/stat.h>
#include<sys/sendfile.h>
#include<fcntl.h>
#include<ctype.h>

#define BACKLOG 5
#define BUFF_NUM 1024

#define NORMAL 0
#define WARNING 1
#define ERROR 2

#define WEBROOT "wwwroot"     //系统根目录
#define HOMEPAGE "index.html" //首页
#define HTML_404 "wwwroot/404.html"

const char* ErrLevel[] = {
	"Normal", //正常
	"Warning", //警告
	"Error"   //错误
};

void log(std::string msg,int level,std::string file,int line)
{
	std::cout << file << ": "<<line << " " <<msg << ": " << ErrLevel[level] <<std::endl;
}

#define LOG(msg,level) log(msg,level,__FILE__,__LINE__);


class Util{
	public:
		static void MakeKV(std::string s,std::string& k,std::string &v)
		{
			std::size_t pos = s.find(": ");
			k = s.substr(0,pos);
			v = s.substr(pos+2);
		}
		static std::string IntToString(int &x)
		{
			std::stringstream ss;
			ss << x;
			return ss.str();
		}
		static std::string CodeToDesc(int code)
		{
			switch(code)
			{
				case 200:
					return "OK";
				case 400:
					return "Bad Request";
				case 404:
					return "Not Found";
				case 500:
					return "Internal Server Error";
				default:
					break;
			}
			return "Unknow";
		}
        static std::string CodeToExceptFile(int code)
        {
            switch(code)
            {
                case 404:
                    return HTML_404;
                case 500 :
                    return HTML_404;
                case 503:
                    return HTML_404;
                default:
                    return "";
            }
        }
        static std::string SuffixToContent(std::string suffix)
        {
            if(suffix == ".css")
            {
                return "text/css";
            }
            else if(suffix == ".js")
            {
                return "application/x-javescript";
            }
            else if(suffix == ".html" || suffix == ".htm")
            {
                return "text/html";
            }
            else if(suffix == ".jpg")
            {
                return "application/x-jpg";
            }
            return "text/html";
        }
        static int FileSize(std::string& except_path)
        {
            struct stat st;
            stat(except_path.c_str(),&st);
            return st.st_size;
        }
};

class SocketApi{
	public:
		static int Socket()
		{
			int sock = socket(AF_INET,SOCK_STREAM,0);
			if(sock < 0)
			{
				LOG("socket error!",ERROR);
				exit(2);
			}
			int opt = 1;
			setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));
			return sock;
		}
		static void Bind(int sock,int port)
		{
			struct sockaddr_in local;
			bzero(&local,sizeof(local));
			local.sin_family = AF_INET;
			local.sin_port = htons(port);
			local.sin_addr.s_addr = htonl(INADDR_ANY);

			if(bind(sock,(struct sockaddr*)&local,sizeof(local)) < 0)
			{	
				LOG("bind error!",ERROR);
				exit(3);
			}
		}
		static void Listen(int sock)
		{
			if(listen(sock,BACKLOG) < 0)
			{
				LOG("listen error!",ERROR);
				exit(4);
			}
		}
		static int Accept(int listen_sock,std::string& ip,int &port)
		{
			struct sockaddr_in peer;
			socklen_t len = sizeof(peer);
			int sock = accept(listen_sock,(struct sockaddr*)&peer,&len);
			if(sock < 0)
			{
				LOG("accept error!",WARNING);
				return -1;
			}
			port = ntohs(peer.sin_port);
			ip = inet_ntoa(peer.sin_addr); // 可以整体赋值 
			return sock;
		}

};




class Http_Response{     // 响应
	public:
		//基本协议字段
		std::string status_line;  // 响应行
		std::vector<std::string> response_header; // 响应首行
		std::string blank;
		std::string response_text; // 响应正文
	private:
		int code;     // 状态码
		std::string path;  // 资源路径
		int recource_size; // 资源大小正文大小
	public:
		Http_Response() 
			: blank("\r\n")
			, code(200)
            , recource_size(0)
		{}
		int& Code()
		{
			return code;
		}
		void SetPath(std::string &path_)
		{
			path = path_;
		}
		std::string& Path()
		{
			return path;
		}
		void SetRecourceSize(int rs)
		{
			recource_size = rs;
		}
		int RecourceSize()
		{
			return recource_size;
		}
		void MakeStatusLine()
		{
			status_line = "HTTP/1.0"; // 协议版本
			status_line += " ";
			status_line += Util::IntToString(code);
			status_line += " ";
			status_line += Util::CodeToDesc(code);
			status_line += "\r\n";
            LOG("Make Status Line Done",NORMAL);
		}
		void MakeResponseHeader() 
        {
			std::string line;
            std::string suffix;
            //Content_Type
            line = "Content-Type: ";
            std::size_t pos = path.rfind('.');
            if(std::string::npos != pos) // 假设它绝对有后缀
            {
                suffix = path.substr(pos);
                transform(suffix.begin(),suffix.end(),suffix.begin(),::tolower);
            }
            line += Util::SuffixToContent(suffix);
            line += "\r\n";
            response_header.push_back(line);

            // Content_Length
			line += "Content-Length: ";
			line += Util::IntToString(recource_size);
			line += "\r\n";
			response_header.push_back(line);
			line = "\r\n"; //空行
			response_header.push_back(line);
            LOG("Make Response Header DOne!",NORMAL);
		}
		void MakeResponseText()
		{}

		~Http_Response()
		{}
};

class Http_Request{
	public:
		//基本协议字段
		std::string request_line;  // 请求行
		std::vector<std::string> request_header; // 请求报头
		std::string blank;  // 空行
		std::string request_text;  // 正文
		
		//解析字段
	private:
		std::string method;  // 方法
		std::string uri;     //url path?arg
		std::string version; //版本
		std::string path;   // 资源路径
		//int recource_size; //资源大小
		std::string query_string;  // 要处理的参数 
		std::unordered_map<std::string,std::string> header_kv; // 
		bool cgi;  //表明是否传参
	public:
		Http_Request() 
			: path(WEBROOT)
			, cgi(false)
			, blank("\r\n")
		{}
		void RequestLineParse()
		{
			//1->3
			//GET x/y/z HTTP/1.1\n
			std::stringstream ss(request_line);
			ss >> method >> uri >> version;
			transform(method.begin(),method.end(),method.begin(),toupper);
		}
		void UriParse()
		{
			if(method == "GET") //需要特殊处理，因为url里面有参数
			{
				std::size_t pos = uri.find('?');
				if(pos != std::string::npos)
				{
					cgi = true;
					path += uri.substr(0,pos);
					query_string = uri.substr(pos+1);
				}
				else
				{
					path += uri; 
				}
			}
			else
			{
				cgi = true;
				path += uri;
			}
			if(path[path.size()-1] == '/') //检测
			{
				path += HOMEPAGE;  // wwwroot/index.html
			}

		}
		void HeaderParse()
		{
			std::string k,v;
			for(auto it = request_header.begin();it != request_header.end();it++)
			{
				Util::MakeKV(*it,k,v);
				header_kv.insert({k,v});
			}
		}
		bool IsMethodLegal()  //方法合法性
		{
			if(method != "GET" && method == "POST")
				return false;
			return true;
		}
		int IsPathLegal(Http_Response* rsp)  // wwwroot/a/b/c/d.html
		{
			//需要判断文件是否存在，不能使用open，因为这个文件可能是二进制文件或者可执行文件或者图片
            int rs = 0;
			struct stat st;
			if(stat(path.c_str(),&st) < 0 ) //文件不存在
			{
                LOG("file is not exist!",WARNING);
				return 404;
			}
			else //文件存在
			{
				rs = st.st_size;
				if(S_ISDIR(st.st_mode)) //是目录的情况
				{
					path += "/";
					path += HOMEPAGE;  
					stat(path.c_str(),&st);
					rs = st.st_size;
				}
				else if((st.st_mode & S_IXUSR) || (st.st_mode & S_IXGRP) || (st.st_mode & S_IXOTH)) //判断是否有执行权限
				{
					cgi = true;
				}
				else //
				{
					
				}
			}
			rsp->SetPath(path);
			rsp->SetRecourceSize(rs);
            LOG("Path is OK!",NORMAL);
			return 200;
		}
		bool IsNeedRecv()
		{
			return method == "POST" ? true : false;
		}
		bool IsCgi()
		{
			return cgi;
		}
		int ContentLength()
		{
			int content_length = -1;
			std::string cl = header_kv["Content-Length"];
			std::stringstream ss(cl);
			ss >> content_length;
			return content_length;
		}
        std::string GetParam()
        {
            if(method == "GET") // GET 方法的话,参数在请求行
            {
                return query_string;
            }
            else  // POST 的话,参数在正文中
            {
                return request_text;
            }
        }
		~Http_Request()
		{}
};


class Connect{
	private:
		int sock;
	public:
		Connect(int sock_) : sock(sock_)
		{}
		int RecvOneLine(std::string& line)
		{
			char buff[BUFF_NUM];
			int i = 0;
			char c = 'x'; //这个初始化可以随便
			while(c != '\n' && i < BUFF_NUM - 1 )
			{
				ssize_t s = recv(sock,&c,1,0);
				if(s > 0)
				{
					if(c == '\r')
					{
						recv(sock,&c,1,MSG_PEEK);
						if(c == '\n')  //已经确定是‘\n’.所以读并且一定读到的是‘\n’
							recv(sock,&c,1,0);
						else
							c = '\n'; //所以读到'\r'后也就读完了
					}
					buff[i++] = c;
				}
				else
					break;
			}
			buff[i] = 0;
			line = buff;
			return i;
		}
		void RecvRequestHeader(std::vector<std::string>& v) //读头部
		{
			std::string line = "X"; //保证第一次读不会是"\n"
			while(line != "\n")
			{
				RecvOneLine(line);
				if(line != "\n")
					v.push_back(line);
			}
            LOG("Header Recv OK!",NORMAL);
		}
		void RecvText(std::string &text,int content_length) // 需要确定读多少字节,否则就会发生粘包
		{
			char c;
			for(auto i = 0; i < content_length; ++i)	
			{
				recv(sock,&c,1,0);
				text.push_back(c);
			}
		}
		void SendStatusLine(Http_Response* rsp)
		{
            std::string &sl = rsp->status_line;
			send(sock,sl.c_str(),sl.size(),0);
		}
		void SendHeader(Http_Response* rsp)   //add \n
		{
			std::vector<std::string> &v = rsp->response_header;
			for(auto it = v.begin();it != v.end();++it)
			{
				send(sock,it->c_str(),it->size(),0);
			}
		}
		void SendText(Http_Response* rsp,bool cgi_)
		{
            if(!cgi_)
            {

                std::string &path = rsp->Path();
                int fd = open(path.c_str(),O_RDONLY);
                if(fd < 0)
                {
                    LOG("open file error!",WARNING);
                    return;
                }
                sendfile(sock,fd,NULL,rsp->RecourceSize());
                close(fd);
            }
            else 
            {
                std::string &rsp_text = rsp->response_text;
                send(sock,rsp_text.c_str(),rsp_text.size(),0);
            }
		}
        void ClearRequest()
        {
            std::string line;
            while(line != "\n")
            {
                RecvOneLine(line);
            }
        }
		~Connect()
        {
            close(sock);
        }
};

class Entry{
	public:	
		static int ProcessCgi(Connect* conn,Http_Request* rq,Http_Response* rsp)
        {
            int input[2];
            int output[2];
            pipe(input);   // 子进程想从父进程获取数据，子进程读数据 
            pipe(output);  // 子进程要想父进程发送数据，子进程写数据
            
            std::string bin = rsp->Path(); // wwwroot/a/b/binX
            std::string param = rq->GetParam(); 
            int size = param.size();
            std::string param_size = "CONTENT-LENGTH=";
            param_size += Util::IntToString(size);

            std::string &response_text = rsp->response_text;

            pid_t id = fork();
            if(id < 0)
            {
                LOG("fork error",WARNING);
                return 500;
            }
            else if(id == 0) // child
            {
                close(input[1]);
                close(output[0]);
                putenv((char*)param_size.c_str());
                dup2(input[0],0);
                dup2(output[1],1); 

                execl(bin.c_str(),bin.c_str(),NULL); // 调用失败直接让子进程退出exit(1),如果替换成功则直接不用替换了,就不会执行exit了
                LOG("child process go bye",NORMAL);
                exit(1);
            }
            else // father
            {
                close(input[0]);
                close(output[1]);
        
                char c;
                for(auto i = 0;i < size; ++i) // 读参数写到文件中
                {
                    c = param[i];
                    write(input[1],&c,1);
                }
                waitpid(id,NULL,0);
                LOG("wait child process success!",NORMAL);

                // 这块没有可能出现read 阻塞在这里，因为这里能读的前提条件是wait成功，wait 成功意味着子进程已经关闭,
                // 子进程打开的文件描述符也将关闭，因为文件描述符的关闭是随进程的，因此read 肯定会读到0
                while(read(output[0],&c,1) > 0) 
                {
                    response_text.push_back(c);
                }
                LOG("read output success!",NORMAL);
                LOG("read response_text finish",NORMAL);
                rsp->MakeStatusLine();
                rsp->SetRecourceSize(response_text.size());
                rsp->MakeResponseHeader();

                conn->SendStatusLine(rsp);
                conn->SendHeader(rsp);
                conn->SendText(rsp,true);
            }
            return 200;
        }

		static int ProcessNonCgi(Connect* conn,Http_Request* rq,Http_Response* rsp)
		{
			rsp->MakeStatusLine(); // 响应行
			rsp->MakeResponseHeader(); // 响应首部
		//	rsp->MakeResponseText(rq);  // 响应正文

			conn->SendStatusLine(rsp);
			conn->SendHeader(rsp); // 发送首部的时候把空行加上
			conn->SendText(rsp,false);
            LOG("Send Response Done!",NORMAL);
            return 200;
		}
		static int ProcessResponse(Connect* conn,Http_Request* rq,Http_Response* rsp)
		{
			if(rq->IsCgi())
			{
                LOG("MakeREsponse Use Cgi",NORMAL);
			    return ProcessCgi(conn,rq,rsp);
			}
			else
			{
                LOG("MakeREsponse Use Non Cgi",NORMAL);
				return ProcessNonCgi(conn,rq,rsp);
			}

		}
		static void HandlerRequest(int sock)
		{
			pthread_detach(pthread_self());

			Connect* conn = new Connect(sock);
			Http_Request *rq = new Http_Request;
			Http_Response* rsp = new Http_Response();
			int& code = rsp->Code();

			conn->RecvOneLine(rq->request_line);	
			rq->RequestLineParse();

			if(!rq->IsMethodLegal()) // 判断方法是否合法 
			{
				code = 400;
                conn->ClearRequest();
				LOG("Request Method Is Not Legal",WARNING);
				goto end; //异常处理
			}

			rq->UriParse();
			
			if((code = rq->IsPathLegal(rsp)) != 200) // 判断路径是否合法 
			{
                conn->ClearRequest();
				LOG("File Is Not Exist!",WARNING);
				goto end;
			}
			conn->RecvRequestHeader(rq->request_header); //拿到了第二部分
			rq->HeaderParse(); //将首部按KV模型放在vector中
			if(rq->IsNeedRecv()) // 是否需要读,POST 就是需要读;GET 没有正文,不需要去读
			{
                LOG("POST Method, Need REcv Begin!",NORMAL);
				conn->RecvText(rq->request_text,rq->ContentLength()); // 读完了正文部分
			}

            LOG("Http Requst Recv Done, OK!",NORMAL);
			code = ProcessResponse(conn,rq,rsp); //响应
			
end:	
            if(code != 200)
            {
                std::string except_path = Util::CodeToExceptFile(code);
                int rs = Util::FileSize(except_path);
                rsp->SetPath(except_path);
                rsp->SetRecourceSize(rs);
                ProcessNonCgi(conn,rq,rsp);
            }
			delete conn;
			delete rq;
			delete rsp;

		}
};

#endif


