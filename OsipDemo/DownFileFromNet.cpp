#include <iostream>
#include "threadtask.h"
#include "filenameio.h"
#include "taskthread.h"
#include "threadpool.h"
#include "curldownload.h"

using namespace std;

//文件指针
typedef struct _FileData{
	FILE *fp;
}FileData, *pFileData;

//文件存盘
size_t WriteToDisk(void *contents, size_t size, size_t mem, void* userp)
{
	pFileData pData = (pFileData)userp;
	size_t num = mem*size;
	size_t write_num = fwrite(contents, 1, num, pData->fp);
	return write_num;
}

//线程函数
void *RunTaskFunc(void * arg)
{
	int* i = (int*)arg;
	cout << "thread index: " << *i << endl;
	CurlDownload* manager = new CurlDownload();
	static const char* url = "https://timgsa.baidu.com/timg?image&quality=80&size=b9999_10000&sec=1517664143964&di=e97ea8cf8b63706194c0d2e7a8ed1f76&imgtype=0&src=http%3A%2F%2Fimg.tupianzj.com%2Fuploads%2FBizhi%2Fmn53_12802.jpg";

	FileData dd;
	char path[8];
	memset(path, 0, sizeof(path));
	sprintf(path, "%d.jpg", *i);
	dd.fp = fopen(path, "wb");
	manager->Process(url, &WriteToDisk, &dd);

	fclose(dd.fp);
	return NULL;
}


int main(void)
{
	cout << "测试程序开始" << endl;
	CThreadPool *pool = new CThreadPool(5);
	pool->Activate();

	for (int o = 0; o < 10; ++o)
	{
		int *i = new int;
		*i = o;
		pool->AddAsynTask(&RunTaskFunc, i);
	}

	getchar();

	pool->Destroy();
	delete pool;
	cout << "测试线程池结束"<<endl;

	system("pause");
	return 0;
}