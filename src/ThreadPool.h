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
	std::condition_variable condition_todo;// ˯�ߵȴ�ʱʹ��
	std::vector<void*> argv_fun; 
	std::vector<bool> isActive;// ���߳��Ƿ񱻼���
	std::vector<bool> isUnDone;// ���̵߳������Ƿ�δ���
	std::vector<bool> isBusyWait;// ���̵߳��Ƿ��趨Ϊ��æ�ȴ���״̬
	std::mutex mutex_task;
	spin_mutex mutex_spin;
	bool isQuit;// �߳��˳�
};

class ThreadPool {
public:
	void InitPool();// ��ʼ���̳߳�

	void Start(AGENT_FUN imp, std::vector<void*> &argv_fun);// �������߳�����,impΪ�߳���Ҫʵ�ֵĴ����,argv_funΪ�����б�,�ж��ٸ�Ԫ�ش�����Ҫ���������߳�

	void Wait();// ����ͬ����

	void purse(); // ������߳̽���˯�ߵȴ�

	void UnInitPool();// ����ʼ���̳߳�
private:
	ThreadArg _argv;
	std::vector<std::thread> _vThreadPool;
	const int _MAX_THREAD_NUM = 16;
};