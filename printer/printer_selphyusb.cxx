#include "printer_selphyusb.h"
#include <libusb.h>
#include <QDebug>
#include "printJob.h"
#include <QTemporaryFile>
#include <QTimer>
#include <QPainter>

#define USB_SUBCLASS_PRINTER            0x1
#define USB_INTERFACE_PROTOCOL_BIDIR    0x2
#define USB_INTERFACE_PROTOCOL_IPP      0x4
#define URB_XFER_SIZE  (64*1024)

printerSelphyUsb::printerSelphyUsb()
{
    mVid = 0x04a9;
    mPids.insert(0x327b, "SELPHY CP820");
    mPids.insert(0x327a, "SELPHY CP910");
    mPids.insert(0x32ae, "SELPHY CP1000");
    mPids.insert(0x32b1, "SELPHY CP1200");
    mPids.insert(0x32db, "SELPHY CP1300");
    mXferTimeout = 15000;
    mInterface = -1;
    mDev = NULL;
    mCtx = NULL;
    mTimer = new QTimer();
    mTimer->setInterval(1000);
    mTimer->setSingleShot(false);
    connect(mTimer, SIGNAL(timeout()), this, SLOT(processPrintJob()));
    mErrorState = false;
}

printerSelphyUsb::~printerSelphyUsb()
{
    if(mInterface >= 0) {
        libusb_release_interface(mDev, mInterface);
    }
    if(mDev) {
        libusb_close(mDev);
    }
    if(mCtx) {
        libusb_exit(mCtx);
    }
    mTimer->stop();
    mTimer->deleteLater();
}

bool printerSelphyUsb::canPrintFiles()
{
    return true;
}

bool printerSelphyUsb::printImage(QPixmap image, int numcopies)
{
    return prepareSpoolData(image, numcopies);
}

bool printerSelphyUsb::initPrinter()
{
    struct libusb_device **list = NULL;

    uint8_t iface;
    uint8_t altset;
    uint8_t endp_up;
    uint8_t endp_down;
    int ret;
    int found = -1;

    ret = libusb_init(&mCtx);
    if(ret) {
        qDebug() << "Error initializing libusb";
        goto done;
    }

    found = libusb_get_device_list(mCtx, &list);

    for(int i=0; i<found; i++) {
        struct libusb_device_descriptor desc;
        libusb_get_device_descriptor(list[i], &desc);
        if(desc.idVendor != mVid)
            continue;
        foreach(quint16 pid, mPids.keys()) {
            if(desc.idProduct == pid) {
                struct libusb_config_descriptor *config = NULL;
                struct libusb_device_handle *dev;

                if(libusb_open(list[i], &dev)) {
                    qDebug() << "Error opening device: " << desc.idVendor << desc.idProduct;
                    goto done;
                }

                if(libusb_get_active_config_descriptor(list[i], &config)) {
                    qDebug() << "Error getting active config descriptor";
                    libusb_close(dev);
                    goto done;
                }

                for(iface = 0; iface < config->bNumInterfaces; iface++) {
                    qDebug() << "Checking iface: " << iface;
                    for(altset = 0; altset < config->interface[iface].num_altsetting; altset++) {
                        qDebug() << "Checking altset: " << altset;
                        if(config->interface[iface].altsetting[altset].bNumEndpoints < 2) {
                            qDebug() << "Not enough endpoints.";
                            continue;
                        }
                        /* Explicitly exclude IPP-over-USB interfaces */
                        if (desc.bDeviceClass == LIBUSB_CLASS_PER_INTERFACE &&
                            config->interface[iface].altsetting[altset].bInterfaceClass == LIBUSB_CLASS_PRINTER &&
                            config->interface[iface].altsetting[altset].bInterfaceSubClass == USB_SUBCLASS_PRINTER &&
                            config->interface[iface].altsetting[altset].bInterfaceProtocol == USB_INTERFACE_PROTOCOL_IPP) {
                            qDebug() << "IPP-over-USB interface.";
                            continue;
                        }

                        endp_up = 0;
                        endp_down = 0;
                        for(int ep=0; ep < config->interface[iface].altsetting[altset].bNumEndpoints; ep++) {
                            qDebug() << "Checking ep: " << ep;
                            if ((config->interface[iface].altsetting[altset].endpoint[ep].bmAttributes & LIBUSB_TRANSFER_TYPE_MASK) == LIBUSB_TRANSFER_TYPE_BULK) {
                                if (config->interface[iface].altsetting[altset].endpoint[ep].bEndpointAddress & LIBUSB_ENDPOINT_IN) {
                                    endp_up = config->interface[iface].altsetting[altset].endpoint[ep].bEndpointAddress;
                                    qDebug() << "Getting address up" << endp_up;
                                } else {
                                    endp_down = config->interface[iface].altsetting[altset].endpoint[ep].bEndpointAddress;
                                    qDebug() << "Getting address down" << endp_down;
                                }
                            }
                            qDebug() << "endp_up:" << endp_up;
                            qDebug() << "endp_down: " << endp_down;
                            if(endp_up && endp_down) {

                                qDebug() << "Got both endpoint descriptors.";
                                if(libusb_kernel_driver_active(dev, iface)) {
                                    qDebug() << "Detaching kernel driver...";
                                    libusb_detach_kernel_driver(dev, iface);
                                }

                                ret = libusb_claim_interface(dev, iface);
                                if(ret || ret == LIBUSB_ERROR_BUSY) {
                                    qDebug() << "Failed to claim interface";
                                    libusb_free_config_descriptor(config);
                                    libusb_release_interface(dev,  iface);
                                    libusb_close(dev);
                                    goto done;
                                }

                                qDebug() << "Successfully claimed interface.";

                                if(config->interface[iface].num_altsetting > 1) {
                                    if(libusb_set_interface_alt_setting(dev, iface, altset)) {
                                        qDebug() << "Error setting interface alternative";
                                        libusb_free_config_descriptor(config);
                                        libusb_release_interface(dev, iface);
                                        libusb_close(dev);
                                        goto done;
                                    }
                                }

                                qDebug() << "Got all interface details and claimed interface";
                                mDev = dev;
                                mEndpointUp = endp_up;
                                mEndpointDown = endp_down;
                                mInterface = iface;
                                mAltSetting = altset;
                                libusb_free_config_descriptor(config);
                                libusb_free_device_list(list, 1);
                                getStatus(true);
                                mJobState = JOB_STATE_IDLE;
                                mTimer->start();
                                return true;
                            }
                        }
                    }
                }
            }
        }
    }

done:
    if(list)
        libusb_free_device_list(list, 1);
    libusb_exit(mCtx);
    mCtx = NULL;
    return false;

}

void printerSelphyUsb::processPrintJob() {



    if(mPrintJobs.isEmpty()) {
        QStringList status = getStatusAndError();
        qDebug() << status;
        if(status.at(0) != "ok") {
            if(!mErrorState) {
                mErrorState = true;
                emit printerError("media", tr("The printer is either out of paper or out of ribbon and needs your attention!"));
            }
            return;
        }

        mErrorState = false;
        return;
    }

    // We call this function periodcally and wait for the printer to become idle
    // this way

    switch(mJobState) {
    case JOB_STATE_IDLE:
    {
        if(!mErrorState) {
            // Read status to clear last state
            getStatus();
        }
        // Wait for printer idle
        QString status = getStatus();
        if(status != "ok") {
            if(!mErrorState) {
                mErrorState = true;
                emit printerError("media", tr("The printer is either out of paper or out of ribbon and needs your attention!"));
            }
            return;
        }
        mTimer->stop();
        // The printer is idle now, we're good to go!
        mErrorState = false;

        printJob &job = mPrintJobs.first();


        QFile file(job.getSpoolFile());
        if(!file.open(QIODevice::ReadOnly)) {
            qDebug() << "Error opening spool file!";
            mJobState = JOB_STATE_IDLE;
        }
        QByteArray data = file.readAll();
        file.close();

        qDebug() << "Sending spool data...";
        int chunk = 256*1024;
        int sent = 0;
        while(chunk > 0) {
            if(!writeData(data.mid(sent, chunk))) {
                qDebug() << "Error sending chunk!";
                emit printerError("comm", tr("Error communicating with the printer"));
                mPrintJobs.removeFirst();
                mTimer->start();
                mJobState = JOB_STATE_IDLE;
                return;
            }
            sent += chunk;
            chunk = data.size() - sent;
            if(chunk > 256*1024)
                chunk = 256*1024;
        }
        qDebug() << "Data spooled, waiting for completion; JOB_STATE_DATA_SENT";
        // Clear previous state
        getStatus();
        mJobState = JOB_STATE_DATA_SENT;
        mTimer->start();
        break;
    }
    case JOB_STATE_DATA_SENT:
    {
        // Wait for printer idle
        QStringList status = getStatusAndError();
        if(status.at(0) != "ok") {
            if(!mErrorState) {
                mErrorState = true;
                emit printerError("media", tr("The printer is either out of paper or out of ribbon and needs your attention!"));
            }
            break;
        }
        mErrorState = false;
        qDebug() << status.at(1);
        if(status.at(1) != "idle")
            break;

        // The printer is idle now, we're good to go!
        mJobState = JOB_STATE_DONE;
        break;
    }
    case JOB_STATE_DONE:
        mPrintJobs.first().setCopiesPrinted(mPrintJobs.first().getCopiesPrinted() + 1);
        qDebug() << "So far, I printed " << mPrintJobs.first().getCopiesPrinted() << " copies.";
        if(mPrintJobs.first().getCopiesPrinted() >= mPrintJobs.first().getCopies()) {
            qDebug() << "Removing job as all copies have been printed.";
            QFile file(mPrintJobs.first().getSpoolFile());
            if(file.exists())
                file.remove();
            mPrintJobs.removeFirst();
        }
        mJobState = JOB_STATE_IDLE;
    }
}

bool printerSelphyUsb::printFile(QString filename, int numcopies)
{
    QPixmap image(filename);
    return printImage(image, numcopies);
}

bool printerSelphyUsb::prepareSpoolData(QPixmap image, int numcopies) {
    printJob job;
    int size_x = 1872;
    int size_y = 1248;
    int off_x = 0;
    int off_y = 0;
    uint8_t header[32] = {0x0f, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00,
                          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
                          0x01, 0x00, 0x50, 0x00, 0x00, 0x00, 0x00, 0x01,
                          0xe0, 0x04, 0x00, 0x00, 0x50, 0x07, 0x00, 0x00};
    // header[18]
    // 0x50 = P size -> e0 04   50 07   1248x1872
    // 0x4c = L size -> 80 04   c0 05   1152x1472
    // 0x32 = C size -> 40 04   9c 02   1088x668

    // header[23]
    // 0x00 = Y'CbCr data
    // 0x01 = CMY data

    QImage scaled = image.scaled(size_x, size_y, Qt::KeepAspectRatio, Qt::SmoothTransformation).toImage();

    if(scaled.width() < size_x) {
        off_x = (size_x - scaled.width()) / 2;
    }
    if(scaled.height() < size_y) {
        off_y = (size_y - scaled.height()) / 2;
    }

    QPixmap spoolImage(size_x, size_y);
    QPainter painter(&spoolImage);
    painter.setCompositionMode(QPainter::CompositionMode_Source);
    painter.fillRect(spoolImage.rect(), Qt::white);
    painter.drawImage(off_x, off_y, scaled);
    painter.end();

    QImage spooledImage = spoolImage.toImage();

    QFile file(job.getSpoolFile());
    if(!file.open(QIODevice::ReadWrite)) {
        return false;
    }

    job.setCopies(numcopies);

    QByteArray planeCyan(size_x * size_y, '\0');
    QByteArray planeMagenta(size_x * size_y, '\0');
    QByteArray planeYellow(size_x * size_y, '\0');

    file.write((char*)header, 32);

    for(int x=0; x<size_x; x++) {
        for(int y=0; y<size_y; y++) {
            QColor rgb = spooledImage.pixel(x, y);
            int cyan = 255 - rgb.red();
            int magenta = 255 - rgb.green();
            int yellow = 255 - rgb.blue();
            int newx = x;
            int newy = size_y - y - 1;
            planeCyan[newx*size_y + newy] = (char)(cyan & 0xff);
            planeMagenta[newx*size_y + newy] = (char)(magenta & 0xff);
            planeYellow[newx*size_y + newy] = (char)(yellow & 0xff);
        }
    }

    file.write(planeCyan);
    file.write(planeMagenta);
    file.write(planeYellow);

    file.close();

    qDebug() << "Created spool file: " << file.fileName();
    mPrintJobs.append(job);
    return true;
}

bool printerSelphyUsb::resetStatus() {
    const char data[12] = {0x40, 0x10, 0x00, 0x00,
                           0x00, 0x00, 0x00, 0x00,
                           0x00, 0x00, 0x00, 0x00};
    QByteArray cmd = QByteArray::fromRawData(data, 12);
    return writeData(cmd);
}

QString printerSelphyUsb::parseError(uint8_t err) {
    switch(err) {
    case 0x00:
        return "ok";
    case 0x02:
        return "paper feed";
    case 0x03:
        return "no paper";
    case 0x05:
        return "incorrect paper";
    case 0x06:
        return "ink cassette empty";
    case 0x07:
        return "no ink";
    case 0x09:
        return "no paper and ink";
    case 0x0a:
        return "incorrect media for job";
    case 0x0b:
        return "paper jam";
    default:
        return "unknown error";
    }
}

QString printerSelphyUsb::parseStatus(uint8_t status) {
    switch(status) {
    case 0x01:
        return "idle";
    case 0x02:
        return "feeding paper";
    case 0x04:
        return "printing yellow";
    case 0x08:
        return "printing magenta";
    case 0x10:
        return "printing cyan";
    case 0x20:
        return "printing laminate";
    default:
        return "unknown";
    }
}

QStringList printerSelphyUsb::getStatusAndError() {
    QStringList ret;
    if(mInterface < 0) {
        ret.append("error");
        ret.append("interface not defined");
        return ret;
    }
    QByteArray data = readData();
    if(data.isEmpty()) {
        ret.append("error");
        ret.append("no data received");
        return ret;
    }

    qDebug() << "Printer State: " << QString("%1").arg((quint8)data.at(0), 0, 16);
    qDebug() << "Media type: " << QString("%1").arg((quint8)data.at(6), 0, 16);
    switch(data.at(6)) {
    case 0x00:
        ret.append("error");
        ret.append("paper and ribbon missing");
        return ret;
    case 0x01:
        ret.append("error");
        ret.append("paper missing");
        return ret;
    case 0x10:
        ret.append("error");
        ret.append("ribbon missing");
        return ret;
    }

    if(data.at(2)) {
        qDebug() << "Printer Error: " << QString("%1").arg((quint8)data.at(2), 0, 16);
        ret.append("error");
        ret.append(parseError(data.at(2)));
        return ret;
    }
    ret.append("ok");
    ret.append(parseStatus(data.at(0)));
    return ret;
}

QString printerSelphyUsb::getStatus(bool readTwice)
{
    if(mInterface < 0) {
        return "error";
    }
    QByteArray data = readData();
    if(data.isEmpty())
        return "error";
    if(readTwice)
        data = readData();
    qDebug() << "Printer State: " << QString("%1").arg((quint8)data.at(0), 0, 16);;
    qDebug() << "Media type: " << QString("%1").arg((quint8)data.at(6), 0, 16);
    if(data.at(2)) {
        qDebug() << "Printer Error: " << QString("%1").arg((quint8)data.at(2), 0, 16);
        return parseError(data.at(2));
    }
    return "ok";
}

QByteArray printerSelphyUsb::readData()
{
    int ret;
    uint8_t data[12];
    int len = 0;

    memset(data, 0, 12);

    ret = libusb_bulk_transfer(mDev, mEndpointUp, data, 12, &len, mXferTimeout);
    if(ret < 0) {
        qDebug() << "Error reading data from printer.";
        return QByteArray();
    }
    qDebug() << data;
    qDebug() << len;
    const char *c = reinterpret_cast<const char*>(data);
    return QByteArray(c, 12);
}

bool printerSelphyUsb::writeData(const QByteArray data)
{
    int length = data.length();
    int len;
    int ret;
    int num = 0;

    const char *buf = data.constData();

    uint8_t *sendBuf = (uint8_t *)buf;

    while(length) {
        if(length > URB_XFER_SIZE)
            len = URB_XFER_SIZE;
        else
            len = length;

        ret = libusb_bulk_transfer(mDev, mEndpointDown, sendBuf, len, &num, mXferTimeout);

        if(ret < 0) {
            qDebug() << "Error sending data to printer";
            return false;
        }

        length -= num;
        sendBuf += num;
    }

    return true;
}
