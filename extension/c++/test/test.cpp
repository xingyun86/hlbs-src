
#include "wCore.h"
#include "agent_api.h"

int Get()
{
	cout << "connect:" <<ConnectAgent(&g_handle) << endl;

	struct SvrNet_t stSvr;
	stSvr.mGid = 1;
	stSvr.mXid = 1;

	string s;
	cout << "res:" << QueryNode(stSvr, 0.2, s) << endl;
	cout << "host:" << stSvr.mHost << endl;
	//Release(&g_handle);
}

int Post()
{
	cout << "connect:" <<InitShm(&g_handle) << endl;

	SvrNet_t stSvr;
	stSvr.mGid = 1;
	stSvr.mXid = 1;
	stSvr.mPort = 3306;
	memcpy(stSvr.mHost,"192.168.8.13",sizeof("192.168.8.13"));
	
	string s;
	cout << "res:" << NotifyCallerRes(stSvr, 0, 2000, s) << endl;
	cout << "host:" << stSvr.mHost << endl;
	//Release(&g_handle);
}

int main(int argc,char **argv)
{
	Get();
	//Post();
	return 0;
}

