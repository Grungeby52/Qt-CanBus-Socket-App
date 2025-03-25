#include <QCoreApplication>
#include <QCanBus>
#include <QCanBusDevice>
#include <QCanBusFrame>
#include <QObject>
#include <QDebug>
#include <QCanFrameProcessor>
#include <QCanDbcFileParser>
#include <QCanUniqueIdDescription>
#include <QCanMessageDescription>
#include <QCanSignalDescription>
#include <QAbstractListModel>
#include <QVariant>
class CanReceiver : public QObject {
    Q_OBJECT

public:
    CanReceiver(const QString &interface, QObject *parent = nullptr)
        : QObject(parent), device(QCanBus::instance()->createDevice("socketcan", interface)) {
        if (!device) {
            qWarning() << "CAN bus arayüzü oluşturulamadı!";
            return;
        }

        connect(device, &QCanBusDevice::framesReceived, this, &CanReceiver::onFramesReceived);

        if (!device->connectDevice()) {
            qWarning() << "Cihaz bağlanamadı!";
            delete device;
            return;
        }
        // DBC dosyasını yükle
        if (!dbcFileParser.parse("CAN3_All.dbc")) {
            qDebug() << "DBC dosyası yüklenemedi:" << dbcFileParser.errorString();
        } else {
            qDebug() << "DBC dosyası başarıyla yüklendi.";
            frameProcessor.setUniqueIdDescription(QCanDbcFileParser::uniqueIdDescription());
            frameProcessor.setMessageDescriptions(dbcFileParser.messageDescriptions());

            foreach (auto msg, frameProcessor.messageDescriptions()) {
                qDebug()<<msg.name()<<msg.uniqueId();
            }
        }

        qDebug() << "CAN alıcı başlatıldı.";
    }

    ~CanReceiver() {
        if (device) {
            device->disconnectDevice();
            delete device;
        }
    }

private slots:
    void onFramesReceived() {
        while (device->framesAvailable()) {
            QCanBusFrame frame = device->readFrame();
            frame.setExtendedFrameFormat(true);
            qDebug() << "Alınan frame:" << frame.toString();
            // Frame'i çözümle
            auto result = frameProcessor.parseFrame(frame);
            if (result.signalValues.isEmpty()) {

                qDebug() << "Çözümleme başarısız veya mesaj DBC dosyasına uygun değil."<<"  "<<result.uniqueId;
                return;
            }

            // Signal değerlerini döndür
            qDebug() << "Mesaj ID'si:" << result.uniqueId;

            for (const auto &signal : result.signalValues.keys()) {
                qDebug() << "Signal:" << signal << "Value:" << result.signalValues.value(signal).toDouble();
            }
        }
    }

private:
    QCanBusDevice *device;
    QCanFrameProcessor frameProcessor; // CAN mesajlarını işlemek için
    QCanDbcFileParser dbcFileParser; // DBC dosyası için parser

};

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);

    CanReceiver receiver("vcan0"); // 'can0' arayüzünü kullanarak alıcıyı başlat

    return a.exec();
}

#include "main.moc"
