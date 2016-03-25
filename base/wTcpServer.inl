
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

template <typename T>
wTcpServer<T>::wTcpServer(string ServerName)
{
	Initialize();
	
	mStatus = SERVER_INIT;
	mServerName = ServerName;
}

template <typename T>
wTcpServer<T>::~wTcpServer()
{
	Final();
}

template <typename T>
void wTcpServer<T>::Initialize()
{
	mTimeout = 10;
	mServerName = "";

	mLastTicker = GetTickCount();
	mCheckTimer = wTimer(KEEPALIVE_TIME);
	mIsCheckTimer = true;

	mEpollFD = FD_UNKNOWN;
	memset((void *)&mEpollEvent, 0, sizeof(mEpollEvent));
	mTaskCount = 0;
	mEpollEventPool.reserve(LISTEN_BACKLOG);	//容量
	mTask = NULL;

	mListenSock = NULL;
	mChannelSock = NULL;
	mWorker = NULL;
	mExiting = 0;
}

template <typename T>
void wTcpServer<T>::Final()
{
	CleanEpoll();
	CleanTaskPool();
}

template <typename T>
void wTcpServer<T>::CleanEpoll()
{
	if(mEpollFD != -1)
	{
		close(mEpollFD);
	}
	mEpollFD = -1;
	memset((void *)&mEpollEvent, 0, sizeof(mEpollEvent));
	mEpollEventPool.clear();
}

template <typename T>
void wTcpServer<T>::CleanTaskPool()
{
	if(mTaskPool.size() > 0)
	{
		vector<wTask*>::iterator iter;
		for(iter = mTaskPool.begin(); iter != mTaskPool.end(); iter++)
		{
			SAFE_DELETE(*iter);
		}
	}
	mTaskPool.clear();
	mTaskCount = 0;
}

template <typename T>
void wTcpServer<T>::PrepareMaster(string sIpAddr, unsigned int nPort)
{
	LOG_DEBUG(ELOG_KEY, "[startup] listen socket on ip(%s) port(%d)", sIpAddr.c_str(), nPort);
	
	//初始化Listen Socket
	if(InitListen(sIpAddr ,nPort) < 0)
	{
		exit(2);
	}

	//运行前工作
	PrepareRun();
}

template <typename T>
void wTcpServer<T>::WorkerStart(wWorker *pWorker, bool bDaemon)
{
	mStatus = SERVER_RUNNING;
	LOG_DEBUG(ELOG_KEY, "[startup] Master start succeed");
	
	//初始化epoll
	if(InitEpoll() < 0)
	{
		exit(2);
	}
	
	//Listen Socket 添加到epoll中
	if (mListenSock == NULL)
	{
		exit(2);
	}
	mTask = NewTcpTask(mListenSock);
	if(NULL != mTask)
	{
		mTask->Status() = TASK_RUNNING;
		if (AddToEpoll(mTask) >= 0)
		{
			AddToTaskPool(mTask);
		}
	}

	//Unix Socket 添加到epoll中（worker自身channel[1]被监听）
	if(pWorker != NULL)
	{
		mWorker = pWorker;
		
		if(mWorker->mWorkerPool != NULL)
		{
			mChannelSock = &mWorker->mWorkerPool[mWorker->mSlot]->mCh;	//当前worker进程表项
			if(mChannelSock != NULL)
			{
				//new unix task
				mChannelSock->IOFlag() = FLAG_RECV;
				mChannelSock->SockStatus() = STATUS_LISTEN;
				
				mTask = NewChannelTask(mChannelSock);
				if(NULL != mTask)
				{
					mTask->Status() = TASK_RUNNING;
					if (AddToEpoll(mTask) >= 0)
					{
						AddToTaskPool(mTask);
					}
				}
			}
			else
			{
				LOG_ERROR(ELOG_KEY, "[startup] worker pool slot(%d) illegal", mWorker->mSlot);
				exit(2);
			}
		}
	}
	
	//进入服务主循环
	do {
		Recv();
		Run();
		HandleSignal();
		if(mIsCheckTimer) CheckTimer();
	} while(IsRunning() && bDaemon);
}

template <typename T>
void wTcpServer<T>::HandleSignal()
{
	if(mExiting)
	{
		WorkerExit();
	}
	
	if(g_terminate)
	{
		LOG_ERROR(ELOG_KEY, "[rumtime] worker exiting");
		WorkerExit();
	}
	
	if(g_quit)
	{
		g_quit = 0;
		LOG_ERROR(ELOG_KEY, "[rumtime] gracefully shutting down");
		
		if(!mExiting)
		{
			mExiting = 1;
			mListenSock->Close();
		}
	}
}

template <typename T>
void wTcpServer<T>::WorkerExit()
{
	if(mExiting)
	{
		//
	}
	
	LOG_ERROR(ELOG_KEY, "[rumtime] exit");
	exit(0);
}

template <typename T>
void wTcpServer<T>::PrepareStart(string sIpAddr ,unsigned int nPort)
{
	LOG_DEBUG(ELOG_KEY, "[startup] Server Prepare start succeed");
	
	//初始化epoll
	if(InitEpoll() < 0)
	{
		exit(2);
	}
	
	//初始化Listen Socket
	if(InitListen(sIpAddr ,nPort) < 0)
	{
		exit(2);
	}

	//Listen Socket 添加到epoll中
	if (mListenSock == NULL)
	{
		exit(2);
	}
	mTask = NewTcpTask(mListenSock);
	if(NULL != mTask)
	{
		mTask->Status() = TASK_RUNNING;
		if (AddToEpoll(mTask) >= 0)
		{
			AddToTaskPool(mTask);
		}
	}
	
	//运行前工作
	PrepareRun();
}

template <typename T>
void wTcpServer<T>::Start(bool bDaemon)
{
	mStatus = SERVER_RUNNING;
	LOG_DEBUG(ELOG_KEY, "[startup] Server start succeed");
	
	//进入服务主循环
	do {
		Recv();
		Run();
		if(mIsCheckTimer) CheckTimer();
	} while(IsRunning() && bDaemon);
}

template <typename T>
void wTcpServer<T>::PrepareRun()
{
	//accept前准备工作
}

/**
 * epoll初始化
 */
template <typename T>
int wTcpServer<T>::InitEpoll()
{
	mEpollFD = epoll_create(LISTEN_BACKLOG); //511
	if(mEpollFD < 0)
	{
		mErr = errno;
		LOG_ERROR(ELOG_KEY, "[startup] epoll_create failed:%s", strerror(mErr));
		return -1;
	}
	return mEpollFD;
}

template <typename T>
int wTcpServer<T>::InitListen(string sIpAddr ,unsigned int nPort)
{
	mListenSock = new wSocket();
	int iFD = mListenSock->Open();
	if (iFD == -1)
	{
		LOG_ERROR(ELOG_KEY, "[startup] listen socket open failed");
		SAFE_DELETE(mListenSock);
		return -1;
	}
	
	//listen socket
	if(mListenSock->Listen(sIpAddr, nPort) < 0)
	{
		mErr = errno;
		LOG_ERROR(ELOG_KEY, "[startup] listen failed: %s", strerror(mErr));
		SAFE_DELETE(mListenSock);
		return -1;
	}
	
	//nonblock
	if(mListenSock->SetNonBlock() < 0) 
	{
		LOG_ERROR(ELOG_KEY, "[startup] Set non block failed: %d, close it", iFD);
		SAFE_DELETE(mListenSock);
		return -1;
	}
	
	mListenSock->SockStatus() = STATUS_LISTEN;	
	return iFD;
}

template <typename T>
wTask* wTcpServer<T>::NewTcpTask(wIO *pIO)
{
	wTask *pTask = new wTcpTask(pIO);
	return pTask;
}

template <typename T>
wTask* wTcpServer<T>::NewChannelTask(wIO *pIO)
{
	wTask *pTask = new wChannelTask(pIO);
	return pTask;
}

template <typename T>
int wTcpServer<T>::AddToEpoll(wTask* pTask, int iEvents, int iOp)
{
	mEpollEvent.events = iEvents | EPOLLERR | EPOLLHUP; //|EPOLLET
	mEpollEvent.data.fd = pTask->IO()->FD();
	mEpollEvent.data.ptr = pTask;	
	LOG_DEBUG(ELOG_KEY, "[runtime] %s fd %d events read %d write %d", iOp == EPOLL_CTL_MOD ? "mod":"add", mEpollEvent.data.fd, mEpollEvent.events & EPOLLIN, mEpollEvent.events & EPOLLOUT);

	int iRet = epoll_ctl(mEpollFD, iOp, pTask->IO()->FD(), &mEpollEvent);
	if(iRet < 0)
	{
		mErr = errno;
		LOG_ERROR(ELOG_KEY, "[runtime] fd(%d) add into epoll failed: %s", pTask->IO()->FD(), strerror(mErr));
		return -1;
	}
	return 0;
}

template <typename T>
int wTcpServer<T>::AddToTaskPool(wTask* pTask)
{
	W_ASSERT(pTask != NULL, return -1;);

	mTaskPool.push_back(pTask);
	//epoll_event大小
	mTaskCount = mTaskPool.size();
	if (mTaskCount > mEpollEventPool.capacity())
	{
		mEpollEventPool.reserve(mTaskCount * 2);
	}
	
	LOG_DEBUG(ELOG_KEY, "[runtime] fd(%d) add into task pool", pTask->IO()->FD());
	return 0;
}

template <typename T>
int wTcpServer<T>::AcceptMutex()
{
	return 0;
}

template <typename T>
void wTcpServer<T>::Recv()
{
	/**
	 * 惊群锁
	 */
	if(AcceptMutex() != 0)
	{
		return;
	}

	int iRet = epoll_wait(mEpollFD, &mEpollEventPool[0], mTaskCount, mTimeout /*10ms*/);
	if(iRet < 0)
	{
		mErr = errno;
		LOG_ERROR(ELOG_KEY, "[runtime] epoll_wait failed: %s", strerror(mErr));
		return;
	}
	
	wTask *pTask = NULL;
	int iFD = FD_UNKNOWN;
	SOCK_TYPE sockType;
	SOCK_STATUS sockStatus;
	IO_FLAG iOFlag;
	for(int i = 0 ; i < iRet ; i++)
	{
		pTask = (wTask *)mEpollEventPool[i].data.ptr;
		iFD = pTask->IO()->FD();
		sockType = pTask->IO()->SockType();
		sockStatus = pTask->IO()->SockStatus();
		iOFlag = pTask->IO()->IOFlag();

		if(iFD < 0)
		{
			mErr = errno;
			LOG_ERROR(ELOG_KEY, "[runtime] socketfd error fd(%d), close it: %s", iFD, strerror(mErr));
			if (RemoveEpoll(pTask) >= 0)
			{
				RemoveTaskPool(pTask);
			}
			continue;
		}
		if (!pTask->IsRunning())	//多数是超时设置
		{
			LOG_ERROR(ELOG_KEY, "[runtime] task status is quit, fd(%d), close it", iFD);
			if (RemoveEpoll(pTask) >= 0)
			{
				RemoveTaskPool(pTask);
			}
			continue;
		}
		if (mEpollEventPool[i].events & (EPOLLERR | EPOLLPRI))	//出错
		{
			mErr = errno;
			LOG_ERROR(ELOG_KEY, "[runtime] epoll event recv error from fd(%d), close it: %s", iFD, strerror(mErr));
			if (RemoveEpoll(pTask) >= 0)
			{
				RemoveTaskPool(pTask);
			}
			continue;
		}
		
		if(sockType == SOCK_LISTEN && sockStatus == STATUS_LISTEN)
		{
			if (mEpollEventPool[i].events & EPOLLIN)
			{
				AcceptConn();	//accept connect
			}
		}
		else if(sockType == SOCK_CONNECT && sockStatus == STATUS_CONNECTED)
		{
			if (mEpollEventPool[i].events & EPOLLIN)
			{
				//套接口准备好了读取操作
				if (pTask->TaskRecv() <= 0)	//==0主动断开
				{
					mErr = errno;
					LOG_ERROR(ELOG_KEY, "[runtime] EPOLLIN(read) failed or tcp socket closed: %s", strerror(mErr));
					if (RemoveEpoll(pTask) >= 0)
					{
						RemoveTaskPool(pTask);
					}
				}
			}
			else if (mEpollEventPool[i].events & EPOLLOUT)
			{
				if (pTask->IsWritting() <= 0)
				{
					AddToEpoll(pTask, EPOLLIN, EPOLL_CTL_MOD);
					return;
				}
				//套接口准备好了写入操作
				if (pTask->TaskSend() < 0)	//写入失败，半连接
				{
					mErr = errno;
					LOG_ERROR(ELOG_KEY, "[runtime] EPOLLOUT(write) failed or tcp socket closed: %s", strerror(mErr));
					if (RemoveEpoll(pTask) >= 0)
					{
						RemoveTaskPool(pTask);
					}
				}
			}
		}
		else if(sockType == SOCK_UNIX && sockStatus == STATUS_LISTEN)
		{
			if (mEpollEventPool[i].events & EPOLLIN)
			{
				//channel准备好了读取操作
				if (pTask->TaskRecv() <= 0)	//==0主动断开
				{
					LOG_ERROR(ELOG_KEY, "[runtime] EPOLLIN(read) failed or unix socket closed");
					if (RemoveEpoll(pTask) >= 0)
					{
						RemoveTaskPool(pTask);
					}
				}
			}
		}
	}
}

template <typename T>
int wTcpServer<T>::Send(wTask *pTask, const char *pCmd, int iLen)
{
	if (pTask != NULL && pTask->IsRunning() && (pTask->IO()->IOFlag() == FLAG_SEND || pTask->IO()->IOFlag() == FLAG_RVSD))
	{
		if(pTask->SendToBuf(pCmd, iLen) > 0)
		{
			return AddToEpoll(pTask, EPOLLIN | EPOLLOUT, EPOLL_CTL_MOD);
		}
	}
	return -1;
}

template <typename T>
void wTcpServer<T>::Broadcast(const char *pCmd, int iLen)
{
	if(mTaskPool.size() > 0)
	{
		vector<wTask*>::iterator iter;
		for(iter = mTaskPool.begin(); iter != mTaskPool.end(); iter++)
		{
			Send(*iter, pCmd, iLen);
		}
	}
}

/**
 *  接受新连接
 */
template <typename T>
int wTcpServer<T>::AcceptConn()
{
	if(mListenSock == NULL)
	{
		return -1;
	}
	
	struct sockaddr_in stSockAddr;
	socklen_t iSockAddrSize = sizeof(stSockAddr);	
	int iNewFD = mListenSock->Accept((struct sockaddr*)&stSockAddr, &iSockAddrSize);
	if(iNewFD <= 0)
	{
		mErr = errno;
		LOG_ERROR(ELOG_KEY, "[runtime] client connect failed:%s", strerror(mErr));
	    return -1;
    }
		
	//new tcp task
	wSocket *pSocket = new wSocket();
	pSocket->FD() = iNewFD;
	pSocket->Host() = inet_ntoa(stSockAddr.sin_addr);
	pSocket->Port() = stSockAddr.sin_port;
	pSocket->SockType() = SOCK_CONNECT;
	pSocket->IOFlag() = FLAG_RVSD;
	pSocket->SockStatus() = STATUS_CONNECTED;
	if (pSocket->SetNonBlock() < 0)
	{
		SAFE_DELETE(pSocket);
		return -1;
	}
	
	mTask = NewTcpTask(pSocket);
	if(NULL != mTask)
	{
		if(mTask->VerifyConn() < 0 || mTask->Verify())
		{
			LOG_ERROR(ELOG_KEY, "[runtime] connect illegal or verify timeout: %d, close it", iNewFD);
			SAFE_DELETE(mTask);
			return -1;
		}
		
		mTask->Status() = TASK_RUNNING;
		if(AddToEpoll(mTask) >= 0)
		{
			AddToTaskPool(mTask);
		}
		LOG_ERROR(ELOG_KEY, "[connect] client connect succeed: ip(%s) port(%d)", pSocket->Host().c_str(), pSocket->Port());
	}
	return iNewFD;
}

template <typename T>
int wTcpServer<T>::RemoveEpoll(wTask* pTask)
{
	int iFD = pTask->IO()->FD();
	mEpollEvent.data.fd = iFD;
	if(epoll_ctl(mEpollFD, EPOLL_CTL_DEL, iFD, &mEpollEvent) < 0)
	{
		mErr = errno;
		LOG_ERROR(ELOG_KEY, "[runtime] epoll remove socket fd(%d) error : %s", iFD, strerror(mErr));
		return -1;
	}
	return 0;
}

//返回下一个迭代器
template <typename T>
std::vector<wTask*>::iterator wTcpServer<T>::RemoveTaskPool(wTask* pTask)
{
    std::vector<wTask*>::iterator it = std::find(mTaskPool.begin(), mTaskPool.end(), pTask);
    if(it != mTaskPool.end())
    {
    	if ((*it)->IO()->SockType() != SOCK_UNIX)
    	{
    		(*it)->DeleteIO();
    	}
    	SAFE_DELETE(*it);
        it = mTaskPool.erase(it);
    }
    mTaskCount = mTaskPool.size();
    return it;
}

template <typename T>
void wTcpServer<T>::Run()
{
	//...
}

template <typename T>
void wTcpServer<T>::CheckTimer()
{
	unsigned long long iInterval = (unsigned long long)(GetTickCount() - mLastTicker);

	if(iInterval < 100) 	//100ms
	{
		return;
	}

	//加上间隔时间
	mLastTicker += iInterval;

	//检测客户端超时
	if(mCheckTimer.CheckTimer(iInterval))
	{
		CheckTimeout();
	}
}

template <typename T>
void wTcpServer<T>::CheckTimeout()
{
	unsigned long long iNowTime = GetTickCount();
	unsigned long long iIntervalTime;
	
	if(mTaskPool.size() > 0)
	{
		SOCK_TYPE sockType;
		vector<wTask*>::iterator iter;
		for(iter = mTaskPool.begin(); iter != mTaskPool.end(); iter++)
		{
			sockType = (*iter)->IO()->SockType();
			if (sockType == SOCK_CONNECT)
			{
				//心跳检测
				iIntervalTime = iNowTime - (*iter)->IO()->SendTime();	//上一次发送心跳时间间隔
				if(iIntervalTime >= CHECK_CLIENT_TIME)	//3s
				{
					if((*iter)->Heartbeat() < 0 && (*iter)->HeartbeatOutTimes())
					{
						LOG_ERROR(ELOG_KEY, "[runtime] client fd(%d) heartbeat pass limit times, close it", (*iter)->IO()->FD());
						if(RemoveEpoll(*iter) >= 0)
						{
							iter = RemoveTaskPool(*iter);
							iter--;
						}
					}
				}
			}
		}
	}
}