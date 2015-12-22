
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _AGENT_SERVER_TASK_H_
#define _AGENT_SERVER_TASK_H_

#include <arpa/inet.h>

#include "wType.h"
#include "wTcpTask.h"
#include "wLog.h"
#include "RtblCommand.h"

class AgentServerTask : public wTcpTask
{
	public:
		AgentServerTask() {}
		~AgentServerTask();
		AgentServerTask(wSocket *pSocket);
		
		virtual int VerifyConn();
		virtual int Verify();

		virtual int HandleRecvMessage(char * pBuffer, int nLen);
		
		int ParseRecvMessage(struct wCommand* pCommand ,char *pBuffer,int iLen);
};

#endif
