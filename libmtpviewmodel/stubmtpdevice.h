#ifndef STUBMTPDEVICE_H
#define STUBMTPDEVICE_H

#include "imtdevice.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>

class StubMtpDevice : public IMtpDevice {
public:
    StubMtpDevice() {
        QDir().mkpath("/tmp/mtp_sim");
    }

    QVector<MtpDeviceInfo> detectDevices() override {
        return {{"Stub Device", "1.0"}};
    }
    QString getDeviceVersion() override { return "Stub MTP 1.0"; }
    QString getDeviceInfo() override { return "Stub Device"; }
    quint64 getFreeSpace() override {
        return 1024 * 1024 * 1024; // 1GB
    }

    QStringList getFileList(const QString &path = "/") override {
        QString basePath = "/tmp/mtp_sim";
        QString relPath = path;
        if (relPath.startsWith("/")) relPath = relPath.mid(1);
        QString fullPath = basePath + (relPath.isEmpty() ? "" : "/" + relPath);

        QStringList result;
        QDir dir(fullPath);
        QFileInfoList entries = dir.entryInfoList(QDir::NoDotAndDotDot | QDir::AllEntries);
        for (const QFileInfo &fi : entries) {
            QString entryPath = (path.endsWith("/") ? path : path + "/") + fi.fileName();
            if (fi.isDir()) {
                result << entryPath + "/";
                result << getFileList(entryPath + "/");
            } else {
                result << entryPath;
            }
        }
        return result;
    }

    bool readFile(const QString &path, QByteArray &data) override {
        QString fullPath = makeFullPath(path);
        QFile file(fullPath);
        if (!file.exists() || !file.open(QIODevice::ReadOnly))
            return false;
        data = file.readAll();
        return true;
    }

    bool writeFile(const QString &path, const QByteArray &data) override {
        QString fullPath = makeFullPath(path);
        QDir().mkpath(QFileInfo(fullPath).absolutePath());
        QFile file(fullPath);
        if (!file.open(QIODevice::WriteOnly))
            return false;
        file.write(data);
        return true;
    }

    bool deleteFile(const QString &path) override {
        QString fullPath = makeFullPath(path);
        QFile file(fullPath);
        return file.remove();
    }

    bool createDirectory(const QString &path) override {
        QString fullPath = makeFullPath(path);
        return QDir().mkpath(fullPath);
    }

    bool deleteDirectory(const QString &path) override {
        QString fullPath = makeFullPath(path);
        QDir dir(fullPath);
        return dir.removeRecursively();
    }

private:
    QString makeFullPath(const QString &path) const {
        QString basePath = "/tmp/mtp_sim";
        QString relPath = path;
        if (relPath.startsWith("/")) relPath = relPath.mid(1);
        return basePath + (relPath.isEmpty() ? "" : "/" + relPath);
    }
};

#endif // STUBMTPDEVICE_H
