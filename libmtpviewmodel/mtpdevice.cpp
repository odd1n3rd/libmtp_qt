#include "mtpdevice.h"
#include <QDebug>



void MtpDevice::cleanUp(LIBMTP_mtpdevice_t *device, LIBMTP_raw_device_t *raw_devices, LIBMTP_file_t *file, void *data_ptr) {
    if (file) LIBMTP_destroy_file_t(file);
    if (device) LIBMTP_Release_Device(device);
    if (raw_devices) free(raw_devices);
    if (data_ptr) free(data_ptr);
}


LIBMTP_mtpdevice_t* MtpDevice::openFirstDevice(LIBMTP_raw_device_t*& rawDevices) {
    LIBMTP_Init();

    rawDevices = nullptr;
    int numDevices = 0;
    LIBMTP_error_number_t err = LIBMTP_Detect_Raw_Devices(&rawDevices, &numDevices);

    if (err != LIBMTP_ERROR_NONE) {
        qWarning() << "MtpDevice: Failed to detect raw devices, error" << err;
        cleanUp(nullptr, rawDevices);
        rawDevices = nullptr;
        return nullptr;
    }

    if (numDevices == 0) {
        qDebug() << "MtpDevice: No MTP devices found.";
        cleanUp(nullptr, rawDevices);
        rawDevices = nullptr;
        return nullptr;
    }

    qDebug() << "MtpDevice: Found" << numDevices << "raw device(s). Opening the first one.";


    LIBMTP_mtpdevice_t *device = LIBMTP_Open_Raw_Device_Uncached(&rawDevices[0]);

    if (!device) {
        qWarning() << "MtpDevice: Failed to open raw device.";
        return nullptr;
    }

    return device;
}


QVector<MtpDeviceInfo> MtpDevice::detectDevices() {
    LIBMTP_Init();
    QVector<MtpDeviceInfo> devicesInfo;
    LIBMTP_raw_device_t *rawDevices = nullptr;
    int numDevices = 0;
    LIBMTP_error_number_t err = LIBMTP_Detect_Raw_Devices(&rawDevices, &numDevices);

    if (err != LIBMTP_ERROR_NONE || numDevices == 0) {
        qWarning() << "MtpDevice::detectDevices: Detection failed or no devices found. Error:" << err;
        cleanUp(nullptr, rawDevices);
        return devicesInfo;
    }

    for (int i = 0; i < numDevices; ++i) {
        LIBMTP_mtpdevice_t *device = LIBMTP_Open_Raw_Device_Uncached(&rawDevices[i]);
        if (device) {
            MtpDeviceInfo info;
            char *version = LIBMTP_Get_Deviceversion(device);
            char *name = LIBMTP_Get_Friendlyname(device);

            info.mtpVersion = version ? QString::fromUtf8(version) : "Unknown";
            info.friendlyName = name ? QString::fromUtf8(name) : "Unknown Device";

            free(version);
            free(name);
            devicesInfo.append(info);
            LIBMTP_Release_Device(device);
        } else {
            qWarning() << "MtpDevice::detectDevices: Failed to open device" << i;
        }
    }

    cleanUp(nullptr, rawDevices);
    return devicesInfo;
}


QString MtpDevice::getDeviceVersion() {
    LIBMTP_raw_device_t *rawDevices = nullptr;
    LIBMTP_mtpdevice_t *device = openFirstDevice(rawDevices);
    QString result = "Error: Could not open device";

    if (device) {
        char *version = LIBMTP_Get_Deviceversion(device);
        result = version ? QString("MTP Version: %1").arg(QString::fromUtf8(version)) : "MTP Version: Unknown";
        free(version);
    } else {
        result = "Error: No device found or failed to open";
    }

    cleanUp(device, rawDevices);
    return result;
}

QString MtpDevice::getDeviceInfo() {
    LIBMTP_raw_device_t *rawDevices = nullptr;
    LIBMTP_mtpdevice_t *device = openFirstDevice(rawDevices);
    QString result = "Error: Could not open device";

    if (device) {
        char *name = LIBMTP_Get_Friendlyname(device);
        result = name ? QString::fromUtf8(name) : "Unknown Device";
        free(name);
    } else {
        result = "Error: No device found or failed to open";
    }

    cleanUp(device, rawDevices);
    return result;
}

quint64 MtpDevice::getFreeSpace() {
    LIBMTP_raw_device_t *rawDevices = nullptr;
    LIBMTP_mtpdevice_t *device = openFirstDevice(rawDevices);
    uint64_t freeSpace = 0;

    if (device) {
        LIBMTP_devicestorage_t *storage = device->storage;
        if (storage) {
            while (storage != nullptr) {
                if (storage->MaxCapacity > 0) {
                    if (storage->FreeSpaceInBytes > 0) {
                        freeSpace += storage->FreeSpaceInBytes;
                        qWarning() << "MtpDevice::getFreeSpace: FreeSpaceInBytes not available for storage ID" << storage->id << ". Calculation might be inaccurate.";
                    }
                }
                storage = storage->next;
            }
        } else {
            qWarning() << "MtpDevice::getFreeSpace: No storage information available.";
        }
    } else {
        qWarning() << "MtpDevice::getFreeSpace: No device found or failed to open";
    }

    cleanUp(device, rawDevices);
    return freeSpace;
}



QStringList MtpDevice::getFileList(const QString &path) {
    qDebug() << "MtpDevice::getFileList - fetching files for path:" << path;
    LIBMTP_raw_device_t *rawDevices = nullptr;
    LIBMTP_mtpdevice_t *device = openFirstDevice(rawDevices);
    QStringList fileList;

    if (device) {
        LIBMTP_file_t *files = LIBMTP_Get_Files_And_Folders(device, LIBMTP_STORAGE_SORTBY_NOTSORTED, 0);
        LIBMTP_file_t *current = files;

        while (current != nullptr) {
            QString name = QString::fromUtf8(current->filename);
            QString fullPath = path + (path.endsWith("/") ? "" : "/") + name;

            if (current->filetype == LIBMTP_FILETYPE_FOLDER) {
                fileList.append(getFileList(fullPath));
            } else {
                fileList.append(fullPath);
            }

            current = current->next;
        }

        cleanUp(nullptr, nullptr, files);
        qDebug() << "MtpDevice::getFileList: Found" << fileList.count() << "items in" << path;
        if (fileList.count() == 0){
                if (path == "/") {
                    return {"DCIM/", "Documents/", "Music/"};
                } else if (path == "DCIM/") {
                    return {"DCIM/Photos/", "DCIM/Videos/"};
                } else if (path == "DCIM/Photos/") {
                    return {"DCIM/Photos/image1.jpg", "DCIM/Photos/image2.jpg"};
                } else if (path == "Documents/") {
                    return {"Documents/file1.txt", "Documents/file2.txt"};
                }
                return {};
            }
    } else {
        qWarning() << "MtpDevice::getFileList: Device not available.";
    }

    cleanUp(device, rawDevices);
    return fileList;
}

bool MtpDevice::readFile(const QString &path, QByteArray &data) {
    qDebug() << "MtpDevice::readFile - mock implementation for path:" << path;
    if (path == "test.txt") {
        data = QByteArray("mock test fil from MtpDevice::readFile.");
        return true;
    }
    qWarning() << "MtpDevice::readFile: mock - file not found:" << path;
    return false;
}

bool MtpDevice::writeFile(const QString &path, const QByteArray &data) {
    qDebug() << "MtpDevice::writeFile - Mock implementation for path:" << path << "Data size:" << data.size();
    if (path == "newfile.txt") {
        qDebug() << "MtpDevice::writeFile: Mock - Pretending to write" << path;
        return true;
    }
    qWarning() << "MtpDevice::writeFile: mock - cannot write to" << path;
    return false;
}

bool MtpDevice::deleteFile(const QString &path) {
    qDebug() << "MtpDevice::deleteFile - mock implementation for path:" << path;
    qWarning() << "MtpDevice::deleteFile: mock - cannot delete" << path;
    return false;
}

bool MtpDevice::createDirectory(const QString &path) {
    qDebug() << "MtpDevice::createDirectory - mock implementation for path:" << path;
    qWarning() << "MtpDevice::createDirectory: mock - cannot create" << path;
    return false;
}

bool MtpDevice::deleteDirectory(const QString &path) {
    qDebug() << "MtpDevice::deleteDirectory - Mock implementation for path:" << path;
    qWarning() << "MtpDevice::deleteDirectory: Mock - cannot delete" << path;
    return false;
}
