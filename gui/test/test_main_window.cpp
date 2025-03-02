#include <QtTest/QtTest>
#include <QTimer>
#include <chrono>
#include <thread>
#include <QDebug>
#include <sys/resource.h>
#include "main_window.h"
#include "process_dialog.h"

class TestMainWindow : public QObject {
    Q_OBJECT

private slots:
    void init();     // Setup method, runs before each test
    void cleanup();  // Cleanup method, runs after each test

    void testCreateNewProcess();
    void testAddProcessSquare();
    void testIsUniqueId();
    void testStartProcesses();
    void testEndProcesses();
    void testDeleteSquare();
    void testSetCoreDumpLimit();
    void testShowTimerInput();
    void addId(int id);
    void testRunScriptAndGetJsonPath();
    void testUpdateProcessJsonFile();
    void testDeleteProcessFromJsonFile();
    void testOpenDependencySelectionProject_BuildDirectoryExists();
    void testOpenDependencySelectionProject_BuildDirectoryNotExists();
    void testCheckJsonFileAndSetButtons_FileExists();
    void testCheckJsonFileAndSetButtons_FileNotExists();
private:
    MainWindow *window;  // Pointer to MainWindow to allow reuse in each test
};

void TestMainWindow::init()
{
    // Initialize the MainWindow before each test
    window = new MainWindow();
}

void TestMainWindow::cleanup()
{
    // Clean up any resources used during the test
    delete window;
    window = nullptr;
}

void TestMainWindow::testCreateNewProcess()
{
    int newProcessId = 6;
    QString processName = "NewProcess";
    QString cmakeProject = "../test/dummy_program2";
    QString qemuPlatform = "QEMUPlatform";
    QMap<KeyPermission, bool> permissionsMap;

    permissionsMap[KeyPermission::ENCRYPT] = true;
    permissionsMap[KeyPermission::SIGN] = false;

    Process *newProcess =
        new Process(newProcessId, processName, cmakeProject, qemuPlatform, permissionsMap);
    window->addProcessSquare(newProcess);
    window->addId(newProcessId);

    Process *retrievedProcess = window->getProcessById(newProcessId);
    QVERIFY(retrievedProcess != nullptr);
    QCOMPARE(retrievedProcess->getName(), processName);
    QCOMPARE(retrievedProcess->getExecutionFile(), cmakeProject);
    QCOMPARE(retrievedProcess->getQEMUPlatform(), qemuPlatform);
    QCOMPARE(retrievedProcess->getSecurityPermissions(), permissionsMap);

    // Cleanup for this specific test
    delete newProcess;
}

void TestMainWindow::testAddProcessSquare()
{
    QMap<KeyPermission, bool> permissionsMap;

    permissionsMap[KeyPermission::ENCRYPT] = true;
    permissionsMap[KeyPermission::SIGN] = false;
    Process *newProcess =
        new Process(5, "Test Process", "../dummy_program1", "QEMUPlatform", permissionsMap);
    window->addProcessSquare(newProcess);
    QCOMPARE(window->squares.size(), 5);  // Check if square is added

    delete newProcess;  // Ensure we clean up the process
}

void TestMainWindow::testIsUniqueId()
{
    window->addId(5);
    QCOMPARE(window->isUniqueId(5), false);  // Check if the ID is unique
    QCOMPARE(window->isUniqueId(10),
             true);  // Check if a different ID is unique
}

void TestMainWindow::testStartProcesses()
{
    window->compileProjects();
    window->runProjects();
    QVERIFY(
        !window->runningProcesses.isEmpty());  // Ensure processes are started
}

void TestMainWindow::testEndProcesses()
{
    window->compileProjects();
    window->runProjects();
    window->endProcesses();
    QVERIFY(
        window->runningProcesses.isEmpty());  // Ensure processes are stopped
}

void TestMainWindow::testDeleteSquare()
{
    QString cmakeProject = "../test/dummy_program1";
    QMap<KeyPermission, bool> permissionsMap;

    permissionsMap[KeyPermission::ENCRYPT] = true;
    permissionsMap[KeyPermission::SIGN] = false;

    Process *process =
        new Process(5, "Test Process", cmakeProject, "QEMUPlatform", permissionsMap);
    window->addProcessSquare(process);

    window->deleteSquare(5);

    // Ensure that only the initial 4 squares remain
    QCOMPARE(window->squares.size(), 4);
    QVERIFY(!window->squarePositions.contains(5));
    QVERIFY(!window->usedIds.contains(5));

    delete process;  // Cleanup after test
}

void TestMainWindow::addId(int id)
{
    window->addId(77);
    QVERIFY(window->usedIds.contains(77));
}

void TestMainWindow::testShowTimerInput()
{
    // Initially, the time input and label should be hidden
    QVERIFY(!window->timeInput->isVisible());
    QVERIFY(!window->timeLabel->isVisible());

    window->show();
    // Trigger the showTimerInput function
    window->showTimerInput();

    // Now, the time input and label should be visible
    QVERIFY(window->timeInput->isVisible());
    QVERIFY(window->timeLabel->isVisible());
}

void TestMainWindow::testSetCoreDumpLimit()
{
    window->compileProjects();
    struct rlimit core_limit;
    getrlimit(RLIMIT_CORE, &core_limit);

    QCOMPARE(core_limit.rlim_cur, RLIM_INFINITY);
    QCOMPARE(core_limit.rlim_max, RLIM_INFINITY);
}
void TestMainWindow::testRunScriptAndGetJsonPath()
{
    MainWindow mainWindow;

    // This test checks the function that runs a script and returns the path to the generated JSON file.
    // We assume the script is located in the dummy_program1 directory.
    QString cmakeFilePath = "../test/dummy_program1/CMakeLists.txt";

    // Run the function and verify that the returned path is not empty and the JSON file was created.
    QString jsonPath = mainWindow.runScriptAndGetJsonPath(cmakeFilePath);

    QVERIFY(!jsonPath.isEmpty()); // Ensure the function returns a non-empty path
    QVERIFY(QFile::exists(jsonPath)); // Ensure the file was created
}

void TestMainWindow::testUpdateProcessJsonFile()
{
    MainWindow mainWindow;
    int testId = 1;
    QString testName = "Test Process";
    QString testJsonPath = "../test/dummy_program1/generated_json_file.json";

    // This test checks the function that updates the sensors.json file with a new process.
    mainWindow.updateProcessJsonFile(testId, testName, testJsonPath);

    // Open the JSON file to verify that the process was correctly added.
    QFile file("sensors.json");
    QVERIFY(file.exists()); // Ensure the file exists

    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
    QJsonDocument jsonDoc = QJsonDocument::fromJson(file.readAll());
    QJsonObject jsonObject = jsonDoc.object();
    file.close();

    // Verify that the process data was added correctly
    QVERIFY(jsonObject.contains(QString::number(testId))); // Ensure process ID exists
    QJsonObject processDetails = jsonObject[QString::number(testId)].toObject();
    QCOMPARE(processDetails["name"].toString(), testName); // Ensure the name is correct
    QCOMPARE(processDetails["pathToJson"].toString(), testJsonPath); // Ensure the path is correct
}

void TestMainWindow::testDeleteProcessFromJsonFile()
{
    MainWindow mainWindow;
    int testId = 1;

    // This test checks the function that removes a process from the sensors.json file.
    mainWindow.deleteProcessFromJsonFile(testId);

    // Open the JSON file to verify that the process was correctly removed.
    QFile file("sensors.json");
    QVERIFY(file.exists()); // Ensure the file exists

    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
    QJsonDocument jsonDoc = QJsonDocument::fromJson(file.readAll());
    QJsonObject jsonObject = jsonDoc.object();
    file.close();

    // Verify that the process was removed from the file
    QVERIFY(!jsonObject.contains(QString::number(testId))); // Ensure process ID was deleted
}


// Test if build directory exists
void TestMainWindow::testOpenDependencySelectionProject_BuildDirectoryExists()
{
    MainWindow mainWindow;
    QString buildDir = "../../control-build_conditons/build";

    // Ensure the directory exists
    QDir dir(buildDir);
    QVERIFY(dir.exists());

    // Test function to ensure it doesn't create the directory again
    mainWindow.openDependencySelectionProject();

    // Ensure the directory still exists
    QVERIFY(dir.exists());
}

// Test when build directory does not exist
void TestMainWindow::testOpenDependencySelectionProject_BuildDirectoryNotExists()
{
    MainWindow mainWindow;
    QString buildDir = "../../control-build_conditons/build";

    // Remove the directory if it exists
    QDir dir(buildDir);
    if (dir.exists()) {
        dir.removeRecursively();
    }

    QVERIFY(!dir.exists());

    // Run the function to create the directory
    mainWindow.openDependencySelectionProject();

    // Verify that the directory was created
    QVERIFY(dir.exists());
}

// Test checkJsonFileAndSetButtons when BSON file exists
void TestMainWindow::testCheckJsonFileAndSetButtons_FileExists()
{
    MainWindow mainWindow;
    QString bsonFilePath = "../../control/conditions.bson";

    // Create the BSON file for the test
    QFile file(bsonFilePath);
    if (!file.exists()) {
        file.open(QIODevice::WriteOnly);
        file.close();
    }

    QVERIFY(QFile::exists(bsonFilePath));

    // Call the function and check button states
    mainWindow.checkJsonFileAndSetButtons();

    QVERIFY(mainWindow.compileButton->isEnabled());
    QVERIFY(mainWindow.runButton->isEnabled());

    // Clean up
    file.remove();
}

// Test checkJsonFileAndSetButtons when BSON file does not exist
void TestMainWindow::testCheckJsonFileAndSetButtons_FileNotExists()
{
    MainWindow mainWindow;
    QString bsonFilePath = "../../control/conditions.bson";

    // Ensure the BSON file does not exist
    QFile file(bsonFilePath);
    if (file.exists()) {
        file.remove();
    }

    QVERIFY(!QFile::exists(bsonFilePath));

    // Call the function and check button states
    mainWindow.checkJsonFileAndSetButtons();

    QVERIFY(!mainWindow.compileButton->isEnabled());
    QVERIFY(!mainWindow.runButton->isEnabled());
}
QTEST_MAIN(TestMainWindow)
#include "test_main_window.moc"