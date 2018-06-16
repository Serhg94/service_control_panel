#include "connectionwindow.h"

#include <QtWidgets/QMessageBox>

#include "HostItem.h"
#include "HostsManager.h"
#include "application.h"

ConnectionWindow::ConnectionWindow(QWidget * parent, HostItem * srv)
    : QDialog(parent), inEditMode(false) {
    ui.setupUi(this);

    ui.hostEdit->setInputMask("000.000.000.000; ");
    ui.sshHost->setInputMask("000.000.000.000; ");

    this->setWindowFlags(Qt::Tool);
    this->setModal(true);

    ui.validationWarning->hide();

    if (parent) {
        mainForm = qobject_cast<MainWin *>(parent);
    }

    connect(ui.okButton, SIGNAL(clicked()), this, SLOT(OnOkButtonClick()));
    connect(ui.showPasswordCheckbox, SIGNAL(stateChanged(int)), this,
            SLOT(OnShowPasswordCheckboxChanged(int)));

    if (srv != nullptr) {
        server = srv;
        loadValuesFromConnection(srv->getConnection());
    } else {
        ui.connectionTimeout->setValue(ConnectionConfig::DEFAULT_TIMEOUT_IN_MS /
                                       1000);
        ui.executionTimeout->setValue(ConnectionConfig::DEFAULT_TIMEOUT_IN_MS /
                                      1000);
        ui.updateTimeout->setValue(DEFAULT_TIMEOUT_UPDATE);
        ui.stateTimeout->setValue(DEFAULT_TIMEOUT_STATES);
    }
}

void ConnectionWindow::loadValuesFromConnection(ConnectionBridge * c) {
    inEditMode = true;

    ConnectionConfig config = c->getConfig();

    ui.nameEdit->setText(config.name());
    ui.hostEdit->setText(config.host());
    ui.portSpinBox->setValue(config.port());
    ui.authEdit->setText(config.auth());
    ui.connectionTimeout->setValue(config.connectionTimeout() / 1000);
    ui.executionTimeout->setValue(config.executeTimeout() / 1000);

    if (config.useSshTunnel()) {
        ui.useSshTunnel->setCheckState(Qt::Checked);
        ui.sshHost->setText(config.sshHost());
        ui.sshUser->setText(config.sshUser());
        ui.sshPass->setText(config.sshPassword());
        ui.sshPort->setValue(config.sshPort());
    }
    ui.updateTimeout->setValue(c->getRedisRefrashTimeout());
    ui.stateTimeout->setValue(c->getStatesRefrashTimeout());
}

void ConnectionWindow::OnOkButtonClick() {
    ui.okButton->setEnabled(false);
    ui.validationWarning->hide();

    if (!isFormDataValid() || mainForm->connections == nullptr) {
        ui.validationWarning->show();
        ui.okButton->setEnabled(true);
        return;
    }

    ConnectionConfig conf = getConectionConfigFromFormData();

    ConnectionBridge * connection;

    if (inEditMode) {
        connection = server->getConnection();
        connection->setConnectionConfig(conf);
        mainForm->connections->connectionChanged();

    } else {
        connection = new ConnectionBridge(conf);
        mainForm->connections->AddConnection(connection);
    }
    connection->setRedisRefrashTimeout(ui.updateTimeout->value());
    connection->setStatesRefrashTimeout(ui.stateTimeout->value());

    close();
}

void ConnectionWindow::OnShowPasswordCheckboxChanged(int state) {
    if (state == Qt::Unchecked) {
        ui.sshPass->setEchoMode(QLineEdit::Password);
    } else {
        ui.sshPass->setEchoMode(QLineEdit::Normal);
    }
}

bool ConnectionWindow::isFormDataValid() {
    return isConnectionSettingsValid() && isSshSettingsValid() &&
           isAdvancedSettingsValid();
}

bool ConnectionWindow::isConnectionSettingsValid() {
    ui.nameEdit->setStyleSheet("");
    ui.hostEdit->setStyleSheet("");

    QString name = ui.nameEdit->text();
    QString hostaddr = ui.hostEdit->text();
    QRegExp * re = new QRegExp("[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}", Qt::CaseInsensitive);
    bool hostValid = re->exactMatch(hostaddr);

    bool isValid = !name.isEmpty() && hostValid && ui.portSpinBox->value() > 0;

    if (isValid) {
        return true;
    };

    if (name.isEmpty()) {
        ui.nameEdit->setStyleSheet("border: 1px solid red;");
    };

    if (!hostValid) {
        ui.hostEdit->setStyleSheet("border: 1px solid red;");
    };

    return false;
}

bool ConnectionWindow::isAdvancedSettingsValid() {
    return true;
}

bool ConnectionWindow::isSshSettingsValid() {
    ui.sshHost->setStyleSheet("");
    ui.sshUser->setStyleSheet("");
    ui.sshPass->setStyleSheet("");

    if (!isSshTunnelUsed()) {
        return true;
    }

    QString hostaddr = ui.sshHost->text();
    QRegExp * re = new QRegExp("[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}", Qt::CaseInsensitive);
    bool hostValid = re->exactMatch(hostaddr);
    bool isValid = hostValid && !ui.sshUser->text().isEmpty() && !ui.sshPass->text().isEmpty() && ui.sshPort->value() > 0;
    if (isValid) {
        return true;
    };

    if (!hostValid) {
        ui.sshHost->setStyleSheet("border: 1px solid red;");
    };

    if (ui.sshUser->text().isEmpty()) {
        ui.sshUser->setStyleSheet("border: 1px solid red;");
    };

    if (ui.sshPass->text().isEmpty()) {
        ui.sshPass->setStyleSheet("border: 1px solid red;");
    };

    return false;
}

bool ConnectionWindow::isSshTunnelUsed() {
    qDebug() << "check state" << (ui.useSshTunnel->checkState() == Qt::Checked);
    return ui.useSshTunnel->checkState() == Qt::Checked;
}

ConnectionConfig ConnectionWindow::getConectionConfigFromFormData() {
    ConnectionConfig conf(ui.hostEdit->text().trimmed(), "",
                          ui.portSpinBox->value(),
                          ui.nameEdit->text().trimmed());

    // conf.namespaceSeparator = ui.namespaceSeparator->text();
    conf.setConnectionTimeout(ui.connectionTimeout->value() * 1000);
    conf.setExecutionTimeout(ui.executionTimeout->value() * 1000);

    if (!ui.authEdit->text().isEmpty()) {
        conf.setAuth(ui.authEdit->text());
    }

    if (isSshTunnelUsed()) {
        conf.setSshTunnelSettings(
            ui.sshHost->text().trimmed(), ui.sshUser->text().trimmed(),
            ui.sshPass->text().trimmed(), ui.sshPort->value());
    }

    return conf;
}
