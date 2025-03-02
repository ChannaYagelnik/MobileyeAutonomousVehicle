#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QPainter>
#include <QTextStream>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>
#include <QBuffer>
#include "log_handler.h"
#include "draggable_square.h"
#include "simulation_state_manager.h"
#include "main_window.h"

bool LogHandler::end = false;

void LogHandler::readLogFile(const QString &fileContent, bool isRealTime)
{
    if (!isRealTime) {
        if (fileContent.isEmpty()) {
            MainWindow::guiLogger.logMessage(logger::LogLevel::ERROR,
                                             "File content is empty.");
            return;
        }

        MainWindow::guiLogger.logMessage(logger::LogLevel::INFO,
                                         "File content successfully loaded.");

        QByteArray byteArray = fileContent.toUtf8();
        QBuffer buffer(&byteArray);
        buffer.open(QIODevice::ReadOnly);

        QTextStream in(&buffer);

        while (!LogHandler::end && !in.atEnd()) {
            QString line = in.readLine().trimmed();

            if (line.isEmpty()) {
                continue;
            }

            if (line.contains("SRC") && line.contains("DST")) {
                LogEntry entry = parseLogLine(line);

                if (entry.srcId != -1) { //Assuming -1 indicate an invalid entry.
                    logEntries.push_back(entry);
                }
            }
        }

        buffer.close();
    } else {
        QFile file(fileContent);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            MainWindow::guiLogger.logMessage(
                logger::LogLevel::ERROR,
                "Cannot open file: " + fileContent.toStdString());
            return;
        }

        MainWindow::guiLogger.logMessage(
            logger::LogLevel::DEBUG,
            "File successfully opened: " + fileContent.toStdString());

        QTextStream in(&file);
        while (!LogHandler::end) {
            QString line = in.readLine().trimmed();
            if (line.contains("SRC") && line.contains("DST")) {
                LogEntry entry = parseLogLine(line);
                if (entry.srcId != -1) {
                    logEntries.push_back(entry);
                }
            }
        }
        file.close();
    }
}

LogHandler::LogEntry LogHandler::parseLogLine(const QString &line)
{
    LogEntry entry;
    QStringList fields = line.split(' ');
    if (fields.size() < 12) {
        MainWindow::guiLogger.logMessage(
            logger::LogLevel::DEBUG,
            "Skipping malformed line: " + line.toStdString());
        entry.srcId = -1;
        return entry;
    }

    QString timestampStr = fields[0];
    timestampStr.chop(2);
    qint64 nanoseconds = timestampStr.toLongLong();
    const qint64 nanosecondsPerMillisecond = 1000000;
    qint64 milliseconds = nanoseconds / nanosecondsPerMillisecond;

    entry.timestamp = QDateTime::fromMSecsSinceEpoch(milliseconds);

    entry.srcId = fields[3].toInt();
    entry.dstId = fields[5].toInt();
    return entry;
}

QVector<LogHandler::LogEntry> *LogHandler::getLogEntries()
{
    return &logEntries;
}

void LogHandler::sortLogEntries()
{
    std::sort(logEntries.begin(), logEntries.end());
}

void LogHandler::analyzeLogEntries(QMainWindow *mainWindow,
                                   QVector<DraggableSquare *> *squares,
                                   bson_t *bsonObj)
{
    if (squares) {
        for (const auto &square : *squares) {
            processSquares.insert(square->getId(), square);
        }
        MainWindow::guiLogger.logMessage(logger::LogLevel::INFO,
                                         "Analyzing log entries in real-time.");
        return;
    }

    if (!bsonObj) {
        MainWindow::guiLogger.logMessage(logger::LogLevel::ERROR, 
            "BSON object is null.");
        return;
    }

    MainWindow::guiLogger.logMessage(logger::LogLevel::INFO, 
        "Analyzing log entries from BSON object.");

    bson_iter_t iter;
    if (!bson_iter_init(&iter, bsonObj) || !bson_iter_find(&iter, "squares") 
                                        || !BSON_ITER_HOLDS_ARRAY(&iter)) {
        MainWindow::guiLogger.logMessage(logger::LogLevel::ERROR, 
            "Invalid BSON structure or missing 'squares' array.");
        return;
    }

    const uint8_t *arrayData;
    uint32_t arrayLength;
    
    bson_iter_array(&iter, &arrayLength, &arrayData);
    if (arrayData == nullptr || arrayLength == 0) {
        MainWindow::guiLogger.logMessage(logger::LogLevel::ERROR, 
            "BSON array data is invalid or empty.");
        return;
    }

    bson_t array;
    bson_iter_t arrayIter;
    if (bson_init_static(&array, arrayData, arrayLength) && 
        bson_iter_init(&arrayIter, &array)) {
        while (bson_iter_next(&arrayIter)) {
            const uint8_t* docData = nullptr;
            uint32_t docLength = 0;

            bson_iter_document(&arrayIter, &docLength, &docData);
            if (docData == nullptr || docLength == 0) {
                MainWindow::guiLogger.logMessage(logger::LogLevel::ERROR, 
                    "Invalid BSON document data.");
                continue;
            }

            bson_t document;
            bson_init_static(&document, docData, docLength);

            int id = -1, x = 0, y = 0, width = 0, height = 0;
            QString name, cmakeProject, qemuPlatform;
            QMap<KeyPermission, bool> permissionsMap;

            bson_iter_t docIter;
            bson_iter_init(&docIter, &document);
            while (bson_iter_next(&docIter)) {
                const char* key = bson_iter_key(&docIter);
                if (strcmp(key, "id") == 0) id = bson_iter_int32(&docIter);
                else if (strcmp(key, "name") == 0) name =
                    QString::fromUtf8(bson_iter_utf8(&docIter, nullptr));
                else if (strcmp(key, "CMakeProject") == 0) cmakeProject = 
                    QString::fromUtf8(bson_iter_utf8(&docIter, nullptr));
                else if (strcmp(key, "QEMUPlatform") == 0) qemuPlatform = 
                    QString::fromUtf8(bson_iter_utf8(&docIter, nullptr));
                else if (strcmp(key, "position") == 0) {
                    bson_iter_t posIter;
                    bson_iter_recurse(&docIter, &posIter);
                    while (bson_iter_next(&posIter)) {
                        if (strcmp(bson_iter_key(&posIter), "x") == 0) x = 
                            bson_iter_int32(&posIter);
                        else if (strcmp(bson_iter_key(&posIter), "y") == 0) y = 
                            bson_iter_int32(&posIter);
                    }
                }
                else if (strcmp(key, "width") == 0) 
                    width = bson_iter_int32(&docIter);
                else if (strcmp(key, "height") == 0) 
                    height = bson_iter_int32(&docIter);
                else if (strcmp(key, "securityPermissions") == 0) {
                    bson_iter_t permIter;
                    bson_iter_recurse(&docIter, &permIter);
                    QMap<KeyPermission, bool> permissionsMap;

                    while (bson_iter_next(&permIter)) {
                        QString permKey = 
                            QString::fromUtf8(bson_iter_key(&permIter));
                        bool conversionOk = false;
                        int permInt = permKey.toInt(&conversionOk);

                        if (!conversionOk) {
                            MainWindow::guiLogger.logMessage(
                                logger::LogLevel::ERROR, 
                            "Failed to convert permission key to integer.");
                            continue;
                        }
                        KeyPermission perm = 
                            static_cast<KeyPermission>(permInt);                        
                        bool value = bson_iter_bool(&permIter);
                        permissionsMap.insert(perm, value);
                    }

                }
            }

            if (id != -1) {
                Process* process = new Process(id, name, cmakeProject, 
                qemuPlatform, permissionsMap);
                DraggableSquare* square = 
                    new DraggableSquare(mainWindow,"", width, height);
                square->setProcess(process);
                square->move(x, y);
                processSquares.insert(id, square);
            }
        }
    } else {
        MainWindow::guiLogger.logMessage(logger::LogLevel::ERROR, 
            "Failed to initialize BSON array.");
        return;
    }

    MainWindow::guiLogger.logMessage(logger::LogLevel::INFO,
        "Size of logEntries: " + std::to_string(logEntries.size()));
}

const QMap<int, DraggableSquare *> &LogHandler::getProcessSquares() const
{
    return processSquares;
}