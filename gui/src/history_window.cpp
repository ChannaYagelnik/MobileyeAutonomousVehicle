#include <QMessageBox>
#include <QHeaderView>
#include <QDesktopWidget>
#include <QApplication>
#include <QGuiApplication>
#include <QScreen>
#include <main_window.h>
#include "history_window.h"

HistoryWindow::HistoryWindow(DbManager *dataHandler, QWidget *parent)
    : QDialog(parent), dataHandler(dataHandler)
{
    // Set window title
    setWindowTitle("Simulations History");

    // Initialize tableWidget
    tableWidget = new QTableWidget(this);
    tableWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(tableWidget);
    setMinimumSize(600, 400);

    // Center the window on the screen
    QScreen* screen = QGuiApplication::screens().at(0);
    QRect screenGeometry = screen->geometry();
    int x = (screenGeometry.width() - this->width()) / 2;
    int y = (screenGeometry.height() - this->height()) / 2;
    this->move(x, y);

    // Connect the button to load history data
    loadHistory();
}

void HistoryWindow::loadHistory()
{
    // Get data from the database
    QList<QVariantMap> simulationData = dataHandler->getAllDataSimulation();

    // Set up table columns
    tableWidget->setRowCount(simulationData.size());
    tableWidget->setColumnCount(4);  // ID, Input String, Datetime, Rerun Button

    QStringList headers;
    headers << "ID"
            << "Input String"
            << "Datetime"
            << "Another run";
    tableWidget->setHorizontalHeaderLabels(headers);

    if (simulationData.size() == 0)
        MainWindow::guiLogger.logMessage(logger::LogLevel::INFO,
                                         "the table is empty");
    // Fill table with data
    for (int i = 0; i < simulationData.size(); ++i) {
        // Set ID, Input String, and Datetime
        tableWidget->setItem(
            i, 0, new QTableWidgetItem(simulationData[i]["id"].toString()));
        tableWidget->setItem(
            i, 1,
            new QTableWidgetItem(simulationData[i]["input_string"].toString()));
        tableWidget->setItem(
            i, 2,
            new QTableWidgetItem(simulationData[i]["datetime"].toString()));

        QPushButton *rerunButton = new QPushButton("Another run", this);
        tableWidget->setCellWidget(i, 3, rerunButton);

        // Connect the rerun button to display a message
        connect(rerunButton, &QPushButton::clicked, [=]()
        {
            QMessageBox::information(this, "Another run",
                                     "Thank you");
        });
    }
}