#ifndef _CSR_LOG_H_
#define _CSR_LOG_H_

#include "filenameio.h"
//#include <windows.h>
//#include <fileio.h>
//#include <fileapi.h>

#include <winsock2.h>


class csrlog{

private:
	FILE *m_fp;

public:
	void InitCSRLog()
	{
		std::string logPath = GetMoudlePath();
		logPath += "LogFile\\";
		CreateDirectory(logPath.c_str(), FALSE);
		//获取本地时间，并创建文件名
		SYSTEMTIME st;
		GetLocalTime(&st);
		char chLogFileName[MAX_PATH];
		sprintf(chLogFileName, "%d-%d-%d-%d-%d-%d.txt", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
		logPath += chLogFileName;
		m_fp = fopen(logPath.c_str(), "wt");
	}

	//输出日志
	void CsrPrintLog(const char* str, ...)
	{
		// (1) 定义参数列表
		va_list ap;
		
		// (2) 初始化参数列表
		va_start(ap, str);

		// 获取参数值
		fprintf(m_fp, ap);

		//直接输出控制台
		printf(ap);			

		// 关闭参数列表
		va_end(ap);
	}


	void ReleaseCsrLog()
	{
		if (m_fp != NULL)
		{
			fclose(m_fp);
			m_fp == NULL;
		}
	}
};


#endif