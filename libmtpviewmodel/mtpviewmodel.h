#ifndef MTPVIEWMODEL_H
#define MTPVIEWMODEL_H

#include <QObject>
#include <QStringList>
#include <QByteArray>
#include <functional>
#include "imtdevice.h"

class MtpViewModel : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool isBusy READ isBusy NOTIFY busyChanged)

public:
    explicit MtpViewModel(IMtpDevice *device, QObject *parent = nullptr);
    ~MtpViewModel();

    QString deviceInfo() const;
    QString freeSpace() const;
    QStringList fileList();
    QStringList fileList(const QString &path);


    bool isBusy() const { return m_isBusy; }

public slots:
    void refreshDevice();
    void readFile(const QString &path);
    void writeFile(const QString &path, const QByteArray &data);
    void deleteFile(const QString &path);
    void createDirectory(const QString &path);
    void deleteDirectory(const QString &path);
signals:
    void deviceUpdated();
    void fileListUpdated(const QStringList &files);
    void fileRead(const QByteArray &data);
    void operationFailed(const QString &error);
    void busyChanged(bool busy);

private:
    void runAsyncOperation(std::function<bool()> operation, const QString& successMessage, const QString& failureMessageBase);
    void refreshFileListOnly();
    void setBusy(bool busy);

    IMtpDevice *m_device;
    bool m_isBusy;
};

#endif // MTPVIEWMODEL_H
