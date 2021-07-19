#include<ThreadPool.h>

int theadPool()
{
	int k[4] = { 0,1,2,4 };
	std::vector<void*> params;
	params.push_back(k);
	params.push_back(k + 1);
	params.push_back(k + 2);

	ThreadPool pool;
	pool.InitPool();

	pool.Start([](void *param) { auto *pInt = static_cast<int*>(param); printf("first imp %d\n",*pInt);  }, params);
	pool.Wait();
	//while (1){} // uncomment tiis line will see,CPU is busy

	k[0] = 111;
	k[1] = 111;
	k[2] = 222;

	pool.Start([](void *param) { auto *pInt = static_cast<int*>(param); printf("Second imp %d\n", *pInt);  }, params);
	pool.purse();
	pool.Wait();
	while (1){} // uncomment tiis line will see,CPU is sleep
	std::this_thread::sleep_for(std::chrono::milliseconds(2000));

	pool.UnInitPool();
	return 0;
}

int main(int argc,char **argv)
{
	return theadPool();
}