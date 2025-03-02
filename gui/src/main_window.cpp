#include <QMessageBox>
#include <QHBoxLayout>
#include <QFileInfo>
#include <QPixmap>
#include <QFileDialog>
#include <QTimer>
#include <QJsonDocument>
#include <csignal>
#include <fstream>
#include <ctime>
#include <thread>
#include <sys/resource.h>
#include <iostream>
#include <sys/wait.h>
#include <unistd.h>
#include <QProcess>
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <cstdlib>
#include <string>
#include <iostream>
#include <QSysInfo>
#include <csignal>
#include <fstream>
#include <ctime>
#include <thread>
#include <sys/resource.h>
#include <iostream>
#include <sys/wait.h>
#include <unistd.h>
#include <QProcess>
#include <QFileInfo>
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <cstdlib>
#include <string>
#include <iostream>
#include <QSysInfo>
#include <QJsonObject>
#include <QJsonArray>
#include <QFileSystemWatcher>
#include <QtConcurrent/QtConcurrent>
#include "process.h"
#include "main_window.h"
#include "draggable_square.h"
#include "process_dialog.h"
#include "frames.h"
#include "log_handler.h"
#include "compiler.h"

int sizeSquare = 120;
int rotationTimerIntervals = 100;
bool flag=false;
logger MainWindow::guiLogger("gui");
const QString LOG_FILE_BUS_MANAGER_PATH =  "../../main_bus/build/shared_log_file_name.txt";

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), 
    timer(nullptr),sqlDataManager(new DbManager(this))
{
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget);

    QWidget *toolbox = new QWidget(this);
    toolboxLayout = new QVBoxLayout(toolbox);
    compileButton = new QPushButton("Compile", this);
    runButton = new QPushButton("Run", this);
    endButton = new QPushButton("End", this);
    timerButton = new QPushButton("Set Timer", this);
    timeInput = new QLineEdit(this);
    timeLabel = new QLabel("Enter time in seconds:", this);
    logOutput = new QTextEdit(this);
    QPushButton *chooseButton = new QPushButton("Choose Image", this);
    QPushButton *showSimulationButton =
        new QPushButton("Show\nSimulation", this);
    showSimulationButton->setStyleSheet(
        "QPushButton {"
        "border: 2px solid #8f8f91;"
        "border-radius: 25px;"
        "background-color: #d1d1d1;"
        "min-width: 50px;"
        "min-height: 50px;"
        "font-size: 14px;"
        "}"
        "QPushButton:pressed {"
        "background-color: #a8a8a8;"
        "}");
    QPushButton *loadSimulationButton =
        new QPushButton("LOAD\nSIMULATION", toolbox);
    loadSimulationButton->setFixedHeight(
        loadSimulationButton->sizeHint().height() * 1.5);

    timeLabel->hide();
    timeInput->hide();
    logOutput->setReadOnly(true);

    mainLayout->addWidget(toolbox);

    logHandler = LogHandler();
    frames = new Frames(logHandler);

    QPushButton *addProcessButton = new QPushButton("Add Process", toolbox);
    debugCheckBox = new QCheckBox("debug", this);

    toolboxLayout->addWidget(addProcessButton);
    toolboxLayout->insertWidget(1, showSimulationButton);
    toolboxLayout->insertWidget(2, loadSimulationButton);
    toolboxLayout->addStretch();
    dataHandler = new DbManager(this);
    QPushButton *historyButton = new QPushButton("Show Simulation History", this);
    
    connect(historyButton, &QPushButton::clicked, this, &MainWindow::openHistoryWindow);
    connect(addProcessButton, &QPushButton::clicked, this,
            &MainWindow::createNewProcess);
    connect(compileButton, &QPushButton::clicked, this,
            &MainWindow::compileProjects);
    connect(runButton, &QPushButton::clicked, this, &MainWindow::runProjects);
    connect(endButton, &QPushButton::clicked, this, &MainWindow::endProcesses);
    connect(timerButton, &QPushButton::clicked, this,
            &MainWindow::showTimerInput);
    connect(chooseButton, &QPushButton::clicked, this,
            &MainWindow::openImageDialog);
    connect(showSimulationButton, &QPushButton::clicked, this,
            &MainWindow::showSimulation);
    connect(loadSimulationButton, &QPushButton::clicked, this,
            &MainWindow::loadSimulation);
    connect(frames, &Frames::simulationFinished, this,
            &MainWindow::updateShowSimulationButton);

    // Create and set up the loading spinner (static PNG)
    loadingLabel = new QLabel(this);
    // Set up the loading spinner (rotating static image)
    loadingLabel->setFixedSize(80, 80);  // Set smaller fixed size (adjust as needed)
    loadingLabel->setScaledContents(true);  // Make sure the image scales with the label
    loadingPixmap = QPixmap("../loading.png");  // Path to the PNG image
    loadingLabel->setPixmap(loadingPixmap);
    loadingLabel->setAlignment(Qt::AlignCenter);
    loadingLabel->hide();  // Initially hidden

    // Initialize rotation-related variables
    rotationTimer = new QTimer(this);
    rotationAngle = 0;

    // Connect the timer to the rotation function
    connect(rotationTimer, &QTimer::timeout, this, &MainWindow::rotateImage);

    toolbox->setMaximumWidth(100);
    toolbox->setMinimumWidth(100);

    // Create the new button to open the Dependency selection project
    openDependencySelectionProjectButton = 
        new QPushButton("DependencySelection", this);
    toolboxLayout->addWidget(openDependencySelectionProjectButton);  // Add button to the toolbox layout
    
    // Connect the button's click signal to the function that opens the second project
    connect(openDependencySelectionProjectButton,
            &QPushButton::clicked, this,
            &MainWindow::openDependencySelectionProject
        );

    // Add the loading label to the toolbox layout (under the buttons)
    toolboxLayout->addWidget(loadingLabel);
    debugCheckBox->setVisible(true);
    toolboxLayout->addWidget(debugCheckBox);
    toolboxLayout->addWidget(compileButton);
    toolboxLayout->addWidget(runButton);
    runButton->setEnabled(false);
    toolboxLayout->addWidget(endButton);
    toolboxLayout->addWidget(timerButton);
    toolboxLayout->addWidget(timeLabel);
    toolboxLayout->addWidget(timeInput);
    toolboxLayout->addWidget(logOutput);
    toolboxLayout->addWidget(chooseButton);
    toolboxLayout->addWidget(historyButton);
    


    workspace = new QWidget(this);
    workspace->setStyleSheet("background-color: white;");

    imageLabel = new QLabel(this);
    imageLabel->setAlignment(Qt::AlignCenter);
    imageLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);

    setDefaultBackgroundImage();
    mainLayout->addWidget(workspace);
    centralWidget->setLayout(mainLayout);
    // Check if the JSON file exists and delete it if it does
    QFile file("sensors.json");
    if (file.exists()) {
        // Remove the file if it exists
        if (file.remove()) {
            guiLogger.logMessage(logger::LogLevel::INFO,
                "Existing sensors.json file removed.");
        } else {
            guiLogger.logMessage(logger::LogLevel::ERROR, 
                "Failed to remove sensors.json file.");
        }
    }

    stateManager = new SimulationStateManager(this);
    void setDefaultBackgroundImage();

    int id = 0;
    const QString styleSheet =
        "QWidget {"
        "  background: qlineargradient(x1: 0, y1: 0, x2: 1, y2: 1, "
        "    stop: 0 #FDE1E1, stop: 1 #D4A5FF);"  // Gradient from purple to pink
        "  border: 3px solid silver;"             // Silver-colored borders
        "  border-radius: 10px;"                  // Rounded corners
        "}";

        QMap<KeyPermission, bool> allPermissions = {
            {VERIFY, true},
            {SIGN, true},
            {ENCRYPT, true},
            {DECRYPT, true},
            {EXPORTABLE, true}
        };

    Process *mainProcess =
        new Process(id, "Bus_Manager", "../../main_bus/CMakeLists.txt",
            "QEMUPlatform", allPermissions);
    addProcessSquare(
        mainProcess,
        QPoint((id % 2) * (sizeSquare + 10), (id / 2) * (sizeSquare + 10)),
        sizeSquare, sizeSquare, styleSheet);
    addId(id++);
    Process *hsmProcess =
        new Process(id, "HSM", "../test/dummy_program1/CMakeLists.txt",
            "QEMUPlatform", allPermissions);
    addProcessSquare(
        hsmProcess,
        QPoint((id % 2) * (sizeSquare + 10), (id / 2) * (sizeSquare + 10)),
        sizeSquare, sizeSquare, styleSheet);
    addId(id++);
    Process *logsDbProcess =
        new Process(id, "LogsDb", "path/to/LogsDb/directory/CMakeLists.txt",
            "QEMUPlatform", allPermissions);
    addProcessSquare(
        logsDbProcess,
        QPoint((id % 2) * (sizeSquare + 10), (id / 2) * (sizeSquare + 10)),
        sizeSquare, sizeSquare, styleSheet);
    addId(id++);
    Process *busManagerProcess =
        new Process(id, "Control", "../../control/CMakeLists.txt",
            "QEMUPlatform", allPermissions);
    addProcessSquare(
        busManagerProcess,
        QPoint((id % 2) * (sizeSquare + 10), (id / 2) * (sizeSquare + 10)),
        sizeSquare, sizeSquare, styleSheet);
    addId(id++);

    // Create QFileSystemWatcher to monitor the directory
    QFileSystemWatcher *watcher = new QFileSystemWatcher(this);
    watcher->addPath("../../control");

    // Initial check if the file exists
    checkJsonFileAndSetButtons();

    // Connect to the directoryChanged event
    connect(watcher, &QFileSystemWatcher::directoryChanged, this, [this]() {
        checkJsonFileAndSetButtons();
    });
}

MainWindow::~MainWindow()
{
    qDeleteAll(squares);
    if (timer) {
        delete timer;
    }
}
    
void MainWindow::createNewProcess()
{
    ProcessDialog dialog(this);

    if (dialog.exec() == QDialog::Accepted && dialog.isValid()) {
        int id = dialog.getId();
        if (id <= 4) {
            QMessageBox::warning(this, "Invalid ID",
                                 "The ID must be greater than 4.");
            MainWindow::guiLogger.logMessage(
                logger::LogLevel::ERROR, "MainWindow", "createNewProcess",
                "Invalid ID entered: " + std::to_string(id));
            return;
        }
        if (!isUniqueId(id)) {
            QMessageBox::warning(this, "Non-unique ID",
                                 "The ID entered is already in use. Please "
                                 "choose a different ID.");
            MainWindow::guiLogger.logMessage(
                logger::LogLevel::ERROR, "MainWindow", "createNewProcess",
                "Non-unique ID entered: " + std::to_string(id));
            return;
        }
        Process *newProcess;
        if (!dialog.pluginsEdit->text().isEmpty()) { 
            newProcess = new Process(id, dialog.getName(), dialog.getExecutionFile(),
                                     dialog.getQEMUPlatform(), dialog.getPlugins(),
                                     dialog.getSelectedPermissionsMap());
        } else {
            newProcess = new Process(id, dialog.getName(), dialog.getExecutionFile(),
                                     dialog.getQEMUPlatform(),
                                     dialog.getSelectedPermissionsMap());
        }
        addProcessSquare(newProcess);
        addId(id);
        MainWindow::guiLogger.logMessage(
            logger::LogLevel::INFO, "MainWindow", "createNewProcess",
            "New process created with ID: " + std::to_string(id));
    }
}

void MainWindow::openDialog()
{
    QDialog dialog;    
    dialog.setWindowTitle("Saving the simulation?");
    dialog.resize(300, 150);

    QLabel *label = new QLabel("Save the simulation?");
    label->setAlignment(Qt::AlignCenter); 
    QPushButton *yesButton = new QPushButton("Yes");
    QPushButton *noButton = new QPushButton("No");

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(yesButton);
    layout->addWidget(noButton);
    dialog.setLayout(layout);

    QObject::connect(yesButton, &QPushButton::clicked, [&]()
    {
        dialog.close();
        QDialog inputDialog;
        inputDialog.setWindowTitle("insert simulation name");

        QLineEdit *input = new QLineEdit();
        QPushButton *saveButton = new QPushButton("save");

        QVBoxLayout *inputLayout = new QVBoxLayout;
        inputLayout->addWidget(new QLabel("insert simulation name"));
        inputLayout->addWidget(input);
        inputLayout->addWidget(saveButton);
        inputDialog.setLayout(inputLayout);

        QObject::connect(saveButton, &QPushButton::clicked, [&]()
        {
            QString simulationName =input->text();
            inputDialog.close();
            QMessageBox::information(nullptr, "save", 
                "The name of the simulation is saved: " + simulationName);
            QString filePath = LOG_FILE_BUS_MANAGER_PATH;
            QString fullPath = getPathLogBus(filePath);  
            QString bsonFilePath = "simulation_state.bson";     //  BSON

            QString logData =sqlDataManager->readLogFile(fullPath);
            if (logData.isEmpty()) {
                        MainWindow::guiLogger.logMessage(logger::LogLevel::ERROR,
                        "Log data is empty!");
            }

            // BSON
            QByteArray bsonData =sqlDataManager->readBsonFile(bsonFilePath);
            if (bsonData.isEmpty()) {
                MainWindow::guiLogger.logMessage(logger::LogLevel::ERROR,
                "BSON data is empty!");
            }  

            if (!sqlDataManager->insertDataToDatabase(simulationName,
                bsonData, logData)) {
                MainWindow::guiLogger.logMessage(logger::LogLevel::ERROR,
                "Failed to insert data into the database.");
            } else {
                MainWindow::guiLogger.logMessage(logger::LogLevel::INFO,
                "Data successfully inserted into the database.");
            }
        });

        inputDialog.exec();
    });

    QObject::connect(noButton, &QPushButton::clicked, [&]()
    {
        dialog.close();
    });

    dialog.exec();
}

Process *MainWindow::getProcessById(int id)
{
    for (DraggableSquare *square : squares) {
        if (square->getId() == id) {
            return square->getProcess();
        }
    }
    return nullptr; 
}

void MainWindow::addProcessSquare(Process *&process)
{
    DraggableSquare *square = new DraggableSquare(workspace);
    square->setProcess(process);
    square->setId(process->getId());
    QPoint pos = squarePositions.value(process->getId(), QPoint(0, 0));
    square->move(pos);
    square->show();
    square->setDragStartPosition(pos);
    squarePositions[process->getId()] = pos;
    squares.push_back(square);

    ProcessDialog::addingNewProcess = true;

    createProcessConfigFile(process->getId(), process->getExecutionFile());
    QString jsonFilePath = runScriptAndGetJsonPath(process->getExecutionFile());
    if (jsonFilePath.isEmpty()) {
            guiLogger.logMessage(logger::LogLevel::DEBUG,
                         "Failed to run script and retrieve JSON file path.");
    } else {
        // Update the JSON file with the new process
        updateProcessJsonFile(process->getId(), process->getName(), jsonFilePath);
    }
}

void MainWindow::addProcessSquare(Process *process, QPoint position, int width,
                                  int height, const QString &color)
{
    DraggableSquare *square =
        new DraggableSquare(workspace, color, width, height);

    square->setProcess(process);
    square->setId(process->getId());
    QPoint pos = squarePositions.value(process->getId(), position);
    square->move(pos);
    square->show();

    ProcessDialog::addingNewProcess = true;

    squarePositions[process->getId()] = pos;
    squares.push_back(square);
    createProcessConfigFile(process->getId(), process->getExecutionFile());
    QString jsonFilePath = runScriptAndGetJsonPath(process->getExecutionFile());

    if (jsonFilePath.isEmpty()) {
            guiLogger.logMessage(logger::LogLevel::DEBUG,
                         "Failed to run script and retrieve JSON file path.");
    } else {
        // Update the JSON file with the new process
        updateProcessJsonFile(process->getId(), process->getName(), jsonFilePath);
    }
}

void MainWindow::openHistoryWindow()
{
    historyWindow = new HistoryWindow(dataHandler, this);
    historyWindow->show();
}

bool MainWindow::isUniqueId(int id)
{
    return !usedIds.contains(id);
}

void MainWindow::addId(int id)
{
    usedIds.insert(id);
}

void MainWindow::updateTimer()
{
    QString inputText = timeInput->text();
    bool ok = true;
    int time = 0;

    if (!inputText.isEmpty()) {
        time = inputText.toInt(&ok);
    }

    if (!ok || (time <= 0 && !inputText.isEmpty())) {
        QMessageBox::warning(this, "Invalid Input",
                             "Please enter a valid number of seconds.");
        timeInput->clear();
        MainWindow::guiLogger.logMessage(
            logger::LogLevel::ERROR, "MainWindow", "startProcesses",
            "Invalid timer input: " + inputText.toStdString());
        return;
    }

    // Set the text to be centered and styled for better clarity
    timeInput->setAlignment(Qt::AlignCenter);
    QFont font = timeInput->font();
    font.setPointSize(10);  // Increase font size for better visibility
    timeInput->setFont(font);

    if (time > 0) {
        logOutput->append("Timer started for " + QString::number(time) +
                          " seconds.");
        MainWindow::guiLogger.logMessage(
            logger::LogLevel::INFO, "MainWindow", "startProcesses",
            "Timer started for " + std::to_string(time) + " seconds");

        if (timer) {
            timer->stop();
            delete timer;
        }

        timer = new QTimer(this);
        connect(timer, &QTimer::timeout, this, [this, time]() mutable
        {

            if (time > 0) {
                time--;
                timeInput->setText(QString::number(time));
                timeInput->setAlignment(Qt::AlignCenter);  // Keep text centered
            } else {
                timerTimeout();
            }
        });

        timer->start(1000);
        timeLabel->hide();
        timeInput->setEnabled(false);
    }
}

void MainWindow::endProcesses()
{
    enableAllButtons();
    hideLoadingIndicator();
    logOutput->append("Ending processes...");
    MainWindow::guiLogger.logMessage(
        logger::LogLevel::INFO,
        "MainWindow::endProcesses()   Ending processes");
    
    if (timer) {
        timer->stop();
        delete timer;
        timer = nullptr;
    }
    timeLabel->show();
    timeInput->clear();
    stateManager->saveSimulationState("simulation_state.bson", squares,
                                      currentImagePath);
    MainWindow::guiLogger.logMessage(logger::LogLevel::INFO,
                                     "MainWindow::endProcesses  Simulation "
                                     "data saved to simulation_state.bson");
    for (const QPair<QProcess *, int> &pair : runningProcesses) {
        QProcess *process = pair.first;
        int id = pair.second;
        if (process->state() != QProcess::NotRunning) {
            // Use a singleShot timer to avoid blocking
            QTimer::singleShot(0, process, [process]() {
                process->terminate();
                QObject::connect(
                    process,
                    QOverload<int, QProcess::ExitStatus>::of(
                        &QProcess::finished),
                    [process]() {
                        process
                            ->deleteLater();  // Cleanup process after it's finished
                    });
            });
        }
        // Hide stop button for the corresponding DraggableSquare
        for (DraggableSquare *square : squares) {
            if (square->getProcess()->getId() == id) {
                square->setStopButtonVisible(false);  // Hide the stop button
                break;
            }
            // Use a singleShot timer to avoid blocking
            QTimer::singleShot(0, process, [process]() {
                process->terminate();
                QObject::connect(
                    process,
                    QOverload<int, QProcess::ExitStatus>::of(
                        &QProcess::finished),
                    [process]() {
                        process
                            ->deleteLater();  // Cleanup process after it's finished
                    });
            });
        }
    }
    delete frames;
    runningProcesses.clear();
    openDialog();
}

void MainWindow::stopProcess(int deleteId)
{
    for (int i = 0; i < runningProcesses.size(); ++i) {
        QProcess *process = runningProcesses[i].first;
        int id = runningProcesses[i].second;

        if (id == deleteId && id > 3) {
            if (process->state() != QProcess::NotRunning) {
                logOutput->append("Ending process...");
                process->terminate();
                process->waitForFinished();
            }

            process->deleteLater();
            process->setProperty("isStopped", true);
            break;
        }
    }
    guiLogger.logMessage(logger::LogLevel::DEBUG,
                         "Failed to run script and retrieve JSON file path.");
}

void MainWindow::showTimerInput()
{
    timeLabel->show();
    timeInput->show();

    // Center the text and set the style
    timeInput->setAlignment(Qt::AlignCenter);
    QFont font = timeInput->font();
    font.setPointSize(10);  // Increase font size for better visibility
    timeInput->setFont(font);

    // Connect to textChanged signal to ensure text stays centered
    connect(timeInput, &QLineEdit::textChanged, this, [this]()
    {
        timeInput->setAlignment(Qt::AlignCenter);
    });

    // Set input validator to ensure only numbers are entered
    QIntValidator* validator = new QIntValidator(0, 999999, this);
    timeInput->setValidator(validator);

    guiLogger.logMessage(
        logger::LogLevel::DEBUG,
        "showTimerInput() called: 
        Timer input elements are 
        ssshown and centered.");
}

void MainWindow::timerTimeout()
{
    logOutput->append("Timer timeout reached.");
    guiLogger.logMessage(logger::LogLevel::INFO, "Timer timeout reached.");

    endProcesses();
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);  

    if (!currentImagePath.isEmpty()) {
        QPixmap pixmap(currentImagePath);
        imageLabel->setPixmap(pixmap.scaled(
            workspace->size(), Qt::KeepAspectRatio, 
            Qt::SmoothTransformation));
        imageLabel->setGeometry(workspace->rect());
    }
}

void MainWindow::setDefaultBackgroundImage()
{
    // Set default image path
    QString defaultImagePath =
        "../img/1.jpg";  // Assuming default image is in resources
    currentImagePath = defaultImagePath;

    // Load and set the default imageדד
    QPixmap pixmap(defaultImagePath);
    if (!pixmap.isNull()) {
        // Clear any existing layout in the workspace (if necessary)
        QLayout *layout = workspace->layout();
        if (layout) {
            QLayoutItem *item;
            while ((item = layout->takeAt(0)) != nullptr) {
                delete item->widget();
                delete item;
            }
        }

        // Add imageLabel to workspace and set the pixmap
        imageLabel->setPixmap(pixmap.scaled(workspace->size(),
                                            Qt::IgnoreAspectRatio,
                                            Qt::SmoothTransformation));
        imageLabel->setAlignment(Qt::AlignCenter);
        imageLabel->setGeometry(workspace->rect());
        imageLabel->setScaledContents(true);

        // Add imageLabel to the workspace layout
        QVBoxLayout *newLayout = new QVBoxLayout(workspace);
        newLayout->addWidget(imageLabel);
        workspace->setLayout(newLayout);
    } else {
        MainWindow::guiLogger.logMessage(logger::LogLevel::ERROR,
                                         "There is no difult picture");
    }
}

void MainWindow::openImageDialog()
{
    // Open file dialog to select a new image
    QString imagePath = QFileDialog::getOpenFileName(
        this, tr("Select Image"), "", tr("Image Files (*.png *.jpg *.jpeg)"));

    if (!imagePath.isEmpty()) {
        currentImagePath = imagePath;

        // Load and set the new image
        QPixmap pixmap(imagePath);
        if (!pixmap.isNull()) {
            imageLabel->clear();  // Clear previous image
            imageLabel->setPixmap(pixmap.scaled(workspace->size(),
                                                Qt::IgnoreAspectRatio,
                                                Qt::SmoothTransformation));
            imageLabel->setGeometry(workspace->rect());
            imageLabel->setScaledContents(true);
            guiLogger.logMessage(
                logger::LogLevel::INFO,
                "openImageDialog() called: Image loaded and displayed.");
        } else {
            guiLogger.logMessage(
                logger::LogLevel::ERROR,
                "openImageDialog() failed: Unable to load image from path " +
                    imagePath.toStdString());
        }
    } else {
        guiLogger.logMessage(
            logger::LogLevel::INFO,
            "openImageDialog() canceled: No image path selected.");
    }
}

QString MainWindow::getPathLogBus(const QString &pathFile)
{
    QString path;
    
    QFile file(pathFile);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        path = in.readLine();
        guiLogger.logMessage(
            logger::LogLevel::INFO,
            "Successful to open file: " + pathFile.toStdString());
        file.close();
    } else {
        guiLogger.logMessage(
            logger::LogLevel::ERROR,
            "Unable to open file: " + pathFile.toStdString());
    }

    QString fullPath = "../../main_bus/build/" + path;
    
    if (!QFile::exists(fullPath)) {
        guiLogger.logMessage(
            logger::LogLevel::ERROR,
            "File does not exist: " + fullPath.toStdString());
    }

    return fullPath;
}

void MainWindow::showSimulation(bool isRealTime)
{
    if (isRealTime) {
        QString filePath = LOG_FILE_BUS_MANAGER_PATH;
        QString fullPath = getPathLogBus(filePath);
        QFuture<void> future = QtConcurrent::run([this, fullPath]() {
            logHandler.readLogFile(fullPath, true);
        });
        logHandler.analyzeLogEntries(this, &squares, nullptr);
        frames = new Frames(logHandler);  // Initialize Frames
        QLayout *oldLayout = workspace->layout();
        if (oldLayout) {
            delete oldLayout;
        }
        QVBoxLayout *framesLayout = new QVBoxLayout(workspace);
        framesLayout->addWidget(frames);
        workspace->setLayout(framesLayout);
        frames->startFrames();
    } else {
        if(!isSimulationRunning){
            QVariantMap record = dataHandler->getRecordById(offlineId);
            QString logData;
            if (!record.isEmpty())
                logData = record["log_data"].toString();
            if (!logData.isEmpty()) {
                logHandler.readLogFile(logData);
                QByteArray bsonData = record["bson_data"].toByteArray();
                bsonDocument = bson_new_from_data(
                    reinterpret_cast<const uint8_t *>(bsonData.constData()),
                    bsonData.size());
                logHandler.analyzeLogEntries(this, nullptr, bsonDocument);

                frames = new Frames(logHandler);  // Initialize Frames

                QLayout *oldLayout = workspace->layout();
                if (oldLayout) {
                    delete oldLayout;
                }

                QVBoxLayout *framesLayout = new QVBoxLayout(workspace);
                framesLayout->addWidget(frames);
                workspace->setLayout(framesLayout);

                frames->startFrames();
                isSimulationRunning = true;
            } else {
                MainWindow::guiLogger.logMessage(
                    logger::LogLevel::ERROR,
                    "Failed to retrieve log data from database");
            }
        } else {
            frames->stopFrames();
        }
    }
}

void MainWindow::loadSimulation()
{
    QList<QVariantMap> simulations = dataHandler->getAllDataSimulation();

    if (simulations.isEmpty()) {
        QMessageBox::warning(this, "Error", "No simulations found to load.");
        return;
    }

    QStringList simulationNames;
    for (const auto &sim : simulations) {
        simulationNames.append(sim["input_string"].toString());
    }

    bool ok;
    QString selectedSimulation = QInputDialog::getItem(
        this, "Select Simulation", "Choose a simulation:", simulationNames, 0,
        false, &ok);
    if (ok && !selectedSimulation.isEmpty()) {
        loadSelectedSimulation(simulations, selectedSimulation);
    }
}

void MainWindow::loadSelectedSimulation(const QList<QVariantMap> &simulations,
                                        const QString &selectedSimulation)
{
    for (const auto &sim : simulations) {
        if (sim["input_string"].toString() == selectedSimulation) {
            int simulationId = sim["id"].toInt();
            offlineId = simulationId;
            QByteArray bsonData = sim["bson_data"].toByteArray();
            SimulationStateManager *state = new SimulationStateManager();
            bsonDocument = bson_new_from_data(
                reinterpret_cast<const uint8_t *>(bsonData.constData()),
                bsonData.size());
            if (bsonDocument)
                state->loadSimulationState(bsonDocument);
            for (auto sq : squares) {
                delete (sq);
            }
            squares.clear();
            squarePositions.clear();
            if (!state->data.squares.empty()) {
                for (auto sqr : state->data.squares) {
                    Process *process =
                        new Process(sqr->getProcess()->getId(),
                                    sqr->getProcess()->getName(),
                                    sqr->getProcess()->getExecutionFile(),
                                    sqr->getProcess()->getQEMUPlatform(),
                                    sqr->getProcess()->getSecurityPermissions());
                    addProcessSquare(process, sqr->pos(), sqr->width(),
                                     sqr->height(), sqr->styleSheet());
                    addId(sqr->getId());
                }
            }
            break;
        }
    }
}

QString MainWindow::getExecutableName(const QString &buildDirPath)
{
    QDir buildDir(buildDirPath);

    // Check if the directory exists
    if (!buildDir.exists()) {
        QMessageBox::critical(this, "Directory Error",
                              "The specified build directory does not exist.");
        guiLogger.logMessage(
            logger::LogLevel::ERROR,
            "getExecutableName() failed: Directory does not exist: " +
                buildDirPath.toStdString());
        return QString();
    }

    QStringList files = buildDir.entryList(QDir::Files | QDir::NoSymLinks);

    // If the directory is empty or has no files
    if (files.isEmpty()) {
        QMessageBox::critical(
            this, "File Error",
            "No files found in the specified build directory.");
        guiLogger.logMessage(
            logger::LogLevel::ERROR,
            "getExecutableName() failed: No files found in directory: " +
                buildDirPath.toStdString());
        return QString();
    }

    foreach (const QString &file, files) {
        QFileInfo fileInfo(buildDir.filePath(file));

        // Skip files with a suffix (i.e., non-executables)
        if (!fileInfo.suffix().isEmpty())
            continue;

        // If the file is executable, return its name
        if (fileInfo.isExecutable()) {
            return fileInfo.fileName();
        }
    }

    // No executable found
    QMessageBox::critical(
        this, "Executable Not Found",
        "No executable file found in the specified build directory.");
    guiLogger.logMessage(
        logger::LogLevel::ERROR,
        "getExecutableName() failed: No executable file found in directory: " +
            buildDirPath.toStdString());
    return QString();
}

void MainWindow::setCoreDumpLimit()
{
    struct rlimit core_limit;
    core_limit.rlim_cur = RLIM_INFINITY;  // Set soft limit to unlimited
    core_limit.rlim_max = RLIM_INFINITY;  // Set hard limit to unlimited

    if (setrlimit(RLIMIT_CORE, &core_limit) != 0) {
        MainWindow::guiLogger.logMessage(
            logger::LogLevel::ERROR,
            "MainWindow::setCoreDumpLimit Failed to set core dump limit.");
    } else {
        MainWindow::guiLogger.logMessage(
            logger::LogLevel::INFO,
            "MainWindow::setCoreDumpLimit Core dump limit successfully set to "
            "unlimited.");
    }
}

void MainWindow::compileProjects()
{
    guiLogger.logMessage(logger::LogLevel::INFO,
                         "Compiling and running projects started.");

    // Disable the run button until compilation finishes
    runButton->setEnabled(false);
    setCoreDumpLimit();

    // Clear previous running processes
    for (const QPair<QProcess *, int> &pair : runningProcesses) {
        QProcess *process = pair.first;
        process->terminate();
        process->waitForFinished();
        delete process;
    }

    runningProcesses.clear();
    bool compileSuccessful = true;  // Track if all compilations succeed

    QList<QThread *> threads;
    QSet<QString> uniquePaths;  // Set to hold unique paths
    for (DraggableSquare *square : squares) {
        QString executionFilePath = square->getProcess()->getExecutionFile();
        if(square->getProcess()->getConstructorType()==Process::ParameterizedConstructor2)
        {
          QString Plug=square->getProcess()->getPluginsEdit();
          flag=true;
        }
        uniquePaths.insert(executionFilePath);
    }

    for (QString executionFilePath : uniquePaths) {
        
        Compiler *compiler =
            new Compiler(executionFilePath, &compileSuccessful,flag,debugCheckBox->isChecked(),this);
        connect(compiler, &Compiler::logMessage, this,
                [this](const QString &message) {
                    guiLogger.logMessage(logger::LogLevel::ERROR,
                                         message.toStdString());
                });
        compiler->start();
        threads.append(compiler);
    }

    // Wait for all threads to finish
    for (QThread *thread : threads) {
        thread->wait();
        delete thread;  // Clean up threads after completion
    }

    if (compileSuccessful) {
        runButton->setEnabled(true);
        logOutput->append(
            "Compilation completed successfully. You can now run the "
            "projects.");
    } else {
        logOutput->append("Compilation failed. Please check the logs.");
    }
}

std::string MainWindow::getCoreDumpPath(qint64 pid,
                                        const std::string &executableName)
{
    std::string coreDumpPath;

    QString unameInfo = QSysInfo::kernelType();
    if (unameInfo == "linux" && QFile::exists("/mnt/wslg")) {
        coreDumpPath = "/mnt/wslg/dumps/core." + executableName + "." +
                       std::to_string(pid);
    } else if (unameInfo == "linux") {
        coreDumpPath = "/var/lib/systemd/coredump/core." + executableName +
                       "." + std::to_string(pid);
    }
    return coreDumpPath;
}

bool createBacktrace(const std::string &executablePath,
                     const std::string &coreDumpPath,
                     const std::string &outputFilePath)
{
    // Open the output file for writing
    std::ofstream outFile(outputFilePath, std::ios::out);

    // Get the current time (timestamp)
    auto now =
        std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::string timestamp = std::ctime(&now);
    timestamp.pop_back();  // Remove the newline at the end of ctime output

    // Append additional information to the output file
    outFile << "# Crash Report\n\n";
    outFile << "Timestamp: " << timestamp << "\n";
    outFile << "Executable: " << executablePath << "\n";
    outFile << "Core Dump Path: " << coreDumpPath << "\n";
    outFile << "\n# Backtrace\n";
    // Close the file before running gdb
    outFile.close();
    // Construct the gdb command
    std::string gdbCommand = "gdb -batch -ex 'bt' " + executablePath + " " +
                             coreDumpPath + " >> " + outputFilePath;
    // Execute the command
    int result = system(gdbCommand.c_str());
    // Re-open the file to log any errors after gdb has executed
    std::ofstream logFile(outputFilePath, std::ios::app);
    if (result != 0) {
        logFile.close();
        return false;
    } else {
        MainWindow::guiLogger.logMessage(
            logger::LogLevel::INFO,
            "MainWindow::createBacktrace Backtrace successfully saved to: " +
                outputFilePath);
    }
    logFile.close();
    return true;  // Return true if the gdb command succeeded
}

void MainWindow::runProjects()
{
    disableButtonsExceptEnd();
    showLoadingIndicator();
    updateTimer();
    QMetaObject::invokeMethod(this, "showSimulation", Qt::QueuedConnection, Q_ARG(bool, true));
    // bool isDebugMode=debugCheckBox->isChecked();
    MainWindow::guiLogger.logMessage(
        logger::LogLevel::INFO,
        "MainWindow::runProjects Starting to run projects.");

    for (DraggableSquare *square : squares) {
        QString executionFilePath = square->getProcess()->getExecutionFile();
        MainWindow::guiLogger.logMessage(
            logger::LogLevel::INFO,
            "MainWindow::runProjects Processing square with ID: " +
                std::to_string(square->getId()) +
                ", CMake path: " + executionFilePath.toStdString());

        if (executionFilePath.endsWith(".sh")) {
             MainWindow::guiLogger.logMessage(
                logger::LogLevel::ERROR,
                "MainWindow::runProjects sh: " + executionFilePath.toStdString());
            QProcess *scriptProcess = new QProcess(this);
            QStringList arguments;
            arguments<<executionFilePath;
            // if(isDebugMode){
            //     arguments<<"--debug";
            // }
            scriptProcess->start("bash",arguments);
            if (!scriptProcess->waitForStarted()) {
                MainWindow::guiLogger.logMessage(
                    logger::LogLevel::ERROR,
                    "MainWindow::runProjects Failed to start the bash "
                    "script: " +
                        executionFilePath.toStdString());
                logOutput->append("Failed to start the bash script: " +
                                  executionFilePath);
                logOutput->append(scriptProcess->readAllStandardError());
                delete scriptProcess;
                continue;
            }

            connect(
                scriptProcess, &QProcess::readyReadStandardOutput,
                [this, scriptProcess]()
                {
                    logOutput->append(scriptProcess->readAllStandardOutput());
                });
            connect(
                scriptProcess, &QProcess::readyReadStandardError,
                [this, scriptProcess]()
                {
                    logOutput->append(scriptProcess->readAllStandardError());
                });
           connect(
            scriptProcess, &QProcess::errorOccurred,
            [this, executionFilePath, scriptProcess, square]() {
                qint64 pid = scriptProcess->processId();

                // Extract the project directory from the CMakeLists.txt path
                QString projectDir = QFileInfo(executionFilePath).absolutePath();

                // Get the executable name from the build directory
                std::string executableName = QFileInfo(getExecutableName(projectDir + "/build")).fileName().toStdString();
                
                // Get the core dump path and backtrace file path
                std::string coreDumpPath = getCoreDumpPath(pid, executableName);
                std::string backtraceFilePath = projectDir.toStdString() + "/build/backtrace_" + std::to_string(pid) + ".txt";
                
                // Delay for core dump creation
                std::this_thread::sleep_for(std::chrono::milliseconds(500));

                // Call createBacktrace with the correct executable path
                bool success = createBacktrace(projectDir.toStdString() + "/build/" + executableName, coreDumpPath, backtraceFilePath);

                if (success) {
                    square->setDumpFilePath(QString::fromStdString(backtraceFilePath));
                    square->setCrashIndicator(true);

                    MainWindow::guiLogger.logMessage(logger::LogLevel::ERROR,
                                                    "Process crashed. Crash details in: " + backtraceFilePath);

                    if (square->getId() < 4) {
                        MainWindow::guiLogger.logMessage(logger::LogLevel::ERROR,
                                                        "Error: Simulation stopped because an initial process crashed.");
                        QTimer::singleShot(0, this, &MainWindow::endProcesses);
                        QMessageBox msgBox;
                            msgBox.setIcon(QMessageBox::Critical);
                            msgBox.setWindowTitle("CRITICAL ERROR!");
                            msgBox.setText(
                                "<div align='center'><b><font color='red' "
                                "size='+2'>One of the initial processes has "
                                "CRASHED!</font></b></div>");
                            msgBox.setInformativeText(
                                "<div align='center'>The simulation will stop "
                                "immediately.<br>"
                                "Please check the details below:</div>");
                            msgBox.setDetailedText(
                                "The crash report has been saved at: " +
                                square->getDumpFilePath());
                            msgBox.setStyleSheet(
                                "QLabel{min-width: 350px; font-size: 16px; "
                                "text-align: center;}");
                            msgBox.exec();
                    }
                }
            }
        );
        runningProcesses.append(qMakePair(scriptProcess, square->getProcess()->getId()));
        } else {
            QDir cmakeDir(QFileInfo(executionFilePath).absolutePath());
            QString buildDirPath = cmakeDir.absoluteFilePath("build");
            QDir buildDir(buildDirPath);

            if (!buildDir.exists()) {
                if (!buildDir.mkpath(buildDirPath)) {
                    guiLogger.logMessage(logger::LogLevel::ERROR,
                                         "Failed to create build directory: " +
                                             buildDirPath.toStdString());
                    logOutput->append("Failed to create build directory: " +
                                      buildDirPath);
                    continue;
                }
            }

            QString exeFile = getExecutableName(buildDirPath);
            QString executablePath = buildDir.absoluteFilePath(exeFile);
            QProcess *runProcess = new QProcess(this);
            runProcess->setWorkingDirectory(buildDirPath);
            QStringList runArguments;
            // if (isDebugMode) {
            //     runArguments << "--debug"; // 
            // }

            runProcess->start(executablePath, runArguments);

            connect(runProcess, &QProcess::readyReadStandardOutput,
                    [this, runProcess]()
                    {
                        logOutput->append(runProcess->readAllStandardOutput());
                    });
            connect(runProcess, &QProcess::readyReadStandardError,
                    [this, runProcess]()
                    {
                        logOutput->append(runProcess->readAllStandardError());
                    });
            connect(
                runProcess, &QProcess::errorOccurred,
                [this, executablePath, runProcess, buildDirPath, square]() {
                    qint64 pid = runProcess->processId();
                    std::string executableName =
                        QFileInfo(executablePath).fileName().toStdString();
                    std::string coreDumpPath =
                        getCoreDumpPath(pid, executableName);
                    std::string backtraceFilePath =
                        buildDirPath.toStdString() + "/backtrace_" +
                        std::to_string(pid) + ".txt";
                    std::this_thread::sleep_for(std::chrono::milliseconds(500));

                    bool success =
                        createBacktrace(executablePath.toStdString(),
                                        coreDumpPath, backtraceFilePath);

                    if (success) {
                        square->setDumpFilePath(
                            QString::fromStdString(backtraceFilePath));
                        square->setCrashIndicator(true);

                        MainWindow::guiLogger.logMessage(
                            logger::LogLevel::ERROR,
                            "MainWindow::runProjects Process crashed. Crash "
                            "details in: " +
                                backtraceFilePath);

                        if (square->getId() < 4) {
                            MainWindow::guiLogger.logMessage(
                                logger::LogLevel::ERROR,
                                "MainWindow::runProjects Error: Simulation "
                                "stopped because an initial process crashed.");
                            QTimer::singleShot(0, this,
                                               &MainWindow::endProcesses);
                            QMessageBox msgBox;
                            msgBox.setIcon(QMessageBox::Critical);
                            msgBox.setWindowTitle("CRITICAL ERROR!");
                            msgBox.setText(
                                "<div align='center'><b><font color='red' "
                                "size='+2'>One of the initial processes has "
                                "CRASHED!</font></b></div>");
                            msgBox.setInformativeText(
                                "<div align='center'>The simulation will stop "
                                "immediately.<br>"
                                "Please check the details below:</div>");
                            msgBox.setDetailedText(
                                "The crash report has been saved at: " +
                                square->getDumpFilePath());
                            msgBox.setStyleSheet(
                                "QLabel{min-width: 350px; font-size: 16px; "
                                "text-align: center;}");
                            msgBox.exec();
                        }
                    }
                });
            connect(
                runProcess,
                QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                this, &MainWindow::processFinished);
            runProcess->start(executablePath, QStringList());
            if (!runProcess->waitForStarted()) {
                guiLogger.logMessage(logger::LogLevel::ERROR,
                                     "Failed to start the program in " +
                                         buildDirPath.toStdString());
                logOutput->append("Failed to start the program in " +
                                  buildDirPath);
                logOutput->append(runProcess->readAllStandardError());
                delete runProcess;
                continue;
            }

            runningProcesses.append(
                qMakePair(runProcess, square->getProcess()->getId()));
        }
        square->setStopButtonVisible(true);
    }

    guiLogger.logMessage(logger::LogLevel::INFO,
                         "Compiling and running projects finished.");
}

void MainWindow::editSquare(int id)
{
    guiLogger.logMessage(logger::LogLevel::INFO,
                         "Editing square with ID: " + std::to_string(id));

    ProcessDialog::addingNewProcess = false;

    for (DraggableSquare *square : squares) {
        if (square->getProcess()->getId() == id) {
            ProcessDialog dialog(this);
            dialog.setId(square->getProcess()->getId());
            dialog.setName(square->getProcess()->getName());
            dialog.setExecutionFile(square->getProcess()->getExecutionFile());
            dialog.setQEMUPlatform(square->getProcess()->getQEMUPlatform());

            QMap<KeyPermission, bool> currentPermissions =
                square->getProcess()->getSecurityPermissions();
            dialog.setSecurityPermissions(currentPermissions); 

            if (dialog.exec() == QDialog::Accepted && dialog.isValid()) {
                Process *updatedProcess = new Process(
                    dialog.getId(), dialog.getName(),
                    dialog.getExecutionFile(),
                    dialog.getQEMUPlatform(),
                    dialog.getSelectedPermissionsMap());
                square->setProcess(updatedProcess);
                guiLogger.logMessage(logger::LogLevel::INFO,
                                     "Updated process details for square ID: " +
                                         std::to_string(id));
                // Remove the old proces from the JSON file
                deleteProcessFromJsonFile(id);
                // Update the JSON file with the new details
                updateProcessJsonFile(updatedProcess->getId(), updatedProcess->getName(), updatedProcess->getExecutionFile());
            }
            break;
        }
    }
}

void MainWindow::deleteSquare(int id)
{
    guiLogger.logMessage(logger::LogLevel::INFO,
                         "Deleting square with ID: " + std::to_string(id));
    auto it = std::find_if(
        squares.begin(), squares.end(), [id](DraggableSquare *square) {
            return square && square->getProcess()->getId() == id;
        });

    if (it != squares.end()) {
        DraggableSquare *toDelete = *it;
        it = squares.erase(it);
        if (toDelete) {
            toDelete->deleteLater();
            guiLogger.logMessage(
                logger::LogLevel::INFO,
                "Square with ID: " + std::to_string(id) + " deleted.");
        }
    } else {
        guiLogger.logMessage(
            logger::LogLevel::ERROR,
            "Square with ID: " + std::to_string(id) + " not found.");
    }
    usedIds.remove(id);
    squarePositions.remove(id);
    // Remove the process from the JSON file
    deleteProcessFromJsonFile(id);
}

void MainWindow::createProcessConfigFile(int id, const QString &processPath)
{
    // Creating a JSON object with the process ID
    QJsonObject jsonObject;
    jsonObject["ID"] = id;

    // Converting the object to a JSON document
    QJsonDocument jsonDoc(jsonObject);

    // Defining the file name and its path
    QString filePath = processPath + "/config.json";
    QFile configFile(filePath);

    // Opening the file for writing and checking if the file opened successfully
    if (configFile.open(QIODevice::WriteOnly)) {
        configFile.write(jsonDoc.toJson());
        configFile.close();

        guiLogger.logMessage(
            logger::LogLevel::INFO,
            "Config file created at: " + filePath.toStdString());
    } else {
        guiLogger.logMessage(
            logger::LogLevel::ERROR,
            "Failed to create config file at: " + filePath.toStdString());
    }
}

void MainWindow::updateProcessJsonFile(int id, const QString &name, const QString &pathToJson)
{
    QFile file("sensors.json");
    QJsonObject processesObject;

    if (file.exists()) {
        // The file exists, let's load it
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QJsonDocument jsonDoc = QJsonDocument::fromJson(file.readAll());
            processesObject = jsonDoc.object();
            file.close();
        }
    }

    // Add or update the process in the JSON object
    QJsonObject processDetails;
    processDetails["name"] = name;
    processDetails["pathToJson"] = pathToJson;

    processesObject[QString::number(id)] = processDetails;

    // Write the updated JSON object back to the file
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QJsonDocument jsonDoc(processesObject);
        file.write(jsonDoc.toJson());
        file.close();
    }
}

void MainWindow::deleteProcessFromJsonFile(int id)
{
    QFile file("sensors.json");
    if (!file.exists()) return;

    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        // Load the existing JSON content
        QJsonDocument jsonDoc = QJsonDocument::fromJson(file.readAll());
        QJsonObject processesObject = jsonDoc.object();
        file.close();

        // Remove the process from the JSON object
        processesObject.remove(QString::number(id));

        // Write the updated JSON object back to the file
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QJsonDocument newDoc(processesObject);
            file.write(newDoc.toJson());
            file.close();
        }
    }
}

void MainWindow::processFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    QProcess *finishedProcess = qobject_cast<QProcess *>(sender());
    if (finishedProcess) {
        int finishedProcessId =-1;
        // Find the ID of the process that finished
        for (const QPair<QProcess *, int> &pair : runningProcesses) {
            if (pair.first == finishedProcess) {
                finishedProcessId = pair.second;
                // Find the corresponding DraggableSquare
                for (DraggableSquare *square : squares) {
                    if (square->getProcess()->getId() == finishedProcessId) {
                        square->setStopButtonVisible(false);
                        break;
                    }
                }
                break;
            }
        }
        // Remove the finished process based on ID from runningProcesses
        for (int i = 0; i < runningProcesses.size(); ++i) {
            if (runningProcesses[i].second == finishedProcessId) {
                runningProcesses.removeAt(i);  // Remove the process with the matching ID
                break;
            }
        }
        // Check if all processes have finished
        if (runningProcesses.isEmpty()) {
            hideLoadingIndicator();  // Stop spinner
            enableAllButtons();      // Re-enable buttons
        }
    }
}

// Disable all buttons except the "End" button
void MainWindow::disableButtonsExceptEnd() 
{
    compileButton->setEnabled(true);
    runButton->setEnabled(true);
    timerButton->setEnabled(false);
}

// Enable all buttons
void MainWindow::enableAllButtons() 
{
    compileButton->setEnabled(true);
    runButton->setEnabled(true);
    timerButton->setEnabled(true);
}

void MainWindow::rotateImage()
{
    // Rotate the pixmap by the current angle
    QTransform transform;
    transform.rotate(rotationAngle);
    QPixmap rotatedPixmap = loadingPixmap.transformed(transform);

    // Update the label with the rotated pixmap
    loadingLabel->setPixmap(rotatedPixmap);

    // Increment the rotation angle for the next frame
    rotationAngle =
        (rotationAngle + 10) % 360;  // Rotate by 10 degrees each time
}

// Show the loading spinner with rotation
void MainWindow::showLoadingIndicator() 
{
    loadingLabel->show();
    rotationTimer->start(rotationTimerIntervals);  // Start the timer with ms intervals
}

// Hide the loading spinner and stop the rotation
void MainWindow::hideLoadingIndicator() 
{
    rotationTimer->stop();
    loadingLabel->hide();
}

void MainWindow::updateShowSimulationButton()
{
    MainWindow::guiLogger.logMessage(logger::LogLevel::INFO,
                                     "Updating button text to Show Simulation");
    showSimulationButton->setText("Show Simulation");
}

void MainWindow::openDependencySelectionProject()
{
    // Define the build directory
    QString buildDir = "../../control-build_conditons/build";
    
    // Check if the build directory exists, if not create it
    QDir dir(buildDir);
    if (!dir.exists()) {
        MainWindow::guiLogger.logMessage(
            logger::LogLevel::INFO,
            "Build directory does not exist. Creating build directory: " + 
            buildDir.toStdString());
        
        if (!dir.mkpath(buildDir)) {
            MainWindow::guiLogger.logMessage(
                logger::LogLevel::ERROR,
                "Failed to create build directory: " + buildDir.toStdString());
            return;
        }

        MainWindow::guiLogger.logMessage(
            logger::LogLevel::INFO,
            "Build directory created successfully: " + buildDir.toStdString());
    }

    // Add execution permissions to the compiled file
    QProcess *chmodProcess = new QProcess(this);
    chmodProcess->setWorkingDirectory(buildDir);
    chmodProcess->start("chmod", QStringList() << "+x" << "BuildCondition");

    if (!chmodProcess->waitForFinished()) {
        qDebug() << "Failed to set execution permissions.";
        qDebug() << chmodProcess->errorString();
        return;
    }

    qDebug() << "Execution permissions set successfully.";

    // Run the compiled executable
    QString program = buildDir + "/BuildCondition";
    qDebug() << "Attempting to start:" << program;

    // Using startDetached to run the program independently
    qint64 pid;
    QStringList arguments;
    if (QProcess::startDetached(program, arguments, buildDir, &pid)) {
        MainWindow::guiLogger.logMessage(
            logger::LogLevel::INFO,
             "Second project started successfully with PID:" + 
             QString::number(pid).toStdString());
    } else {
        MainWindow::guiLogger.logMessage(
            logger::LogLevel::INFO,
            "Failed to start the second project.");
    }
}

QString MainWindow::runScriptAndGetJsonPath(const QString& cmakeFilePath)
{
    // Derive the script path from the CMake file path
    QString scriptPath = QFileInfo(cmakeFilePath).absolutePath() 
        + "/create_json.sh"; // Adjusted path

    // Set script permissions using QProcess
    QProcess chmodProcess;
    QStringList chmodArgs;
    chmodArgs << "+x" << scriptPath; // Arguments for chmod
    chmodProcess.start("chmod", chmodArgs);

    if (!chmodProcess.waitForFinished()) {
        guiLogger.logMessage(logger::LogLevel::ERROR,
                             "Failed to set execute permissions for script: "
                             + chmodProcess.errorString().toStdString());
        return QString(); // Return empty if failed
    }

    // Prepare arguments for the script: output directory and filename
    QString outputDirectory = QFileInfo(cmakeFilePath).absolutePath(); // Set the output directory
    QString outputFilename = "generated_json_file.json"; // Set the desired JSON filename
    QStringList scriptArgs;
    scriptArgs << outputDirectory << outputFilename; // Pass arguments to the script

    // Execute the script
    QProcess scriptProcess;
    scriptProcess.start(scriptPath, scriptArgs); // Use the updated start method

    if (!scriptProcess.waitForFinished()) {
        guiLogger.logMessage(logger::LogLevel::ERROR,
                             "Failed to execute script: " 
                             + scriptProcess.errorString().toStdString());
        return QString(); // Return empty if failed
    }

    QString jsonFilePath = outputDirectory + "/" + outputFilename;  // Update with actual generated file name
    return jsonFilePath;
}

void MainWindow::checkJsonFileAndSetButtons()
{
    QString jsonFilePath = "../../control/conditions.bson";
    bool fileExists = QFile::exists(jsonFilePath);

    compileButton->setEnabled(true);
    runButton->setEnabled(true);
}

#include "moc_main_window.cpp"