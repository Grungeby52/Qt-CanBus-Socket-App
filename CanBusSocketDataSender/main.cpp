#include <QCoreApplication>
#include <QCanBus>
#include <QCanBusFrame>
#include <QCanBusDevice>
#include <QThread>
#include <QRandomGenerator>

uint32_t crc32(const QByteArray &data) {
    uint32_t crc = 0xFFFFFFFF;
    for (int i = 0; i < data.size(); i++) {
        crc ^= static_cast<uint8_t>(data[i]);
        for (int j = 0; j < 8; j++) {
            if (crc & 1) {
                crc = (crc >> 1) ^ 0xEDB88320;
            } else {
                crc >>= 1;
            }
        }
    }
    return ~crc;
}

QByteArray generateRandomHexData(int length) {
    QByteArray data;
    for (int i = 0; i < length; ++i) {
        // 0x00 ile 0xFF arasında rastgele bir değer oluştur
        char randomByte = static_cast<char>(QRandomGenerator::global()->bounded(256));
        data.append(randomByte);
    }
    qDebug()<<data.toHex();
    //data.append(crc32(data));
    qDebug()<<data.toHex();
    return data;
}


int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);

    // CAN bus arayüzünü al
    QCanBusDevice *device = QCanBus::instance()->createDevice("socketcan", "vcan1");

    if (!device) {
        qWarning() << "CAN bus arayüzü oluşturulamadı!";
        return -1;
    }

    // Cihazı aç
    if (!device->connectDevice()) {
        qWarning() << "Cihaz bağlanamadı!";
        delete device;
        return -1;
    }
    while (true) {
        // CAN frame oluşturma
        QCanBusFrame frame;
        frame.setExtendedFrameFormat(true);

        frame.setFrameId(0x50002); // ID'yi ayarlayın
        QByteArray data;
        data.append(0x01);
        data.append(0x80);
        data.append(0xFF);
        data.append(static_cast<char>(0));
        data.append(static_cast<char>(0));
        data.append(static_cast<char>(0));
        data.append(static_cast<char>(0));
        data.append(static_cast<char>(0));
        frame.setPayload(generateRandomHexData(8)); // Gönderilecek veriyi ayarlayın
        // Frame'i gönderme
        if (device->writeFrame(frame)) {
            qDebug() << "Frame gönderildi:" << frame.toString();
        } else {
            qWarning() << "Frame gönderilemedi!";
        }

        QThread::currentThread()->msleep(400);
    }

    //QList<QCanBusFrame> listFrame;


    // Temizlik
    device->disconnectDevice();
    delete device;

    return a.exec();
}
