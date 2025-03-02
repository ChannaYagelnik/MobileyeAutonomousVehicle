#ifndef __FRAMES_H__
#define __FRAMES_H__

#include "log_handler.h"
#include <QDateTime>
#include <QHash>
#include <QMap>
#include <QPainter>
#include <QPen>
#include <QSet>
#include <QString>
#include <vector>

class Frames : public QWidget {
    Q_OBJECT

public:
    struct Frame {
        QString color;
        double thickness;
    };

    // Ctor
    Frames(LogHandler &logHandler, QWidget *parent = nullptr);

    // Getters
    const LogHandler &getLogHandler() const;
    const std::vector<std::vector<Frame>> &getFramesMat() const;
    const std::map<QDateTime, LogHandler::LogEntry> &getActiveLogEntries()
        const;
    QMap<int, int> getIdMapping() const;

    // Setters
    void setLogHandler(LogHandler &logHandler);
    void setFramesMat(const std::vector<std::vector<Frame>> &framesMat);
    void setActiveLogEntries(
        const std::map<QDateTime, LogHandler::LogEntry> &logEntriesVector);
    void setIdMapping(const QMap<int, int> &idMapping);
    void fillFramesMat();
    void initialFramesMat(int size);
    void createSequentialIds();
    void stopFrames();
    void startFrames();
    void clearFrames();

public slots:
    void updateFrames();

signals:
    void simulationFinished();

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    LogHandler &logHandler;
    std::vector<std::vector<Frame>> framesMat;
    std::map<QDateTime, LogHandler::LogEntry> activeLogEntries;
    QMap<int, int> idMapping;
    qint64 differenceTime;
    QTimer *timer;
    QString generateRandomColor();
};

#endif  // __FRAMES_H__