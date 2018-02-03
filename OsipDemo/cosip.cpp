#include "cosip.h"
#include "csrlog.h"
#include <process.h>
#include <mxml.h>
//#include <windows.h>
#include "timestramp.h"

//jrtp库需要的头文件
#include <iostream>
#include "rtpsession.h"
#include "rtppacket.h"
#include "rtpudpv4transmitter.h"
#include "rtpipv4address.h"
#include "rtpsessionparams.h"
#include "rtperrors.h"
#include "rtpsourcedata.h"

using namespace sip;
using namespace jrtplib;

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "mxml1.lib")
#pragma comment(lib, "eXosip.lib")
#pragma comment(lib, "libcares.lib")
#pragma comment(lib, "osip2.lib")

//Dnsapi.lib;Iphlpapi.lib;ws2_32.lib;eXosip.lib;osip2.lib;osipparser2.lib;Qwave.lib;libcares.lib;delayimp.lib;
//忽略 libcmt.lib默认库
#pragma comment(lib, "Dnsapi.lib")
#pragma comment(lib, "Iphlpapi.lib")
#pragma comment(lib, "osipparser2.lib")
#pragma comment(lib, "Qwave.lib")
#pragma comment(lib, "delayimp.lib")

#ifdef DEBUG
#pragma comment(lib, "jrtplib_d.lib") 
#pragma comment(lib,"jthread_d.lib")
#pragma comment(lib,"WS2_32.lib")
#else
#pragma comment(lib, "jrtplib.lib") 
#pragma comment(lib,"jthread.lib")
#pragma comment(lib,"WS2_32.lib")
#endif

//加载配置信息，启动线程监听事件
cosip::cosip(std::string iniFilePath)
{
	m_log = new csrlog();
	m_log->InitCSRLog();
	if (!LoadIniFilePath(iniFilePath))
	{
		m_log->CsrPrintLog("记载配置文件失败%s\n");
		exit(-1);
	}

	//初始化服务
	if (!InitSIPService())
	{
		printf("初始化服务失败\n");
		exit(-1);
	}

}

cosip::~cosip()
{
	ReleaseService();
}
void cosip::DecodeH264(int nType /* = 0*/)
{

}


//加载配置文件
bool cosip::LoadIniFilePath(std::string iniFilePath)
{
	if (iniFilePath == "")
	{
		m_log->CsrPrintLog("配置文件路径为空\n");
		return false;
	}
	::GetPrivateProfileString("GB28181", "platform_id", "你好", m_VideoInfo.SParams.chPlatfromSipID, MAX_PATH, iniFilePath.c_str());	//获取平台ID
	m_VideoInfo.SParams.nPlatformSipPort = GetPrivateProfileInt("GB28181", "platform_port", 0, iniFilePath.c_str());					//获取平台端口
	::GetPrivateProfileString("GB28181", "platform_ip", "你好", m_VideoInfo.SParams.chPlatfromIPAddr, MAX_PATH, iniFilePath.c_str());	//获取平台IP
	::GetPrivateProfileString("GB28181", "local_id", "你好", m_VideoInfo.SParams.chLocalSipID, MAX_PATH, iniFilePath.c_str());		//获取本地ID
	m_VideoInfo.SParams.nLocalSipPort = GetPrivateProfileInt("GB28181", "local_port", 0, iniFilePath.c_str());						//获取本地端口
	::GetPrivateProfileString("GB28181", "local_ip", "你好", m_VideoInfo.SParams.chLocalIPAddr, MAX_PATH, iniFilePath.c_str());		//获取平台IP
	m_VideoInfo.nCameraNum = GetPrivateProfileInt("GB28181", "camera_num", 0, iniFilePath.c_str());										//相机数量

	if (m_VideoInfo.nCameraNum > 0 && m_VideoInfo.nCameraNum < CAMERA_SUPPORT_MAX)
	{
		m_VideoInfo.pCameraParams = (CameraInfo *)malloc(sizeof(CameraInfo)*m_VideoInfo.nCameraNum);
		if (m_VideoInfo.pCameraParams == NULL) 
		{
			m_log->CsrPrintLog("camera params malloc, failed\n");
			return false;
		}
		memset(m_VideoInfo.pCameraParams, 0, sizeof(CameraInfo)*m_VideoInfo.nCameraNum);
		CameraInfo *p;

		p = m_VideoInfo.pCameraParams;

		GetPrivateProfileString("GB28181", "camera1_sip_id", "", p->chSipID, MAX_PATH, iniFilePath.c_str());
		p->nRecvPort = GetPrivateProfileInt("GB28181", "camera1_recv_port", 0, iniFilePath.c_str());

		//获取相机登录名和密码
		GetPrivateProfileString("GB28181", "UserPwd", "", p->chUserPwd, MAX_PATH, iniFilePath.c_str());
		GetPrivateProfileString("GB28181", "UserName", "", p->chUserName, MAX_PATH, iniFilePath.c_str());
	}

	m_VideoInfo.SParams.nSN = 1;
	m_VideoInfo.SParams.nCallID = -1;
	m_VideoInfo.SParams.nDialogID = -1;
	m_VideoInfo.SParams.nRegisterOK = 0;
	m_VideoInfo.bRunning = 1;
	m_VideoInfo.SParams.bRunning = 1;

	m_log->CsrPrintLog("加载配置文件完成\n");
	return true;
}

static void RegisterSuccess(struct eXosip_t * pectx, eXosip_event_t *je)
{
	int ireturncode = 0;
	osip_message_t * psregister = nullptr;
	ireturncode = eXosip_message_build_answer(pectx, je->tid, 200, &psregister);
	if (ireturncode == 0 && psregister != nullptr)
	{
		eXosip_lock(pectx);
		eXosip_message_send_answer(pectx, je->tid, 200, psregister);
		eXosip_unlock(pectx);
		//osip_message_free(psregister);
	}
}

void Registerfailed(struct eXosip_t * pectx, eXosip_event_t *je)
{
	int ireturncode = 0;
	osip_message_t * psregister = nullptr;
	ireturncode = eXosip_message_build_answer(pectx, je->tid, 401, &psregister);
	if (ireturncode == 0 && psregister != nullptr)
	{
		eXosip_lock(pectx);
		eXosip_message_send_answer(pectx, je->tid, 401, psregister);
		eXosip_unlock(pectx);
	}
}

void cosip::MsgProcess(SIPParams* params, struct eXosip_t * exp)
{
	char *p;
	int keepAliveFlag = 0;
	struct eXosip_t * peCtx = (struct eXosip_t *)exp;

	//监听并回复摄像机消息
	while (params->bRunning)
	{
		eXosip_event_t *je = NULL;
		//处理事件
		je = eXosip_event_wait(peCtx, 0, 4);
		if (je == NULL)
		{
			osip_usleep(100000);
			continue;
		}

		switch (je->type)
		{
		case EXOSIP_MESSAGE_NEW:				//新消息到来
		{
			m_log->CsrPrintLog("new msg method:%s\n", je->request->sip_method);
			if (MSG_IS_REGISTER(je->request))
			{
				m_log->CsrPrintLog("recv Register");
				params->nRegisterOK = 1;
			}
			else if (MSG_IS_MESSAGE(je->request))
			{
				osip_body_t *body = NULL;
				osip_message_get_body(je->request, 0, &body);
				if (body != NULL)
				{
					p = strstr(body->body, "Keepalive");
					if (p != NULL)
					{
						if (keepAliveFlag == 0)
						{
							m_log->CsrPrintLog("msg body:%s\n", body->body);
							keepAliveFlag = 1;
							params->nRegisterOK = 1;
						}
					}
					else
					{
						m_log->CsrPrintLog("msg body:%s\n", body->body);
					}
				}
				else
				{
					m_log->CsrPrintLog("get body failed");
				}
			}
			else if (strncmp(je->request->sip_method, "BYE", 4) != 0)
			{
				m_log->CsrPrintLog("unsupport new msg method : %s", je->request->sip_method);
			}
			RegisterSuccess(peCtx, je);
			break;
		}
		case EXOSIP_MESSAGE_ANSWERED:				//查询
		{
			m_log->CsrPrintLog("answered method:%s\n", je->request->sip_method);
			RegisterSuccess(peCtx, je);
			break;
		}
		case EXOSIP_CALL_ANSWERED:
		{
			osip_message_t *ack = NULL;
			params->nCallID = je->cid;
			params->nDialogID = je->did;
			m_log->CsrPrintLog("call answered method:%s, call_id:%d, dialog_id:%d\n", je->request->sip_method, params->nCallID, params->nDialogID);
			eXosip_call_build_ack(peCtx, je->did, &ack);
			eXosip_lock(peCtx);
			eXosip_call_send_ack(peCtx, je->did, ack);
			eXosip_unlock(peCtx);
			break;
		}
		case EXOSIP_CALL_PROCEEDING:
		{
			m_log->CsrPrintLog("recv EXOSIP_CALL_PROCEEDING\n");
			RegisterSuccess(peCtx, je);
			break;
		}
		case EXOSIP_CALL_REQUESTFAILURE:
		{
			m_log->CsrPrintLog("recv EXOSIP_CALL_REQUESTFAILURE\n");
			RegisterSuccess(peCtx, je);
			break;
		}
		case EXOSIP_CALL_MESSAGE_ANSWERED:
		{
			m_log->CsrPrintLog("recv EXOSIP_CALL_MESSAGE_ANSWERED\n");
			RegisterSuccess(peCtx, je);
			break;
		}
		case EXOSIP_CALL_RELEASED:         //请求视频流回复成功
		{
			m_log->CsrPrintLog("recv EXOSIP_CALL_RELEASED\n");
			RegisterSuccess(peCtx, je);
			break;
		}
		case EXOSIP_CALL_CLOSED:
			m_log->CsrPrintLog("recv EXOSIP_CALL_CLOSED\n");
			RegisterSuccess(peCtx, je);
			break;
		case EXOSIP_CALL_MESSAGE_NEW:
			m_log->CsrPrintLog("recv EXOSIP_CALL_MESSAGE_NEW\n");
			RegisterSuccess(peCtx, je);
			break;
		default:
			m_log->CsrPrintLog("##test,%s:%d, unsupport type:%d\n", __FILE__, __LINE__, je->type);
			RegisterSuccess(peCtx, je);
			break;
		}
		eXosip_event_free(je);
	}
}

//SIP消息通信线程
unsigned __stdcall cosip::SipServiceThread(void* pObj)
{
	cosip* pOwn = (cosip*)pObj;
	int nRet = 0;
	struct eXosip_t *pExp;
	SIPParams* pSipInfo = &pOwn->m_VideoInfo.SParams;

	//初始化跟踪信息
	TRACE_INITIALIZE(6, NULL);

	//初始化eXosip和osip栈
	pExp = eXosip_malloc();
	nRet = eXosip_init(pExp);
	if (nRet != OSIP_SUCCESS)
	{
		//m_log->CsrPrintLog("Init eXosip failed");
		printf("Init eXosip fialed");
		return -1;
	}
	else
	{
		printf("eXosip init success\n");
	}

	//本地消息端口不是视频流接收端口
	nRet = eXosip_listen_addr(pExp, IPPROTO_UDP, NULL, pSipInfo->nLocalSipPort, AF_INET, 0);
	if (nRet != OSIP_SUCCESS)
	{
		printf("eXosip_listner_addr error!\n");
		return -1;
	}

	pSipInfo->eXp = pExp;
	pOwn->MsgProcess(pSipInfo, pExp);

	eXosip_quit(pExp);
	osip_free(pExp);
	pExp = NULL;
	pSipInfo->eXp = NULL;

	printf("%s run over\n", __FUNCTION__);
	return 0;
}

bool cosip::InitSIPService()
{
	//消息线程句柄
	HANDLE hHandle = (HANDLE)_beginthreadex(NULL, 0, SipServiceThread, (void*)(this), 0, NULL);
	if (hHandle == INVALID_HANDLE_VALUE)
	{
		m_log->CsrPrintLog("创建消息线程失败\n");
		return false;
	}
	else
	{
		m_log->CsrPrintLog("创建消息线程成功\n");
		CloseHandle(hHandle);
	}

	int nWaittime = 20;
	while (!m_VideoInfo.SParams.nRegisterOK)
	{
		m_log->CsrPrintLog("等待相机注册%d...\n", nWaittime--);
		Sleep(1000);
		if (nWaittime == 0)
		{
			m_log->CsrPrintLog("相机注册超时\n");
			return false;
		}
	}

	//请求相机catalog信息
	if (!QueryCatalogInfo())
	{
		return false;
	}

}

//回调返回空
const char *whitespace_cb(mxml_node_t *node, int where)
{
	return NULL;
}

//请求catalog信息
bool cosip::QueryCatalogInfo()
{
	char chsn[32];			//存储sn码
	int ret;				//返回值

	mxml_node_t *tree, *query, *node;
	struct eXosip_t* pExp = m_VideoInfo.SParams.eXp;
	char *DeviceID = m_VideoInfo.SParams.chLocalSipID;

	tree = mxmlNewXML("1.0");
	if (tree != NULL)
	{
		query = mxmlNewElement(tree, "Query");
		if (query != NULL)
		{
			char buf[256] = { 0 };
			char dest_call[256], source_call[256];
			node = mxmlNewElement(query, "CmdType");
			mxmlNewText(node, 0, "Catalog");
			
			node = mxmlNewElement(query, "SN");
			_snprintf(chsn, 32, "%d", m_VideoInfo.SParams.nSN++);
			mxmlNewText(node, 0, chsn);

			node = mxmlNewElement(query, "DeviceID");
			mxmlNewText(node, 0, DeviceID);

			//将xml格式信息存入buf
			mxmlSaveString(tree, buf, 256, whitespace_cb);

			osip_message_t *msg = NULL;
			_snprintf(dest_call, 256, "sip:%s@%s:%d", m_VideoInfo.SParams.chPlatfromSipID, m_VideoInfo.SParams.chPlatfromIPAddr, m_VideoInfo.SParams.nPlatformSipPort);
			_snprintf(source_call, 256, "sip:%s@%s", m_VideoInfo.SParams.chLocalSipID, m_VideoInfo.SParams.chLocalIPAddr);
		
			//发送请求
			ret = eXosip_message_build_request(pExp, &msg, "MESSAGE", dest_call, source_call, NULL);
			if (ret == 0 && msg != NULL)
			{
				osip_message_set_body(msg, buf, strlen(buf));
				osip_message_set_content_type(msg, "Application/MAXSCDP+xml");
				eXosip_lock(pExp);
				eXosip_message_send_request(pExp, msg);
				eXosip_unlock(pExp);
				m_log->CsrPrintLog("xml:%s, dest_call:%s, source_call:%s, 0k\n", buf, dest_call, source_call);
			}
			else
			{
				m_log->CsrPrintLog("eXosip_message_build_request failed\n");
				return false;
			}
		}
		else
		{
			m_log->CsrPrintLog("mxmlNewElement Query failed");
			return false;
		}
		mxmlDelete(tree);
	}
	else
	{
		m_log->CsrPrintLog("mxmlNewXML failed\n");
		return false;
	}
	return true;
}

//请求PS流视频
void cosip::StartPSStream(int nType /*= 0*/)
{
	//请求视频并通过udp或者jrtplib接收视频
	HANDLE hGetPsHandle, hSetAliveHandle;
	CameraInfo *cp;
	for (int inx = 0; inx < m_VideoInfo.nCameraNum; inx++)
	{
		cp = m_VideoInfo.pCameraParams + inx;
		cp->nStatusErr = 0;
		cp->bRuning = 1;

		if (nType == 0)
		{
			if ((hGetPsHandle = (HANDLE)_beginthreadex(NULL, 0, jrtp_rtp_recv_thread, (void*)cp, 0, NULL)) == INVALID_HANDLE_VALUE)
			{
				m_log->CsrPrintLog("jrtplib 接受视频线程出错\n", cp->chSipID, cp->nRecvPort);
			}
			else
			{
				CloseHandle(hGetPsHandle);
			}
		}
		else
		{
			if ((hGetPsHandle = (HANDLE)_beginthreadex(NULL, 0, rtp_recv_thread, (void*)cp, 0, NULL)) == INVALID_HANDLE_VALUE)
			{
				m_log->CsrPrintLog("jrtplib 接受视频线程出错\n", cp->chSipID, cp->nRecvPort);
			}
			else
			{
				CloseHandle(hGetPsHandle);
			}
			/*	if ((hHandleAlive = (HANDLE)_beginthreadex(NULL, 0, stream_keep_alive_thread, (void*)p, 0, NULL)) == INVALID_HANDLE_VALUE) {
			APP_ERR("pthread_create stream_keep_alive_thread err, %s:%d", p->sipId, p->recvPort + 1);
			}
			else
			{
			CloseHandle(hHandleAlive);
			}*/
		}

		Sleep(1000);

		//发送请求视频信息
		if (InviteRealStream(*cp))
			m_log->CsrPrintLog("%s-%d请求视频失败\n", cp->chSipID, cp->nRecvPort);
	}
}

//请求视频流SDP
bool cosip::InviteRealStream(const CameraInfo& pParams)
{
	char dest_call[256], source_call[256], subject[128];
	osip_message_t *invite = NULL;
	int ret;
	struct eXosip_t *peCtx = m_VideoInfo.SParams.eXp;

	_snprintf(dest_call, 256, "sip:%s@%s:%d", pParams.chSipID, m_VideoInfo.SParams.chPlatfromIPAddr, m_VideoInfo.SParams.nPlatformSipPort);
	_snprintf(source_call, 256, "sip:%s@%s", m_VideoInfo.SParams.chLocalSipID, m_VideoInfo.SParams.chLocalIPAddr);
	_snprintf(subject, 128, "%s:0,%s:0", pParams.chSipID, m_VideoInfo.SParams.chLocalSipID);

	//创建初始化请求信息
	ret = eXosip_call_build_initial_invite(peCtx, &invite, dest_call, source_call, NULL, subject);
	if (ret != 0)
	{
		m_log->CsrPrintLog("eXosip_call_build_initial_invite failed, %s,%s,%s", dest_call, source_call, subject);
		return false;
	}

	//sdp
	char body[500];
	int bodyLen = _snprintf(body, 500,
		"v=0\r\n"
		"o=%s 0 0 IN IP4 %s\r\n"
		"s=Play\r\n"
		"c=IN IP4 %s\r\n"
		"t=0 0\r\n"
		"m=video %d RTP/AVP 96 97 98\r\n"
		"a=rtpmap:96 PS/90000\r\n"
		"a=rtpmap:97 MPEG4/90000\r\n"
		"a=rtpmap:98 H264/90000\r\n"
		"a=recvonly\r\n"
		"y=0100000001\n", pParams.chSipID, m_VideoInfo.SParams.chLocalIPAddr,
		m_VideoInfo.SParams.chLocalIPAddr, pParams.nRecvPort);
	osip_message_set_body(invite, body, bodyLen);
	osip_message_set_content_type(invite, "APPLICATION/SDP");
	eXosip_lock(peCtx);

	//发送请求
	eXosip_call_send_initial_invite(peCtx, invite);
	eXosip_unlock(peCtx);

	return 0;
}

//查错
void checkerror(int rtperr)
{
	if (rtperr < 0)
	{
		std::cout << "ERROR: " << RTPGetErrorString(rtperr) << std::endl;
		exit(-1);
	}
}

class MyRTPSession : public RTPSession
{
protected:
	void OnNewSource(RTPSourceData *dat)
	{
		if (dat->IsOwnSSRC())
			return;

		uint32_t ip;
		uint16_t port;

		if (dat->GetRTPDataAddress() != 0)
		{
			const RTPIPv4Address *addr = (const RTPIPv4Address *)(dat->GetRTPDataAddress());
			ip = addr->GetIP();
			port = addr->GetPort();
		}
		else if (dat->GetRTCPDataAddress() != 0)
		{
			const RTPIPv4Address *addr = (const RTPIPv4Address *)(dat->GetRTCPDataAddress());
			ip = addr->GetIP();
			port = addr->GetPort() - 1;
		}
		else
			return;

		RTPIPv4Address dest(ip, port);
		AddDestination(dest);

		struct in_addr inaddr;
		inaddr.s_addr = htonl(ip);
		std::cout << "Adding destination " << std::string(inet_ntoa(inaddr)) << ":" << port << std::endl;
	}

	void OnBYEPacket(RTPSourceData *dat)
	{
		if (dat->IsOwnSSRC())
			return;

		uint32_t ip;
		uint16_t port;

		if (dat->GetRTPDataAddress() != 0)
		{
			const RTPIPv4Address *addr = (const RTPIPv4Address *)(dat->GetRTPDataAddress());
			ip = addr->GetIP();
			port = addr->GetPort();
		}
		else if (dat->GetRTCPDataAddress() != 0)
		{
			const RTPIPv4Address *addr = (const RTPIPv4Address *)(dat->GetRTCPDataAddress());
			ip = addr->GetIP();
			port = addr->GetPort() - 1;
		}
		else
			return;

		RTPIPv4Address dest(ip, port);
		DeleteDestination(dest);

		struct in_addr inaddr;
		inaddr.s_addr = htonl(ip);
		std::cout << "Deleting destination " << std::string(inet_ntoa(inaddr)) << ":" << port << std::endl;
	}

	void OnRemoveSource(RTPSourceData *dat)
	{
		if (dat->IsOwnSSRC())
			return;
		if (dat->ReceivedBYE())
			return;

		uint32_t ip;
		uint16_t port;

		if (dat->GetRTPDataAddress() != 0)
		{
			const RTPIPv4Address *addr = (const RTPIPv4Address *)(dat->GetRTPDataAddress());
			ip = addr->GetIP();
			port = addr->GetPort();
		}
		else if (dat->GetRTCPDataAddress() != 0)
		{
			const RTPIPv4Address *addr = (const RTPIPv4Address *)(dat->GetRTCPDataAddress());
			ip = addr->GetIP();
			port = addr->GetPort() - 1;
		}
		else
			return;

		RTPIPv4Address dest(ip, port);
		DeleteDestination(dest);

		struct in_addr inaddr;
		inaddr.s_addr = htonl(ip);
		std::cout << "Deleting destination " << std::string(inet_ntoa(inaddr)) << ":" << port << std::endl;
	}
};
	
//接受视频流线程jrtplib
unsigned __stdcall cosip::jrtp_rtp_recv_thread(void* pCameraInfo)
{
	//获取相机参数
	CameraInfo *p = (CameraInfo *)pCameraInfo;
	
	#ifdef WIN32
		WSADATA dat;
		WSAStartup(MAKEWORD(2, 2), &dat);
	#endif // WIN32
	
		MyRTPSession sess;
		uint16_t portbase;
		std::string ipstr;
		int status, i, num;
	
		RTPUDPv4TransmissionParams transparams;
		RTPSessionParams sessparams;
	
		transparams.SetRTPReceiveBuffer(5*1024*1024);

		sessparams.SetOwnTimestampUnit(1.0 / 9000.0);
	
		portbase = p->nRecvPort;
	
		sessparams.SetAcceptOwnPackets(true);
		transparams.SetPortbase(portbase);
		status = sess.Create(sessparams, &transparams);
		checkerror(status);
	
		//写入视频文件
		//获取当前程序路径
		std::string strPath = GetMoudlePath();
		char filename[MAX_PATH];
		strPath += p->chSipID;
		_snprintf(filename, 128, "%s1234.264", strPath.c_str());
		p->fpH264 = fopen(filename, "wb");
		if (p->fpH264 == NULL)
		{
			printf("fopen %s failed", filename);
			return NULL;
		}

		//临时变量用来存储一帧数据
		char* h264Buf;
		h264Buf = (char*)malloc(PS_BUF_SIZE);
		memset(h264Buf, '\0', PS_BUF_SIZE);
		int h264Len = 0;
	
		//开始接收流包
		while (p->bRuning)
		{
			sess.BeginDataAccess();
	
			// check incoming packets
			if (sess.GotoFirstSourceWithData())
			{
				do
				{
					RTPPacket *pack;
	
					while ((pack = sess.GetNextPacket()) != NULL)
					{
						// You can examine the data here

						//这里开始一阵一阵的获取数据
						if (pack->HasMarker())
						{
							//fprintf(g_fp, "%x\n", pack->GetPayloadData());

							memcpy(h264Buf + h264Len, pack->GetPayloadData(), pack->GetPayloadLength());
							h264Len += pack->GetPayloadLength();
							printf("%d\n", h264Len);
							//写入文件
							fwrite(h264Buf, 1, h264Len, p->fpH264);
							timestramp tim;
							//ptrHandle->OnStreamReady(ptrHandle->pUserData, (void*)h264Buf, h264Len, 0);

							//将h264接触rgb



							h264Len = 0;
						}
						else
						{
							memcpy(h264Buf + h264Len, pack->GetPayloadData(), pack->GetPayloadLength());
							h264Len += pack->GetPayloadLength();
						}

						//写入文件
						fwrite(pack->GetPayloadData(), 1, pack->GetPayloadLength(), p->fpH264);
						sess.DeletePacket(pack);
					}
				} while (sess.GotoNextSourceWithData());
			}
	
			sess.EndDataAccess();
	
	#ifndef RTP_SUPPORT_THREAD
			status = sess.Poll();
			checkerror(status);
	#endif // RTP_SUPPORT_THREAD
	
			//RTPTime::Wait(RTPTime(0, 0));
		}
	
		sess.BYEDestroy(RTPTime(10, 0), 0, 0);
	
	#ifdef WIN32
		WSACleanup();
	#endif // WIN32
	
	//在西析构函数里关闭文件指针
	//	fclose(p->fpH264);
	//	p->fpH264 == NULL;
	
	return 0;
}

int init_udpsocket(int port, struct sockaddr_in *servaddr, char *mcast_addr)
{
	int err = -1;
	int socket_fd;

	socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (socket_fd < 0)
	{
		printf("socket failed, port:%d", port);
		return -1;
	}

	memset(servaddr, 0, sizeof(struct sockaddr_in));
	servaddr->sin_family = AF_INET;
	servaddr->sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr->sin_port = htons(port);

	err = bind(socket_fd, (struct sockaddr*)servaddr, sizeof(struct sockaddr_in));
	if (err < 0)
	{
		printf("bind failed, port:%d", port);
		return -2;
	}

	/*set enable MULTICAST LOOP */
	char loop;
	err = setsockopt(socket_fd, IPPROTO_IP, IP_MULTICAST_LOOP, &loop, sizeof(loop));
	if (err < 0)
	{
		printf("setsockopt IP_MULTICAST_LOOP failed, port:%d", port);
		return -3;
	}

	return socket_fd;
}

void release_udpsocket(int socket_fd, char *mcast_addr)
{
	closesocket(socket_fd);
}
int inline ProgramStreamPackHeader(char* Pack, int length, char **NextPack, int *leftlength)
{
	//printf("[%s]%x %x %x %x\n", __FUNCTION__, Pack[0], Pack[1], Pack[2], Pack[3]);
	//通过 00 00 01 ba头的第14个字节的最后3位来确定头部填充了多少字节
		program_stream_pack_header *PsHead = (program_stream_pack_header *)Pack;
	unsigned char pack_stuffing_length = PsHead->stuffinglen & '\x07';

	*leftlength = length - sizeof(program_stream_pack_header)-pack_stuffing_length;//减去头和填充的字节
	*NextPack = Pack + sizeof(program_stream_pack_header)+pack_stuffing_length;
	if (*leftlength<4)
		return 0;

	return *leftlength;
}

inline int ProgramStreamMap(char* Pack, int length, char **NextPack, int *leftlength, char **PayloadData, int *PayloadDataLen)
{
	program_stream_map* PSMPack = (program_stream_map*)Pack;

	//no payload
	*PayloadData = 0;
	*PayloadDataLen = 0;

	if ((unsigned int)length < sizeof(program_stream_map)) return 0;

	littel_endian_size psm_length;
	psm_length.byte[0] = PSMPack->PackLength.byte[1];
	psm_length.byte[1] = PSMPack->PackLength.byte[0];

	*leftlength = length - psm_length.length - sizeof(program_stream_map);
	if (*leftlength <= 0) return 0;

	*NextPack = Pack + psm_length.length + sizeof(program_stream_map);

	return *leftlength;
}

inline int ProgramShHead(char* Pack, int length, char **NextPack, int *leftlength, char **PayloadData, int *PayloadDataLen)
{
	program_stream_map* PSMPack = (program_stream_map*)Pack;

	//no payload
	*PayloadData = 0;
	*PayloadDataLen = 0;

	if ((unsigned int)length < sizeof(program_stream_map)) return 0;

	littel_endian_size psm_length;
	psm_length.byte[0] = PSMPack->PackLength.byte[1];
	psm_length.byte[1] = PSMPack->PackLength.byte[0];

	*leftlength = length - psm_length.length - sizeof(program_stream_map);
	if (*leftlength <= 0)
		return 0;

	*NextPack = Pack + psm_length.length + sizeof(program_stream_map);

	return *leftlength;
}


inline int Pes(char* Pack, int length, char **NextPack, int *leftlength, char **PayloadData, int *PayloadDataLen)
{
	program_stream_e* PSEPack = (program_stream_e*)Pack;

	*PayloadData = 0;
	*PayloadDataLen = 0;

	if ((unsigned int)length < sizeof(program_stream_e)) return 0;

	littel_endian_size pse_length;
	pse_length.byte[0] = PSEPack->PackLength.byte[1];
	pse_length.byte[1] = PSEPack->PackLength.byte[0];

	*PayloadDataLen = pse_length.length - 2 - 1 - PSEPack->stuffing_length;
	if (*PayloadDataLen>0)
		*PayloadData = Pack + sizeof(program_stream_e)+PSEPack->stuffing_length;

	*leftlength = length - pse_length.length - sizeof(pack_start_code)-sizeof(littel_endian_size);
	if (*leftlength <= 0) return 0;

	*NextPack = Pack + sizeof(pack_start_code)+sizeof(littel_endian_size)+pse_length.length;

	return *leftlength;
}

int inline GetH246FromPs(char* buffer, int length, char *h264Buffer, int *h264length, char *sipId)
{
	int leftlength = 0;
	char *NextPack = 0;

	*h264length = 0;

	if (ProgramStreamPackHeader(buffer, length, &NextPack, &leftlength) == 0)
		return 0;

	char *PayloadData = NULL;
	int PayloadDataLen = 0;

	while ((unsigned int)leftlength >= sizeof(pack_start_code))
	{
		PayloadData = NULL;
		PayloadDataLen = 0;

		if (NextPack
			&& NextPack[0] == '\x00'
			&& NextPack[1] == '\x00'
			&& NextPack[2] == '\x01'
			&& NextPack[3] == '\xE0')
		{
			//接着就是流包，说明是非i帧
			if (Pes(NextPack, leftlength, &NextPack, &leftlength, &PayloadData, &PayloadDataLen))
			{
				if (PayloadDataLen)
				{
					if (PayloadDataLen + *h264length < H264_FRAME_SIZE_MAX)
					{
						memcpy(h264Buffer, PayloadData, PayloadDataLen);
						h264Buffer += PayloadDataLen;
						*h264length += PayloadDataLen;
					}
					else
					{
						printf("h264 frame size exception!! %d:%d", PayloadDataLen, *h264length);
					}
				}
			}
			else
			{
				if (PayloadDataLen)
				{
					if (PayloadDataLen + *h264length < H264_FRAME_SIZE_MAX)
					{
						memcpy(h264Buffer, PayloadData, PayloadDataLen);
						h264Buffer += PayloadDataLen;
						*h264length += PayloadDataLen;
					}
					else
					{
						printf("h264 frame size exception!! %d:%d", PayloadDataLen, *h264length);
					}
				}
				break;
			}
		}
		else if (NextPack
			&& NextPack[0] == '\x00'
			&& NextPack[1] == '\x00'
			&& NextPack[2] == '\x01'
			&& NextPack[3] == '\xBB')
		{
			if (ProgramShHead(NextPack, leftlength, &NextPack, &leftlength, &PayloadData, &PayloadDataLen) == 0)
				break;
		}
		else if (NextPack
			&& NextPack[0] == '\x00'
			&& NextPack[1] == '\x00'
			&& NextPack[2] == '\x01'
			&& NextPack[3] == '\xBC')
		{
			if (ProgramStreamMap(NextPack, leftlength, &NextPack, &leftlength, &PayloadData, &PayloadDataLen) == 0)
				break;
		}
		else if (NextPack
			&& NextPack[0] == '\x00'
			&& NextPack[1] == '\x00'
			&& NextPack[2] == '\x01'
			&& (NextPack[3] == '\xC0' || NextPack[3] == '\xBD'))
		{
			printf("audio ps frame, skip it\n");
			break;
		}
		else
		{
			printf("[%s]no know %x %x %x %x\n", sipId, NextPack[0], NextPack[1], NextPack[2], NextPack[3]);
			break;
		}
	}

	return *h264length;
}

//呼叫保持
unsigned __stdcall cosip::stream_keep_alive_thread(void *pCameraInfo)
{
	int socket_fd;
	CameraInfo *p = (CameraInfo *)pCameraInfo;
	int rtcp_port = p->nRecvPort + 1;
	struct sockaddr_in servaddr;
	//struct timeval tv;

	SYSTEMTIME st;

	socket_fd = init_udpsocket(rtcp_port, &servaddr, NULL);
	if (socket_fd >= 0)
	{
		printf("start socket port %d success\n", rtcp_port);
	}

	char *buf = (char *)malloc(1024);
	if (buf == NULL)
	{
		//APP_ERR("malloc failed buf");
		return NULL;
	}
	int recvLen;
	int addr_len = sizeof(struct sockaddr_in);

	//APP_DEBUG("%s:%d starting ...", p->sipId, rtcp_port);

	memset(buf, 0, 1024);
	while (p->bRuning)
	{
		recvLen = recvfrom(socket_fd, buf, 1024, 0, (struct sockaddr*)&servaddr, (int*)&addr_len);
		if (recvLen > 0)
		{
			printf("stream_keep_alive_thread, rtcp_port %d, recv %d bytes\n", rtcp_port, recvLen);
			recvLen = sendto(socket_fd, buf, recvLen, 0, (struct sockaddr*)&servaddr, sizeof(struct sockaddr_in));
			if (recvLen <= 0)
			{
				printf("sendto %d failed", rtcp_port);
			}
		}
		else
		{
			perror("recvfrom() alive");
		}
		//gettimeofday(&tv, NULL);
		GetLocalTime(&st);
	}

	release_udpsocket(socket_fd, NULL);
	if (buf != NULL)
	{
		free(buf);
	}

	printf("%s:%d run over", p->chSipID, rtcp_port);

	return NULL;
}

//接收视频流线程udp
unsigned __stdcall cosip::rtp_recv_thread(void* pCameraInfo)
{
	int socket_fd;
	CameraInfo *p = (CameraInfo *)pCameraInfo;
	int rtp_port = p->nRecvPort;
	struct sockaddr_in servaddr;

	socket_fd = init_udpsocket(rtp_port, &servaddr, NULL);
	if (socket_fd >= 0)
	{
		printf("start socket port %d success\n", rtp_port);
	}

	char *buf = (char *)malloc(RTP_MAXBUF);
	if (buf == NULL)
	{
		printf("malloc failed buf");
		return NULL;
	}
	char *psBuf = (char *)malloc(PS_BUF_SIZE);
	if (psBuf == NULL)
	{
		printf("malloc failed");
		return NULL;
	}
	memset(psBuf, '\0', PS_BUF_SIZE);
	char *h264buf = (char *)malloc(H264_FRAME_SIZE_MAX);
	if (h264buf == NULL)
	{
		printf("malloc failed");
		return NULL;
	}
	int recvLen;
	int addr_len = sizeof(struct sockaddr_in);
	int rtpHeadLen = sizeof(RTP_HEADER);

	//写入视频文件
	//获取当前程序路径
	std::string strPath = GetMoudlePath();
	char filename[MAX_PATH];
	strPath += p->chSipID;
	_snprintf(filename, 128, "%s1234.264", strPath.c_str());
	p->fpH264 = fopen(filename, "wb");
	if (p->fpH264 == NULL)
	{
		printf("fopen %s failed", filename);
		return NULL;
	}

	printf("%s:%d starting ...", p->chSipID, p->nRecvPort);

	int cnt = 0;
	int rtpPsLen, h264length, psLen = 0;
	char *ptr;
	memset(buf, 0, RTP_MAXBUF);

	ptr = (char*)malloc(PS_BUF_SIZE);
	memset(ptr, 0, PS_BUF_SIZE);
	int ntotal = 0;
	while (p->bRuning)
	{
		//接收到的rtp流数据长度
		recvLen = recvfrom(socket_fd, buf, RTP_MAXBUF, 0, (struct sockaddr*)&servaddr, (int*)&addr_len);

		//如果接收到字字段长度还没有rtp数据头长，就直接将数据舍弃
		if (recvLen > rtpHeadLen)
		{
			unsigned char *buffer = (unsigned char *)buf;

			//写入文件
				/*fwrite(buffer, 1, recvLen, p->fpH264);

				if (buffer[0] == 0x00 && buffer[0 + 1] == 0x00 && buffer[0 + 2] == 0x01 && buffer[0 + 3] == 0xBA)
				{
				printf("符合要求的数据%x,,%x,,%x,,%x", buffer[0], buffer[1], buffer[2], buffer[3]);
				}*/
			fprintf(g_fp, "符合要求的数据---------------------%x,,%x,,%x,,%x\n", buffer[0], buffer[1], buffer[2], buffer[3]);
			ptr = psBuf + psLen;			//最新数据的头
			rtpPsLen = recvLen - rtpHeadLen;
#if 1
			if (psLen + rtpPsLen < PS_BUF_SIZE)
			{
				memcpy(ptr, buf + rtpHeadLen, rtpPsLen);
			}
			else
			{
				printf("psBuf memory overflow, %d\n", psLen + rtpPsLen);
				psLen = 0;
				continue;
			}
#endif
			//打印视频流
			printf("符合要求的数据%x,,%x,,%x,,%x\n", ptr[0], ptr[1], ptr[2], ptr[3]);
			//视频流解析
			if (/*(*//*(*/ptr[0] == 0x00 && ptr[1] == 0x00 && ptr[2] == 0x01 && ptr[3] == 0xffffffBA/*) *//*|| (ptr[0] == 0x00 && ptr[1] == 0x00 && ptr[2] == 0x01 && ptr[3] == 0xBA)*//*) */ && psLen > 0)
			{
				if (cnt % 10000 == 0)
				{
					printf("rtpRecvPort:%d, cnt:%d, pssize:%d\n", rtp_port, cnt++, psLen);
				}
				if (cnt % 25 == 0)
				{
					p->nStatus = 1;
				}
				GetH246FromPs((char *)psBuf, psLen, h264buf, &h264length, p->chSipID);			//如果
				if (h264length > 0)
				{
					//写入文件
					//fwrite(h264buf, 1, h264length, p->fpH264);

					//拷贝到指定内存
					memcpy(p->h264buffer, h264buf, h264length);
					
					//char chBufShow[1024 * 1024 * 4 + 10] = "";
					//sprintf(chBufShow, "ffplay %s", h264buf);
					//system(chBufShow);
				}
				memcpy(psBuf, ptr, rtpPsLen);
				psLen = 0;
				cnt++;
			}
			/*else if (psLen > 0)
			{
			memcpy(psBuf + psLen, ptr, rtpPsLen);
			}*/
			psLen += rtpPsLen;
		}
		else
		{
			perror("recvfrom() long");
		}

		if (recvLen > 1500)
		{
			printf("udp frame exception, %d\n", recvLen);
		}
	}

	release_udpsocket(socket_fd, NULL);
	if (buf != NULL)
	{
		free(buf);
	}
	if (psBuf != NULL)
	{
		free(psBuf);
	}
	if (h264buf != NULL)
	{
		free(h264buf);
	}
	if (p->fpH264 != NULL)
	{
		fclose(p->fpH264);
		p->fpH264 = NULL;
		fclose(g_fp);
		g_fp = NULL;
	}

	printf("%s:%d run over", p->chSipID, p->nRecvPort);

	return 0;
}

int sendPlayBye(SIPParams *p28181Params)
{
	struct eXosip_t *peCtx = p28181Params->eXp;

	eXosip_lock(peCtx);
	eXosip_call_terminate(peCtx, p28181Params->nCallID, p28181Params->nDialogID);
	eXosip_unlock(peCtx);
	return 0;
}


int cosip::stopCameraRealStream(VideoStreamParams *pVideoInfo)
{
	int i, tryCnt;
	CameraInfo *p;
	SIPParams *pSp = &(pVideoInfo->SParams);

	for (i = 0; i < pVideoInfo->nCameraNum; i++)
	{
		p = pVideoInfo->pCameraParams + i;
		pSp->nCallID = -1;
		InviteRealStream(*p);
		tryCnt = 10;
		while (tryCnt-- > 0)
		{
			if (pSp->nCallID != -1)
			{
				break;
			}
			Sleep(1000);
		}
		if (pSp->nCallID == -1)
		{
			printf("exception wait call_id:%d, %s", pSp->nCallID, p->chSipID);
		}
		sendPlayBye(pSp);

		p->bRuning = 0;
	}

	return 0;
}

static int stopStreamRecv(VideoStreamParams *pliveVideoParams)
{
	int i;
	CameraInfo *p;

	for (i = 0; i < pliveVideoParams->nCameraNum; i++)
	{
		p = pliveVideoParams->pCameraParams + i;
		p->bRuning = 0;
	}

	return 0;
}

//释放
bool cosip::ReleaseService()
{
	m_VideoInfo.bRunning = 0;
	stopCameraRealStream(&m_VideoInfo);
	Sleep(300);
	stopStreamRecv(&m_VideoInfo);
	m_VideoInfo.SParams.bRunning = 0;
	if (m_VideoInfo.pCameraParams->fpH264 != NULL)
	{
		fclose(m_VideoInfo.pCameraParams->fpH264);
		m_VideoInfo.pCameraParams->fpH264 = NULL;
	}
	m_log->ReleaseCsrLog();
	Sleep(1000);
	return false;
}