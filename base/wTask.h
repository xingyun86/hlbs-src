
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#ifndef _W_TASK_H_
#define _W_TASK_H_

#include "wCore.h"
#include "wCommand.h"
#include "wIO.h"
#include "wLog.h"
#include "wMisc.h"
#include "wNoncopyable.h"

class wTask : private wNoncopyable
{
	public:
		wTask();
		wTask(wIO *pIO);
		void Initialize();
		virtual ~wTask();
		
		wIO *IO() { return mIO; }
		void DeleteIO();
		TASK_STATUS &Status() { return mStatus; }
		bool IsRunning() { return mStatus == TASK_RUNNING; }
		
		virtual void CloseTask(int iReason);	//iReason关闭原因
		virtual int VerifyConn() { return 0;}	//验证接收到连接
		virtual int Verify() {return 0;}		//发送连接验证请求
		
		virtual int Heartbeat();
		virtual int HeartbeatOutTimes() { return mHeartbeatTimes > KEEPALIVE_CNT; }
		virtual int ClearbeatOutTimes() { return mHeartbeatTimes = 0; }
		/**
		 *  处理接受到数据
		 *  每条消息大小[2b,128k)
		 *  核心逻辑：接受整条消息，然后进入用户定义的业务函数HandleRecvMessage
		 *  return ：<0 对端发生错误|消息超长 =0 对端关闭(FIN_WAIT1) >0 接受字符
		 */
		virtual int TaskRecv();
		/**
		 *  异步发送客户端消息
		 *  return 
		 *  -1 ：消息长度不合法
		 *  -2 ：发送缓冲剩余空间不足，请稍后重试
		 *   0 : 发送成功
		 */
		virtual int SendToBuf(const char *pCmd, int iLen);
		/**
		 * 发送缓冲区有数据
		 */
		int WritableLen() { return mSendWrite - mSendBytes; }
		virtual int TaskSend();
		
		/**
		 *  同步发送确切长度消息
		 */
		int SyncSend(const char *pCmd, int iLen);
		/**
		 *  同步接受确切长度消息(需保证此sock未加入epoll中，防止出现竞争！！)
		 *  确保pCmd有足够长的空间接受自此同步消息
		 */
		int SyncRecv(char *pCmd, int iLen, int iTimeout = 10/*s*/);
		
		//业务逻辑入口函数
		virtual int HandleRecvMessage(char * pBuffer, int nLen) {}
		
	protected:
		wIO	*mIO;
		TASK_STATUS mStatus;
		int mHeartbeatTimes;
		
		//接收消息的缓冲区 32M
		int mRecvBytes;	//接收的字节数
		char mRecvMsgBuff[MAX_RECV_BUFFER_LEN];	
		
		//发送消息时的临时缓冲区 32M
		int mSendBytes;						//已发送字节数（发送线程更新）
		int mSendWrite;						//发送缓冲被写入字节数（写入线程更新）
		char mSendMsgBuff[MAX_SEND_BUFFER_LEN];
		
		char mTmpSendMsgBuff[MAX_CLIENT_MSG_LEN + sizeof(int)];	//同步发送，临时缓冲区
		char mTmpRecvMsgBuff[MAX_CLIENT_MSG_LEN + sizeof(int)];	//同步接受，临时缓冲区
};

#endif

