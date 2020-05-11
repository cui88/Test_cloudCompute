#ifndef __THREADPOOL_HPP__
#define __THREADPOOL_HPP__

#include<iostream>
#include<pthread.h>
#include<queue>

#define IF_DEBUG_PRINTF 0

typedef void (*Handler)(int);

class Task
{
    private:
	    int sock;
	    Handler handler;
	public:
        Task(int sock_,Handler handler_):sock(sock_),handler(handler_)
        {}
        void Run()
        {
        	handler(sock);
        }
        ~Task()
        {}
};

class ThreadPool
{
    private:
    	int num;
    	int idle_num;
    	std::queue<Task> task_queue;
    	pthread_mutex_t lock;
    	pthread_cond_t cond;
    public:
    	ThreadPool(int num_):num(num_),idle_num(0)
    	{
    		pthread_mutex_init(&lock,NULL);
    		pthread_cond_init(&cond,NULL);
    	}
    	void PushTask(Task &t)
    	{ 
    		pthread_mutex_lock(&lock);
    		task_queue.push(t);
            pthread_mutex_unlock(&lock);
            Wakeup();
    	}
    	void Wakeup()
    	{
    		pthread_cond_signal(&cond);
    	}
    	void Idle()
    	{
    		idle_num ++;
    		pthread_cond_wait(&cond,&lock);
    		idle_num --;
    	}
    	void InitThreadPool()
    	{
    		pthread_t tid;
    		for(auto i = 0;i< num;i++)
    		{
    			pthread_create(&tid,NULL,ThreadRoutine,(void*)this);
    		}
    	}
    	void LockQueue()
    	{
            pthread_mutex_lock(&lock);
    	}
    	void UnlockQueue()
    	{
    		pthread_mutex_unlock(&lock);
    	}
    	bool IsTaskQueueEmpty()
    	{
    		return task_queue.empty();
    	}
    	Task PopTask()
    	{
            Task t = task_queue.front();
            task_queue.pop();
            return t;
    	}
    	static void* ThreadRoutine(void* arg)
    	{
    		pthread_detach(pthread_self());
            ThreadPool* tp = (ThreadPool*)arg;

    		for(;;)
            {
            	tp->LockQueue();
                while(tp->IsTaskQueueEmpty())
                {
                	tp->Idle();
                }
                Task t = tp->PopTask();
            	tp->UnlockQueue();   
            	if(IF_DEBUG_PRINTF)
            	   std::cout<< "task is handler by: " << pthread_self() << std::endl;
            	t.Run();
            }
    	}
    	~ThreadPool()
    	{
    		pthread_mutex_destroy(&lock);
    		pthread_cond_destroy(&cond);
    	}
};

class singleton
{
private:
	static ThreadPool* p;
	static pthread_mutex_t lock;
	int num_thread;

public:
	singleton(int num_thread_):num_thread(num_thread_){}
    ThreadPool* GetInstance()
    {
    	if(NULL == p)
    	{
    		pthread_mutex_lock(&lock);
    		if(NULL == p)
    		{
    			p = new ThreadPool(num_thread);
    			p->InitThreadPool();

    		}
    		pthread_mutex_unlock(&lock);
    	}
    	return p;
    }
    ~singleton()
    {}
};

ThreadPool *singleton::p = NULL;
pthread_mutex_t singleton::lock = PTHREAD_MUTEX_INITIALIZER;


#endif