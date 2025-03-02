#ifndef PROCESSDIALOG_H
#define PROCESSDIALOG_H

#include <QComboBox>
#include <QDialog>
#include <QFormLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QGridLayout>
#include <QLabel>
#include <QMap>

enum KeyPermission {
        VERIFY,
        SIGN,
        ENCRYPT,
        DECRYPT,
        EXPORTABLE
    };

class ProcessDialog : public QDialog {
    Q_OBJECT

public:
    explicit ProcessDialog(QWidget *parent = nullptr);

    int getId() const;
    QString getName() const;
    QString getExecutionFile() const;
    QString getQEMUPlatform() const;
    QMap<KeyPermission, bool> getSelectedPermissionsMap() const;
    static bool addingNewProcess;
    
    QString getPlugins() const;
    bool isValid() const;
    void setId(int id);
    void setName(const QString &name);
    void setExecutionFile(const QString &executableFile);
    void setQEMUPlatform(const QString &qemuPlatform);
    void setCMakeProject(const QString &cmakeProject);
    void setSecurityPermissions(const QMap<KeyPermission, bool>& permissions);
    void setPlugins(const QString &plugins);
    QString permToString(KeyPermission perm) const;
    QString getSelectedOption() const;
    QLineEdit *pluginsEdit;

private slots:
    bool validateAndAccept();
    void selectExecutableFile();

private:
    void setupPermissionCheckboxes();
    QComboBox *optionsComboBox;
    QComboBox *AsyComboBox;
    QComboBox *AESCComboBox;
    QComboBox *AESKComboBox;
    QLineEdit *idEdit;
    QLineEdit *nameEdit;
    QLineEdit *executionFile;
    QComboBox *qemuPlatformCombo;
    QMap<KeyPermission, QCheckBox*> permissionCheckboxes;
    QLineEdit *cmakeProjectEdit;            
    QPushButton *se
    lectExecutableFileButton; 
    QMap<KeyPermission, QString> permMap;

    friend class ProcessDialogTests;
};

#endif  // PROCESSDIALOG_H