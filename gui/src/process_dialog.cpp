#include <QComboBox>
#include <QHBoxLayout>
#include <QIntValidator>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>
#include "process_dialog.h"
#include "main_window.h"

bool ProcessDialog::addingNewProcess = true;

ProcessDialog::ProcessDialog(QWidget *parent) : QDialog(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);

    MainWindow::guiLogger.logMessage(logger::LogLevel::INFO,
                                     "Initializing ProcessDialog");

    permMap = {
        {VERIFY, "Verify"},
        {SIGN, "Sign"},
        {ENCRYPT, "Encrypt"},
        {DECRYPT, "Decrypt"},
        {EXPORTABLE, "Exportable"}
    };

    QLabel *idLabel = new QLabel("ID:");
    idEdit = new QLineEdit(this);
    idEdit->setValidator(new QIntValidator(11, 10000, this));
    layout->addWidget(idLabel);
    layout->addWidget(idEdit);

    QLabel *nameLabel = new QLabel("Name:");
    nameEdit = new QLineEdit(this);
    layout->addWidget(nameLabel);
    layout->addWidget(nameEdit);

    QLabel *executableFileLabel = new QLabel("Executable File:");
    executionFile = new QLineEdit(this);
    executionFile->setReadOnly(true);
    layout->addWidget(executableFileLabel);
    layout->addWidget(executionFile);
    
    QPushButton *selectExecutableFileButton = 
        new QPushButton("Select Executable File", this);
    layout->addWidget(selectExecutableFileButton);  
    
    QLabel *pluginsLabel = new QLabel("Plugins:", this);
    pluginsEdit = new QLineEdit(this);
    layout->addWidget(pluginsLabel);
    layout->addWidget(pluginsEdit);
    
    QLabel *qemuPlatformLabel = new QLabel("QEMU Platform:");
    qemuPlatformCombo = new QComboBox(this);
    qemuPlatformCombo->addItems({"x86_64", "arm", "aarch64"});
    qemuPlatformCombo->setEnabled(false);

    // Adding widgets to layout
    layout->addWidget(qemuPlatformLabel);
    layout->addWidget(qemuPlatformCombo);

    // Currently, hardware selection is not needed, so the widgets are hidden.
    // If you need to use this in the future, remove the following lines:
    qemuPlatformCombo->setVisible(false);
    qemuPlatformLabel->setVisible(false);
    
    QLabel *SHAAlgorithm = new QLabel("SHAAlgorithm:", this);
    optionsComboBox = new QComboBox(this);
    optionsComboBox->addItems({"SHA_256", "SHA_512"}); // Add your options here
    layout->addWidget(SHAAlgorithm);
    layout->addWidget(optionsComboBox);
    
    QLabel *AESKeyLength = new QLabel("AESKeyLength:", this);
    AESKComboBox = new QComboBox(this);
    AESKComboBox->addItems({"AES_128","AES_192","AES_256"}); 
    layout->addWidget(AESKeyLength);
    layout->addWidget(AESKComboBox);
    
    QLabel *AESChainingMode = new QLabel("AESChainingMode:", this);
    AESCComboBox = new QComboBox(this);
    AESCComboBox->addItems({"CFB", "OFB" , "CBC" , "CTR", "ECB"}); 
    layout->addWidget(AESChainingMode);
    layout->addWidget(AESCComboBox);

    QLabel *AsymmetricFunction = new QLabel("AsymmetricFunction:", this);
    AsyComboBox = new QComboBox(this);
    AsyComboBox->addItems({"ECC","RSA"}); 
    layout->addWidget(AsymmetricFunction);
    layout->addWidget(AsyComboBox);

    // Key Permissions section
    QLabel *permissionsLabel = new QLabel(tr("Key Permissions:"), this);
    layout->addWidget(permissionsLabel);

    // Set up permission checkboxes
    setupPermissionCheckboxes();
    QGridLayout *permissionLayout = new QGridLayout();
    int row = 0;
    int col = 0;
    for (auto permission : {VERIFY, SIGN, ENCRYPT, DECRYPT, EXPORTABLE}) {
        if (col == 2) {
            col = 0;
            ++row;
        }
        permissionLayout->addWidget(permissionCheckboxes[permission], row, col);
        ++col;
    }
    layout->addLayout(permissionLayout);

    // Add spacer to push the select file button to the bottom
    layout->addStretch();

    QRegExp regex("[a-zA-Z0-9]*");  // Allows only English letters and numbers
    QRegExpValidator *validator = new QRegExpValidator(regex, this);
    nameEdit->setValidator(validator);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *okButton = new QPushButton("OK", this);
    QPushButton *cancelButton = new QPushButton("Cancel", this);
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);
    layout->addLayout(buttonLayout);

    connect(okButton, &QPushButton::clicked, this,
            &ProcessDialog::validateAndAccept);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    connect(selectExecutableFileButton, &QPushButton::clicked, this, &ProcessDialog::selectExecutableFile);

    setLayout(layout);
}

void ProcessDialog::selectExecutableFile()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Select Executable File", "","Executable Files (*.sh *.cmake);;Text Files (*.txt)");
    if (!fileName.isEmpty()) {
        executionFile->setText(fileName);  // Update the renamed QLineEdit
    }
}

QString ProcessDialog::getSelectedOption() const
{
    return optionsComboBox->currentText();
}

int ProcessDialog::getId() const
{
    return idEdit->text().toInt();
}

QString ProcessDialog::getName() const
{
    return nameEdit->text();
}

QString ProcessDialog::getExecutionFile() const
{
    return executionFile->text();
}

QString ProcessDialog::getQEMUPlatform() const
{
    return qemuPlatformCombo->currentText();
}

QString ProcessDialog::getPlugins() const
{
    return pluginsEdit->text();
}

void ProcessDialog::setPlugins(const QString &plugins)
{
    pluginsEdit->setText(plugins);
}

bool ProcessDialog::isValid() const
{
    if (!idEdit || !nameEdit || !executionFile || !qemuPlatformCombo) {
        return false;
    }
    return !idEdit->text().isEmpty() && !nameEdit->text().isEmpty() &&
           !executionFile->text().isEmpty() &&
           !qemuPlatformCombo->currentText().isEmpty();
}

bool ProcessDialog::validateAndAccept()
{
    MainWindow::guiLogger.logMessage(logger::LogLevel::INFO,
                                     "Validating ProcessDialog inputs");

    if (isValid()) {
         QMap<KeyPermission, bool> selectedPermissions = getSelectedPermissionsMap();

        if (selectedPermissions.isEmpty()) {
            MainWindow::guiLogger.logMessage(
                logger::LogLevel::ERROR,
                "Validation failed, no security permissions selected");
            QMessageBox::warning(this, "Input Error",
                                 "Please select at least one security permission.");
            return false;
        }

        MainWindow::guiLogger.logMessage(
            logger::LogLevel::INFO, "Validation successful, accepting dialog");
        MainWindow::guiLogger.logMessage(
            logger::LogLevel::DEBUG,
            "Entered values: ID = " + idEdit->text().toStdString() +
            ", Name = " + nameEdit->text().toStdString() +
            ", Executable File = " + executionFile->text().toStdString() +
            ", QEMU Platform = " +
                    qemuPlatformCombo->currentText().toStdString());
        if(!pluginsEdit->text().isEmpty()){
            MainWindow::guiLogger.logMessage(
                logger::LogLevel::DEBUG,
                "Entered Plugin" +pluginsEdit->text().toStdString());
         }
        accept();
        return true;
    } else {
        MainWindow::guiLogger.logMessage(
            logger::LogLevel::ERROR,
            "Validation failed, missing or incorrect input");
        QMessageBox::warning(this, "Input Error",
                             "Please fill in all fields correctly.");
        return false;
    }
}

void ProcessDialog::setId(int id)
{
    idEdit->setText(QString::number(id));
}

void ProcessDialog::setName(const QString &name)
{
    nameEdit->setText(name);
}

void ProcessDialog::setExecutionFile(const QString &executableFile)
{
    executionFile->setText(executableFile);
}

void ProcessDialog::setQEMUPlatform(const QString &qemuPlatform)
{
    int index = qemuPlatformCombo->findText(qemuPlatform);
    if (index != -1) {
        qemuPlatformCombo->setCurrentIndex(index);
    }
}

void ProcessDialog::setupPermissionCheckboxes() 
{
    QStringList permissions = { "Verify", "Sign", "Encrypt", "Decrypt", "Exportable" };
    
    for (int i = 0; i < permissions.size(); ++i) {
        KeyPermission perm = static_cast<KeyPermission>(i);  
        permissionCheckboxes[perm] = new QCheckBox(permissions[i], this);

        if (!ProcessDialog::addingNewProcess) {
            permissionCheckboxes[perm]->setDisabled(true);
        }
    }
}

QMap<KeyPermission, bool> ProcessDialog::getSelectedPermissionsMap() const
{
     QMap<KeyPermission, bool> selectedPermissions;

    for (auto it = permissionCheckboxes.constBegin(); it != 
        permissionCheckboxes.constEnd(); ++it) {
        if (it.value()->isChecked()) {
            selectedPermissions[it.key()] = true;
        }
    }
    return selectedPermissions;
}

void ProcessDialog::setSecurityPermissions(const QMap<KeyPermission, bool>& permissions)
{
    for (auto it = permissions.constBegin(); it != permissions.constEnd(); ++it) {
        KeyPermission key = it.key();
        bool value = it.value();

        if (permissionCheckboxes.contains(key)) {
            permissionCheckboxes[key]->setChecked(value);
        } else {
            QCheckBox* newCheckbox = new QCheckBox();
            newCheckbox->setChecked(value);
            permissionCheckboxes.insert(key, newCheckbox);
        }
    }
}

QString ProcessDialog::permToString(KeyPermission perm) const
{
    return permMap.value(perm, "Unknown Permission");
}