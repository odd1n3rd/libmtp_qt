#ifndef MTPDEVICE_H
#define MTPDEVICE_H

#include <libmtp.h>
#include <QString>
#include <QStringList>
#include <QByteArray>
#include <QVector>


struct MtpDeviceInfo {
    QString friendlyName;
    QString mtpVersion;
};



class MtpDevice {
public:
    static QVector<MtpDeviceInfo> detectDevices();

    static QString getDeviceVersion();
    static QString getDeviceInfo();
    static quint64 getFreeSpace();

    static QStringList getFileList(const QString &path = "/");
    static bool readFile(const QString &path, QByteArray &data);
    static bool writeFile(const QString &path, const QByteArray &data);
    static bool deleteFile(const QString &path);
    static bool createDirectory(const QString &path);
    static bool deleteDirectory(const QString &path);

private:
    static LIBMTP_mtpdevice_t* openFirstDevice(LIBMTP_raw_device_t*& rawDevices);

    static void cleanUp(LIBMTP_mtpdevice_t *device = nullptr, LIBMTP_raw_device_t *raw_devices = nullptr, LIBMTP_file_t *file = nullptr, void *data_ptr = nullptr);

};

#endif // MTPDEVICE_H
