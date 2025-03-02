#include <QtTest/QtTest>
#include "log_handler.h"
#include "simulation_state_manager.h"
#include "draggable_square.h"
#include "process.h"
#include <QCoreApplication>
#include <QVector>
#include <QString>
#include <QFile>
#include <QTextStream>

class LogHandlerTests : public QObject {
    Q_OBJECT
    SimulationStateManager state;

private slots:
    void testReadLogFile();
    void testSortLogEntries();
    void testGetProcessSquares();
};

void LogHandlerTests::testReadLogFile()
{
    LogHandler logHandler;
    logHandler.readLogFile("../log_file.log");

    QVERIFY(!logHandler.getLogEntries()->isEmpty());
}

void LogHandlerTests::testSortLogEntries()
{
    LogHandler logHandler;
    logHandler.readLogFile("../log_file.log");

    logHandler.sortLogEntries();
    QVector<LogHandler::LogEntry> logEntries = *(logHandler.getLogEntries());

    for (int i = 1; i < logEntries.size(); ++i) {
        QVERIFY(logEntries[i - 1] < logEntries[i]);
    }
}

// The code for this test is currently disabled because it requires an update.
//The plan is to update them later.
#ifdef DISABLE_TEST_GET_PROCESS_SQUARES
void LogHandlerTests::testGetProcessSquares()
{
    QSKIP("This test is currently disabled for further updates.");
}
#else
void LogHandlerTests::testGetProcessSquares()
{
    LogHandler logHandler;

    QMap<KeyPermission, bool> permissionsMap;
    permissionsMap[KeyPermission::ENCRYPT] = true;
    permissionsMap[KeyPermission::SIGN] = false;

    Process process1(1, "Process1", "CMakeProject1", "QEMUPlatform1",permissionsMap);
    DraggableSquare square1;
    square1.setProcess(&process1);

    QMap<int, DraggableSquare *> &processSquares = const_cast<QMap<int, DraggableSquare *> &>(
        logHandler.getProcessSquares());
    processSquares.insert(1, &square1);

    const QMap<int, DraggableSquare *> &squares = logHandler.getProcessSquares();
    QVERIFY(squares.contains(1));
    QCOMPARE(squares[1]->getProcess()->getId(), 1);
}
#endif

QTEST_MAIN(LogHandlerTests)
#include "test_log_handler.moc"