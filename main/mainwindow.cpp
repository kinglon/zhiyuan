#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDateTime>
#include "settingmanager.h"
#include "uiutil.h"
#include "baokaocontroller.h"
#include <QDesktopServices>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setWindowFlags(windowFlags() & ~Qt::WindowMaximizeButtonHint);
    setWindowFlag(Qt::MSWindowsFixedSizeDialogHint, true);

    initCtrls();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::initCtrls()
{
    ui->comboBoxSex->addItem(QString::fromWCharArray(L"男"));
    ui->comboBoxSex->addItem(QString::fromWCharArray(L"女"));
    ui->comboBoxSex->setCurrentIndex(0);

    connect(ui->pushButtonBaoKao, &QPushButton::clicked, this, &MainWindow::onBaoKaoButtonClicked);
    connect(ui->pushButtonOpenExcelPath, &QPushButton::clicked, [](){
        QDesktopServices::openUrl(QUrl(SettingManager::getInstance()->m_excelSavedPath));
    });
}

void MainWindow::updateButtonStatus()
{
    ui->pushButtonBaoKao->setEnabled(!m_doing);
}

void MainWindow::onPrintLog(QString content)
{
    static int lineCount = 0;
    if (lineCount >= 1000)
    {
        ui->textEditLog->clear();
        lineCount = 0;
    }
    lineCount++;

    qInfo(content.toStdString().c_str());
    QDateTime currentDateTime = QDateTime::currentDateTime();
    QString currentTimeString = currentDateTime.toString("[MM-dd hh:mm:ss] ");
    QString line = currentTimeString + content;
    ui->textEditLog->append(line);
}

void MainWindow::onBaoKaoButtonClicked()
{
    FilterSetting filterSetting;
    filterSetting.m_name = ui->lineEditName->text();
    if (filterSetting.m_name.isEmpty())
    {
        UiUtil::showTip(QString::fromWCharArray(L"请输入姓名"));
        return;
    }
    filterSetting.m_sex = ui->comboBoxSex->currentText()==0?FilterSetting::BOY:FilterSetting::GIRL;

    bool ok = false;
    filterSetting.m_totalScorePos = ui->lineEditTotalScore->text().toInt(&ok);
    if (!ok || filterSetting.m_totalScorePos <= 0)
    {
        UiUtil::showTip(QString::fromWCharArray(L"请正确输入总分位次"));
        return;
    }

    filterSetting.m_isWuHua = ui->radioButtonWuHua->isChecked();
    if (filterSetting.m_isWuHua)
    {
        filterSetting.m_wuLiScorePos = ui->lineEditWuLiScore->text().toInt(&ok);
        if (!ok || filterSetting.m_wuLiScorePos <= 0)
        {
            UiUtil::showTip(QString::fromWCharArray(L"请正确输入物理位次"));
            return;
        }

        filterSetting.m_huaXueScorePos = ui->lineEditHuaXueScore->text().toInt(&ok);
        if (!ok || filterSetting.m_huaXueScorePos <= 0)
        {
            UiUtil::showTip(QString::fromWCharArray(L"请正确输入化学位次"));
            return;
        }
    }
    else
    {
        filterSetting.m_notWuHuaScorePos = ui->lineEditNotWuHuaScore->text().toInt(&ok);
        if (!ok || filterSetting.m_notWuHuaScorePos <= 0)
        {
            UiUtil::showTip(QString::fromWCharArray(L"请正确输入非物化组合位次"));
            return;
        }
    }

    SettingManager::getInstance()->m_filterSetting = filterSetting;

    m_doing = true;

    BaoKaoController* controller = new BaoKaoController();
    connect(controller, &BaoKaoController::printLog, this, &MainWindow::onPrintLog);
    connect(controller, &BaoKaoController::runFinish, [this, controller](bool success) {
        m_doing = false;
        if (success)
        {
            ui->pushButtonOpenExcelPath->setEnabled(true);
        }
        controller->deleteLater();
    });
    controller->run();
}
