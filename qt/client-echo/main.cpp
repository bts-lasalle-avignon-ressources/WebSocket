#include <QCoreApplication>
#include <QtWebSockets>

int main(int argc, char* argv[])
{
    QCoreApplication a(argc, argv);

    QWebSocket webSocket;
    QObject::connect(&webSocket,
                     &QWebSocket::connected,
                     [&webSocket]()
                     {
                         qDebug() << Q_FUNC_INFO << "Connexion établie";
                         QString message = "Hello world!";
                         webSocket.sendTextMessage(message);
                         qDebug() << "> " << message;
                     });

    QObject::connect(&webSocket,
                     &QWebSocket::textMessageReceived,
                     [](const QString& message)
                     {
                         qDebug() << "< " << message;
                     });

    QObject::connect(&webSocket,
                     &QWebSocket::disconnected,
                     [&webSocket]()
                     {
                         qDebug() << Q_FUNC_INFO << "Connexion fermée";
                         QCoreApplication::quit();
                     });

    webSocket.open(QUrl("ws://localhost:5000/"));

    return a.exec();
}
