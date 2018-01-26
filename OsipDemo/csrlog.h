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
		//��ȡ����ʱ�䣬�������ļ���
		SYSTEMTIME st;
		GetLocalTime(&st);
		char chLogFileName[MAX_PATH];
		sprintf(chLogFileName, "%d-%d-%d-%d-%d-%d.txt", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
		logPath += chLogFileName;
		m_fp = fopen(logPath.c_str(), "wt");
	}

	//�����־
	void CsrPrintLog(const char* str, ...)
	{
		// (1) ��������б�
		va_list ap;
		
		// (2) ��ʼ�������б�
		va_start(ap, str);

		// ��ȡ����ֵ
		fprintf(m_fp, ap);

		//ֱ���������̨
		printf(ap);			

		// �رղ����б�
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