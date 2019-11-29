#ifndef RecvMessage_H
#define RecvMessage_H

#include "common.h"
#include "writelog.h"

#include <QThread>

#include <decaf/lang/Thread.h>
#include <decaf/lang/Runnable.h>
#include <decaf/util/concurrent/CountDownLatch.h>
#include <activemq/core/ActiveMQConnectionFactory.h>
#include <activemq/core/ActiveMQConnection.h>
#include <activemq/transport/DefaultTransportListener.h>
#include <activemq/library/ActiveMQCPP.h>
#include <decaf/lang/Integer.h>
#include <activemq/util/Config.h>
#include <decaf/util/Date.h>
#include <cms/Connection.h>
#include <cms/Session.h>
#include <cms/TextMessage.h>
#include <cms/BytesMessage.h>
#include <cms/MapMessage.h>
#include <cms/ExceptionListener.h>
#include <cms/MessageListener.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>

using namespace activemq;
using namespace activemq::core;
using namespace activemq::transport;
using namespace decaf::lang;
using namespace decaf::util;
using namespace decaf::util::concurrent;
using namespace cms;
using namespace std;

class PushMessage : public QThread
{
    Q_OBJECT
public:
    PushMessage(QString logDir = nullptr, QWidget *parent = nullptr);
    ~PushMessage();

public:
    void setup(QString, QString, QString, QString, bool, bool);

private:
    Connection *connection;
    Session *session;
    Destination *destination;
    MessageProducer *producer;

    void cleanUp();
    QString logdir;

    WriteLog *log;
};

#endif // RecvMessage_H
