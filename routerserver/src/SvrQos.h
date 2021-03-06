
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#ifndef _SVR_QOS_H_
#define _SVR_QOS_H_

#include <map>
#include <list>
#include "wCore.h"
#include "wNoncopyable.h"
#include "SvrCmd.h"

using namespace hnet;

class RouterConfig;

class SvrQos : private wNoncopyable {
	typedef std::map<struct SvrNet_t, struct SvrStat_t*> MapSvr_t;
	typedef std::map<struct SvrNet_t, struct SvrStat_t*>::iterator MapSvrIt_t;

	typedef std::map<struct SvrKind_t, std::multimap<float, struct SvrNode_t>* > MapKind_t;
	typedef std::map<struct SvrKind_t, std::multimap<float, struct SvrNode_t>* >::iterator MapKindIt_t;
	typedef std::multimap<float, struct SvrNode_t> MultiMapNode_t;
	typedef std::multimap<float, struct SvrNode_t>::iterator MultiMapNodeIt_t;
	typedef std::multimap<float, struct SvrNode_t>::reverse_iterator MultiMapNodeRIt_t;

    typedef std::map<struct SvrKind_t, std::list<struct SvrNode_t>* > MapNode_t;
    typedef std::map<struct SvrKind_t, std::list<struct SvrNode_t>* >::iterator MapNodeIt_t;
    typedef std::list<struct SvrNode_t> ListNode_t;
    typedef std::list<struct SvrNode_t>::iterator ListNodeIt_t;

public:
	~SvrQos();
	SvrQos() : mRateWeight(7), mDelayWeight(1), mRebuildTm(60), mReqTimeout(500),
			mPreRoute(1), mAllReqMin(false), mAvgErrRate(0.0) { }

	// 指定节点是否存在
	bool IsExistNode(const struct SvrNet_t& svr);

	// 节点version是否变化
	bool IsVerChange(const struct SvrNet_t& svr);

	// 节点weight、name、idc是否变化
	bool IsWNIChange(const struct SvrNet_t& svr);

	// 获取所有节点（调用者自行解决溢出问题）
	// num为实际buf的缓冲索引起始地址
	// start为从start索引开始获取mMapReqSvr的svr节点
	// size为最大获取svr个数
	int GetNodeAll(struct SvrNet_t buf[], int32_t num, int32_t start, int32_t size);
	
	// 保存节点信息
	int SaveNode(const struct SvrNet_t& svr);

	// 添加新节点&&路由
	int AddNode(const struct SvrNet_t& svr);

	// 修改 节点&&路由 权重（权重为0删除节点）
	int ModifyNode(const struct SvrNet_t& svr);

	// 删除 节点&&路由
	int DeleteNode(const struct SvrNet_t& svr);

	// 清除所有路由信息
	int CleanNode();

	// 过滤SVR
	int FilterSvrBySid(struct SvrNet_t buf[], int32_t size, const std::vector<struct Rlt_t>& rlts);

protected:
	friend class RouterConfig;

	// 添加新路由
	int AddRouteNode(const struct SvrNet_t& svr, struct SvrStat_t* stat);
	// 删除路由节点
	int DeleteRouteNode(const struct SvrNet_t& stSvr);
	// 修改路由节点
	int ModifyRouteNode(const struct SvrNet_t& svr);
	// 加载阈值配置
	int LoadStatCfg(const struct SvrNet_t& svr, struct SvrStat_t* stat);

	struct SvrReqCfg_t	mReqCfg;	// 访问量控制
	struct SvrDownCfg_t mDownCfg;	// 宕机控制

	int mRateWeight;	// 成功率因子 1~100000
	int mDelayWeight;	// 时延因子 1~100000
	int mRebuildTm;		// 重建时间间隔 默认为60s
	int mReqTimeout;	// 请求超时时间 默认为500ms
	int mPreRoute;		// 是否开启预取缓存功能。开启后可以预先将某个gid, xid对应的host,port加载到内存中，以节省访问资源。
	bool mAllReqMin;	// 所有节点都过载
	float mAvgErrRate;	// 错误平均值，过载时

	MapSvr_t mMapReqSvr;	// 节点信息。1:1，节点-统计
	MapKind_t mRouteTable;	// 路由信息。1:n，种类-节点
	MapNode_t mErrTable;	// 宕机路由表，1:n，种类-节点
};

#endif
