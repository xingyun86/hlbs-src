
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _W_TCP_TASK_H_
#define _W_TCP_TASK_H_

#include <string>
#include <arpa/inet.h>

#include "wType.h"
#include "wSocket.h"

class wTcpTask
{
	public:
		wTcpTask();
		wTcpTask(wSocket *pSocket);
		virtual ~wTcpTask();

		virtual int VerifyConn();	//验证接收到连接
		virtual int Verify();		//发送连接验证请求
		
		void Initialize();
		
		virtual void CloseTcpTask(int eReason);
		
		/**
		 *  接受消息
		 */
		int ListeningRecv();
		
		/**
		 *  基于事件发送消息
		 */
		int ListeningSend();
		int WriteToSendBuf(const char *pCmd, int iLen);
		
		int SyncSend(const char *pCmd, int iLen);
		int SyncRecv(char *pCmd, int iLen);
		
		virtual int HandleRecvMessage(char * pBuffer, int nLen) {} //业务逻辑入口函数
		
		wSocket * Socket() { return mSocket; }
		
		int Heartbeat();
		
		int HeartbeatOutTimes()
		{
			return mHeartbeatTimes > 30;
		}
		
	protected:
		wSocket *mSocket;
		
		int mHeartbeatTimes;
		//接收消息的缓冲区 100k
		int mRecvBytes;						//接收的字节数
		char mRecvMsgBuff[MAX_RECV_BUFFER_LEN];	
		
		//发送消息时的临时缓冲区 1M
		int mSendBytes;						//已发送字节数（发送线程更新）
		int mSendWrite;						//发送缓冲被写入字节数（写入线程更新）
		char mSendMsgBuff[MAX_SEND_BUFFER_LEN];
		
		char mTmpSendMsgBuff[MAX_CLIENT_MSG_LEN + sizeof(int)];	//同步发送，临时缓冲区
		char mTmpRecvMsgBuff[MAX_CLIENT_MSG_LEN + sizeof(int)];	//同步接受，临时缓冲区
};

#endif
