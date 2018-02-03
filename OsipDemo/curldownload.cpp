#include "curldownload.h"
#include <curl.h>

//���캯��
CurlDownload::CurlDownload()
{
	//��ʼ��curl��
	curl_global_init(CURL_GLOBAL_ALL);
}

//��������
CurlDownload::~CurlDownload()
{
	//������
	curl_global_cleanup();
}

//�����߳�
int CurlDownload::Process(const char* url, WriteFileCallBack callback_fun, void *pParams)
{
	//��ʼ�����
	CURL *curl_handle = curl_easy_init();

	//����ѡ��
	curl_easy_setopt(curl_handle, CURLOPT_URL, url);						//���ص�ַ
	curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, callback_fun);		//����д�뺯��
	curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, pParams);				//����д������
	curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "test-agent/1.0");		//�����û�����
	//  curl_easy_setopt(curl_handle, CURLOPT_PROXY, "192.168.0.1:808");	//���ô����ַ
	//  curl_easy_setopt(curl_handle, CURLOPT_HTTPPROXYTUNNEL, 1L);			//���ô���Э��ͨ��
	int ret = curl_easy_perform(curl_handle);

	curl_easy_cleanup(curl_handle);
	return 0;
}