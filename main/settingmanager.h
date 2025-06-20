﻿#pragma once

#include <QString>
#include <QVector>

class FilterSetting
{
public:
    enum Sex
    {
        BOY,
        GIRL
    };

public:
    // 姓名
    QString m_name;

    // 性别
    Sex m_sex = BOY;

    // 总分位次
    int m_totalScorePos = 0;

    // 标志是否为物化组合
    bool m_isWuHua = true;

    // 物理位次
    int m_wuLiScorePos = 0;

    // 化学位次
    int m_huaXueScorePos = 0;

    // 非物化组合位次
    int m_notWuHuaScorePos = 0;

public:
    QString getSexString() const
    {
        static QString boy = QString::fromWCharArray(L"男");
        static QString girl = QString::fromWCharArray(L"女");
        if (m_sex == BOY)
        {
            return boy;
        }
        else
        {
            return girl;
        }
    }
};

// 物化分段排名设置
class WuHuaRankSetting
{
public:
    // 排名开始
    int m_rankBegin = 0;

    // 排名结束
    int m_rankEnd = 0;

    // 提升排名数
    int m_subtractRank = 0;
};

class SettingManager
{
protected:
    SettingManager();

public:
    static SettingManager* getInstance();

private:
    void load();

    void loadWuHuaRankSetting();

public:
    int m_logLevel = 2;  // info

    FilterSetting m_filterSetting;

    // 表格存储路径，尾部有斜杆
    QString m_excelSavedPath;

    QVector<WuHuaRankSetting> m_wuHuaRankSettings;
};
