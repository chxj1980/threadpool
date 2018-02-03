#ifndef _THREAD_POOL_H_
#define _THREAD_POOL_H_

/*
	�̳߳ش���
	----------------------------------------------------------------------------

	---->�����̳߳�---->�������--->ѡ������̴߳�������--->���������ͷ��߳�
	//�����̵߳Ĺ������ݺ�ÿ���̵߳Ķ�������
*/ 

#include <pthread.h>
#include <stdio.h>
#include <iostream>
#include <vector>
#include <queue>
#include "threadtask.h"
#include "ProcDef.h"

//�����߳�
class CTaskThread;

//�̳߳�
class CThreadPool{
public:
	CThreadPool(size_t maxthreadnum);
	~CThreadPool();

	void AddAsynTask(TaskFunc taskfunc, void* pParams);		//�������
	void Activate();		//�����̳߳�
	void Destroy();			//�����̳߳�
	void WaitTaskFinishAndDestroy();				//���������߳��������
	void AddFreeThreadToQueue(CTaskThread *thread);	//��ӿ����߳̽��̳߳�

	/*
		�������ܣ�ִ���߳�
		�������1��������ʣ���߳�����
		�������2��������ʣ������߳�����
	*/
	void Start(size_t *queue_remain_num, size_t *free_thread_num);

	bool bDestroy;		//�����߳�
	bool bWaitDestroy;	//�ȴ������߳�
	int GetQueueTaskCount();
	void WaitQueueTaskDignal();

private:
		TPGETSET(unsigned, WatiTime)		//	�ȴ�ʱ��
		TPGETSET(size_t, MaxThreadNum)		//	�̳߳�����
		TPGETSET(int, FreeThreadNum)		//	�����߳�

		static void* ScanTask(void* pParams);	//ɨ������

		void SignalTaskQueue();
		void LockTaskQueue();
		void UnLockTaskQueue();

		void LockFreeThreadQueue();
		void UnLockFreeThreadQueue();

		std::vector<CTaskThread*> m_arrThreads;			//	�̳߳�����
		std::queue<CTaskThread*>  m_queFreeThreads;		//	�����̳߳ض���
		pthread_mutex_t			  m_freeThreadMutex;	//	�̳߳ػ�����
		std::queue<CThreadTask*>  m_queTask;			//  �������
		pthread_mutex_t			  m_taskThreadMutex;		//  �̳߳ػ�����
		pthread_cond_t			  m_threadCond;			//	�̳߳ض����ź�
		pthread_t                 m_taskThread;			//	����ɨ���̣߳��ж��̳߳��Ƿ���δ�������
};

#endif		//_THREAD_POOL_H_