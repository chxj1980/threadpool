#ifndef __THREAD_TASK_H_
#define __THREAD_TASK_H_

//任务函数
typedef void* (*TaskFunc)(void* pParams);


//任务类
class CThreadTask{
public:
	CThreadTask(TaskFunc task, void* pParams)
	{
		m_params = pParams;
		m_task = task;
	}

public:
	void*	  m_params;
	TaskFunc  m_task;
};

#endif	//__THREAD_TASK_H_