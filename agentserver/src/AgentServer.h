
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _AGENT_SERVER_H_
#define _AGENT_SERVER_H_

#include <arpa/inet.h>
#include <vector>

#include "wType.h"
#include "wTimer.h"
#include "wSingleton.h"

#include "wTcpServer.h"
#include "wTcpTask.h"

#include "AgentServerTask.h"
#include "wMTcpClient.h"
#include "wTcpClient.h"

#define ROUTER_SERVER_TYPE 1

class AgentServer: public wTcpServer<AgentServer>
{
	public:
		AgentServer();
		virtual ~AgentServer();
		
		virtual void Initialize();
		
		virtual void PrepareRun();
		
		virtual void Run();
		
		virtual wTcpTask* NewTcpTask(wSocket *pSocket);

		void ConnectRouter();

		void CheckTimer();
		void CheckTimeout();
		
		wMTcpClient<AgentServerTask>* RouterConn()
		{
			return mRouterConn;
		}
	private:
		wMTcpClient<AgentServerTask> *mRouterConn;	//连接router
		
		//客户端检测计时器
		wTimer mClientCheckTimer;
};

#endif
