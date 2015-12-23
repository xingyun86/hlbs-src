
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

/**
 *  消息头
 */
#ifndef _W_COMMAND_H_
#define _W_COMMAND_H_

#include <string.h>
#include "wType.h"

#define ASSERT_CMD(cmd, para) ((para)<<8 | (cmd))

#pragma pack(1)

const BYTE CMD_NULL = 0;
const BYTE PARA_NULL = 0;

enum
{
	SERVER,
	CLIENT,
};

struct _Null_t
{
	_Null_t(const BYTE cmd, const BYTE para) : mCmd(cmd), mPara(para) {};

	union
	{
		WORD mId;
		struct {BYTE mCmd; BYTE mPara;};
	};
	WORD GetId() const { return mId; }
	BYTE GetCmd() const { return mCmd; }
	BYTE GetPara() const { return mPara; }
};

struct wCommand : public _Null_t
{
	BYTE mConnType;
	wCommand(const BYTE cmd = CMD_NULL, const BYTE para = PARA_NULL , const BYTE ctype = SERVER) : _Null_t(cmd, para), mConnType(ctype) {}
	BYTE GetConnType() const { return mConnType; }
};

#pragma pack()

#endif
