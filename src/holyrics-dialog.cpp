/*
Holyrics Finder Plugin
Copyright (C) 2024

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
*/

#include "holyrics-dialog.h"
#include "holyrics-finder.h"
#include "translations.h"
#include <obs-module.h>
#include <obs-frontend-api.h>
#include <plugin-support.h>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QIntValidator>
#include <QNetworkInterface>
#include <QHostAddress>
#include <QRegularExpression>
#include <QClipboard>
#include <QApplication>
#include <QStandardPaths>
#include <QFile>
#include <QTextStream>
#include <QSettings>
#include <QDir>

HolyricsDialog::HolyricsDialog(QWidget *parent, HolyricsFinder *finder)
	: QDialog(parent),
	  m_finder(finder)
{
	// Language is already set in plugin-main.cpp
	setWindowTitle(Translations::get("window.title"));
	setMinimumWidth(600);
	setMinimumHeight(500);

	setupUI();
	detectLocalIP();
	refreshSourcesList();
	refreshDocksList();

	connect(m_finder, &HolyricsFinder::connectionSuccess, this,
		&HolyricsDialog::onConnectionSuccess);
	connect(m_finder, &HolyricsFinder::connectionFailed, this,
		&HolyricsDialog::onConnectionFailed);
	connect(m_finder, &HolyricsFinder::scanProgress, this,
		&HolyricsDialog::onScanProgress);
	connect(m_finder, &HolyricsFinder::scanComplete, this,
		&HolyricsDialog::onScanComplete);
}

HolyricsDialog::~HolyricsDialog() 
{
	obs_log(LOG_INFO, "[HolyricsDialog] Destructor called");
	
	if (m_finder) {
		disconnect(m_finder, nullptr, this, nullptr);
		obs_log(LOG_DEBUG, "[HolyricsDialog] Disconnected signals from finder");
	}
	
	if (m_sourcesList) {
		m_sourcesList->clear();
		obs_log(LOG_DEBUG, "[HolyricsDialog] Cleared sources list");
	}
	
	hide();
	
	obs_log(LOG_INFO, "[HolyricsDialog] Destructor complete");
}

void HolyricsDialog::setupUI()
{
	QVBoxLayout *mainLayout = new QVBoxLayout(this);

	QGroupBox *connectionGroup = new QGroupBox(Translations::get("connection.group"), this);
	QVBoxLayout *connectionLayout = new QVBoxLayout(connectionGroup);

	QLabel *ipLabel = new QLabel(Translations::get("connection.ip_label"), this);
	connectionLayout->addWidget(ipLabel);

	QHBoxLayout *ipLayout = new QHBoxLayout();
	
	m_octet1 = new QSpinBox(this);
	m_octet1->setRange(0, 255);
	m_octet1->setButtonSymbols(QAbstractSpinBox::NoButtons);
	m_octet1->setAlignment(Qt::AlignCenter);
	m_octet1->setMaximumWidth(60);
	ipLayout->addWidget(m_octet1);

	ipLayout->addWidget(new QLabel(".", this));

	m_octet2 = new QSpinBox(this);
	m_octet2->setRange(0, 255);
	m_octet2->setButtonSymbols(QAbstractSpinBox::NoButtons);
	m_octet2->setAlignment(Qt::AlignCenter);
	m_octet2->setMaximumWidth(60);
	ipLayout->addWidget(m_octet2);

	ipLayout->addWidget(new QLabel(".", this));

	m_octet3 = new QSpinBox(this);
	m_octet3->setRange(0, 255);
	m_octet3->setButtonSymbols(QAbstractSpinBox::NoButtons);
	m_octet3->setAlignment(Qt::AlignCenter);
	m_octet3->setMaximumWidth(60);
	ipLayout->addWidget(m_octet3);

	ipLayout->addWidget(new QLabel(".", this));

	m_octet4 = new QSpinBox(this);
	m_octet4->setRange(0, 255);
	m_octet4->setButtonSymbols(QAbstractSpinBox::NoButtons);
	m_octet4->setAlignment(Qt::AlignCenter);
	m_octet4->setMaximumWidth(60);
	ipLayout->addWidget(m_octet4);

	ipLayout->addSpacing(20);
	ipLayout->addWidget(new QLabel(Translations::get("connection.port_label"), this));

	m_portInput = new QSpinBox(this);
	m_portInput->setRange(1, 65535);
	m_portInput->setValue(80);
	m_portInput->setButtonSymbols(QAbstractSpinBox::NoButtons);
	m_portInput->setAlignment(Qt::AlignCenter);
	m_portInput->setMaximumWidth(80);
	ipLayout->addWidget(m_portInput);

	ipLayout->addStretch();

	connectionLayout->addLayout(ipLayout);

	QHBoxLayout *buttonLayout = new QHBoxLayout();
	
	m_testButton = new QPushButton(Translations::get("connection.test_button"), this);
	connect(m_testButton, &QPushButton::clicked, this,
		&HolyricsDialog::onTestConnection);
	buttonLayout->addWidget(m_testButton);

	m_scanButton = new QPushButton(Translations::get("connection.scan_button"), this);
	connect(m_scanButton, &QPushButton::clicked, this,
		&HolyricsDialog::onScanNetwork);
	buttonLayout->addWidget(m_scanButton);

	m_copyIpButton = new QPushButton(Translations::get("connection.copy_button"), this);
	m_copyIpButton->setVisible(false);
	connect(m_copyIpButton, &QPushButton::clicked, this, [this]() {
		QString ip = getIpFromInputs();
		int port = getPortFromInput();
		QString ipPort = QString("%1:%2").arg(ip).arg(port);
		
		QClipboard *clipboard = QApplication::clipboard();
		clipboard->setText(ipPort);
		
		updateStatus(Translations::get("status.copied").arg(ipPort));
	});
	buttonLayout->addWidget(m_copyIpButton);

	connectionLayout->addLayout(buttonLayout);

	m_statusLabel = new QLabel(Translations::get("status.ready"), this);
	m_statusLabel->setWordWrap(true);
	connectionLayout->addWidget(m_statusLabel);

	m_progressBar = new QProgressBar(this);
	m_progressBar->setVisible(false);
	connectionLayout->addWidget(m_progressBar);

	mainLayout->addWidget(connectionGroup);

	// Create tab widget for Sources and Docks
	m_tabWidget = new QTabWidget(this);
	
	// Browser Sources Tab
	QWidget *sourcesTab = new QWidget(this);
	QVBoxLayout *sourcesLayout = new QVBoxLayout(sourcesTab);

	QLabel *sourcesLabel = new QLabel(Translations::get("sources.label"), this);
	sourcesLayout->addWidget(sourcesLabel);

	m_sourcesList = new QListWidget(this);
	connect(m_sourcesList, &QListWidget::itemPressed, this, [this](QListWidgetItem *item) {
		Qt::CheckState newState = (item->checkState() == Qt::Checked) ? Qt::Unchecked : Qt::Checked;
		item->setCheckState(newState);
	});
	sourcesLayout->addWidget(m_sourcesList);

	QHBoxLayout *sourcesButtonLayout = new QHBoxLayout();
	
	QPushButton *selectAllButton = new QPushButton(Translations::get("sources.select_all"), this);
	connect(selectAllButton, &QPushButton::clicked, this, [this]() {
		for (int i = 0; i < m_sourcesList->count(); ++i) {
			QListWidgetItem *item = m_sourcesList->item(i);
			item->setCheckState(Qt::Checked);
		}
	});
	sourcesButtonLayout->addWidget(selectAllButton);

	QPushButton *deselectAllButton = new QPushButton(Translations::get("sources.deselect_all"), this);
	connect(deselectAllButton, &QPushButton::clicked, this, [this]() {
		for (int i = 0; i < m_sourcesList->count(); ++i) {
			QListWidgetItem *item = m_sourcesList->item(i);
			item->setCheckState(Qt::Unchecked);
		}
	});
	sourcesButtonLayout->addWidget(deselectAllButton);

	QPushButton *refreshButton = new QPushButton(Translations::get("sources.refresh"), this);
	connect(refreshButton, &QPushButton::clicked, this,
		&HolyricsDialog::refreshSourcesList);
	sourcesButtonLayout->addWidget(refreshButton);

	sourcesButtonLayout->addStretch();
	sourcesLayout->addLayout(sourcesButtonLayout);

	m_updateButton = new QPushButton(Translations::get("sources.update_button"), this);
	m_updateButton->setEnabled(false);
	connect(m_updateButton, &QPushButton::clicked, this,
		&HolyricsDialog::onUpdateSources);
	sourcesLayout->addWidget(m_updateButton);

	// Docks Tab
	QWidget *docksTab = new QWidget(this);
	QVBoxLayout *docksLayout = new QVBoxLayout(docksTab);

	QLabel *docksLabel = new QLabel(Translations::get("docks.label"), this);
	docksLabel->setWordWrap(true);
	docksLayout->addWidget(docksLabel);

	m_docksList = new QListWidget(this);
	docksLayout->addWidget(m_docksList);

	QPushButton *refreshDocksButton = new QPushButton(Translations::get("docks.refresh"), this);
	connect(refreshDocksButton, &QPushButton::clicked, this,
		&HolyricsDialog::refreshDocksList);
	docksLayout->addWidget(refreshDocksButton);

	// Add tabs to tab widget
	m_tabWidget->addTab(sourcesTab, Translations::get("sources.tab_title"));
	m_tabWidget->addTab(docksTab, Translations::get("docks.tab_title"));
	
	mainLayout->addWidget(m_tabWidget);

	QHBoxLayout *closeLayout = new QHBoxLayout();
	closeLayout->addStretch();
	QPushButton *closeButton = new QPushButton(Translations::get("button.close"), this);
	connect(closeButton, &QPushButton::clicked, this, &QDialog::close);
	closeLayout->addWidget(closeButton);
	mainLayout->addLayout(closeLayout);

	m_octet4->setFocus();
	m_octet4->selectAll();
}

void HolyricsDialog::detectLocalIP()
{
	QString currentDeviceIp;
	QList<QHostAddress> addresses = QNetworkInterface::allAddresses();
	
	for (const QHostAddress &address : addresses) {
		if (address.protocol() == QAbstractSocket::IPv4Protocol && 
		    !address.isLoopback()) {
			currentDeviceIp = address.toString();
			break;
		}
	}
	
	QList<HolyricsFinder::ConnectionInfo> history = m_finder->getConnectionHistory();
	
	if (!history.isEmpty()) {
		const HolyricsFinder::ConnectionInfo &lastConnection = history.first();
		
		obs_log(LOG_INFO, "[HolyricsDialog] Found connection history. Last: %s:%d",
			lastConnection.ip.toUtf8().constData(), lastConnection.port);
		
		setIpToInputs(lastConnection.ip);
		m_portInput->setValue(lastConnection.port);
		
		obs_log(LOG_INFO, "[HolyricsDialog] Loaded IP from history: %s:%d",
			lastConnection.ip.toUtf8().constData(), lastConnection.port);
		return;
	}
	
	obs_log(LOG_INFO, "[HolyricsDialog] No connection history found");
	
	if (!currentDeviceIp.isEmpty()) {
		QStringList parts = currentDeviceIp.split('.');
		if (parts.size() == 4) {
			m_octet1->setValue(parts[0].toInt());
			m_octet2->setValue(parts[1].toInt());
			m_octet3->setValue(parts[2].toInt());
			m_octet4->setValue(parts[3].toInt());
			
			obs_log(LOG_INFO, "[HolyricsDialog] Using current device IP: %s",
				currentDeviceIp.toUtf8().constData());
			return;
		}
	}
	
	m_octet1->setValue(192);
	m_octet2->setValue(168);
	m_octet3->setValue(0);
	m_octet4->setValue(1);
	obs_log(LOG_INFO, "[HolyricsDialog] Using default IP: 192.168.0.1");
}

QString HolyricsDialog::getIpFromInputs() const
{
	return QString("%1.%2.%3.%4")
		.arg(m_octet1->value())
		.arg(m_octet2->value())
		.arg(m_octet3->value())
		.arg(m_octet4->value());
}

int HolyricsDialog::getPortFromInput() const
{
	return m_portInput->value();
}

void HolyricsDialog::setIpToInputs(const QString &ip)
{
	QStringList parts = ip.split('.');
	if (parts.size() == 4) {
		m_octet1->setValue(parts[0].toInt());
		m_octet2->setValue(parts[1].toInt());
		m_octet3->setValue(parts[2].toInt());
		m_octet4->setValue(parts[3].toInt());
	}
}

void HolyricsDialog::onTestConnection()
{
	QString ip = getIpFromInputs();
	int port = getPortFromInput();

	updateStatus(Translations::get("status.testing").arg(ip).arg(port));
	m_testButton->setEnabled(false);
	m_scanButton->setEnabled(false);
	m_updateButton->setEnabled(false);

	m_finder->testConnection(ip, port);
}

void HolyricsDialog::onScanNetwork()
{
	QString ip = getIpFromInputs();
	int port = getPortFromInput();

	updateStatus(Translations::get("status.scanning"));
	m_progressBar->setVisible(true);
	m_progressBar->setValue(0);
	m_testButton->setEnabled(false);
	m_scanButton->setEnabled(false);
	m_updateButton->setEnabled(false);

	m_finder->scanNetwork(ip, port);
}

void HolyricsDialog::onUpdateSources()
{
	QString ip = getIpFromInputs();
	int port = getPortFromInput();

	int updatedCount = 0;
	for (int i = 0; i < m_sourcesList->count(); ++i) {
		QListWidgetItem *item = m_sourcesList->item(i);
		if (item->checkState() == Qt::Checked) {
			QString sourceName = item->text().section(" - ", 0, 0);
			QString urlPath = item->data(Qt::UserRole + 1).toString();
			
			if (!urlPath.isEmpty()) {
				QString newUrl = QString("http://%1:%2%3").arg(ip).arg(port).arg(urlPath);
				m_finder->updateBrowserSourceUrl(sourceName, newUrl);
				updatedCount++;
			}
		}
	}

	if (updatedCount > 0) {
		updateStatus(Translations::get("status.updated_sources").arg(updatedCount).arg(ip).arg(port));
		refreshSourcesList();
	} else {
		updateStatus(Translations::get("status.no_sources_selected"), true);
	}
}

void HolyricsDialog::onConnectionSuccess(const QString &ip)
{
	m_testButton->setEnabled(true);
	m_scanButton->setEnabled(true);
	m_updateButton->setEnabled(true);
	m_copyIpButton->setVisible(true);

	updateStatus(Translations::get("status.connection_success").arg(ip));
	
	setIpToInputs(ip);
	
	refreshSourcesList();
	refreshDocksList();
	
	int port = getPortFromInput();
	int selectedCount = 0;
	
	for (int i = 0; i < m_sourcesList->count(); ++i) {
		QListWidgetItem *item = m_sourcesList->item(i);
		QString itemText = item->text();
		
		QRegularExpression ipPortRegex("([0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}):([0-9]{1,5})");
		QRegularExpressionMatch match = ipPortRegex.match(itemText);
		
		if (match.hasMatch()) {
			QString sourceIp = match.captured(1);
			int sourcePort = match.captured(2).toInt();
			
			if (sourceIp != ip || sourcePort != port) {
				item->setCheckState(Qt::Checked);
				selectedCount++;
			} else {
				item->setCheckState(Qt::Unchecked);
			}
		}
	}
	
	if (selectedCount > 0) {
		updateStatus(Translations::get("status.connection_success_sources")
			.arg(ip).arg(port).arg(selectedCount));
	} else if (m_sourcesList->count() > 0) {
		updateStatus(Translations::get("status.connection_success_uptodate")
			.arg(ip).arg(port));
	}
}

void HolyricsDialog::onConnectionFailed(const QString &ip)
{
	m_testButton->setEnabled(true);
	m_scanButton->setEnabled(true);
	m_updateButton->setEnabled(false);
	m_copyIpButton->setVisible(false);

	updateStatus(Translations::get("status.connection_failed").arg(ip), true);
}

void HolyricsDialog::onScanProgress(int current, int total)
{
	m_progressBar->setMaximum(total);
	m_progressBar->setValue(current);
	updateStatus(Translations::get("status.scanning_progress").arg(current).arg(total));
}

void HolyricsDialog::onScanComplete()
{
	m_progressBar->setVisible(false);
	m_testButton->setEnabled(true);
	m_scanButton->setEnabled(true);
	updateStatus(Translations::get("status.scan_complete"));
}

void HolyricsDialog::updateStatus(const QString &message, bool isError)
{
	m_statusLabel->setText(message);
	
	if (isError) {
		m_statusLabel->setStyleSheet("QLabel { color: red; }");
	} else if (message.startsWith("✓")) {
		m_statusLabel->setStyleSheet("QLabel { color: green; }");
	} else {
		m_statusLabel->setStyleSheet("");
	}
}

void HolyricsDialog::refreshSourcesList()
{
	m_sourcesList->clear();

	obs_enum_sources([](void *param, obs_source_t *source) {
		HolyricsDialog *dialog = static_cast<HolyricsDialog *>(param);
		
		const char *sourceId = obs_source_get_id(source);
		if (strcmp(sourceId, "browser_source") != 0) {
			return true;
		}

		obs_data_t *settings = obs_source_get_settings(source);
		const char *url = obs_data_get_string(settings, "url");
		QString urlStr = QString::fromUtf8(url);
		obs_data_release(settings);

		QRegularExpression ipPortRegex("https?://([0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}):([0-9]{1,5})(.*)");
		QRegularExpressionMatch match = ipPortRegex.match(urlStr);

		if (match.hasMatch()) {
			QString sourceIp = match.captured(1);
			QString sourcePort = match.captured(2);
			QString urlPath = match.captured(3);
			const char *sourceName = obs_source_get_name(source);
			
			QString displayText = QString("%1 - %2:%3").arg(sourceName).arg(sourceIp).arg(sourcePort);
			
			QListWidgetItem *item = new QListWidgetItem(displayText, dialog->m_sourcesList);
			item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
			item->setCheckState(Qt::Unchecked);
			item->setData(Qt::UserRole, urlStr);
			item->setData(Qt::UserRole + 1, urlPath);
		}

		return true;
	}, this);

	m_updateButton->setEnabled(m_sourcesList->count() > 0);
}

QString HolyricsDialog::getObsConfigPath() const
{
	// On Windows, OBS stores config in %APPDATA%/obs-studio
	QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
	
	// QStandardPaths::AppDataLocation returns something like:
	// C:/Users/Username/AppData/Roaming/obs-holyrics-plugin-finder
	// We need to get to C:/Users/Username/AppData/Roaming/obs-studio
	
	QDir appDataDir(appDataPath);
	appDataDir.cdUp(); // Go to Roaming directory
	QString obsConfigPath = appDataDir.absolutePath() + "/obs-studio";
	
	obs_log(LOG_INFO, "Detected OBS config path: %s", obsConfigPath.toUtf8().constData());
	
	return obsConfigPath;
}

void HolyricsDialog::refreshDocksList()
{
	m_docksList->clear();

	QString configPath = getObsConfigPath();
	QString userIniPath = configPath + "/user.ini";
	
	obs_log(LOG_INFO, "Looking for OBS user.ini at: %s", userIniPath.toUtf8().constData());
	
	QFile file(userIniPath);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		obs_log(LOG_WARNING, "OBS user.ini not found or cannot be opened");
		QListWidgetItem *item = new QListWidgetItem(Translations::get("docks.not_found"), m_docksList);
		item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
		return;
	}
	
	QString docksJson;
	QTextStream in(&file);
	while (!in.atEnd()) {
		QString line = in.readLine().trimmed();
		if (line.startsWith("ExtraBrowserDocks=")) {
			docksJson = line.mid(18);
			break;
		}
	}
	file.close();
	
	obs_log(LOG_INFO, "ExtraBrowserDocks JSON: %s", docksJson.toUtf8().constData());
	
	if (docksJson.isEmpty() || docksJson == "[]") {
		QListWidgetItem *item = new QListWidgetItem(Translations::get("docks.none_found"), m_docksList);
		item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
		return;
	}
	
	QString ip = getIpFromInputs();
	int port = getPortFromInput();
	QRegularExpression ipPortRegex("https?://([0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}):([0-9]{1,5})(.*)");
	QRegularExpression dockRegex("\\{\"title\":\\s*\"([^\"]+)\"\\s*,\\s*\"url\":\\s*\"([^\"]+)\"");
	QRegularExpressionMatchIterator matches = dockRegex.globalMatch(docksJson);
	
	int totalDocks = 0;
	int matchedDocks = 0;
	
	while (matches.hasNext()) {
		QRegularExpressionMatch match = matches.next();
		QString dockTitle = match.captured(1);
		QString dockUrl = match.captured(2);
		
		if (dockUrl.isEmpty()) continue;
		
		totalDocks++;
		obs_log(LOG_INFO, "Dock '%s' URL: %s", dockTitle.toUtf8().constData(), dockUrl.toUtf8().constData());
		
		QRegularExpressionMatch urlMatch = ipPortRegex.match(dockUrl);
		if (urlMatch.hasMatch()) {
			matchedDocks++;
			QString dockIp = urlMatch.captured(1);
			int dockPort = urlMatch.captured(2).toInt();
			QString urlPath = urlMatch.captured(3);
			
			bool needsUpdate = (dockIp != ip || dockPort != port);
			QString displayText = QString("%1 - %2:%3 %4")
				.arg(dockTitle).arg(dockIp).arg(dockPort)
				.arg(needsUpdate ? "⚠" : "✓");
			
			QListWidgetItem *item = new QListWidgetItem(m_docksList);
			
			if (needsUpdate) {
				QString newUrl = QString("http://%1:%2%3").arg(ip).arg(port).arg(urlPath);
				
				QWidget *itemWidget = new QWidget();
				QHBoxLayout *itemLayout = new QHBoxLayout(itemWidget);
				itemLayout->setContentsMargins(5, 2, 5, 2);
				
				QLabel *label = new QLabel(displayText);
				itemLayout->addWidget(label, 1);
				
				QPushButton *copyButton = new QPushButton(Translations::get("docks.copy_url"));
				copyButton->setMaximumWidth(100);
				connect(copyButton, &QPushButton::clicked, this, [newUrl, dockTitle, this]() {
					QClipboard *clipboard = QApplication::clipboard();
					clipboard->setText(newUrl);
					updateStatus(Translations::get("docks.url_copied").arg(dockTitle));
				});
				itemLayout->addWidget(copyButton);
				
				item->setSizeHint(itemWidget->sizeHint());
				m_docksList->setItemWidget(item, itemWidget);
				item->setData(Qt::UserRole, newUrl);
			} else {
				item->setText(displayText);
			}
		}
	}
	
	if (m_docksList->count() == 0) {
		QListWidgetItem *item = new QListWidgetItem(Translations::get("docks.none_found"), m_docksList);
		item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
	}
	
	obs_log(LOG_INFO, "Found %d docks (%d matched IP:port)", m_docksList->count(), matchedDocks);
}
