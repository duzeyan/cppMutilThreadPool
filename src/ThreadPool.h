#include <iostream>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <queue>
#include <vector>
#include <atomic>

typedef void(*AGENT_FUN)(void *);

class spin_mutex {
	std::atomic<bool> flag = ATOMIC_VAR_INIT(false);
public:
	spin_mutex() = default;
	spin_mutex(const spin_mutex&) = delete;
	spin_mutex& operator= (const spin_mutex&) = delete;
	void lock() {
		bool expected = false;
		while (!flag.compare_exchange_strong(expected, true))
			expected = false;
	}
	void unlock() {
		flag.store(false);
	}
};

class ThreadArg
{
public:

	AGENT_FUN imp;
	std::condition_variable condition_todo;// 睡眠等待时使用
	std::vector<void*> argv_fun; 
	std::vector<bool> isActive;// 该线程是否被激活
	std::vector<bool> isUnDone;// 该线程的任务是否未完成
	std::vector<bool> isBusyWait;// 该线程的是否被设定为“忙等待”状态
	std::mutex mutex_task;
	spin_mutex mutex_spin;
	bool isQuit;// 线程退出
};

class ThreadPool {
public:
	void InitPool();// 初始化线程池

	void Start(AGENT_FUN imp, std::vector<void*> &argv_fun);// 开启多线程任务,imp为线程需要实现的代码段,argv_fun为参数列表,有多少个元素代表需要启动多少线程

	void Wait();// 任务同步点

	void purse(); // 激活的线程进入睡眠等待

	void UnInitPool();// 反初始化线程池
private:
	ThreadArg _argv;
	std::vector<std::thread> _vThreadPool;
	const int _MAX_THREAD_NUM = 16;
};