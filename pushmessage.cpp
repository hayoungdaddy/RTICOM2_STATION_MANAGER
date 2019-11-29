#include "pushmessage.h"

PushMessage::PushMessage(QString logDir, QWidget *parent)
{
    log = new WriteLog();
    logdir = logDir;
}

PushMessage::~PushMessage()
{
    //cleanUp();
}

void PushMessage::setup(QString qBrokerURI, QString qID, QString qPW, QString qTopic, bool useTopic, bool clientAck)
{
    auto_ptr<ConnectionFactory> connectionFactory(ConnectionFactory::createCMSConnectionFactory( qBrokerURI.toStdString() ) );

    if(qID == "" || qPW == "")
    {
        connection = connectionFactory->createConnection();
    }
    else
    {
        connection = connectionFactory->createConnection(qID.toStdString(), qPW.toStdString());
    }

    connection->start();

    // Create a Session
    session = connection->createSession( Session::AUTO_ACKNOWLEDGE );

    useTopic = true;

    // Create the destination (Topic or Queue)
    if( useTopic ) {
        destination = session->createTopic( qTopic.toStdString() );
    } else {
        destination = session->createQueue( qTopic.toStdString() );
    }

    producer = session->createProducer(destination);
    producer->setDeliveryMode(DeliveryMode::NON_PERSISTENT);

    BytesMessage *message4send;
    message4send = session->createBytesMessage();

    _MESSAGE msg;
    msg.time = QDateTime::currentDateTimeUtc().toTime_t();
    msg.MType = 'S';
    msg.value = 1;

    message4send->setBodyBytes((const unsigned char *)&msg, sizeof(_MESSAGE));

    try {
        producer->send(message4send);
    } catch (CMSException &e) {
        e.printStackTrace();
        log->write(logdir, "Can't send a message to " + qBrokerURI + " topic : " + qTopic);
    }

    this->terminate();
}

void PushMessage::cleanUp()
{
    if (connection != nullptr)
        connection->close();

    delete destination;
    destination = nullptr;
    delete producer;
    producer = nullptr;
    delete session;
    session = nullptr;
    delete connection;
}
