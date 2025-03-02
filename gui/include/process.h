#ifndef PROCESS_H
#define PROCESS_H

#include <QString>
#include "process_dialog.h"

class Process {
    
public:
    enum ConstructorType {
        ParameterizedConstructor1,
        ParameterizedConstructor2
    };
    Process(int id, const QString &name, const QString &executionFile,
            const QString &qemuPlatform, 
            const QMap<KeyPermission, bool> &permissions);
    Process(int id, const QString &name, const QString &executionFile,
            const QString &qemuPlatform, const QString &pluginsEdit,
            const QMap<KeyPermission, bool> &permissions);
    Process();
    Process(const Process &other);

    int getId() const;
    QString getName() const;
    QString getExecutionFile() const;
    QString getQEMUPlatform() const;
    bool getSecurityPermission(KeyPermission key) const;
    QMap<KeyPermission, bool> getSecurityPermissions() const;
    QString getPluginsEdit() const;
    ConstructorType getConstructorType()const;
    void setId(int newId);
    void setName(const QString &newName);
    void setExecutionFile(const QString &newExecutionFile);
    void setQEMUPlatform(const QString &newQEMUPlatform);
    void setSecurityPermission(KeyPermission key, bool value);
    void setPluginsEdit(const QString &newPluginsEdit);
    
private:
    int id;
    QString name;
    QString executionFile;
    QString qemuPlatform;
    QMap<KeyPermission, bool> securityPermissions;
    QString pluginsEdit;
    ConstructorType constructorType;
};

#endif  // PROCESS_H