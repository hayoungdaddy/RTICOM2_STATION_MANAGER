#ifndef COMMON_H
#define COMMON_H

#include <QString>
#include <QDateTime>
#include <QVector>
#include <QMultiMap>
#include <QDebug>
#include <QFile>

#define MAX_QSCD_CNT 100
#define MAX_NUM_NETWORK 100
#define MAX_NUM_STATION 1000
#define MAX_NUM_EVENTINITSTA 5

typedef struct _message
{
    int time;
    char MType;
    int value;
} _MESSAGE;

typedef struct _configure
{
    QString configFileName;
    QString logDir;
    QString amq_ip, amq_port, amq_user, amq_passwd, amq_topic;
    QString db_ip, db_name, db_user, db_passwd;
} _CONFIGURE;

typedef struct _station
{
    int index;
    QString sta;
    QString comment;
    QString staType;
    double lat;
    double lon;
    double elev;
    int inUse;
} _STATION;

static QDateTime convertKST(QDateTime oriTime)
{
    oriTime.setTimeSpec(Qt::UTC);
    oriTime = oriTime.addSecs(3600 * 9);
    return oriTime;
}

static QDateTime convertUTC(QDateTime oriTime)
{
    oriTime = oriTime.addSecs( - (3600 * 9) );
    return oriTime;
}

static unsigned int GetBits(unsigned x, int p, int n) { return (x >> (p-n+1)) & ~(~0 << n) ; }

static void SwapFloat(float *data)
{
    char temp;

    union {
        char c[4];
    } dat;

    memcpy( &dat, data, sizeof(float) );
    temp     = dat.c[0];
    dat.c[0] = dat.c[3];
    dat.c[3] = temp;
    temp     = dat.c[1];
    dat.c[1] = dat.c[2];
    dat.c[2] = temp;
    memcpy( data, &dat, sizeof(float) );
    return;
}

static void SwapInt(int *data)
{
    char temp;

    union {
        char c[4];
    } dat;

    memcpy( &dat, data, sizeof(int) );
    temp     = dat.c[0];
    dat.c[0] = dat.c[3];
    dat.c[3] = temp;
    temp     = dat.c[1];
    dat.c[1] = dat.c[2];
    dat.c[2] = temp;
    memcpy( data, &dat, sizeof(int) );
    return;
}

#endif // COMMON_H
