#ifndef __C_OSIP_H_
#define __C_OSIP_H_

#include <iostream>
//#include <windows.h> //��winsock2��ͻ
#include <eXosip2\eXosip.h>

#define MAX_PATH1 256

//����꣬�����������С
#define CAMERA_SUPPORT_MAX		500				//���֧�ֶ���·���
#define RTP_MAXBUF				4096			//udp����ÿ�����ݶδ�С
#define PS_BUF_SIZE				(1024*1024*4)	//PS����󻺴�ֵ
#define H264_FRAME_SIZE_MAX		(1024*024*2)	//ÿ֡H264���Ĵ�С

//sipЭ�������Ϣ�ṹ��
typedef struct _SIPParams{
	char chPlatfromSipID[MAX_PATH1];				//SIP������ID
	char chPlatfromIPAddr[MAX_PATH1];			//SIP������IP
	int  nPlatformSipPort;						//SIP�������˿�
	char chLocalSipID[MAX_PATH1];				//SIP�û�ID
	char chLocalIPAddr[MAX_PATH1];				//SIP�û�IP
	int  nLocalSipPort;							//SIP�û��˿�
	int	 nSN;									//SN��
	struct eXosip_t *eXp;						//ʵ�ֵ�SIPЭ���ṹ��
	int  nCallID;								//
	int  nDialogID;								//�ỰID
	int  nRegisterOK;							//ע���Ƿ�ɹ�
	bool bRunning;								//�Ƿ���������
}SIPParams;

//��������Ϣ
typedef struct _CameraInfo{
	char chSipID[MAX_PATH1];						//���ID
	char chUserName[MAX_PATH1];					//�û����ƣ�û�ã�
	char chUserPwd[MAX_PATH1];					//�û����루û�ã�
	int  nRecvPort;								//������Ƶ�˿�
	int  nStatus;								//���״̬
	int  nStatusErr;							//������
	FILE *fpH264;								//�洢H264��Ƶ�ļ�ָ��
	FILE *fpLog;								//��־�ļ�ָ��
	bool bRuning;								//����Ƿ���������
}CameraInfo;

//��Ƶ������
typedef struct _VideoStreamParams{
	int        nCameraNum;						//�������
	CameraInfo *pCameraParams;					//�������
	SIPParams  SParams;                       //SIP����
	int		   nStreamType;                  
	bool       bRunning;                        //����״̬
}VideoStreamParams;

#ifndef uint16_t
typedef unsigned short uint16_t;
#endif
#ifndef uint32_t
typedef unsigned int uint32_t;
#endif

//��Ƶ�������Ϣ�ṹ��  (�����jrtplib������±߼����ṹ��Ͳ���Ҫ��)
typedef struct _RTP_HEADER{
	uint16_t cc : 4;
	uint16_t extbit : 1;
	uint16_t padbit : 1;
	uint16_t version : 2;
	uint16_t paytype : 7;  //��������
	uint16_t markbit : 1;  //1��ʾǰ��İ�Ϊһ�����뵥Ԫ,0��ʾ��ǰ���뵥Ԫδ����
	uint16_t seq_number;  //���
	uint32_t timestamp; //ʱ���
	uint32_t ssrc;  //ѭ��У����
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


//ʵ��sipЭ���࣬�������
namespace sip{

	class cosip{
	public:
		//���캯��
		cosip(std::string iniFilePath);
		~cosip();
		
	private:
		//���������ļ�
		bool LoadIniFilePath(std::string iniFilePath);

		//��ʼ��
		bool InitSIPService();

		//SIP��Ϣͨ���߳�
		unsigned static __stdcall SipServiceThread(void* pSIPParams);

		//������Ƶ���߳�jrtplib
		unsigned static __stdcall jrtp_rtp_recv_thread(void* pCameraInfo);

		//������Ƶ���߳�udp
		unsigned static __stdcall rtp_recv_thread(void* pCameraInfo);

		//����catalog��Ϣ
		bool QueryCatalogInfo();

		//��Ϣ��������
		void MsgProcess(SIPParams* params, struct eXosip_t * exp);

		//ֹͣ������Ƶ��
		int stopCameraRealStream(VideoStreamParams *pVideoInfo);


		//������������Ƶ,ѡ�������Ƶ�ķ�ʽ0:jrtplib, 1:udpĬ��ѡ��jrtplib������Ƶ
		void StartPSStream(int nType = 0);

		//������Ƶ��
		bool InviteRealStream(const CameraInfo & pParams);
	public:
		//������Ƶ
		void GetH264Stream();
	private:
		//�ͷ�
		bool ReleaseService();

	private:
		VideoStreamParams m_VideoInfo;				//��Ƶ��Ϣ����
		csrlog *m_log;								//��־�ļ�
	};
}




#endif