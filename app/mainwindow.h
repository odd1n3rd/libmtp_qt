#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "mtpviewmodel.h"


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void updateDeviceInfo();
    void updateFileList(const QStringList &files);
    void displayError(const QString &error);
    void displayFileData(const QByteArray &data);

    void onRefreshButtonClicked();
    void onCreateDirectoryClicked();
    void onDeleteDirectoryClicked();
    void onWriteFileClicked();
    void onReadFileClicked();
    void onDeleteFileClicked();

private:
    Ui::MainWindow *ui;
    MtpViewModel *viewModel;
};
#endif // MAINWINDOW_H
