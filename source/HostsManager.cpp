#include <QtXml>

#include <QMessageBox>
#include "HostItem.h"
#include "HostsManager.h"
#include "ModuleItem.h"
#include "application.h"

QString getValueFromXml(const QDomNamedNodeMap & attr, const QString & name) {
    if (!attr.contains(name))
        return QString();

    QString value = attr.namedItem(name).nodeValue();

    return value;
}

void saveXmlAttribute(QDomDocument & document, QDomElement & root,
                      const QString & name, const QString & value) {
    QDomAttr attr = document.createAttribute(name);
    attr.setValue(value);
    root.setAttributeNode(attr);
}

ConnectionConfig createFromXml(QDomNode & connectionNode) {
    ConnectionConfig connectionConfig;

    if (!connectionNode.hasAttributes()) {
        return connectionConfig;
    }

    QDomNamedNodeMap attr = connectionNode.attributes();

    connectionConfig.setName(getValueFromXml(attr, "name"));
    connectionConfig.setHost(getValueFromXml(attr, "host"));
    connectionConfig.setPort(getValueFromXml(attr, "port").toInt());
    connectionConfig.setAuth(getValueFromXml(attr, "auth"));

    connectionConfig.setSshHost(getValueFromXml(attr, "sshHost"));
    connectionConfig.setSshUser(getValueFromXml(attr, "sshUser"));
    connectionConfig.setSshPassword(getValueFromXml(attr, "sshPassword"));
    connectionConfig.setSshPort(getValueFromXml(attr, "sshPort").toInt());

    // getValueFromXml(attr, "namespaceSeparator",
    //                connectionConfig.namespaceSeparator);
    connectionConfig.setConnectionTimeout(
        getValueFromXml(attr, "connectionTimeout").toInt());
    connectionConfig.setExecutionTimeout(
        getValueFromXml(attr, "executeTimeout").toInt());

    return connectionConfig;
}

QDomElement toXml(ConnectionConfig conf, QDomDocument dom) {
    QDomElement xml = dom.createElement("connection");

    saveXmlAttribute(dom, xml, "name", conf.name());
    saveXmlAttribute(dom, xml, "host", conf.host());
    saveXmlAttribute(dom, xml, "port", QString::number(conf.port()));

    if (conf.useAuth()) {
        saveXmlAttribute(dom, xml, "auth", conf.auth());
    }

    // if (namespaceSeparator != QString(DEFAULT_NAMESPACE_SEPARATOR)) {
    //    saveXmlAttribute(dom, xml, "namespaceSeparator", namespaceSeparator);
    //}

    saveXmlAttribute(dom, xml, "connectionTimeout",
                     QString::number(conf.connectionTimeout()));
    saveXmlAttribute(dom, xml, "executeTimeout",
                     QString::number(conf.executeTimeout()));

    if (conf.useSshTunnel()) {
        saveXmlAttribute(dom, xml, "sshHost", conf.sshHost());
        saveXmlAttribute(dom, xml, "sshUser", conf.sshUser());
        saveXmlAttribute(dom, xml, "sshPassword", conf.sshPassword());
        saveXmlAttribute(dom, xml, "sshPort", QString::number(conf.sshPort()));
    }

    return xml;
}

HostsManager::HostsManager(QString config, QObject * parent = nullptr)
    : QStandardItemModel(parent),
      configPath(config),
      connectionSettingsChanged(false) {
    if (!config.isEmpty() && QFile::exists(config)) {
        LoadConnectionsConfigFromFile(config);
    }
}

HostsManager::~HostsManager(void) {
    // if (connectionSettingsChanged) {
    SaveConnectionsConfigToFile(configPath);
    //}
}

HostItem * HostsManager::AddConnection(ConnectionBridge * c) {
    // add connection to internal container
    connections.push_back(c);

    // add connection to view container
    HostItem * item         = new HostItem(c);
    MainWin * errorViewForm = (MainWin *)this->parent();
    QObject::connect(item, SIGNAL(error(QString)), errorViewForm,
                     SLOT(OnError(QString)));
    QObject::connect(c, SIGNAL(console(QString)), errorViewForm,
                     SLOT(OnLogMessage(QString)));
    QObject::connect(item, SIGNAL(unlockUI()), errorViewForm,
                     SLOT(OnUIUnlock()));
    QObject::connect(item, SIGNAL(statusMessage(QString)), errorViewForm,
                     SLOT(OnStatusMessage(QString)));
    connect(item, &HostItem::configChanged, this,
            &HostsManager::connectionChanged);

    appendRow(item);

    // mark settings as unsaved
    connectionSettingsChanged = true;
    SaveConnectionsConfigToFile(configPath);
    return item;
}

bool HostsManager::RemoveConnection(HostItem * c) {
    if (c == nullptr) {
        return false;
    }
    bool removedFromContainer = connections.removeOne(c->getConnection());
    bool removedFromModel     = this->removeRow(c->row());

    // mark settings as unsaved
    if (removedFromContainer && removedFromModel)
        connectionSettingsChanged = true;

    return removedFromContainer && removedFromModel;
}

bool HostsManager::ImportConnections(QString & path) {
    if (LoadConnectionsConfigFromFile(path, true)) {
        return true;
    }

    return false;
}

bool HostsManager::LoadConnectionsConfigFromFile(QString & config,
                                                 bool saveChangesToFile) {
    QFile conf(config);

    if (!conf.open(QIODevice::ReadOnly))
        return false;

    QDomDocument xmlConf;

    if (xmlConf.setContent(&conf)) {
        QDomNodeList connectionsList = xmlConf.elementsByTagName("connection");

        for (int i = 0; i < connectionsList.size(); ++i) {
            QDomNode connection = connectionsList.at(i);

            if (connection.nodeName() != "connection")
                continue;

            ConnectionConfig conf = createFromXml(connection);

            if (conf.isNull())
                continue;
            ConnectionBridge * conn_brg = new ConnectionBridge(conf);
            conn_brg->setRedisRefrashTimeout(
                getValueFromXml(connection.attributes(), "redis_refrash")
                    .toInt());
            conn_brg->setStatesRefrashTimeout(
                getValueFromXml(connection.attributes(), "states_refrash")
                    .toInt());
            HostItem * item = AddConnection(conn_brg);

            QDomNodeList modulesList = connection.childNodes();

            for (int i = 0; i < modulesList.size(); ++i) {
                QDomNode module = modulesList.at(i);

                if (module.nodeName() != "module" || !module.hasAttributes())
                    continue;
                QDomNamedNodeMap attr = module.attributes();
                QString name, dbIndex, exec_path, work_dir, app_params, rpl,
                    service_name;
                if (!attr.contains("name") || !attr.contains("db_index") ||
                    !attr.contains("exec_path") || !attr.contains("work_dir") ||
                    !attr.contains("app_params") ||
                    !attr.contains("records_per_logger") ||
                    !attr.contains("service_name")) {
                    QMessageBox::information(
                        nullptr, tr("Warnenk"),
                        QString(tr("Something went wrong during reading "
                                   "settings.xml. Ignoring")));
                    continue;
                }
                name         = attr.namedItem("name").nodeValue();
                service_name = attr.namedItem("service_name").nodeValue();
                dbIndex      = attr.namedItem("db_index").nodeValue();
                work_dir     = attr.namedItem("work_dir").nodeValue();
                exec_path    = attr.namedItem("exec_path").nodeValue();
                dbIndex      = attr.namedItem("db_index").nodeValue();
                app_params   = attr.namedItem("app_params").nodeValue();
                rpl          = attr.namedItem("records_per_logger").nodeValue();

                ModuleItem * newModule =
                    new ModuleItem(dbIndex.toInt(), exec_path, 0, item);
                newModule->renameModule(name);
                newModule->setWorkDir(work_dir);
                newModule->setAppParams(app_params);
                newModule->setMaxRecordsCount(rpl.toInt());
                newModule->setServiceName(service_name);
                item->appendRow(newModule);
            }
        }
    }
    conf.close();

    if (!saveChangesToFile)
        connectionSettingsChanged = false;

    return true;
}

bool HostsManager::SaveConnectionsConfigToFile(QString pathToFile) {
    QDomDocument config;

    QDomProcessingInstruction xmlProcessingInstruction =
        config.createProcessingInstruction("xml", "version=\"1.0\"");
    config.appendChild(xmlProcessingInstruction);

    QDomElement connectionsItem = config.createElement("connections");

    config.appendChild(connectionsItem);

    QList<QStandardItem *> hosts = this->findItems("*", Qt::MatchWildcard);
    foreach (QStandardItem * item, hosts) {
        HostItem * host = dynamic_cast<HostItem *>(item);
        if (!host)
            continue;
        QDomElement hostDom = toXml(host->getConnection()->getConfig(), config);
        saveXmlAttribute(
            config, hostDom, "redis_refrash",
            QString::number(host->getConnection()->getRedisRefrashTimeout()));
        saveXmlAttribute(
            config, hostDom, "states_refrash",
            QString::number(host->getConnection()->getStatesRefrashTimeout()));
        for (int i = 0; i < item->rowCount(); i++) {
            ModuleItem * module = dynamic_cast<ModuleItem *>(host->child(i));
            if (!module)
                continue;
            QDomElement moduleDom = config.createElement("module");
            QDomAttr attrName     = config.createAttribute("name");
            attrName.setValue(module->getName());
            moduleDom.setAttributeNode(attrName);
            QDomAttr attrExecPath = config.createAttribute("exec_path");
            attrExecPath.setValue(module->getExecPath());
            moduleDom.setAttributeNode(attrExecPath);
            QDomAttr attrDirPath = config.createAttribute("work_dir");
            attrDirPath.setValue(module->getWorkDir());
            moduleDom.setAttributeNode(attrDirPath);
            QDomAttr attrDb = config.createAttribute("db_index");
            attrDb.setValue(QString::number(module->getDbIndex()));
            moduleDom.setAttributeNode(attrDb);
            QDomAttr paramDb = config.createAttribute("app_params");
            paramDb.setValue(module->getAppParams());
            moduleDom.setAttributeNode(paramDb);
            QDomAttr attrRpl = config.createAttribute("records_per_logger");
            attrRpl.setValue(QString::number(module->getMaxRecordsCount()));
            moduleDom.setAttributeNode(attrRpl);
            QDomAttr attrSrvName = config.createAttribute("service_name");
            attrSrvName.setValue(module->getServiceName());
            moduleDom.setAttributeNode(attrSrvName);
            hostDom.appendChild(moduleDom);
        }
        connectionsItem.appendChild(hostDom);
    }

    QFile confFile(pathToFile);

    if (confFile.open(QIODevice::WriteOnly)) {
        QTextStream out(&confFile);
        out.setCodec("UTF-8");
        out << config.toString();
        confFile.close();
        return true;
    }

    return false;
}

void HostsManager::connectionChanged() {
    SaveConnectionsConfigToFile(configPath);
}
