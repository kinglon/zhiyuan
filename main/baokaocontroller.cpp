#include "baokaocontroller.h"
#include "xlsxdocument.h"
#include "xlsxchartsheet.h"
#include "xlsxcellrange.h"
#include "xlsxchart.h"
#include "xlsxrichstring.h"
#include "xlsxworkbook.h"
#include <QVector>
#include <QFileInfo>
#include "Utility/ImPath.h"
#include "settingmanager.h"

using namespace QXlsx;

// 表格列名
#define COLUMN_SCHOOL_NAME 2
#define COLUMN_XUANKE 6
#define COLUMN_SCORE_POS 7
#define MAX_COLUMN COLUMN_SCORE_POS

BaoKaoThread::BaoKaoThread(QObject *parent) : QThread(parent)
{

}

void BaoKaoThread::run()
{
    qInfo("the thread of baokao begins");
    runInternal();
    qInfo("the thread of baokao ends");
}

void BaoKaoThread::runInternal()
{
    // 加载往年高考录取数据
    emit printLog(QString::fromWCharArray(L"加载往年高考录取数据"));
    QString rawExcelFile = QString::fromStdWString(CImPath::GetConfPath());
    rawExcelFile += QString::fromWCharArray(L"高考录取数据.xlsx");
    Document rawExcelDoc(rawExcelFile);
    if (!rawExcelDoc.load())
    {
        emit printLog(QString::fromWCharArray(L"加载往年高考录取数据，失败"));
        return;
    }

    QVector<QVector<QString>> datas;
    CellRange range = rawExcelDoc.dimension();
    qint64 lastReportTime = QDateTime::currentSecsSinceEpoch();
    for (int row=2; row <= range.lastRow(); row++)
    {
        QVector<QString> data;
        for (int column = 1; column <= range.lastColumn(); column++)
        {
            Cell* cell = rawExcelDoc.cellAt(row, column);
            if (cell)
            {
                QVariant rawValue = cell->value();
                QString value = rawValue.toString();
                if (rawValue.type() != QVariant::String)
                {
                    value = cell->readValue().toString();
                }
                data.append(value);
                continue;
            }
        }

        if (data.length() < MAX_COLUMN || data[COLUMN_SCHOOL_NAME].isEmpty())
        {
            break;
        }

        datas.append(data);

        qint64 now = QDateTime::currentSecsSinceEpoch();
        if (now - lastReportTime >= 5)
        {
            emit printLog(QString::fromWCharArray(L"加载中"));
        }
    }
    qInfo("total %d", datas.length());

    // 生成报考表格
    emit printLog(QString::fromWCharArray(L"生成报考表格"));

    int min = 0;
    int max = 0;
    getScorePosRange(min, max);
    const auto& filterSetting = SettingManager::getInstance()->m_filterSetting;
    qInfo("total score: %d, wu hua: %d, wu li score: %d, hua xue score: %d, not wu hua: %d, min: %d, max: %d",
            filterSetting.m_totalScorePos, filterSetting.m_isWuHua?1:0,
            filterSetting.m_wuLiScorePos, filterSetting.m_huaXueScorePos,
            filterSetting.m_notWuHuaScorePos, min, max);

    QVector<QVector<QString>> filterDatas;
    QString xuanKeBuXian = QString::fromWCharArray(L"不限");
    QString xuanKeWuLi = QString::fromWCharArray(L"物理");
    QString xuanKeHuaXue = QString::fromWCharArray(L"化学");
    for (int i=0; i<datas.length(); i++)
    {
        const auto& data = datas[i];

        bool ok = false;
        int pos = data[COLUMN_SCORE_POS].toInt(&ok);
        if (!ok)
        {
            qCritical("row %d, school name: %s, score pos is not valid", i+2, data[COLUMN_SCHOOL_NAME].toStdString().c_str());
            continue;
        }

        QString xuanKe = data[COLUMN_XUANKE];
        if (filterSetting.m_isWuHua)
        {
            if (xuanKe.contains(xuanKeBuXian)
                    || xuanKe.contains(xuanKeWuLi)
                    || xuanKe.contains(xuanKeHuaXue))
            {
                if (pos >= min && pos <= max)
                {
                    filterDatas.append(data);
                }
            }
        }
        else
        {
            if (xuanKe.contains(xuanKeBuXian)
                    || (!xuanKe.contains(xuanKeWuLi) && !xuanKe.contains(xuanKeHuaXue)))
            {
                if (pos >= min && pos <= max)
                {
                    filterDatas.append(data);
                }
            }
        }
    }
    qInfo("total %d after filter", filterDatas.length());

    // 保存报考表格
    emit printLog(QString::fromWCharArray(L"保存报考表格"));

    QString excelFileName = QString::fromWCharArray(L"输出表格模板.xlsx");
    QString srcExcelFilePath = QString::fromStdWString(CImPath::GetConfPath()) + excelFileName;
    QString destExcelFileName = QString::fromWCharArray(L"报考");
    QString destExcelFilePath = SettingManager::getInstance()->m_excelSavedPath+destExcelFileName+".xlsx";
    if (QFile(destExcelFilePath).exists())
    {
        for (int i=1; i<10000; i++)
        {
            QString index = QString::fromWCharArray(L"（%1）").arg(i);
            destExcelFilePath = SettingManager::getInstance()->m_excelSavedPath+destExcelFileName+index+".xlsx";
            if (!QFile(destExcelFilePath).exists())
            {
                break;
            }
        }
    }

    if (!::CopyFile(srcExcelFilePath.toStdWString().c_str(), destExcelFilePath.toStdWString().c_str(), TRUE))
    {
        emit printLog(QString::fromWCharArray(L"拷贝表格模板到保存目录失败"));
        return;
    }

    Document outputExcelDoc(destExcelFilePath);
    if (!outputExcelDoc.load())
    {
        emit printLog(QString::fromWCharArray(L"打开输出表格失败"));
        return;
    }

    // 写入个人信息
    QString personInfo;
    if (filterSetting.m_isWuHua)
    {
        personInfo = QString::fromWCharArray(L"姓名：%1 性别：%2 总分位次：%3 物理总分位次：%4 化学总分位次：%5").arg(
                            filterSetting.m_name, filterSetting.getSexString(),
                            QString::number(filterSetting.m_totalScorePos),
                            QString::number(filterSetting.m_wuLiScorePos),
                            QString::number(filterSetting.m_huaXueScorePos));
    }
    else
    {
        personInfo = QString::fromWCharArray(L"姓名：%1 性别：%2 总分位次：%3 非物化总分位次：%4").arg(
                            filterSetting.m_name, filterSetting.getSexString(),
                            QString::number(filterSetting.m_totalScorePos),
                            QString::number(filterSetting.m_notWuHuaScorePos));
    }
    outputExcelDoc.write(1, 1, personInfo);

    // 写入报考信息
    for (int i=0; i<filterDatas.length(); i++)
    {
        int row = i+3;
        for (int j=0; j<filterDatas[i].length(); j++)
        {
            int column = j+1;
            outputExcelDoc.write(row, column, filterDatas[i][j]);
        }
    }

    if (!outputExcelDoc.save())
    {
        emit printLog(QString::fromWCharArray(L"保存输出表格失败"));
        return;
    }

    emit printLog(QString::fromWCharArray(L"报考表格保存到：%1，点击\"打开报考表格目录\"按钮进行查看").arg(destExcelFilePath));
}

void BaoKaoThread::getScorePosRange(int& min, int& max)
{
    const auto& filterSetting = SettingManager::getInstance()->m_filterSetting;
    if (filterSetting.m_isWuHua)
    {
        if (filterSetting.m_wuLiScorePos < filterSetting.m_totalScorePos
                || filterSetting.m_huaXueScorePos < filterSetting.m_totalScorePos)
        {
            max = filterSetting.m_totalScorePos + 25000;
            min = qMax(filterSetting.m_wuLiScorePos, filterSetting.m_huaXueScorePos)-10000;
            min = qMax(min, 1);
        }
        else
        {
            max = filterSetting.m_totalScorePos + 30000;
            min = filterSetting.m_totalScorePos - 10000;
            min = qMax(min, 1);
        }
    }
    else
    {
        max = filterSetting.m_totalScorePos + 60000;
        min = filterSetting.m_totalScorePos - 6000;
        min = qMax(min, 1);
    }
}

BaoKaoController::BaoKaoController(QObject *parent)
    : QObject{parent}
{

}

void BaoKaoController::run()
{
    m_baoKaoThread = new BaoKaoThread();
    connect(m_baoKaoThread, &BaoKaoThread::printLog, this, &BaoKaoController::printLog);
    connect(m_baoKaoThread, &BaoKaoThread::finished, this, &BaoKaoController::onThreadFinish);
    m_baoKaoThread->start();
}

void BaoKaoController::onThreadFinish()
{
    bool success = m_baoKaoThread->m_success;
    m_baoKaoThread->deleteLater();
    m_baoKaoThread = nullptr;
    emit runFinish(success);
}
