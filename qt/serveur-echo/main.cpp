#include <QCoreApplication>
#include <QObject>
#include <QtWebSockets>

int main(int argc, char* argv[])
{
    QCoreApplication a(argc, argv);

    QWebSocketServer webSocketServer(QString("Serveur Echo"), QWebSocketServer::NonSecureMode);
    int              port = 5000;

    if(webSocketServer.listen(QHostAddress::Any, port))
    {
        qDebug() << Q_FUNC_INFO << "Serveur en écoute sur le port" << port;
        QObject::connect(&webSocketServer,
                         &QWebSocketServer::newConnection,
                         [&webSocketServer]()
                         {
                             qDebug() << Q_FUNC_INFO << "Connexion établie";
                             QWebSocket* pWebSocket = webSocketServer.nextPendingConnection();
                             QObject::connect(pWebSocket,
                                              &QWebSocket::textMessageReceived,
                                              [pWebSocket](const QString& message)
                                              {
                                                  qDebug() << "< " << message;
                                                  pWebSocket->sendTextMessage(message);
                                                  qDebug() << "> " << message;
                                              });
                             QObject::connect(pWebSocket,
                                              &QWebSocket::disconnected,
                                              [pWebSocket]()
                                              {
                                                  qDebug() << "Connexion fermée";
                                              });
                         });
        QObject::connect(&webSocketServer,
                         &QWebSocketServer::closed,
                         []()
                         {
                             qDebug() << Q_FUNC_INFO << "Serveur fermé";
                         });
    }

    // webSocketServer.close();

    return a.exec();
}
