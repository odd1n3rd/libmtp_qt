#ifndef MTPDEVICEADAPTER_H
#define MTPDEVICEADAPTER_H

#include "imtdevice.h"
#include "mtpdevice.h"

class MtpDeviceAdapter : public IMtpDevice {
public:
    QVector<MtpDeviceInfo> detectDevices() override { return MtpDevice::detectDevices(); }
    QString getDeviceVersion() override { return MtpDevice::getDeviceVersion(); }
    QString getDeviceInfo() override { return MtpDevice::getDeviceInfo(); }
    quint64 getFreeSpace() override { return MtpDevice::getFreeSpace(); }
    QStringList getFileList(const QString &path = "/") override { return MtpDevice::getFileList(path); }
    bool readFile(const QString &path, QByteArray &data) override { return MtpDevice::readFile(path, data); }
    bool writeFile(const QString &path, const QByteArray &data) override { return MtpDevice::writeFile(path, data); }
    bool deleteFile(const QString &path) override { return MtpDevice::deleteFile(path); }
    bool createDirectory(const QString &path) override { return MtpDevice::createDirectory(path); }
    bool deleteDirectory(const QString &path) override { return MtpDevice::deleteDirectory(path); }
};

#endif // MTPDEVICEADAPTER_H
