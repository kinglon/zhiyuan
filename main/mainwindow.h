#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onPrintLog(QString content);

    void onBaoKaoButtonClicked();

private:
    void initCtrls();

    void updateButtonStatus();

private:
    Ui::MainWindow *ui;

    // 标志是否正在进行操作
    bool m_doing = false;
};
#endif // MAINWINDOW_H
