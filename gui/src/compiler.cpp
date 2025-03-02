#include "compiler.h"
#include <QDebug>

Compiler::Compiler(QString cmakePath, bool *compileSuccessful,bool plug,
                   bool debugCompile,QObject *parent)
    : QThread(parent),
      cmakePath(cmakePath),
      compileSuccessful(compileSuccessful),
      plug(plug),
      debugCompile(debugCompile)
{
}

void Compiler::setUserDefines(QString defines)
{
    userDefines=defines;
}

QString Compiler::getUserDefines()
{
    return userDefines;
}

void Compiler::run()
{
    if (cmakePath.endsWith(".sh")) {  // Handle shell script compilation
        QFile scriptFile(cmakePath);
        if (!scriptFile.exists()) {
            emit logMessage("Shell script file does not exist: " + cmakePath);
            *compileSuccessful = false;
            return;
        }

        // Make shell script executable if needed
        if ((scriptFile.permissions() & QFileDevice::ExeUser) == 0) {
            QProcess makeExecutableProcess;
            makeExecutableProcess.start("chmod", QStringList()
                                                     << "+x" << cmakePath);
            if (!makeExecutableProcess.waitForFinished()) {
                *compileSuccessful = false;
                return;
            }
        }

        QProcess scriptProcess;
        scriptProcess.start("bash", QStringList() << cmakePath);
        scriptProcess.waitForFinished();
    }
    else {  // Handle CMake compilation
        QDir cmakeDir =
            QFileInfo(cmakePath)
                .absolutePath();  // Get the directory of CMakeLists.txt
        QString buildDirPath = cmakeDir.absoluteFilePath("build");
        QDir buildDir(buildDirPath);

        // Create build directory if it doesn't exist
        if (!buildDir.exists() && !buildDir.mkpath(".")) {
            emit logMessage("Failed to create build directory " + buildDirPath);
            *compileSuccessful = false;
            return;
        }

        QProcess cmakeProcess;
        cmakeProcess.setWorkingDirectory(buildDirPath);
        
        QStringList cmakeArgs;
        cmakeArgs << cmakeDir.absolutePath();  // Use project dir
        if(plug){
            QString defines = getUserDefines(); 
            if (!defines.isEmpty()) {
                cmakeArgs << "-D" + defines; 
            }
        }
        if (debugCompile) {
            emit logMessage("Compiling in DEBUG mode");D
            cmakeArgs << "-DLOG_LEVEL=DEBUG"; 
        }
        cmakeProcess.start("cmake", cmakeArgs);
       
        if (!cmakeProcess.waitForFinished()) {
            emit logMessage("Failed to run cmake in " + buildDirPath);
            *compileSuccessful = false;
            return;
        }

        QProcess makeProcess;
        makeProcess.setWorkingDirectory(buildDirPath);
        makeProcess.start("make", QStringList());

        if (!makeProcess.waitForFinished()) {
            emit logMessage("Failed to compile in " + buildDirPath);
            *compileSuccessful = false;
        }
    }
}