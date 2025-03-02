#ifndef LOGHANDLER_H
#define LOGHANDLER_H

#include <QDateTime>
#include <QDockWidget>
#include <QJsonObject>
#include <QMainWindow>
#include <QString>
#include <QTime>
#include <QVector>
#include <bson/bson.h>
#include "draggable_square.h"

class LogHandler {
    friend class TestLogHandler;

public: 
    struct LogEntry {
        QDateTime timestamp;
        int srcId;
        int dstId;

        bool operator<(const LogEntry &other) const
        {
            return timestamp < other.timestamp;
        }
    };

    void readLogFile(const QString &fileName, bool isRealTime = false);
    void analyzeLogEntries(QMainWindow *mainWindow,
                           QVector<DraggableSquare *> *squares,
                           bson_t *bsonObj);

    QVector<LogHandler::LogEntry>* getLogEntries();
    const QMap<int, DraggableSquare *> &getProcessSquares() const;
    void sortLogEntries();
    static bool end;

private:
    QVector<LogEntry> logEntries;
    QMap<int, DraggableSquare *>
        processSquares;
    QMap<int, QMap<int, int>>
        communicationCounts;
    LogEntry parseLogLine(const QString &line);
};

#endif  // LOGHANDLER_H