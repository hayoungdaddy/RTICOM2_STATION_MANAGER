#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QSqlRecord>
#include <QSqlError>

#include <QQuickView>
#include <QtQuick>
#include <QLabel>
#include <QPushButton>
#include <QCheckBox>
#include <QTextCodec>
#include <QTableWidget>
#include <QMessageBox>

#include "pushmessage.h"
#include "writelog.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QString configFile = nullptr, QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

    _CONFIGURE cfg;
    WriteLog *log;

    void readCFG();

    void openDB();
    // About Database & table
    QSqlDatabase stationDB;
    QSqlQueryModel *networkModel;
    QSqlQueryModel *affiliationModel;
    QSqlQueryModel *siteModel;

    QVector<_STATION> staListVT_G_ori;
    QVector<_STATION> staListVT_G;
    void readStationList();

    void setGUI();
    QTextCodec *codec;

    QObject *obj;
    QVariant returnedValue;
    QWidget *mapContainer;

    QStringList networkList;
    QStringList netBoldStatus;
    QLabel *netLB[MAX_NUM_NETWORK];
    QCheckBox *netAllCB[MAX_NUM_NETWORK];
    QTableWidget *netTW[MAX_NUM_NETWORK];
    QPushButton *netBoldPB[MAX_NUM_NETWORK];
    int numNetwork;

    void setCheckBoxInTable(int);
    void getNumInUseStation();

private slots:
    void staCBClicked(bool);
    void netCBClicked(bool);
    void savePBClicked(bool);
    void netBoldPBClicked(bool);
    void qmlSignalfromSetStationMap(QString);
};

#endif // MAINWINDOW_H
