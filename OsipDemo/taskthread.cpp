#include "taskthread.h"
#include "threadpool.h"
#include <iostream>

using namespace std;

//���캯������ʼ����Ա����
CTaskThread::CTaskThread(CThreadPool* pool)
{
	m_task = NULL;
	m_bIsDestroy = false;
	m_mutex = NULL;
	m_cond_t = NULL;
	m_pool = pool;
	m_thread_id = 0;

	//��ʼ����
	pthread_mutex_init(&m_mutex, NULL);
	//��ʼ����������
	pthread_cond_init(&m_cond_t, NULL);
}

//��������������������������
CTaskThread::~CTaskThread()
{
	pthread_mutex_destroy(&m_mutex);
	pthread_cond_destroy(&m_cond_t);
}

//��������������
int CTaskThread::AddToFreeThreadQueue()
{
	m_pool->AddFreeThreadToQueue(this);
	return m_pool->GetFreeThreadNum();					//��ǰ��ʣ�������ڿ��е��߳� 
}
void CTaskThread::Notify()
{
	pthread_cond_signal(&m_cond_t);
}

//����
void CTaskThread::Lock()
{
	pthread_mutex_lock(&m_mutex);
}
void CTaskThread::UnLock()
{
	pthread_mutex_unlock(&m_mutex);
}
void CTaskThread::Wait()
{
	pthread_cond_wait(&m_cond_t, &m_mutex);
}
void CTaskThread::Join()
{
	int res = pthread_join(m_thread, NULL);
	cout << "res :" << res << endl;
}
int CTaskThread::GetId()
{
	return m_thread_id;
}
void CTaskThread::Destroy()
{
	Lock();
	m_bIsDestroy = true;
	Notify();
	UnLock();
	Join();
}

//�����߳�
void CTaskThread::Start()
{
	pthread_create(&m_thread, NULL, &CTaskThread::DoTask, this);
	m_thread_id = (int)m_thread.p;
}

void *CTaskThread::DoTask(void *pParams)
{
	CTaskThread *thread = (CTaskThread*)pParams;
	while (true)
	{
		thread->Lock();
		if (thread->m_bIsDestroy)
		{
			thread->UnLock();
			break;
		}
		thread->UnLock();

		CThreadTask *task = thread->m_task;
		if (task)
		{
			(*task->m_task)(task->m_params);
			cout << "task finish" << thread->GetId() << endl;
			delete task;
			thread->m_task = NULL;
		}

		//֪ͨ���м�������̣߳������̣߳������Ҫ���٣�����Ҫ����ȴ�
		thread->Lock();
		if (thread->m_bIsDestroy)
		{
			thread->UnLock();
			break;
		}
		thread->AddToFreeThreadQueue();
		thread->Wait();
		thread->UnLock();
	}
	cout << "thread finish:" << thread->GetId() << endl;
	return nullptr;
}