#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>

#include <QCloseEvent>
#include <QKeyEvent>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void mousePressEvent(QCloseEvent *event) override;
    void mouseMoveEvent(QKeyEvent *event) override;
    void enterEvent(QKeyEvent *event) override;

private slots:
    void FetchTemperatureData();
    void HandleNetworkReply(QNetworkReply *reply);

private:
    Ui::MainWindow *ui;
    QNetworkAccessManager *networkManager;
    void RestrictMouseToWindow();
};
#endif // MAINWINDOW_H
