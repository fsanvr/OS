#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QMessageBox>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>

#include <QCursor>
#include <QDesktopWidget>
#include <QKeyEvent>
#include <QCloseEvent>


using namespace QtCharts;


void MainWindow::RestrictMouseToWindow() {
    QRect windowRect = geometry();
    QCursor::setPos(windowRect.center());
    setMouseTracking(true);

    QRegion region(windowRect);
    setMask(region);
}

void MainWindow::closeEvent(QCloseEvent *event) {
    event->ignore();
}

void MainWindow::keyPressEvent(QKeyEvent *event) {
    if (event->modifiers() == (Qt::ControlModifier | Qt::AltModifier | Qt::ShiftModifier) &&
        event->key() == Qt::Key_Q) {
        QApplication::quit();  // Выход по комбинации Ctrl+Alt+Shift+Q
    } else {
        event->ignore();
    }
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), networkManager(new QNetworkAccessManager(this)) {
    ui->setupUi(this);

    RestrictMouseToWindow();
    this->setWindowFlags(Qt::FramelessWindowHint | Qt::Window);
    this->showFullScreen();

    connect(ui->getDataButton, &QPushButton::clicked, this, &MainWindow::FetchTemperatureData);
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::FetchTemperatureData() {
    QString startDate = ui->startDateEdit->date().toString("yyyy-MM-dd");
    QString endDate = ui->endDateEdit->date().toString("yyyy-MM-dd");
    QString url = QString("http://127.0.0.1:8080/api/temperature/get?startDate=%1&endDate=%2")
                      .arg(startDate, endDate);

    QNetworkRequest request((QUrl(url)));
    auto *reply = networkManager->get(request);

    connect(reply, &QNetworkReply::finished, this, [this, reply]() { HandleNetworkReply(reply); });
}

void MainWindow::HandleNetworkReply(QNetworkReply *reply) {
    if (reply->error() != QNetworkReply::NoError) {
        QMessageBox::critical(this, "Error", "Failed to fetch data from server.");
        reply->deleteLater();
        return;
    }

    QByteArray responseData = reply->readAll();
    reply->deleteLater();

    if (responseData.isEmpty()) {
        qCritical() << "Received empty data from server!";
        return;
     }

    QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData);
    if (jsonDoc.isNull() || !jsonDoc.isArray()) {
        QMessageBox::critical(this, "Error", "Invalid response from server.");
        return;
    }

    QJsonArray tempSeries = jsonDoc.array();

    if (tempSeries.isEmpty()) {
        qCritical() << "No temperature data in response!";
        return;
    }
    double currentTemp = tempSeries.isEmpty() ? 0.0 : tempSeries.last().toObject().value("temperature").toDouble();
    double averageTemp = 0.0;

    double totalTemp = 0.0;
    int count = tempSeries.size();
    for (const QJsonValue &point : tempSeries) {
        QJsonObject obj = point.toObject();
        totalTemp += obj.value("temperature").toDouble();
    }
    if (count > 0) {
        averageTemp = totalTemp / count;
    }

    ui->currentTempValueLabel->setText(QString::number(currentTemp, 'f', 1) + " °C");
    ui->averageTempValueLabel->setText(QString::number(averageTemp, 'f', 1) + " °C");

    auto *series = new QLineSeries();
    for (const QJsonValue &point : tempSeries) {
        QJsonObject obj = point.toObject();
        QDateTime timestamp = QDateTime::fromSecsSinceEpoch(obj.value("timestamp").toInt());
        double temp = obj.value("temperature").toDouble();
        series->append(timestamp.toMSecsSinceEpoch(), temp);
    }


    auto *chart = new QChart();
    chart->addSeries(series);
    chart->createDefaultAxes();
    chart->axes(Qt::Horizontal).first()->setTitleText("Time");
    chart->axes(Qt::Vertical).first()->setTitleText("Temperature (°C)");

    auto *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);


    if (!ui->graphWidget->layout()) {
        ui->graphWidget->setLayout(new QVBoxLayout());
    }

    QLayoutItem* item;
    while ((item = ui->graphWidget->layout()->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }

    ui->graphWidget->layout()->addWidget(chartView);
}
