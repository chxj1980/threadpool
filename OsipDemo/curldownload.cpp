#include "curldownload.h"
#include <curl.h>

//构造函数
CurlDownload::CurlDownload()
{
	//初始化curl库
	curl_global_init(CURL_GLOBAL_ALL);
}

//析构函数
CurlDownload::~CurlDownload()
{
	//析构库
	curl_global_cleanup();
}

//下载线程
int CurlDownload::Process(const char* url, WriteFileCallBack callback_fun, void *pParams)
{
	//初始化句柄
	CURL *curl_handle = curl_easy_init();

	//设置选项
	curl_easy_setopt(curl_handle, CURLOPT_URL, url);						//下载地址
	curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, callback_fun);		//设置写入函数
	curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, pParams);				//设置写入嘻嘻
	curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "test-agent/1.0");		//设置用户代理
	//  curl_easy_setopt(curl_handle, CURLOPT_PROXY, "192.168.0.1:808");	//设置代理地址
	//  curl_easy_setopt(curl_handle, CURLOPT_HTTPPROXYTUNNEL, 1L);			//设置代理协议通道
	int ret = curl_easy_perform(curl_handle);

	curl_easy_cleanup(curl_handle);
	return 0;
}