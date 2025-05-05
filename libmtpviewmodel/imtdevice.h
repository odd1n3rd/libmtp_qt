#ifndef IMTPDEVICE_H
#define IMTPDEVICE_H

#include <QString>
#include <QStringList>
#include <QByteArray>
#include <QVector>

struct MtpDeviceInfo {
    QString friendlyName;
    QString mtpVersion;
};

class IMtpDevice {
public:
    virtual ~IMtpDevice() = default;

    virtual QVector<MtpDeviceInfo> detectDevices() = 0;
    virtual QString getDeviceVersion() = 0;
    virtual QString getDeviceInfo() = 0;
    virtual quint64 getFreeSpace() = 0;

    virtual QStringList getFileList(const QString &path = "/") = 0;
    virtual bool readFile(const QString &path, QByteArray &data) = 0;
    virtual bool writeFile(const QString &path, const QByteArray &data) = 0;
    virtual bool deleteFile(const QString &path) = 0;
    virtual bool createDirectory(const QString &path) = 0;
    virtual bool deleteDirectory(const QString &path) = 0;
};

#endif // IMTPDEVICE_H
