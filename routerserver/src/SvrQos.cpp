
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#include <vector>
#include <algorithm>
#include <cmath>
#include "wLogger.h"
#include "SvrQos.h"
#include "Define.h"

SvrQos::~SvrQos() {
	CleanNode();
}

bool SvrQos::IsExistNode(const struct SvrNet_t& svr) {
	if (mMapReqSvr.find(svr) == mMapReqSvr.end()) {
		return false;
	}
	return true;
}

bool SvrQos::IsWNIChange(const struct SvrNet_t& svr) {
	MapSvrIt_t mapReqIt = mMapReqSvr.find(svr);
	if (mapReqIt != mMapReqSvr.end()) {
		const struct SvrNet_t& kind = mapReqIt->first;
		if (kind.mWeight == svr.mWeight && kind.mIdc == svr.mIdc && !misc::Strcmp(kind.mName, svr.mName, kMaxName)) {
			return false;
		}
	}
	return true;
}

bool SvrQos::IsVerChange(const struct SvrNet_t& svr) {
	MapSvrIt_t mapReqIt = mMapReqSvr.find(svr);
	if (mapReqIt != mMapReqSvr.end()) {
		const struct SvrNet_t& kind = mapReqIt->first;
		if (kind.mVersion == svr.mVersion) {
			return false;
		}
	}
	return true;
}

int SvrQos::SaveNode(const struct SvrNet_t& svr) {
	if (IsExistNode(svr)) {
		return ModifyNode(svr);
	}
	return AddNode(svr);
}

int SvrQos::AddNode(const struct SvrNet_t& svr) {
	if (svr.mWeight <= 0) {
		HNET_ERROR(soft::GetLogdirPath() + kSvrLogFilename, "SvrQos::AddNode add SvrNet_t failed(weight<=0), GID(%d),XID(%d),HOST(%s),PORT(%d),WEIGHT(%d)",
				svr.mGid, svr.mXid, svr.mHost, svr.mPort, svr.mWeight);
		
		return -1;
	}

	struct SvrStat_t* stat = NULL;
	HNET_NEW(SvrStat_t, stat);
	if (!stat) {
		return -1;
	}

	if (LoadStatCfg(svr, stat) == -1) {
		HNET_ERROR(soft::GetLogdirPath() + kSvrLogFilename, "SvrQos::AddNode add SvrNet_t failed(LoadStatCfg failed), GID(%d),XID(%d),HOST(%s),PORT(%d),WEIGHT(%d)",
				svr.mGid, svr.mXid, svr.mHost, svr.mPort, svr.mWeight);

		HNET_DELETE(stat);
		return -1;
	}

	// 重建绝对时间
	stat->mReqCfg.mRebuildTm = mRebuildTm;
	misc::GetTimeofday(&stat->mInfo.mBuildTm);
	mMapReqSvr.insert(std::make_pair(svr, stat));

	HNET_DEBUG(soft::GetLogdirPath() + kSvrLogFilename, "SvrQos::AddNode add SvrNet_t success, GID(%d),XID(%d),HOST(%s),PORT(%d),WEIGHT(%d)",
			svr.mGid, svr.mXid, svr.mHost, svr.mPort, svr.mWeight);

	return AddRouteNode(svr, stat);
}

int SvrQos::ModifyNode(const struct SvrNet_t& svr) {
	HNET_DEBUG(soft::GetLogdirPath() + kSvrLogFilename, "SvrQos::ModifyNode modify SvrNet_t start, GID(%d),XID(%d),HOST(%s),PORT(%d),WEIGHT(%d)",
			svr.mGid, svr.mXid, svr.mHost, svr.mPort, svr.mWeight);

	if (svr.mWeight < 0) {
		HNET_ERROR(soft::GetLogdirPath() + kSvrLogFilename, "SvrQos::ModifyNode modify SvrNet_t failed(weight < 0), GID(%d),XID(%d),HOST(%s),PORT(%d),WEIGHT(%d)",
				svr.mGid, svr.mXid, svr.mHost, svr.mPort, svr.mWeight);

		return -1;
	} else if (svr.mWeight == 0) {
		// 权重为0删除节点
		return DeleteNode(svr);
	} else {
		// 修改weight
		MapSvrIt_t mapReqIt = mMapReqSvr.find(svr);
		struct SvrNet_t& oldsvr = const_cast<struct SvrNet_t&>(mapReqIt->first);
		oldsvr =  svr;
		return ModifyRouteNode(svr);
	}
}

int SvrQos::DeleteNode(const struct SvrNet_t& svr) {
	MapSvrIt_t mapReqIt = mMapReqSvr.find(svr);
    if (mapReqIt == mMapReqSvr.end()) {
		HNET_ERROR(soft::GetLogdirPath() + kSvrLogFilename, "SvrQos::DeleteNode delete SvrNet_t failed(cannot find the SvrNet_t), GID(%d),XID(%d),HOST(%s),PORT(%d),WEIGHT(%d)",
				svr.mGid, svr.mXid, svr.mHost, svr.mPort, svr.mWeight);

        return -1;
    }

    struct SvrStat_t* stat = mapReqIt->second;
    mMapReqSvr.erase(mapReqIt);
    DeleteRouteNode(svr);
    HNET_DELETE(stat);

	HNET_DEBUG(soft::GetLogdirPath() + kSvrLogFilename, "SvrQos::DeleteNode delete SvrNet_t success, GID(%d),XID(%d),HOST(%s),PORT(%d),WEIGHT(%d)",
			svr.mGid, svr.mXid, svr.mHost, svr.mPort, svr.mWeight);

    return 0;
}

int SvrQos::LoadStatCfg(const struct SvrNet_t& svr, struct SvrStat_t* stat) {
	stat->mInfo.InitInfo(svr);
	stat->mReqCfg = mReqCfg;
	return 0;
}

int SvrQos::GetNodeAll(struct SvrNet_t buf[], int32_t num, int32_t start, int32_t size) {
	HNET_DEBUG(soft::GetLogdirPath() + kSvrLogFilename, "SvrQos::GetNodeAll getall SvrNet_t start, [%d, %d, %d]", num, start, size);

	int i = 0;
	if (static_cast<size_t>(start) >= mMapReqSvr.size()) {
		return i;
	}
	MapSvrIt_t mapReqIt = mMapReqSvr.begin();
	for (std::advance(mapReqIt, start); num < size && mapReqIt != mMapReqSvr.end(); mapReqIt++) {
		buf[num++] = mapReqIt->first;
		i++;
	}
	return i;
}

int SvrQos::FilterSvrBySid(struct SvrNet_t buf[], int32_t size, const std::vector<struct Rlt_t>& rlts) {
	int num = 0;
	for (int32_t i = 0; i < size; i++) {
		struct Rlt_t rlt(buf[i].mGid, buf[i].mXid);
		if (std::find(rlts.begin(), rlts.end(), rlt) != rlts.end()) {
			buf[num++] = buf[i];
		}
	}

	HNET_DEBUG(soft::GetLogdirPath() + kSvrLogFilename, "SvrQos::FilterSvrBySid filter all SvrNet_t start, [%d, %d, %d]", size, rlts.size(), num);
	return num;
}

int SvrQos::AddRouteNode(const struct SvrNet_t& svr, struct SvrStat_t* stat) {
    struct SvrKind_t kind(svr);
    kind.mRebuildTm = stat->mReqCfg.mRebuildTm;

    MultiMapNode_t*& table = mRouteTable[kind];
    if (!table) {
    	HNET_NEW(MultiMapNode_t, table);
    	if (!table) {
    		return -1;
    	}
    }

    // 路由表中已有相关kind（类型）节点，取优先级最低的那个作为新节点统计信息的默认值
    for (MultiMapNodeRIt_t it = table->rbegin(); it != table->rend(); ++it) {
        struct SvrNode_t& node = it->second;

        if (node.mStat->mInfo.mOffSide == 0) {
            // 初始化统计
            // 目的：解决被调扩容时，新加的服务器流量会瞬间爆增，然后在一个周期内恢复正常（正在运行时，突然新加一个服务器，
            // 如果mPreAll=0，GetRoute函数分配服务器时，会连续分配该新加的服务器）
            stat->mInfo = node.mStat->mInfo;
            // 初始化阈值
            stat->mReqCfg.mReqLimit = node.mStat->mReqCfg.mReqLimit;
            stat->mReqCfg.mReqMin = node.mStat->mReqCfg.mReqMin;
            stat->mReqCfg.mReqMax = node.mStat->mReqCfg.mReqMax;
            stat->mReqCfg.mReqErrMin = node.mStat->mReqCfg.mReqErrMin;
            stat->mReqCfg.mReqExtendRate = node.mStat->mReqCfg.mReqExtendRate;
            break;
        }
    }

    struct SvrNode_t node(svr, stat);
    table->insert(std::make_pair(node.mKey, node));

	HNET_DEBUG(soft::GetLogdirPath() + kSvrLogFilename, "SvrQos::AddRouteNode add SvrNet_t success, GID(%d),XID(%d),HOST(%s),PORT(%d),WEIGHT(%d),"
			"ReqLimit(%d),ReqAll(%d),ReqSuc(%d),ReqErrRet(%d),ReqErrTm(%d),LoadX(%f),PreAll(%d)",
			svr.mGid, svr.mXid, svr.mHost, svr.mPort, svr.mWeight,stat->mReqCfg.mReqLimit,
			stat->mInfo.mReqAll,stat->mInfo.mReqSuc,stat->mInfo.mReqErrRet,stat->mInfo.mReqErrTm,stat->mInfo.mLoadX,stat->mInfo.mReqAll);
    
    return 0;
}

int SvrQos::DeleteRouteNode(const struct SvrNet_t& svr) {
    struct SvrKind_t kind(svr);
    MapKindIt_t rtIt = mRouteTable.find(kind);
    MapNodeIt_t etIt = mErrTable.find(kind);

	if (rtIt == mRouteTable.end() && etIt == mErrTable.end()) {
		HNET_ERROR(soft::GetLogdirPath() + kSvrLogFilename, "SvrQos::DeleteRouteNode delete SvrNet_t failed(cannot find node from mRouteTable or mErrTable), GID(%d),XID(%d),HOST(%s),PORT(%d),WEIGHT(%d)",
				svr.mGid, svr.mXid, svr.mHost, svr.mPort, svr.mWeight);

        return -1;
    }

	// 节点路由
	if (rtIt != mRouteTable.end()) {
		MultiMapNode_t* table = rtIt->second;
        if (table != NULL) {
        	MultiMapNodeIt_t it = table->begin();
            while (it != table->end()) {
                if (it->second.mNet == svr) {
                	table->erase(it++);
                    if (table->empty()) {
                        mRouteTable.erase(rtIt);
                        HNET_DELETE(table);
                    }
                    break;
                } else {
                    it++;
                }
            }
        } else {
    		HNET_DEBUG(soft::GetLogdirPath() + kSvrLogFilename, "SvrQos::DeleteRouteNode mRouteTable second(table) is null, GID(%d),XID(%d),HOST(%s),PORT(%d),WEIGHT(%d)",
    				svr.mGid, svr.mXid, svr.mHost, svr.mPort, svr.mWeight);
        }
	}

	// 错误路由
	if (etIt != mErrTable.end()) {
		ListNode_t* node = etIt->second;
		if (node != NULL) {
			ListNodeIt_t it = node->begin();
			while (it != node->end()) {
				if (it->mNet == svr) {
					node->erase(it++);
					if (node->empty()) {
                        mErrTable.erase(etIt);
                        HNET_DELETE(node);
					}
					break;
				} else {
					it++;
				}
			}
		} else {
    		HNET_DEBUG(soft::GetLogdirPath() + kSvrLogFilename, "SvrQos::DeleteRouteNode mErrTable is null, GID(%d),XID(%d),HOST(%s),PORT(%d),WEIGHT(%d)",
    				svr.mGid, svr.mXid, svr.mHost, svr.mPort, svr.mWeight);
		}
	}
	return 0;
}

int SvrQos::ModifyRouteNode(const struct SvrNet_t& svr) {
    struct SvrKind_t kind(svr);
    MapKindIt_t rtIt = mRouteTable.find(kind);
    MapNodeIt_t etIt = mErrTable.find(kind);

	if (rtIt == mRouteTable.end() && etIt == mErrTable.end()) {
		HNET_ERROR(soft::GetLogdirPath() + kSvrLogFilename, "SvrQos::ModifyRouteNode modify SvrNet_t failed(cannot find node from mRouteTable or mErrTable), GID(%d),XID(%d),HOST(%s),PORT(%d),WEIGHT(%d)",
				svr.mGid, svr.mXid, svr.mHost, svr.mPort, svr.mWeight);

        return -1;
    }

	// 节点路由
	if (rtIt != mRouteTable.end()) {
		MultiMapNode_t* table = rtIt->second;
        if (table != NULL) {
        	MultiMapNodeIt_t it = table->begin();
            while (it != table->end()) {
                if (it->second.mNet == svr) {
                	struct SvrNet_t& oldsvr = it->second.mNet;
                	oldsvr.mWeight = svr.mWeight;
					oldsvr.mVersion = svr.mVersion;
					oldsvr.mIdc = svr.mIdc;
					memcpy(oldsvr.mName, svr.mName, kMaxName);
                    break;
                } else {
                    it++;
                }
            }
        } else {
    		HNET_DEBUG(soft::GetLogdirPath() + kSvrLogFilename, "SvrQos::ModifyRouteNode mRouteTable second(table) is null, GID(%d),XID(%d),HOST(%s),PORT(%d),WEIGHT(%d)",
    				svr.mGid, svr.mXid, svr.mHost, svr.mPort, svr.mWeight);
        }
	}

	// 错误路由
	if (etIt != mErrTable.end()) {
		ListNode_t* node = etIt->second;
		if (node != NULL) {
			ListNodeIt_t it = node->begin();
			while (it != node->end()) {
				if (it->mNet == svr) {
                	struct SvrNet_t& oldsvr = it->mNet;
                	oldsvr.mWeight = svr.mWeight;
					oldsvr.mVersion = svr.mVersion;
					oldsvr.mIdc = svr.mIdc;
					memcpy(oldsvr.mName, svr.mName, kMaxName);
					break;
				} else {
					it++;
				}
			}
		} else {
    		HNET_DEBUG(soft::GetLogdirPath() + kSvrLogFilename, "SvrQos::ModifyRouteNode mRouteTable second(table) is null, GID(%d),XID(%d),HOST(%s),PORT(%d),WEIGHT(%d)",
    				svr.mGid, svr.mXid, svr.mHost, svr.mPort, svr.mWeight);
		}
	}
	return 0;
}

int SvrQos::CleanNode() {
    if (mMapReqSvr.size() > 0) {
    	struct SvrNet_t svr;
    	struct SvrStat_t* stat;
    	for (MapSvrIt_t mapReqIt = mMapReqSvr.begin(); mapReqIt != mMapReqSvr.end(); mapReqIt++) {
    		svr = mapReqIt->first;
    		stat = mapReqIt->second;
    		DeleteRouteNode(svr);
    		HNET_DELETE(stat);
    	}
    	mMapReqSvr.clear();
    }
    return 0;
}
