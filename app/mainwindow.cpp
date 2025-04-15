#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QInputDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , viewModel(new MtpViewModel(this))
    , fileModel(new QStandardItemModel(this))
{
    ui->setupUi(this);
    ui->fileTreeView->setModel(fileModel);
    fileModel->setHorizontalHeaderLabels({"Name"});

    connect(viewModel, &MtpViewModel::deviceUpdated, this, &MainWindow::updateDeviceInfo);
    connect(viewModel, &MtpViewModel::fileListUpdated, this, &MainWindow::updateFileList);
    connect(viewModel, &MtpViewModel::operationFailed, this, &MainWindow::displayError);
    connect(viewModel, &MtpViewModel::fileRead, this, &MainWindow::displayFileData);


    connect(ui->refreshButton, &QPushButton::clicked, this, &MainWindow::onRefreshButtonClicked);
    connect(ui->createDirButton, &QPushButton::clicked, this, &MainWindow::onCreateDirectoryClicked);
    connect(ui->deleteDirButton, &QPushButton::clicked, this, &MainWindow::onDeleteDirectoryClicked);
    connect(ui->writeFileButton, &QPushButton::clicked, this, &MainWindow::onWriteFileClicked);
    connect(ui->readFileButton, &QPushButton::clicked, this, &MainWindow::onReadFileClicked);
    connect(ui->deleteFileButton, &QPushButton::clicked, this, &MainWindow::onDeleteFileClicked);

    updateDeviceInfo();
}

MainWindow::~MainWindow()
{
    delete ui;

}

void MainWindow::updateDeviceInfo() {
    QString info = viewModel->deviceInfo() + " - Free: " + viewModel->freeSpace();
    ui->deviceInfoLabel->setText(info);
}

void MainWindow::updateFileList(const QStringList &files) {
    fileModel->clear();
    fileModel->setHorizontalHeaderLabels({"Name"});
    QStandardItem *rootItem = fileModel->invisibleRootItem();

    for (const QString &file : files) {
        QStringList parts = file.split('/');
        QStandardItem *parent = rootItem;

        for (const QString &part : parts) {
            if (part.isEmpty()) continue;

            QStandardItem *parentItem = rootItem;
            for (const QString &part : parts) {
                if (part.isEmpty()) continue;

                QStandardItem *item = nullptr;
                for (int i = 0; i < parentItem->rowCount(); ++i) {
                    QStandardItem *child = parentItem->child(i);
                    if (child->text() == part) {
                        item = child;
                        break;
                    }
                }

                if (!item) {
                    item = new QStandardItem(part);
                    parentItem->appendRow(item);
                }
                parentItem = item;
            }

        }
    }
}

void MainWindow::displayError(const QString &error) {
    QMessageBox::warning(this, "Operation Error", error);
}

void MainWindow::displayFileData(const QByteArray &data) {
    QMessageBox msgBox;
    msgBox.setWindowTitle("File Content");
    QTextStream stream(data);
    QString text = stream.readAll();
    if (data.isEmpty()) {
        text = "<Empty File>";
    }
    msgBox.setText(text);
    msgBox.exec();
}




void MainWindow::onRefreshButtonClicked() {
    viewModel->refreshDevice();
}

void MainWindow::onCreateDirectoryClicked() {
    bool ok;
    QString dirName = QInputDialog::getText(this, "Create Directory", "Enter directory name:", QLineEdit::Normal, "", &ok);
    if (ok && !dirName.isEmpty()) {
        viewModel->createDirectory(dirName);
    }
}


void MainWindow::onDeleteDirectoryClicked() {
    QModelIndex currentIndex = ui->fileTreeView->currentIndex();
    if (!currentIndex.isValid()) {
        QMessageBox::warning(this, "Error", "Select a directory to delete.");
        return;
    }

    QStandardItem *item = fileModel->itemFromIndex(currentIndex);
    if (item) {
        QString path = item->text();
        if (path.endsWith('/')) {
            viewModel->deleteDirectory(path);
        } else {
            QMessageBox::warning(this, "Error", "Please select a directory (ending with '/').");
        }
    }
}

void MainWindow::onWriteFileClicked() {
    QString filePath = QFileDialog::getOpenFileName(this, "Select File to Write", "", "All Files (*)");
    if (!filePath.isEmpty()) {
        QFile file(filePath);
        if (file.open(QIODevice::ReadOnly)) {
            QByteArray data = file.readAll();
            file.close();
            QString destPath = QFileInfo(filePath).fileName();
            viewModel->writeFile(destPath, data);
        } else {
            QMessageBox::warning(this, "Error", "Failed to open local file for reading.");
        }
    }
}

void MainWindow::onReadFileClicked() {
    QModelIndex currentIndex = ui->fileTreeView->currentIndex();
    if (!currentIndex.isValid()) {
        QMessageBox::warning(this, "Error", "Select a file to read.");
        return;
    }

    QStandardItem *item = fileModel->itemFromIndex(currentIndex);
    if (item) {
        QString path = item->text();
        if (!path.endsWith('/')) {
            viewModel->readFile(path);
        } else {
            QMessageBox::warning(this, "Error", "Please select a file, not a directory.");
        }
    }
}

void MainWindow::onDeleteFileClicked() {
    QModelIndex currentIndex = ui->fileTreeView->currentIndex();
    if (!currentIndex.isValid()) {
        QMessageBox::warning(this, "Error", "Select a file to delete.");
        return;
    }

    QStandardItem *item = fileModel->itemFromIndex(currentIndex);
    if (item) {
        QString path = item->text();
        if (!path.endsWith('/')) {
            viewModel->deleteFile(path);
        } else {
            QMessageBox::warning(this, "Error", "Please select a file, not a directory.");
        }
    }
}
