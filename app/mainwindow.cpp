#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QInputDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>

MainWindow::MainWindow(IMtpDevice *device, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , viewModel(new MtpViewModel(device, this))
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

QString MainWindow::getFullPath(QStandardItem *item) const {
    QStringList parts;
    while (item) {
        parts.prepend(item->text());
        item = item->parent();
    }
    QString path = parts.join('/');
    if (!path.isEmpty() && !path.endsWith('/')) {
        QVariant isDir = fileModel->itemFromIndex(fileModel->indexFromItem(fileModel->findItems(parts.last(), Qt::MatchExactly | Qt::MatchRecursive).value(0)))->data(Qt::UserRole + 2);
        if (isDir.isValid() && isDir.toBool())
            path += '/';
    }
    return path;
}

void MainWindow::updateFileList(const QStringList &files) {
    QSet<QString> allPaths;
    for (const QString &file : files) {
        QString normalized = file;
        if (normalized.startsWith('/')) normalized = normalized.mid(1);
        QStringList parts = normalized.split('/', Qt::SkipEmptyParts);
        QString path;
        for (int i = 0; i < parts.size(); ++i) {
            path += (path.isEmpty() ? "" : "/") + parts[i];
            if (i < parts.size() - 1 || file.endsWith('/')) {
                allPaths.insert(path + "/");
            } else {
                allPaths.insert(path);
            }
        }
    }

    QStringList sortedPaths = allPaths.values();
    std::sort(sortedPaths.begin(), sortedPaths.end());

    fileModel->clear();
    fileModel->setHorizontalHeaderLabels({"Name"});
    QStandardItem *rootItem = fileModel->invisibleRootItem();

    for (const QString &file : sortedPaths) {
        QString normalized = file;
        if (normalized.startsWith('/')) normalized = normalized.mid(1);
        QStringList parts = normalized.split('/', Qt::SkipEmptyParts);
        QStandardItem *parent = rootItem;
        QString fullPath;
        for (int i = 0; i < parts.size(); ++i) {
            const QString &part = parts[i];
            fullPath += (fullPath.isEmpty() ? "" : "/") + part;
            QStandardItem *item = nullptr;
            for (int j = 0; j < parent->rowCount(); ++j) {
                if (parent->child(j)->text() == part) {
                    item = parent->child(j);
                    break;
                }
            }
            if (!item) {
                item = new QStandardItem(part);
                bool isDir = (i == parts.size() - 1 && file.endsWith('/')) || (i < parts.size() - 1);
                item->setData(fullPath + (isDir ? "/" : ""), Qt::UserRole + 1);
                item->setData(isDir, Qt::UserRole + 2);
                parent->appendRow(item);
            }
            parent = item;
        }
    }
}

void MainWindow::displayError(const QString &error) {
    QMessageBox::warning(this, "Operation Error", error);
}

void MainWindow::displayFileData(const QByteArray &data) {
    QMessageBox msgBox;
    msgBox.setWindowTitle("File Content");
    QString text = QString::fromUtf8(data);
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
        viewModel->createDirectory(dirName.endsWith('/') ? dirName : dirName + '/');
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
        bool isDir = item->data(Qt::UserRole + 2).toBool();
        QString path = item->data(Qt::UserRole + 1).toString();
        if (isDir) {
            viewModel->deleteDirectory(path);
        } else {
            QMessageBox::warning(this, "Error", "Please select a directory.");
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
        bool isDir = item->data(Qt::UserRole + 2).toBool();
        QString path = item->data(Qt::UserRole + 1).toString();
        if (!isDir) {
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
        bool isDir = item->data(Qt::UserRole + 2).toBool();
        QString path = item->data(Qt::UserRole + 1).toString();
        if (!isDir) {
            viewModel->deleteFile(path);
        } else {
            QMessageBox::warning(this, "Error", "Please select a file, not a directory.");
        }
    }
}
