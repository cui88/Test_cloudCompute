#ifndef __PROTOCOLUTIL_HPP__
#define __PROTOCOLUTIL_HPP__

#include<iostream>
#include<sstream>
#include<arpa/inet.h>
#include<string>
#include<vector>
#include<unordered_map>
#include<sys/stat.h>
#include<unistd.h>
#include<algorithm>
#include<fcntl.h>
#include<sys/sendfile.h>
#include<fstream> 

#define BACKLOG 5
#define BUFF_NUM 1024
#define IF_DEBUG_PRINTF 0

#define NORMAL 0
#define WARNING 1
#define ERROR 2

#define HTML_404 "404.html"
#define HTML_501 "501.html"
#define HOMEPAGE "index.html"
const char* ErrLevel[] = {
	"Normal",
	"Warning",
	"Error"
};

void log(std::string msg,int level,std::string file,int line)
{
	std::cout << file << ": "<<line << " " <<msg <<": " << ErrLevel[level]<< std::endl;
}

#define LOG(msg,level) log(msg,level,__FILE__,__LINE__);

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
    		LOG("listen error",ERROR);
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
    	ip = inet_ntoa(peer.sin_addr);
    	return sock;
    }
};

class Util
{
  public:
    static void MakeKV(std::string s,std::string& k,std::string &v)
    {
       std::size_t pos = s.find(": ");
       k = s.substr(0,pos);
       v = s.substr(pos+2);
    }
    static std::string CodeToExceptFile(int code)
    {
      switch(code)
      {
        case 404:
            return HTML_404;
        case 501:
            return HTML_501;
        default:
            return "";
      }
    }
    static int FileSize(std::string& except_path)
    {
      struct stat st;
      stat(except_path.c_str(),&st);//string转为char*类型
      return st.st_size;
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
        case 404:
          return "Not Found";
        case 501:
          return "Not Implemented";
        default:
          break;
      }
      return "Unknow";
    }
    static std::string SuffixToContent(std::string suffix)
    {
      if(suffix == ".css")
      {
        return "text/css";
      }
      else if(suffix == ".js")
      {
        return "application/x-javascript";
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
};

class Http_Response
{
public:
  std::string status_line;
  std::vector<std::string> response_header;
  std::string blank;
  std::string post_text;
  std::vector<std::string> response_text;
private:
  int code;
  std::string path;
  std::string origin_path;
  std::string method;
  int resource_size;
public:
  Http_Response()
        :blank("\r\n")
        ,code(200)
        ,resource_size(0)
  {}
  int& Code()
  {
    return code;
  }
  void SetOriginPath(std::string& path_)
  {
    origin_path = path_;
  }
  void SetPath(std::string& path_)
  {
    path = path_;
  }
  std::string& GetPath()
  {
    return path;
  }
  void SetResourceSize(int size)
  {
    resource_size = size;
  }
  int resourceSize()
  {
    return resource_size;
  }
  void SetMethod(std::string method_)
  {
    method = method_;
  }
  std::string GetMethod()
  {
    return method;
  }
  void MakeStatusLine()
  {
    status_line = "HTTP/1.0";
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
    std::string server = "Lab2 server";

    line += "Server: ";
    line += server;
    line += "\r\n";
    response_header.push_back(line);

    line = "Content-type: ";
    std::size_t pos = path.rfind('.');
    if(std::string::npos != pos)
    {
      suffix = path.substr(pos);
      transform(suffix.begin(),suffix.end(),suffix.begin(),::tolower);
    }
    line += Util::SuffixToContent(suffix);
    line += "\r\n";
    response_header.push_back(line);

    line = "Content-Length: ";
    line += Util::IntToString(resource_size);
    line += "\r\n";
    response_header.push_back(line);
    line = "\r\n";
    response_header.push_back(line);
    LOG("Make Response Header Done!",NORMAL);
  }
  void TextParse(std::string request_text)
  {
    switch(code)
    {
      case 200:
         Make_200_File(request_text);
         break;
      case 404:
         if(method == "GET")
           Make_GET_404_File(request_text);
         else
           Make_POST_404_File(request_text);
         break;
      case 501:
         Make_501_File(request_text);
         break;
      default:
         break;
    } 
  }
  void Make_GET_404_File(std::string request_text)
  {
    std::string text = request_text;
    std::string line;
    line = "<html><title>404 Not Found</title><body bgcolor=ffffff>\r\n";
    response_text.push_back(line);
    line = "Not Found\r\n";
    response_text.push_back(line);
    line = "<p>Couldn't find this file: ";
    line += origin_path;
    line += "\r\n";
    response_text.push_back(line);
    line = "<hr><em>HTTP Web server</em>\r\n";
    response_text.push_back(line);
    line = "</body></html>";
    line += "\r\n";
    response_text.push_back(line);
  }
  void Make_POST_404_File(std::string request_text)
  {
    std::string text = request_text;
    std::string line;
    line = "<html><title>404 Not Found</title><body bgcolor=ffffff>\r\n";
    response_text.push_back(line);
    line = "Not Found\r\n";
    response_text.push_back(line);
    line = "<hr><em>HTTP Web server</em>\r\n";
    response_text.push_back(line);
    line = "</body></html>";
    line += "\r\n";
    response_text.push_back(line);
  }
  void Make_501_File(std::string request_text)
  {
    std::string text = request_text;
    std::string line;
    line = "<html><title>501 Not Implemented</title><body bgcolor=ffffff>\r\n";
    response_text.push_back(line);
    line = "<p>Does not implement this method: ";
    line += method;
    line += "\r\n";
    response_text.push_back(line);
    line = "<hr><em>HTTP Web server</em>\r\n";
    response_text.push_back(line);
    line = "</body></html>";
    line += "\r\n";
    response_text.push_back(line);
  }
  void Make_200_File(std::string request_text)
  {
    std::string text = request_text;
    std::string line;
    line = "<html><title>Post</title>\r\n";
    response_text.push_back(line);
    std::size_t pos_0 = text.find('=');
    std::size_t pos_1 = text.find('&') - pos_0 -1;
    line = "Your Name: ";
    line += text.substr(pos_0+1,pos_1);
    line += "\r\n";
    response_text.push_back(line);

    pos_0 = text.rfind('=');
    line = "ID: ";
    line += text.substr(pos_0+1);
    line += "\r\n";
    response_text.push_back(line);
    
    line = "<hr><em>Http Web server</em>\r\n";
    response_text.push_back(line);
    line = "</body><html>";
    line += "\r\n";
    response_text.push_back(line);
  }
  void MakeResourceSize()
  {
    int rs = 0;
    struct stat st;
    stat(path.c_str(),&st);
    rs = st.st_size;
    if(IF_DEBUG_PRINTF)
      std::cout<< "resource_size :" << rs << std::endl;
    SetResourceSize(rs);
  }
  ~Http_Response()
  {}
};

class Http_Request
{
public:
  std::string request_line;
  std::vector<std::string> request_header;
  std::string blank;
  std::string request_text;
private:
  std::string method;
  std::string uri;
  std::string version;
  std::string data;
  std::unordered_map<std::string,std::string> header_kv;
public:
  Http_Request()
      :blank("\r\n")
      {} 
  void RequestLineParse()
  {
     std::stringstream ss(request_line);
     char c;
     ss>> method >> uri >> version;
     transform(method.begin(),method.end(),method.begin(),toupper);
     c = uri.at(0);
     if(c == '/')
     {
        uri.insert(0,".");
     }
     if(uri == "./Post_show")
     {
       uri +=".html";
     } 
  }
  std::string GetUri()
  {
    return uri;
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
  int ContentLength()
  {
    int content_length = -1;
    std::string cl = header_kv["Content-Length"];
    std::stringstream ss(cl);
    ss >> content_length;
    return content_length;
  }
  bool IsNeedRecv()
  {
    return method == "POST" ? true : false;
  }
  bool IsMethodLegal(Http_Response* rsp)
  {
    rsp->SetMethod(method);
    if(method == "GET" || method == "POST")
    {
      return true;
    }
    return false;
  }
  int IsPathLegal(Http_Response* rsp)
  {
    rsp->SetPath(uri);
    if(!IsNeedRecv())//GET请求
    {
      struct stat st;
      if(stat(uri.c_str(),&st) < 0 )
      {
        LOG("file is not exist!",WARNING);
        return 404;
      }
    }
    else
    {
      if(uri != "Post_show.html")
        return 404;
    }
    LOG("Path is OK!",NORMAL);
    return 200;
  }
  bool IsTextLegal()
  { 
    if(request_text != "Name=HNU&ID=CS06142")
      return false;
    else
      return true;
  }
  ~Http_Request()
  {}
};

class Connect
{
private:
  int sock;
public:
  Connect(int sock_) : sock(sock_){}
  int RecvOneLine(std::string& line)
  {
     char buff[BUFF_NUM];
     int i = 0;
     char c = 'x';
     while(c != '\n' && i < BUFF_NUM - 1)
     {
      ssize_t s = recv(sock,&c,1,0);
      if(s > 0)
      {
        if(c == '\r')
        {
          recv(sock,&c,1,MSG_PEEK);
          if(c == '\n')
          {
            recv(sock,&c,1,0);
          }
          else
          {
            c = '\n';
          }
        }
        buff[i++] = c;
      }
      else
        break;
     }
     buff[i] = 0;
     line = buff;
     if(IF_DEBUG_PRINTF)
      std::cout<< " OneLine:" << line << std::endl;
     return i;
  }
  void RecvHeaderLine(std::vector<std::string>& header)
  {
    std::string line = "X";
    while(line != "\n")
    {
        RecvOneLine(line);
        if(line != "\n")
          header.push_back(line); 
    }
    LOG("Header Recv OK!",NORMAL);
  }
  void RecvText(std::string &text,int content_length)
  {
    char c;
    for(auto i = 0; i < content_length; ++i)
    {
      recv(sock,&c,1,0);
      text.push_back(c);
    }
    std::cout<< "text: " << text <<std::endl;
  }
  void SendStatusLine(Http_Response* rsp)
  {
    std::string &sl = rsp->status_line;
    send(sock,sl.c_str(),sl.size(),0);
  }
  void SendHeader(Http_Response* rsp)
  {
    std::vector<std::string> &v = rsp->response_header;
    for(auto it = v.begin();it != v.end();++it)
    {
      send(sock,it->c_str(),it->size(),0);
    }
  }
  void SendText(Connect* conn,Http_Response* rsp)
  {
    std::string &path = rsp->GetPath();
    if(rsp->GetMethod() != "GET")
    {
      std::vector<std::string> &v = rsp->response_text; 
      std::fstream textFile;
      textFile.open(path.c_str(),std::ios::out);
      if(!textFile)
      {
        LOG("open file error!",WARNING);
        return;
      }
      for(auto it = v.begin();it != v.end(); it++)
      {
        //std::cout << "response_text:" << (*it) << std::endl;
        textFile<<(*it);
      }
      textFile.close();
    }
    int fd = open(path.c_str(),O_RDONLY);
    if(fd < 0)
    {
      LOG("open file error!",WARNING);
      return;
    }
    //sendfile在两个文件描述符之间直接传输数据（完全在内核中操作）
    //从而避免了内核缓冲区和用户缓冲区之间拷贝数据，效率很高，这被称为零拷贝
    rsp->MakeResourceSize();
    rsp->MakeResponseHeader();
    conn->SendHeader(rsp);
    sendfile(sock,fd,NULL,rsp->resourceSize());
    close(fd);
  } 
  ~Connect()
  {
    close(sock);
  }
};

class Entry
{
public:
  static int ProcessResponse(Connect* conn,Http_Request* rq,Http_Response* rsp)
  {
    rsp->MakeStatusLine();
    conn->SendStatusLine(rsp);
    conn->SendText(conn,rsp);
    LOG("Send Response Done!",NORMAL);
    return 200;
  }
  static void HandlerRequest(int sock)
  {
    pthread_detach(pthread_self());

    Connect* conn = new Connect(sock);
    Http_Request* rq = new Http_Request;
    Http_Response* rsp = new Http_Response();
    int& code = rsp->Code();

    conn->RecvOneLine(rq->request_line);//响应行
    rq->RequestLineParse();

    conn->RecvHeaderLine(rq->request_header);//响应头
    rq->HeaderParse();

    if(!rq->IsMethodLegal(rsp))
    {
      code = 501;
      LOG("Request Method Is Not Legal",WARNING);
      goto end;
    }
    if((code = rq->IsPathLegal(rsp)) != 200)
    {
      LOG("File Is Not Exist!",WARNING);
      goto end;
    }

    if(rq->IsNeedRecv())
    {
      LOG("POST Method,Need Recv Begin!",NORMAL);
      conn->RecvText(rq->request_text,rq->ContentLength());
      if(!rq->IsTextLegal())
      {
        code = 404;
        LOG("POST DATA IS NOT Legal",WARNING);
        goto end;
      }
      rsp->TextParse(rq->request_text);
    }
    
    LOG("HTTP Request Recv Done,OK!",NORMAL);
    code = ProcessResponse(conn,rq,rsp);
end:
    if(code != 200)
    {
      std::string except_path = Util::CodeToExceptFile(code);
      //std::cout<< "rsp->GetPath():"<<rsp->GetPath()<<std::endl;
      rsp->SetOriginPath(rsp->GetPath());
      rsp->SetPath(except_path);
      rsp->TextParse(rq->request_text);
      ProcessResponse(conn,rq,rsp);
    }
    delete conn;
    delete rq;
    delete rsp;
  }
};

#endif