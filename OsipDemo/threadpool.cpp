#include "threadpool.h"
#include <stdlib.h>
#include "cosip.h"
#include "filenameio.h"

using namespace sip;

//#pragma comment(lib,"pthreadVC2.lib")
//
//void *PrintHello(void *threadid)
//{
//	int tid;
//	tid = (int)threadid;
//	printf("Hello World!It's me,thread #%d!\n", tid);
//	pthread_exit(NULL);
//
//	return NULL;
//}

int main(void)
{
	/*pthread_t threads[NUM_THREADS];
	int rc, t;
	for (t = 0; t < NUM_THREADS; t++)
	{
		printf("In main:creating thread %d\n", t);
		rc = pthread_create(&threads[t], NULL, PrintHello, (void *)t);
		if (rc)
		{
			printf("ERROR:return code from pthread_create() is %d\n", rc);
			exit(-1);
		}
	}
	pthread_exit(NULL);*/
	printf("thread\n");

	//测试C++接口
	//加载配置文件
	std::string strIniFile = GetMoudlePath();
	strIniFile += "GB28181.ini";
	cosip soip(strIniFile);

	//初始化服务 用jrtp库接收
	soip.StartPSStream(0);

	system("pause");
}
