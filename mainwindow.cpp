#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QProgressDialog>

MainWindow::MainWindow(QString configFile, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    activemq::library::ActiveMQCPP::initializeLibrary();

    ui->setupUi(this);

    codec = QTextCodec::codecForName("utf-8");

    cfg.configFileName = configFile;
    readCFG();

    log = new WriteLog();

    stationDB = QSqlDatabase::addDatabase("QMYSQL");
    stationDB.setHostName(cfg.db_ip);
    stationDB.setDatabaseName(cfg.db_name);
    stationDB.setUserName(cfg.db_user);
    stationDB.setPassword(cfg.db_passwd);

    networkModel = new QSqlQueryModel();
    affiliationModel = new QSqlQueryModel();
    siteModel = new QSqlQueryModel();

    readStationList();
    setGUI();
    for(int i=0;i<numNetwork;i++)
    {
        setCheckBoxInTable(i);
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::readCFG()
{
    QFile file(cfg.configFileName);
    if(!file.exists())
    {
        qDebug() << "Failed configuration. Parameter file doesn't exists.";
        exit(1);
    }
    if(file.open(QIODevice::ReadOnly))
    {
        QTextStream stream(&file);
        QString line, _line;

        while(!stream.atEnd())
        {
            line = stream.readLine();
            _line = line.simplified();

            if(_line.startsWith(" ") || _line.startsWith("#"))
                continue;
            else if(_line.startsWith("LOGDIR"))
                cfg.logDir = _line.section("=", 1, 1);
            else if(_line.startsWith("AMQ_IP"))
                cfg.amq_ip = _line.section("=", 1, 1);
            else if(_line.startsWith("AMQ_PORT"))
                cfg.amq_port = _line.section("=", 1, 1);
            else if(_line.startsWith("AMQ_USERNAME"))
                cfg.amq_user = _line.section("=", 1, 1);
            else if(_line.startsWith("AMQ_PASSWD"))
                cfg.amq_passwd = _line.section("=", 1, 1);
            else if(_line.startsWith("AMQ_STA_TOPIC"))
                cfg.amq_topic = _line.section("=", 1, 1);
            else if(_line.startsWith("DB_IP"))
                cfg.db_ip = _line.section("=", 1, 1);
            else if(_line.startsWith("DB_NAME"))
                cfg.db_name = _line.section("=", 1, 1);
            else if(_line.startsWith("DB_USERNAME"))
                cfg.db_user = _line.section("=", 1, 1);
            else if(_line.startsWith("DB_PASSWD"))
                cfg.db_passwd = _line.section("=", 1, 1);
        }
        file.close();
    }
}

void MainWindow::openDB()
{
    if(!stationDB.open())
    {
        log->write(cfg.logDir, "Error connecting to DB: " + stationDB.lastError().text());
    }
}

void MainWindow::readStationList()
{
    staListVT_G.clear();
    staListVT_G_ori.clear();

    QString query;
    query = "SELECT * FROM NETWORK";
    openDB();
    networkModel->setQuery(query);

    int count = 0;

    QProgressDialog progress("Loading Stations....", "Abort", 0, networkModel->rowCount(), this);
    progress.setWindowTitle("RTICOM2 - Station Manager");
    progress.setMinimumWidth(700);
    progress.setWindowModality(Qt::WindowModal);

    if(networkModel->rowCount() > 0)
    {
        for(int i=0;i<networkModel->rowCount();i++)
        {
            progress.setValue(i);
            if(progress.wasCanceled())
            {
                exit(1);
            }
            QString network = networkModel->record(i).value("net").toString();
            networkList << network;
            query = "SELECT * FROM AFFILIATION where net='" + network + "'";
            //openDB();
            affiliationModel->setQuery(query);
            //qDebug() << network;

            for(int j=0;j<affiliationModel->rowCount();j++)
            {
                QString affiliation = affiliationModel->record(j).value("aff").toString();
                QString affiliationName = affiliationModel->record(j).value("affname").toString();
                double lat = affiliationModel->record(j).value("lat").toDouble();
                double lon = affiliationModel->record(j).value("lon").toDouble();
                double elev = affiliationModel->record(j).value("elev").toDouble();
                query = "SELECT * FROM SITE where aff='" + affiliation + "'";
                //openDB();
                siteModel->setQuery(query);
                //qDebug() << affiliationName;

                for(int k=0;k<siteModel->rowCount();k++)
                {
                    _STATION sta;
                    sta.index = count;
                    sta.sta = siteModel->record(k).value("sta").toString();
                    sta.comment = affiliationName;
                    sta.lat = lat;
                    sta.lon = lon;
                    sta.elev = elev;
                    sta.staType = siteModel->record(k).value("statype").toString();
                    sta.inUse = siteModel->record(k).value("inuse").toInt();

                    staListVT_G.push_back(sta);
                    staListVT_G_ori.push_back(sta);
                    count++;
                }
            }
        }
        progress.setValue(networkModel->rowCount());
    }

    //qDebug() << staListVT.count();
    log->write(cfg.logDir, "Read Station List (Ground) from DB. Number of Station : " + QString::number(staListVT_G.count()));

    numNetwork = networkList.count();
}

void MainWindow::setGUI()
{
    QFont lbFont("Cantarell", 12, QFont::Bold);

    connect(ui->savePB, SIGNAL(clicked(bool)), this, SLOT(savePBClicked(bool)));

    ui->tNumStaLB->setText(QString::number(staListVT_G.count()));

    ui->netTitleLB->setText(codec->toUnicode("네트워크별 관측소 리스트 (총 ") +
                            QString::number(numNetwork) +
                            codec->toUnicode("개 네트워크)"));

    for(int i=0;i<numNetwork;i++)
    {
        QHBoxLayout *titleLO = new QHBoxLayout();
        ui->sAreaLO->addLayout(titleLO);
        netLB[i] = new QLabel();
        titleLO->addWidget(netLB[i]);

        netLB[i]->setFont(lbFont);
        netLB[i]->setFixedSize(530, 30);
        netAllCB[i] = new QCheckBox();
        netAllCB[i]->setFixedSize(18, 30);
        netAllCB[i]->setObjectName(QString::number(i) + "_" + networkList.at(i));
        connect(netAllCB[i], SIGNAL(clicked(bool)), this, SLOT(netCBClicked(bool)));
        titleLO->addWidget(netAllCB[i]);
        QLabel *myLB = new QLabel();
        myLB->setText(codec->toUnicode("모든 관측소 사용"));
        titleLO->addWidget(myLB);
        netTW[i] = new QTableWidget();
        netTW[i]->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        ui->sAreaLO->addWidget(netTW[i]);

        netTW[i]->setRowCount(0);
        netTW[i]->setColumnCount(7);
        netTW[i]->setAlternatingRowColors(true);
        netTW[i]->setEditTriggers(QAbstractItemView::NoEditTriggers);
        netTW[i]->setSelectionBehavior(QAbstractItemView::SelectRows);
        netTW[i]->setHorizontalHeaderItem(0, new QTableWidgetItem("Index"));
        netTW[i]->setHorizontalHeaderItem(1, new QTableWidgetItem("Station"));
        netTW[i]->setHorizontalHeaderItem(2, new QTableWidgetItem("Comment"));
        netTW[i]->setHorizontalHeaderItem(3, new QTableWidgetItem("Lat."));
        netTW[i]->setHorizontalHeaderItem(4, new QTableWidgetItem("Lon."));
        netTW[i]->setHorizontalHeaderItem(5, new QTableWidgetItem("Elev."));
        netTW[i]->setHorizontalHeaderItem(6, new QTableWidgetItem("In Use"));
        netTW[i]->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        netTW[i]->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

        //bool isAllSiteChecked = true;
        for(int j=0;j<staListVT_G.count();j++)
        {
            if(networkList.at(i).startsWith(staListVT_G.at(j).sta.left(2)))
            {
                netTW[i]->setRowCount(netTW[i]->rowCount()+1);
                netTW[i]->setItem(netTW[i]->rowCount()-1, 0, new QTableWidgetItem(QString::number(staListVT_G.at(j).index)));
                netTW[i]->setItem(netTW[i]->rowCount()-1, 1, new QTableWidgetItem(staListVT_G.at(j).sta));
                netTW[i]->setItem(netTW[i]->rowCount()-1, 2, new QTableWidgetItem(staListVT_G.at(j).comment));
                netTW[i]->setItem(netTW[i]->rowCount()-1, 3, new QTableWidgetItem(QString::number(staListVT_G.at(j).lat, 'f', 4)));
                netTW[i]->setItem(netTW[i]->rowCount()-1, 4, new QTableWidgetItem(QString::number(staListVT_G.at(j).lon, 'f', 4)));
                netTW[i]->setItem(netTW[i]->rowCount()-1, 5, new QTableWidgetItem(QString::number(staListVT_G.at(j).elev, 'f', 4)));
                netTW[i]->setItem(netTW[i]->rowCount()-1, 6, new QTableWidgetItem("-"));
            }
        }

        netTW[i]->horizontalHeader()->setStretchLastSection(true);
        netTW[i]->verticalHeader()->setStretchLastSection(true);
        netTW[i]->verticalHeader()->setDefaultSectionSize(25);
        netTW[i]->setFixedHeight(netTW[i]->rowCount() * 25 + 25);

        //table width : 640px
        netTW[i]->setColumnWidth(0, 40);
        netTW[i]->setColumnWidth(1, 80);
        netTW[i]->setColumnWidth(2, 220);
        netTW[i]->setColumnWidth(3, 80);
        netTW[i]->setColumnWidth(4, 80);
        netTW[i]->setColumnWidth(5, 80);
        netTW[i]->setColumnWidth(6, 70);

        netLB[i]->setText("- 네트워크 : " + networkList.at(i) + codec->toUnicode(" (총 ") +
                          QString::number(netTW[i]->rowCount()) + codec->toUnicode("개 관측소)"));

        for(int j=0;j<netTW[i]->rowCount();j++)
        {
            for(int k=0;k<netTW[i]->columnCount();k++)
            {
                netTW[i]->item(j, k)->setTextAlignment(Qt::AlignCenter);
            }
        }

        QSpacerItem *spacer = new QSpacerItem(10,20, QSizePolicy::Fixed, QSizePolicy::Fixed);
        ui->sAreaLO->addSpacerItem(spacer);

        netBoldStatus << "0";
        netBoldPB[i] = new QPushButton();
        netBoldPB[i]->setObjectName(networkList.at(i) + "_" + QString::number(i));
        connect(netBoldPB[i], SIGNAL(clicked(bool)), this, SLOT(netBoldPBClicked(bool)));
        netBoldPB[i]->setStyleSheet("background-color: rgb(255,255,255)");
        netBoldPB[i]->setText(networkList.at(i));
        netBoldPB[i]->setFixedHeight(25);
        int locationRow, locationCol;
        if(i<10) locationRow = 0;
        else locationRow = i / 10;
        locationCol = i % 10;
        ui->netPBLO->addWidget(netBoldPB[i], locationRow, locationCol);
    }

    QQuickView *mapView = new QQuickView();
    mapContainer = QWidget::createWindowContainer(mapView, this);
    mapView->setResizeMode(QQuickView::SizeRootObjectToView);
    mapContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mapContainer->setFocusPolicy(Qt::TabFocus);
    mapView->setSource(QUrl(QStringLiteral("qrc:/SetSta.qml")));
    ui->mapLO->addWidget(mapContainer);
    obj = mapView->rootObject();

    QMetaObject::invokeMethod(this->obj, "clearMap", Q_RETURN_ARG(QVariant, returnedValue));
    QObject::connect(this->obj, SIGNAL(sendStationIndexSignal(QString)), this, SLOT(qmlSignalfromSetStationMap(QString)));

    // create staMarker
    for(int i=0;i<staListVT_G.count();i++)
    {
        QMetaObject::invokeMethod(this->obj, "createStaMarker", Q_RETURN_ARG(QVariant, returnedValue),
                                  Q_ARG(QVariant, staListVT_G.at(i).index), Q_ARG(QVariant, staListVT_G.at(i).sta.left(2)), Q_ARG(QVariant, staListVT_G.at(i).sta.mid(2, -1)),
                                  Q_ARG(QVariant, staListVT_G.at(i).lat), Q_ARG(QVariant, staListVT_G.at(i).lon), Q_ARG(QVariant, staListVT_G.at(i).inUse));
    }
}

void MainWindow::netBoldPBClicked(bool)
{
    QObject* pObject = sender();
    int netIndex = pObject->objectName().section("_", 1, 1).toInt();
    QString netName = pObject->objectName().section("_", 0, 0);

    if(netBoldStatus.at(netIndex).startsWith("1"))
    {
        netBoldStatus.replace(netIndex, "0");
        netBoldPB[netIndex]->setStyleSheet("background-color: rgb(255,255,255)");

        for(int i=0;i<staListVT_G.count();i++)
        {
            if(netName.startsWith(staListVT_G.at(i).sta.left(2)))
            {
                QMetaObject::invokeMethod(this->obj, "normalStaMarker", Q_RETURN_ARG(QVariant, returnedValue),
                                          Q_ARG(QVariant, staListVT_G.at(i).index), Q_ARG(QVariant, staListVT_G.at(i).inUse));
            }
        }
    }
    else
    {
        netBoldStatus.replace(netIndex, "1");
        netBoldPB[netIndex]->setStyleSheet("background-color: rgb(236, 239, 14)");

        for(int i=0;i<staListVT_G.count();i++)
        {
            if(netName.startsWith(staListVT_G.at(i).sta.left(2)))
            {
                QMetaObject::invokeMethod(this->obj, "boldStaMarker", Q_RETURN_ARG(QVariant, returnedValue), Q_ARG(QVariant, staListVT_G.at(i).index));
            }
        }
    }

}

void MainWindow::setCheckBoxInTable(int i)
{
    bool isAllSiteChecked = true;

    int indexInNetwork = 0;
    for(int j=0;j<staListVT_G.count();j++)
    {
        if(networkList.at(i).startsWith(staListVT_G.at(j).sta.left(2)))
        {
            QWidget *checkBoxWidget = new QWidget();
            QCheckBox *staCB = new QCheckBox();
            staCB->setObjectName(QString::number(i) + "_" + QString::number(staListVT_G.at(j).index));
            QHBoxLayout *layoutCheckBox = new QHBoxLayout(checkBoxWidget);
            layoutCheckBox->addWidget(staCB);
            layoutCheckBox->setAlignment(Qt::AlignCenter);
            layoutCheckBox->setContentsMargins(0,0,0,0);
            checkBoxWidget->setLayout(layoutCheckBox);

            if(staListVT_G.at(j).inUse == 1)
            {
                staCB->setChecked(true);
            }
            else
            {
                isAllSiteChecked = false;
                staCB->setChecked(false);
            }

            connect(staCB, SIGNAL(clicked(bool)), this, SLOT(staCBClicked(bool)));

            netTW[i]->setCellWidget(indexInNetwork, 6, checkBoxWidget);
            indexInNetwork++;
        }
    }

    if(isAllSiteChecked)
        netAllCB[i]->setChecked(true);
    else
        netAllCB[i]->setChecked(false);

    getNumInUseStation();
}

void MainWindow::staCBClicked(bool isChecked)
{
    QObject* pObject = sender();
    int netIndex = pObject->objectName().section("_", 0, 0).toInt();
    int staIndex = pObject->objectName().section("_", 1, 1).toInt();

    _STATION sta;
    sta = staListVT_G.at(staIndex);

    if(isChecked)
        sta.inUse = 1;
    else
        sta.inUse = 0;

    QMetaObject::invokeMethod(this->obj, "changeStaMarkerSource", Q_RETURN_ARG(QVariant, returnedValue),
                              Q_ARG(QVariant, sta.index), Q_ARG(QVariant, sta.inUse));

    staListVT_G.replace(staIndex, sta);

    bool isAllSiteChecked = true;
    for(int i=0;i<staListVT_G.count();i++)
    {
        if(staListVT_G.at(i).sta.left(2).startsWith(networkList.at(netIndex)))
        {
            if(staListVT_G.at(i).inUse == 0)
                isAllSiteChecked = false;
        }
    }

    if(isAllSiteChecked)
        netAllCB[netIndex]->setChecked(true);
    else
        netAllCB[netIndex]->setChecked(false);

    getNumInUseStation();
}

void MainWindow::netCBClicked(bool isChecked)
{
    QObject* pObject = sender();
    QString name = pObject->objectName().section("_", 1, 1);
    int netIndex = pObject->objectName().section("_", 0, 0).toInt();

    _STATION sta;
    for(int i=0;i<staListVT_G.count();i++)
    {
        sta = staListVT_G.at(i);
        if(name.startsWith(sta.sta.left(2)))
        {
            if(isChecked)
                sta.inUse = 1;
            else
                sta.inUse = 0;

            QMetaObject::invokeMethod(this->obj, "changeStaMarkerSource", Q_RETURN_ARG(QVariant, returnedValue),
                                      Q_ARG(QVariant, sta.index), Q_ARG(QVariant, sta.inUse));

            staListVT_G.replace(i, sta);
        }
    }

    setCheckBoxInTable(netIndex);
}

void MainWindow::getNumInUseStation()
{
    int numInUseSta = 0;
    for(int i=0;i<staListVT_G.count();i++)
    {
        if(staListVT_G.at(i).inUse == 1)
            numInUseSta++;
    }
    ui->cNumStaLB->setText(QString::number(numInUseSta));
}

void MainWindow::qmlSignalfromSetStationMap(QString indexS)
{
    int index = indexS.toInt();
    QString net, sta;
    net = staListVT_G.at(index).sta.left(2);
    sta = staListVT_G.at(index).sta;
    int netIndex = 0;

    for(int i=0;i<networkList.count();i++)
    {
        if(net.startsWith(networkList.at(i)))
        {
            netIndex = i;
            break;
        }
    }

    QString tempSta;
    for(int i=0;i<netTW[netIndex]->rowCount();i++)
    {
        tempSta = netTW[netIndex]->item(i, 2)->text();
        if(tempSta.startsWith(sta))
        {
            netTW[netIndex]->setCurrentCell(i-1, 2);
            break;
        }
    }
}

void MainWindow::savePBClicked(bool)
{
    if( !QMessageBox::question( this,
                                codec->toUnicode("저장"),
                                codec->toUnicode("관측소 정보 Database를 변경합니다."),
                                codec->toUnicode("저장"),
                                codec->toUnicode("취소"),
                                QString::null, 1, 1) )
    {
        log->write(cfg.logDir, "============================================================");
        int count = 0;
        int nInUse = 0;
        int nNotUsed = 0;

        openDB();
        QString query;

        for(int i=0;i<staListVT_G.count();i++)
        {
            _STATION sta = staListVT_G.at(i);
            _STATION sta_ori = staListVT_G_ori.at(i);

            if(sta.inUse == 1)
                nInUse++;
            else
                nNotUsed++;

            if(sta.inUse != sta_ori.inUse)
            {
                count++;
                QString temp;
                if(sta.inUse == 0)
                {
                    query = "UPDATE SITE SET inuse=0 WHERE sta=\'" + sta.sta + "\';";
                    temp = "yes to no.";
                }
                else
                {
                    query = "UPDATE SITE SET inuse=1 WHERE sta=\'" + sta.sta + "\';";
                    temp = "no to yes.";
                }

                networkModel->setQuery(query);

                staListVT_G_ori.replace(i, sta);

                log->write(cfg.logDir, "Changed this station Information - " + sta.sta + ", InUse : " + temp);
            }
        }

        QString failover = "failover:(tcp://" + cfg.amq_ip + ":" + cfg.amq_port + ")";

        PushMessage *pushmessage = new PushMessage(cfg.logDir, this);
        if(!pushmessage->isRunning())
        {
            pushmessage->setup(failover, cfg.amq_user, cfg.amq_passwd, cfg.amq_topic, true, false);
            pushmessage->start();
        }

        log->write(cfg.logDir, "Number of changed stations : " + QString::number(count));
        log->write(cfg.logDir, "Total : " + QString::number(staListVT_G.count()));
        log->write(cfg.logDir, "In Use : " + QString::number(nInUse));
        log->write(cfg.logDir, "Not Used : " + QString::number(nNotUsed));
        log->write(cfg.logDir, "============================================================");

        QMessageBox msgBox;
        msgBox.setWindowTitle("RTICOM2 - Station Manager");
        msgBox.setText(codec->toUnicode("저장이 완료되었습니다."));
        msgBox.exec();
    }
}
