#include <QtTest/QtTest>
#include <QFile>
#include <QTemporaryFile>
#include <QVariantMap>
#include "db_manager.h"

class DbManagerTest : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void testReadLogFile();
    void testReadBsonFile();
    void testInsertDataToDatabase();
    void testGetAllDataSimulation();
    void testGetRecordById();
    void testInsertDataDetails();
    void testGetAllDataDetails();
    void testGetDataDetailsBySimulationAndTime();
};

void DbManagerTest::initTestCase()
{
    // Any initial setup if needed
    qDebug() << "Starting DbManager tests...";
}

void DbManagerTest::cleanupTestCase()
{
    // Cleanup operations after all tests
    qDebug() << "Cleaning up...";
}

void DbManagerTest::testReadLogFile()
{
    QTemporaryFile tempFile;
    QVERIFY(tempFile.open());
    QTextStream out(&tempFile);
    out << "This is a log file.\n";
    tempFile.close();

    DbManager dataHandler;
    QString logData = dataHandler.readLogFile(tempFile.fileName());

    QCOMPARE(logData, QString("This is a log file.\n"));
}

void DbManagerTest::testReadBsonFile()
{
    QTemporaryFile tempFile;
    QVERIFY(tempFile.open());
    QByteArray testData("Test BSON data");
    tempFile.write(testData);
    tempFile.close();

    DbManager dataHandler;
    QByteArray bsonData = dataHandler.readBsonFile(tempFile.fileName());

    QCOMPARE(bsonData, testData);
}

void DbManagerTest::testInsertDataToDatabase()
{
    DbManager dataHandler;
    QByteArray bsonData("Test BSON data");
    QString logData("Test log data");

    bool result =
        dataHandler.insertDataToDatabase(
            "TestInput",bsonData, logData);
    QVERIFY(result);
}

void DbManagerTest::testGetAllDataSimulation()
{
    DbManager dataHandler;
    QList<QVariantMap> dataList = 
    dataHandler.getAllDataSimulation();
    QVERIFY(!dataList.isEmpty());
}

void DbManagerTest::testGetRecordById()
{
    DbManager dataHandler;
    QVariantMap record = dataHandler.getRecordById(1);
    QVERIFY(!record.isEmpty());
    QCOMPARE(record["id"].toInt(), 1);
}

void DbManagerTest::testInsertDataDetails()
{
    DbManager dbHandler;

    bool result = dbHandler.insertDataDetails(
        1, "2024-09-15 12:00:00", 100, 101, "Test Message", "sent", "success");
    QVERIFY(result);

    QList<QVariantMap> details = dbHandler.getAllDataDetails();
    QVERIFY(!details.isEmpty());

    QVariantMap lastRecord = details.last();
    QCOMPARE(lastRecord["data_simulation_id"].toInt(), 1);
    QCOMPARE(lastRecord["timesTamp"].toString(), "2024-09-15 12:00:00");
    QCOMPARE(lastRecord["idSrc"].toInt(), 100);
    QCOMPARE(lastRecord["idDst"].toInt(), 101);
    QCOMPARE(lastRecord["message"].toString(), "Test Message");
    QCOMPARE(lastRecord["send_recieved"].toString(), "sent");
    QCOMPARE(lastRecord["success_failed"].toString(), "success");
}

void DbManagerTest::testGetAllDataDetails()
{
    DbManager dbHandler;
    QList<QVariantMap> details = dbHandler.getAllDataDetails();

    QVariantMap firstRecord = details.first();
    QCOMPARE(firstRecord["data_simulation_id"].toInt(), 1);
    QCOMPARE(firstRecord["timesTamp"].toString(), "2024-09-15 12:00:00");
}

void DbManagerTest::testGetDataDetailsBySimulationAndTime()
{
    DbManager dbHandler;
    dbHandler.insertDataDetails(1, "2024-09-15 12:00:00", 100, 101,
                                "Test Message", "sent", "success");

    QList<QVariantMap> details =
        dbHandler.getDataDetailsBySimulationAndTime(1, "2024-09-15 12:00:00");
    QVERIFY(!details.isEmpty());

    QVariantMap record = details.first();
    QCOMPARE(record["data_simulation_id"].toInt(), 1);
    QCOMPARE(record["timesTamp"].toString(), "2024-09-15 12:00:00");
    QCOMPARE(record["idSrc"].toInt(), 100);
    QCOMPARE(record["idDst"].toInt(), 101);
    QCOMPARE(record["message"].toString(), "Test Message");
    QCOMPARE(record["send_recieved"].toString(), "sent");
    QCOMPARE(record["success_failed"].toString(), "success");
}

QTEST_MAIN(DbManagerTest)
#include "test_db_manager.moc"