#include "threadpool.h"
#include "threadtask.h"
#include "taskthread.h"
#include <stdlib.h>
#include <windows.h>
#include "cosip.h"
#include "filenameio.h"

using namespace sip;
using namespace std;

#pragma comment(lib,"pthreadVC2.lib")

#if 0
	//测试C++接口
	//加载配置文件
	std::string strIniFile = GetMoudlePath();
	strIniFile += "GB28181.ini";
	cosip soip(strIniFile);

	//初始化服务 用jrtp库接收
	soip.StartPSStream(0);

	while (true)
	{
		//不断获取视频流
		//解码
		
		//sdl显示
	}
#endif


CThreadPool::CThreadPool(size_t maxthreadnum)
{
	MaxThreadNum = maxthreadnum;
	bDestroy = false;
	bWaitDestroy = false;
	m_taskThreadMutex = NULL;
	m_freeThreadMutex = NULL;
	WatiTime = 500;
	pthread_mutex_init(&m_taskThreadMutex, NULL);
	pthread_cond_init(&m_threadCond, NULL);
	pthread_mutex_init(&m_freeThreadMutex, NULL);
}

//销毁类对象及成员
CThreadPool::~CThreadPool()
{
	cout << "线程池析构函数" << endl;
	pthread_mutex_destroy(&m_taskThreadMutex);
	pthread_cond_destroy(&m_threadCond);
	pthread_mutex_destroy(&m_freeThreadMutex);
}

//添加任务
void CThreadPool::AddAsynTask(TaskFunc taskfunc, void* pParams)
{
	CThreadTask *task = new CThreadTask(taskfunc, pParams);
	LockTaskQueue();

	//添加任务到队列
	m_queTask.push(task);
	SignalTaskQueue();
	UnLockTaskQueue();
}

//激活线程池
void CThreadPool::Activate()
{
	for (int i = 0; i < MaxThreadNum; ++i)
	{
		CTaskThread* thread = new CTaskThread(this);
		m_arrThreads.push_back(thread);
		thread->Start();
	}

	//启动扫描线程
	pthread_create(&m_taskThread, NULL, &ScanTask, this);
}

//销毁线程池
void CThreadPool::Destroy()
{
	cout << "Destory begin" << endl;
	bDestroy = true;
	
	//停止扫描线程
	pthread_join(m_taskThread, NULL);

	//停止工作线程
	size_t size = m_arrThreads.size();
	for (size_t i = 0; i < size; ++i)
	{
		CTaskThread* thread = m_arrThreads[i];
		thread->Destroy();
		delete thread;
	}

	size_t remain = m_queTask.size();
	for (size_t i = 0; i < remain; ++i)
	{
		CThreadTask *task = m_queTask.front();
		m_queTask.pop();
		delete task;
	}

	cout << "Destroy end" << endl;
}

//等在所有线程完成任务
void CThreadPool::WaitTaskFinishAndDestroy()
{
	bWaitDestroy = true;
	pthread_join(m_taskThread, NULL);

	//停止工作线程
	for (size_t i = 0; i < m_arrThreads.size(); ++i)
	{
		CTaskThread* thread = m_arrThreads[i];
		thread->Destroy();
		cout << "thread->Destroy()" << endl;
		delete thread;
	}
}

//添加空闲线程进线程池
void CThreadPool::AddFreeThreadToQueue(CTaskThread *thread)
{
	//等待下一个任务
	LockFreeThreadQueue();
	m_queFreeThreads.push(thread);
	UnLockFreeThreadQueue();
}

//改变条件
void CThreadPool::SignalTaskQueue()
{
	pthread_cond_signal(&m_threadCond);
}

//锁住任务队列
void CThreadPool::LockTaskQueue()
{
	pthread_mutex_lock(&m_taskThreadMutex);
}

void CThreadPool::UnLockTaskQueue()
{
	pthread_mutex_unlock(&m_taskThreadMutex);
}

//锁住空闲线程队列
void CThreadPool::LockFreeThreadQueue()
{
	pthread_mutex_lock(&m_freeThreadMutex);
}

void CThreadPool::UnLockFreeThreadQueue()
{
	pthread_mutex_unlock(&m_freeThreadMutex);
}
//扫描任务
void* CThreadPool::ScanTask(void* pParams)
{
	CThreadPool *pool = (CThreadPool*)pParams;
	size_t queNum = 0;
	size_t freeQueNum = 0;
	while (true)
	{
		if (pool->bDestroy)
			break;
		pool->Start(&queNum, &freeQueNum);
		if (pool->bWaitDestroy && !queNum && freeQueNum == pool->GetMaxThreadNum())
		{
			pool->bDestroy = true;
			break;
		}

		if (queNum)
		{
			if (freeQueNum)
				continue;
			else
				Sleep(pool->GetWatiTime());		//无空闲线程
		}
		else//进入任务队列
		{	
			pool->LockTaskQueue();
			if (!pool->GetQueueTaskCount())
			{
				pool->WaitQueueTaskDignal();
			}
			pool->UnLockTaskQueue();
		}
	}
	cout << "Scantask end" << endl;
	return nullptr;
}

int CThreadPool::GetQueueTaskCount()
{
	return m_queTask.size();
}
void CThreadPool::WaitQueueTaskDignal()
{
	pthread_cond_wait(&m_threadCond, &m_taskThreadMutex);
}

/*
函数功能：执行线程
输出参数1：队列中剩余线程数量
输出参数2：队列中剩余空闲线程数量
*/
void CThreadPool::Start(size_t *queue_remain_num, size_t *free_thread_num)
{
	cout << "开始执行任务" << endl;
	LockFreeThreadQueue();
	if (!m_queFreeThreads.empty())
	{
		LockTaskQueue();
		if (m_queTask.empty())
		{
			CThreadTask* task = m_queTask.front();
			m_queTask.pop();
			CTaskThread* freeThread = m_queFreeThreads.front();
			m_queFreeThreads.pop();
			freeThread->m_task = task;
			freeThread->Notify();

			*queue_remain_num = m_queTask.size();
		}
		else
		{
			*queue_remain_num = 0;
		}

		UnLockTaskQueue();

		*free_thread_num = m_queFreeThreads.size();
	}
	else
	{
		*free_thread_num = 0;
	}
	UnLockFreeThreadQueue();
}