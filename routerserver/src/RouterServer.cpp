
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#include <errno.h>
#include <time.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "wMisc.h"
#include "wLog.h"

#include "RouterConfig.h"
#include "RouterServer.h"
#include "RouterServerTask.h"

RouterServer::RouterServer():wTcpServer<RouterServer>("路由服务器")
{
	if(mStatus == SERVER_STATUS_INIT)
	{
		Initialize();
	}
}

RouterServer::~RouterServer() 
{
	//...
}

void RouterServer::Initialize()
{
	//...
}

wTcpTask* RouterServer::NewTcpTask(wSocket *pSocket)
{
	wTcpTask *pTcpTask = new RouterServerTask(pSocket);
	return pTcpTask;
}

//准备工作
void RouterServer::PrepareRun()
{
	//...
}

void RouterServer::Run()
{
	CheckModSvr();
}

//检测配置文件是否修改
void RouterServer::CheckModSvr()
{
	RouterConfig *pConfig = RouterConfig::Instance();
	if (pConfig->IsModTime())
	{
		SvrResSync_t stSvr;
		stSvr.mCode = 0;
		stSvr.mNum = pConfig->GetModSvr(stSvr.mSvr);	//SvrNet_t
		if (stSvr.mNum > 0)
		{
			cout << "broadcast new svr" << endl;
			Broadcast((char *)&stSvr, sizeof(stSvr));	//广播所有agent
		}
	}
}
