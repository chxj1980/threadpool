#ifndef _TASK_THREAD_H_
#define _TASK_THREAD_H_

#include <pthread.h>
#include "threadtask.h"

class CThreadPool;

//�����߳�
class CTaskThread{
public:
	CTaskThread(CThreadPool* pool);
	~CTaskThread();

	//����Ԫ
	CThreadTask *m_task;

	//��������������
	int AddToFreeThreadQueue();
	void Notify();

	//����
	void Lock();
	void UnLock();
	void Wait();
	void Join();
	int GetId();
	void Destroy();
	void Start();

private:
	pthread_mutex_t m_mutex;			// ��
	pthread_cond_t	m_cond_t;         
	bool			m_bIsDestroy;
	pthread_t		m_thread;
	int				m_thread_id;		// �߳�id
	CThreadPool		*m_pool;            // �����̳߳�

	static void *DoTask(void *pParams);
};


#endif //_TASK_THREAD_H_