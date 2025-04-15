#include "mtpviewmodel.h"
#include "mtpdevice.h"
#include <QtConcurrent>
#include <QFutureWatcher>

MtpViewModel::MtpViewModel(QObject *parent)
    : QObject(parent), m_isBusy(false)
{
    refreshDevice();
}

MtpViewModel::~MtpViewModel() {
}

void MtpViewModel::setBusy(bool busy) {
    if (m_isBusy != busy) {
        m_isBusy = busy;
        emit busyChanged(m_isBusy);
    }
}


QString MtpViewModel::deviceInfo() const {
    return MtpDevice::getDeviceInfo() + " (" + MtpDevice::getDeviceVersion() + ")";
}

QString MtpViewModel::freeSpace() const {
    quint64 bytes = MtpDevice::getFreeSpace();
    return bytes > 0 ? QString::number(bytes / (1024 * 1024)) + " MB" : "Unknown";
}

QStringList MtpViewModel::fileList() {
    return MtpDevice::getFileList();
}

void MtpViewModel::refreshDevice() {
    if (m_isBusy) {
        qDebug() << "ViewModel is busy, refresh skipped.";
        emit operationFailed("Operation skipped: Another operation is in progress.");
        return;
    }
    setBusy(true);
    qDebug() << "Refreshing device info and file list...";

    QFutureWatcher<void> *watcher = new QFutureWatcher<void>(this);
    connect(watcher, &QFutureWatcher<void>::finished, this, [this, watcher]() {
        qDebug() << "Device info and file list refresh finished.";
        emit deviceUpdated();
        emit fileListUpdated(MtpDevice::getFileList());
        setBusy(false);
        watcher->deleteLater();
    });

    QFuture<void> future = QtConcurrent::run([this]() {

        QString deviceInfo = MtpDevice::getDeviceInfo() + " (" + MtpDevice::getDeviceVersion() + ")";
        quint64 bytes = MtpDevice::getFreeSpace();
        QString freeSpace = bytes > 0 ? QString::number(bytes / (1024 * 1024)) + " MB" : "Unknown";

        qDebug() << "Device Info:" << deviceInfo;
        qDebug() << "Free Space:" << freeSpace;
    });
    watcher->setFuture(future);
    watcher->setFuture(future);
}



void MtpViewModel::runAsyncOperation(std::function<bool()> operation, const QString& successMessage, const QString& failureMessageBase) {
    if (m_isBusy) {
        qDebug() << "ViewModel is busy, operation skipped:" << failureMessageBase;
        emit operationFailed(QString("Operation skipped: Another operation is in progress. (%1)").arg(failureMessageBase));
        return;
    }
    setBusy(true);

    QFutureWatcher<bool> *watcher = new QFutureWatcher<bool>(this);
    connect(watcher, &QFutureWatcher<bool>::finished, this, [this, watcher, successMessage, failureMessageBase]() {
        bool result = watcher->result();
        if (result) {
            qDebug() << successMessage;
            refreshFileListOnly();
        } else {
            qWarning() << "Operation failed:" << failureMessageBase;
            emit operationFailed(failureMessageBase);
        }
        watcher->deleteLater();
    });

    QFuture<bool> future = QtConcurrent::run(operation);
    watcher->setFuture(future);
}

void MtpViewModel::refreshFileListOnly() {
    if (m_isBusy) {
        qDebug() << "ViewModel is busy, refresh skipped.";
        emit operationFailed("Operation skipped: Another operation is in progress.");
        return;
    }
    setBusy(true);
    qDebug() << "Refreshing file list only...";

    QFutureWatcher<QStringList> *watcher = new QFutureWatcher<QStringList>(this);
    connect(watcher, &QFutureWatcher<QStringList>::finished, this, [this, watcher]() {
        QStringList fileList = watcher->result();
        emit fileListUpdated(fileList);
        setBusy(false);
        watcher->deleteLater();
    });

    QFuture<QStringList> future = QtConcurrent::run([this]() {
        return MtpDevice::getFileList();
    });
    watcher->setFuture(future);
}


void MtpViewModel::readFile(const QString &path) {
    if (m_isBusy) {
        qDebug() << "ViewModel is busy, readFile skipped:" << path;
        emit operationFailed(QString("Operation skipped: Another operation is in progress. (Read %1)").arg(path));
        return;
    }
    setBusy(true);

    QFutureWatcher<QByteArray> *watcher = new QFutureWatcher<QByteArray>(this);
    connect(watcher, &QFutureWatcher<QByteArray>::finished, this, [this, watcher, path]() {
        QByteArray data = watcher->result();
        if (!data.isNull()) {
            qDebug() << "File read successfully:" << path;
            emit fileRead(data);
        } else {
            qWarning() << "Failed to read file:" << path;
            emit operationFailed("Failed to read file: " + path);
        }
        setBusy(false);
        watcher->deleteLater();
    });

    QFuture<QByteArray> future = QtConcurrent::run([path]() -> QByteArray {
        QByteArray fileData;
        if (MtpDevice::readFile(path, fileData)) {
            return fileData;
        } else {
            return QByteArray();
        }
    });
    watcher->setFuture(future);
}

void MtpViewModel::writeFile(const QString &path, const QByteArray &data) {
    runAsyncOperation(
        [path, data]() { return MtpDevice::writeFile(path, data); },
        "File written successfully: " + path,
        "Failed to write file: " + path
        );
}

void MtpViewModel::deleteFile(const QString &path) {
    runAsyncOperation(
        [path]() { return MtpDevice::deleteFile(path); },
        "File deleted successfully: " + path,
        "Failed to delete file: " + path
        );
}

void MtpViewModel::createDirectory(const QString &path) {
    runAsyncOperation(
        [path]() { return MtpDevice::createDirectory(path); },
        "Directory created successfully: " + path,
        "Failed to create directory: " + path
        );
}

void MtpViewModel::deleteDirectory(const QString &path) {
    runAsyncOperation(
        [path]() { return MtpDevice::deleteDirectory(path); },
        "Directory deleted successfully: " + path,
        "Failed to delete directory: " + path
        );
}
