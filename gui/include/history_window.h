#ifndef HISTORYWINDOW_H
#define HISTORYWINDOW_H

#include <QWidget>
#include <QTableWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QDesktopWidget>
#include <QApplication>
#include <QDialog>
#include "db_manager.h"

class HistoryWindow : public QDialog {
    Q_OBJECT

public:
    HistoryWindow(DbManager *dataHandler, QWidget *parent = nullptr);
    QTableWidget *tableWidget;  // Table to display the history data
    DbManager *dataHandler;     // Pointer to the DbManager instance

private slots:
    void loadHistory();  // Function to load history data
};

#endif  // HISTORYWINDOW_H