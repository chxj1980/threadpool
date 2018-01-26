#ifndef __C_OSIP_H_
#define __C_OSIP_H_

#include <iostream>
//#include <windows.h> //与winsock2冲突
#include <eXosip2\eXosip.h>

#define MAX_PATH1 256

//定义宏，相关数据流大小
#define CAMERA_SUPPORT_MAX		500				//最大支持多少路相机
#define RTP_MAXBUF				4096			//udp接收每个数据段大小
#define PS_BUF_SIZE				(1024*1024*4)	//PS包最大缓存值
#define H264_FRAME_SIZE_MAX		(1024*024*2)	//每帧H264流的大小

//sip协议相关信息结构体
typedef struct _SIPParams{
	char chPlatfromSipID[MAX_PATH1];				//SIP服务器ID
	char chPlatfromIPAddr[MAX_PATH1];			//SIP服务器IP
	int  nPlatformSipPort;						//SIP服务器端口
	char chLocalSipID[MAX_PATH1];				//SIP用户ID
	char chLocalIPAddr[MAX_PATH1];				//SIP用户IP
	int  nLocalSipPort;							//SIP用户端口
	int	 nSN;									//SN码
	struct eXosip_t *eXp;						//实现的SIP协议库结构体
	int  nCallID;								//
	int  nDialogID;								//会话ID
	int  nRegisterOK;							//注册是否成功
	bool bRunning;								//是否正在运行
}SIPParams;

//相机相关信息
typedef struct _CameraInfo{
	char chSipID[MAX_PATH1];						//相机ID
	char chUserName[MAX_PATH1];					//用户名称（没用）
	char chUserPwd[MAX_PATH1];					//用户密码（没用）
	int  nRecvPort;								//接收视频端口
	int  nStatus;								//相机状态
	int  nStatusErr;							//错误码
	FILE *fpH264;								//存储H264视频文件指针
	FILE *fpLog;								//日志文件指针
	bool bRuning;								//相机是否正在运行
}CameraInfo;

//视频流参数
typedef struct _VideoStreamParams{
	int        nCameraNum;						//相机数量
	CameraInfo *pCameraParams;					//相机参数
	SIPParams  SParams;                       //SIP参数
	int		   nStreamType;                  
	bool       bRunning;                        //运行状态
}VideoStreamParams;

#ifndef uint16_t
typedef unsigned short uint16_t;
#endif
#ifndef uint32_t
typedef unsigned int uint32_t;
#endif

//视频流相关信息结构体  (如果用jrtplib库接收下边几个结构体就不许要了)
typedef struct _RTP_HEADER{
	uint16_t cc : 4;
	uint16_t extbit : 1;
	uint16_t padbit : 1;
	uint16_t version : 2;
	uint16_t paytype : 7;  //负载类型
	uint16_t markbit : 1;  //1表示前面的包为一个解码单元,0表示当前解码单元未结束
	uint16_t seq_number;  //序号
	uint32_t timestamp; //时间戳
	uint32_t ssrc;  //循环校验码
	//uint32_t csrc[16];
}RTP_HEADER;

typedef union littel_endian_size_s {
	unsigned short int	length;
	unsigned char		byte[2];
} littel_endian_size;

typedef struct pack_start_code_s {
	unsigned char start_code[3];
	unsigned char stream_id[1];
} pack_start_code;
typedef struct program_stream_pack_header_s {
	pack_start_code PackStart;// 4
	unsigned char Buf[9];
	unsigned char stuffinglen;
} program_stream_pack_header;

typedef struct program_stream_map_s {
	pack_start_code PackStart;
	littel_endian_size PackLength;//we mast do exchange
} program_stream_map;

typedef struct program_stream_e_s {
	pack_start_code		PackStart;
	littel_endian_size	PackLength;//we mast do exchange
	char				PackInfo1[2];
	unsigned char		stuffing_length;
} program_stream_e;


class csrlog;


//实现sip协议类，方便调用
namespace sip{

	class cosip{
	public:
		//构造函数
		cosip(std::string iniFilePath);
		~cosip();
		
	private:
		//加载配置文件
		bool LoadIniFilePath(std::string iniFilePath);

		//初始化
		bool InitSIPService();

		//SIP消息通信线程
		unsigned static __stdcall SipServiceThread(void* pSIPParams);

		//接受视频流线程jrtplib
		unsigned static __stdcall jrtp_rtp_recv_thread(void* pCameraInfo);

		//接收视频流线程udp
		unsigned static __stdcall rtp_recv_thread(void* pCameraInfo);

		//请求catalog信息
		bool QueryCatalogInfo();

		//消息交互函数
		void MsgProcess(SIPParams* params, struct eXosip_t * exp);

		//停止播放视频流
		int stopCameraRealStream(VideoStreamParams *pVideoInfo);


		//启动接受流视频,选择接受视频的方式0:jrtplib, 1:udp默认选用jrtplib接收视频
		void StartPSStream(int nType = 0);

		//请求视频流
		bool InviteRealStream(const CameraInfo & pParams);
	public:
		//接收视频
		void GetH264Stream();
	private:
		//释放
		bool ReleaseService();

	private:
		VideoStreamParams m_VideoInfo;				//视频信息参数
		csrlog *m_log;								//日志文件
	};
}




#endif