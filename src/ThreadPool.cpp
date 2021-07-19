#include<ThreadPool.h>

static void ResetArg(ThreadArg *arg)
{
	std::lock_guard<std::mutex> lk(arg->mutex_task);
	for (int i = 0; i < arg->argv_fun.size(); i++)
	{
		arg->isActive[i] = true;
	}
	for (auto & b : arg->isUnDone)
	{
		b = true;
	}
	return;
}

static void process_thread(void *arg_src,int id)
{
	ThreadArg *arg = static_cast<ThreadArg*>(arg_src);
	while (1)
	{
		if (!arg->isBusyWait[id])
		{
			std::unique_lock<std::mutex> lk(arg->mutex_task);
			printf("[%d] Wait.\n", id);
			while (!(arg->isActive[id]&&arg->isUnDone[id])) // active and this thread don't finish it's work
				arg->condition_todo.wait(lk);
			lk.unlock();
		}
		else
		{
			printf("[%d] Busy Wait.\n", id);
			while (1)
			{
				std::lock_guard<spin_mutex> lk(arg->mutex_spin);
				if ((arg->isActive[id]&&arg->isUnDone[id]) || !arg->isBusyWait[id])
				{
					break;
				}
			}
		}
		
		if (arg->isQuit)
		{
			printf("[%d] Quit.\n",id);
			std::this_thread::sleep_for(std::chrono::milliseconds(2000));
			break;
		}

		if (arg->isActive[id])
		{
			printf("[%d] Do Work\n", id);
			// do the work
			arg->imp(arg->argv_fun[id]);

			std::lock_guard<std::mutex> lk(arg->mutex_task);
			arg->isUnDone[id] = false;
		}
	}
	return ;
}


void ThreadPool::InitPool()
{
	int MAX_NUM = _MAX_THREAD_NUM;
	for (int i = 0; i < MAX_NUM; i++)
	{
		_argv.isUnDone.push_back(true);
		_argv.argv_fun.push_back(NULL);
		_argv.isBusyWait.push_back(false);
		_argv.isActive.push_back(false);
	}
	_argv.isQuit = false;

	for (int i = 1; i < MAX_NUM; i++)
	{
		_vThreadPool.emplace_back(process_thread,(void*)(&_argv), i);// 传入参数和ID
	}
	return;
}

void ThreadPool::Start(AGENT_FUN imp, std::vector<void*> &argv_fun)
{
	if (argv_fun.size() > _MAX_THREAD_NUM)
		return;
	for (int i = 0; i < argv_fun.size(); i++)
	{
		_argv.isBusyWait[i] = true;
	}
	_argv.imp = imp;
	_argv.argv_fun = argv_fun;
	ResetArg(&_argv);
	_argv.condition_todo.notify_all();
}

void ThreadPool::Wait()
{
	int id = 0;
	bool thisDone = false;
	while (1)
	{
		// do
		if (!thisDone)
		{
			_argv.imp(_argv.argv_fun[0]);
			_argv.isUnDone[0] = false;
			thisDone = true;
		}

		// check
		std::lock_guard<std::mutex> lk(_argv.mutex_task);
		bool isAllDone = true;
		for (int i = 0; i < _argv.argv_fun.size(); i++)
		{
			if (_argv.isUnDone[i])
				isAllDone = false;
		}
		if (isAllDone)
		{
			for(auto &ac : _argv.isActive)
				ac = false;
			break;
		}
	}
	return;
}

void ThreadPool::purse()
{
	std::lock_guard<spin_mutex> lk(_argv.mutex_spin);
	for (int i = 0; i < _MAX_THREAD_NUM; i++)
	{
		_argv.isBusyWait[i] = false;
	}
}

void ThreadPool::UnInitPool()
{
	_argv.isQuit = true;
	ResetArg(&_argv);
	_argv.condition_todo.notify_all();

	for (int i = 0; i < _MAX_THREAD_NUM; i++)
	{
		if(_vThreadPool[i].joinable())
			_vThreadPool[i].join();
	}
	return;
}

