#include "settingmanager.h"
#include <QFile>
#include "Utility/ImPath.h"
#include "Utility/ImCharset.h"
#include "Utility/LogMacro.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDir>

SettingManager::SettingManager()
{
    load();
    loadWuHuaRankSetting();

    m_excelSavedPath = QString::fromStdWString(CImPath::GetDataPath()+L"baokao\\");
    QDir().mkpath(m_excelSavedPath);
}

SettingManager* SettingManager::getInstance()
{
    static SettingManager* pInstance = new SettingManager();
	return pInstance;
}

void SettingManager::load()
{
    std::wstring strConfFilePath = CImPath::GetConfPath() + L"configs.json";    
    QFile file(QString::fromStdWString(strConfFilePath));
    if (!file.exists())
    {
        return;
    }

    if (!file.open(QIODevice::ReadOnly))
    {
        LOG_ERROR(L"failed to open the basic configure file : %s", strConfFilePath.c_str());
        return;
    }
    QByteArray jsonData = file.readAll();
    file.close();

    QJsonDocument jsonDocument = QJsonDocument::fromJson(jsonData);
    QJsonObject root = jsonDocument.object();

    if (root.contains("log_level"))
    {
        m_logLevel = root["log_level"].toInt();
    }
}

void SettingManager::loadWuHuaRankSetting()
{
    std::wstring strConfFilePath = CImPath::GetConfPath() + L"物化单排名分段.txt";
    QFile file(QString::fromStdWString(strConfFilePath));
    if (!file.exists())
    {
        return;
    }

    if (!file.open(QIODevice::ReadOnly))
    {
        LOG_ERROR(L"failed to open the keyword configure file : %s", strConfFilePath.c_str());
        return;
    }

    QTextStream in(&file);
    in.setCodec("UTF-8");
    while (!in.atEnd())
    {
        QString line = in.readLine().trimmed();

        // 跳过空行和注释行
        if (line.isEmpty() || line.startsWith('#'))
            continue;

        // 按分隔符'/'分割行
        QStringList parts = line.split('/', Qt::SkipEmptyParts);

        // 确保有3部分：位次范围开始，位次范围结束，提升排名数
        if (parts.size() != 3)
            continue;

        bool ok = false;
        WuHuaRankSetting wuHuaRankSetting;
        wuHuaRankSetting.m_rankBegin = parts[0].trimmed().toInt(&ok);
        if (!ok)
        {
            qCritical("invalid setting: %s", line.toStdString().c_str());
            continue;
        }

        wuHuaRankSetting.m_rankEnd = parts[1].trimmed().toInt(&ok);
        if (!ok)
        {
            qCritical("invalid setting: %s", line.toStdString().c_str());
            continue;
        }

        wuHuaRankSetting.m_subtractRank = parts[2].trimmed().toInt(&ok);
        if (!ok)
        {
            qCritical("invalid setting: %s", line.toStdString().c_str());
            continue;
        }

        m_wuHuaRankSettings.append(wuHuaRankSetting);
    }

    file.close();
}
