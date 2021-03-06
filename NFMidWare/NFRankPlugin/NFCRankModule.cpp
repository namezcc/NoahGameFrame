//------------------------------------------------------------------------ -
//    @FileName			:    NFCRankModule.cpp
//    @Author           :    LvSheng.Huang
//    @Date             :    2016-12-18
//    @Module           :    NFCRankModule
//
// -------------------------------------------------------------------------

#include "NFCRankModule.h"

bool NFCRankModule::Init()
{

    m_pKernelModule = pPluginManager->FindModule<NFIKernelModule>();
    m_pNoSqlModule = pPluginManager->FindModule<NFINoSqlModule>();
    return true;
}

bool NFCRankModule::Shut()
{
    return true;
}

bool NFCRankModule::Execute()
{
    return true;
}

bool NFCRankModule::AfterInit()
{

    return true;
}

std::string NFCRankModule::MakeRanKey(const RANK_TYPE type, const int64_t nAreaID)
{
    if(nAreaID > 0)
    {
        return "Rank_" + to_string(type) + "_" + to_string(nAreaID);
    }

    return "Rank_" + to_string(type);
}

void NFCRankModule::AddValue(const NFGUID& self, const NFIRankModule::RANK_TYPE type, NFINT64 value)
{
    AddValue(0, self, type, value);

}

void NFCRankModule::SetValue(const NFGUID& self, const NFIRankModule::RANK_TYPE type, NFINT64 value)
{
    SetValue(0, self, type, value);
}

void NFCRankModule::SubValue(const NFGUID& self, const NFIRankModule::RANK_TYPE type, NFINT64 value)
{
    SubValue(0, self, type, value);
}

void NFCRankModule::RemoveValue(const NFGUID& self, const NFIRankModule::RANK_TYPE type)
{
    RemoveValue(0, self, type);
}

NFCRankModule::RankValue NFCRankModule::GetIndex(const NFGUID& self, const NFIRankModule::RANK_TYPE type)
{
    return GetIndex(0, self, type);
}

int NFCRankModule::RangeByIndex(const NFINT64 startIndex, const NFINT64 endIndex, const NFIRankModule::RANK_TYPE type, std::vector<NFIRankModule::RankValue>& vector)
{
    return RangeByIndex(0, startIndex, endIndex, type, vector);
}

int NFCRankModule::RangeByScore(const NFINT64 startScore, const NFINT64 endScore, const NFIRankModule::RANK_TYPE type, std::vector<NFIRankModule::RankValue>& vector)
{
    return RangeByScore(0, startScore, endScore, type, vector);
}

int NFCRankModule::GetRankListCount(const NFIRankModule::RANK_TYPE type)
{
    return GetRankListCount(0, type);
}

//for area
void NFCRankModule::AddValue(const int64_t nAreaID, const NFGUID& self, const RANK_TYPE type, NFINT64 value)
{
    std::string strRankKey = MakeRanKey(type, nAreaID);
    NF_SHARE_PTR<NFINoSqlDriver> xNoSqlDriver = m_pNoSqlModule->GetDriver(strRankKey);
    if (xNoSqlDriver)
    {
        xNoSqlDriver->ZIncrBy(strRankKey, self.ToString(), value);
    }
}

void NFCRankModule::SetValue(const int64_t nAreaID, const NFGUID& self, const RANK_TYPE type, NFINT64 value)
{
    std::string strRankKey = MakeRanKey(type, nAreaID);
    NF_SHARE_PTR<NFINoSqlDriver> xNoSqlDriver = m_pNoSqlModule->GetDriver(strRankKey);
    if (xNoSqlDriver)
    {
        xNoSqlDriver->ZAdd(self.ToString(), value, strRankKey);
    }
}

void NFCRankModule::SubValue(const int64_t nAreaID, const NFGUID& self, const RANK_TYPE type, NFINT64 value)
{
    std::string strRankKey = MakeRanKey(type, nAreaID);
    NF_SHARE_PTR<NFINoSqlDriver> xNoSqlDriver = m_pNoSqlModule->GetDriver(strRankKey);
    if (xNoSqlDriver)
    {
        xNoSqlDriver->ZIncrBy(strRankKey, self.ToString(), -value);
    }
}

void NFCRankModule::RemoveValue(const int64_t nAreaID, const NFGUID& self, const NFIRankModule::RANK_TYPE type)
{
    std::string strRankKey = MakeRanKey(type, nAreaID);
    NF_SHARE_PTR<NFINoSqlDriver> xNoSqlDriver = m_pNoSqlModule->GetDriver(strRankKey);
    if (xNoSqlDriver)
    {
        xNoSqlDriver->ZRem(strRankKey, self.ToString());
    }
}

NFIRankModule::RankValue NFCRankModule::GetIndex(const int64_t nAreaID, const NFGUID& self, const NFIRankModule::RANK_TYPE type)
{
    NFCRankModule::RankValue xRankValue;
    std::string strRankKey = MakeRanKey(type, nAreaID);

    NF_SHARE_PTR<NFINoSqlDriver> xNoSqlDriver = m_pNoSqlModule->GetDriver(strRankKey);
    if (xNoSqlDriver)
    {
        double value = 0;
        if (xNoSqlDriver->ZScore(strRankKey, self.ToString(), value))
        {
            xRankValue.id = self;
            xRankValue.score = value;

            xNoSqlDriver->ZRank(strRankKey, self.ToString(), xRankValue.index);
        }
    }

    return xRankValue;
}

int NFCRankModule::RangeByIndex(const int64_t nAreaID, const NFINT64 startIndex, const NFINT64 endIndex, const RANK_TYPE type, std::vector<NFIRankModule::RankValue>& vector)
{
    std::string strRankKey = MakeRanKey(type, nAreaID);

    NF_SHARE_PTR<NFINoSqlDriver> xNoSqlDriver = m_pNoSqlModule->GetDriver(strRankKey);
    if (xNoSqlDriver)
    {
        std::vector<std::pair<std::string, double> > memberScoreVec;
        if (xNoSqlDriver->ZRange(strRankKey, startIndex, endIndex, memberScoreVec))
        {
            for (int i = 0; i < memberScoreVec.size(); ++i)
            {
                RankValue xRankValue;
                xRankValue.id.FromString(memberScoreVec[i].first);
                xRankValue.score = (int)memberScoreVec[i].second;
                xRankValue.index = i + startIndex;
                vector.push_back(xRankValue);
            }
        }
    }

    return vector.size();
}

int NFCRankModule::RangeByScore(const int64_t nAreaID, const NFINT64 startScore, const NFINT64 endScore, const RANK_TYPE type, std::vector<NFIRankModule::RankValue>& vector)
{
    std::string strRankKey = MakeRanKey(type, nAreaID);

    NF_SHARE_PTR<NFINoSqlDriver> xNoSqlDriver = m_pNoSqlModule->GetDriver(strRankKey);
    if (xNoSqlDriver)
    {
        std::vector<std::pair<std::string, double> > memberScoreVec;
        if (xNoSqlDriver->ZRangeByScore(strRankKey, startScore, endScore, memberScoreVec))
        {
            for (int i = 0; i < memberScoreVec.size(); ++i)
            {
                RankValue xRankValue;
                xRankValue.id.FromString(memberScoreVec[i].first);
                xRankValue.score = memberScoreVec[i].second;

                //xNoSqlDriver->ZRank(strRankKey, memberScoreVec[i].first, xRankValue.index);

                vector.push_back(xRankValue);
            }
        }
    }

    return vector.size();
}


int NFCRankModule::GetRankListCount(const int64_t nAreaID, const NFIRankModule::RANK_TYPE type)
{
    std::string strRankKey = MakeRanKey(type, nAreaID);

    NF_SHARE_PTR<NFINoSqlDriver> xNoSqlDriver = m_pNoSqlModule->GetDriver(strRankKey);
    if (xNoSqlDriver)
    {
        int nCount = 0;
        xNoSqlDriver->ZCard(strRankKey, nCount);
        return nCount;
    }

    return 0;
}

