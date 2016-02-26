
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _W_TYPE_H_
#define _W_TYPE_H_

#define _FILE_OFFSET_BITS  64	//off_t 突破单文件4G大小限制，系统库sys/types.h中使用

#include <unistd.h>
#include <errno.h>
#include <sched.h>
#include <ctype.h>
#include <limits.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include <sys/types.h>
#include <sys/param.h>

#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>

//#include <pwd.h>
//#include <grp.h>
//#include <dirent.h>

//#include <sys/wait.h>
//#include <sys/mman.h>

//#include <sys/socket.h>
//#include <netinet/in.h>
//#include <netinet/tcp.h>
//#include <arpa/inet.h>
//#include <sys/un.h>

//#include <sys/epoll.h>

#include <stdio.h>
#include <iostream>
#include <string>

//单字节无符号整数
typedef uint8_t BYTE;

//双字节无符号整数
typedef uint16_t WORD;

//双字节符号整数
typedef int16_t SWORD;

//四字节无符号整数
typedef uint32_t DWORD;

//四字节符号整数
typedef int32_t SDWORD;

//八|十六字节无符号整数
typedef uint64_t QWORD;

//八|十六字节符号整数
typedef int64_t SQWORD;

//八字节无符号整数
typedef unsigned long int LWORD;

//八字节符号整数
typedef signed long int SLWORD;

//十六节无符号整数
typedef unsigned long long int LLWORD;

//十六字节符号整数
typedef signed long long int SLLWORD;

//名字的最大长度
#define MAX_NAME_LEN 32

//最长的IP地址字符串长度
#define MAX_IP_LEN 16

//消息缓冲区大小(每条信息消息主体最大长度)
#define MAX_PACKAGE_LEN 131072	/* 128 * 1024 */

//客户端消息的最长长度
#define MAX_CLIENT_MSG_LEN MAX_PACKAGE_LEN

//客户端消息主体最短长度(一字节消息类型)
#define MIN_CLIENT_MSG_LEN 1

//每一个服务器之间的发送接收缓冲区的大小
#define MAX_SEND_BUFFER_LEN 524288 /* 512 * 1024 */

#define MAX_RECV_BUFFER_LEN 261120 /* 255 * 1024 */

//心跳包间隔时间
#define KEEPALIVE_TIME 3000 /* 3 * 1000 */

//服务器检测心跳时间
#define CHECK_CLIENT_TIME 5000 /* 5 * 1000 */

//超时时间
#define WAITRES_TIME 30000 /* 30 * 1000 */

//消息队列的长度
#ifdef _DEBUG_
#define MSG_QUEUE_LEN 4194304 /* 1024 * 1024 * 4 */
#else
#define MSG_QUEUE_LEN 16777216 /* 1024 * 1024 * 16 */
#endif

//主机名长度
#ifndef MAXHOSTNAMELEN
#define MAXHOSTNAMELEN  256
#endif

//listen socket backlog长度
#define LISTEN_BACKLOG        511

//对齐
#define ALIGNMENT	sizeof(unsigned long)    /* platform word */

#define ALIGN(d, a)		(((d) + (a - 1)) & ~(a - 1))
#define ALIGN_PTR(p, a)	(u_char *) (((uintptr_t) (p) + ((uintptr_t) a - 1)) & ~((uintptr_t) a - 1))

//整形长度
#define NGX_INT32_LEN   (sizeof("-2147483648") - 1)		/* int32长度*/
#define NGX_INT64_LEN   (sizeof("-9223372036854775808") - 1) 	/* int64长度*/

//释放对象
#define SAFE_DELETE(x) { if(x) { delete(x); (x) = NULL; } }
#define SAFE_DELETE_VEC(x) { if(x) { delete [] (x); (x) = NULL; } }

#define SAFE_FREE(x) { if(x) { free(x); (x) = NULL; } }

#define SAFE_SUB(x, y) ((x) > (y) ? (x) - (y) : 0)

#define CAST_CMD(x, y, t)	const t* x = (const t*)y;

#define SETPROCTITLE_PAD       '\0'

extern char **environ;

//命名空间
using namespace std;

#endif
