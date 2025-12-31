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
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QRegularExpressionValidator>
#include <QRegularExpression>

HolyricsDialog::HolyricsDialog(QWidget *parent, HolyricsFinder *finder)
	: QDialog(parent),
	  m_finder(finder)
{
	setWindowTitle("Holyrics Finder");
	setMinimumWidth(500);
	setMinimumHeight(400);

	setupUI();
	loadIpHistory();

	connect(m_finder, &HolyricsFinder::connectionSuccess, this,
		&HolyricsDialog::onConnectionSuccess);
	connect(m_finder, &HolyricsFinder::connectionFailed, this,
		&HolyricsDialog::onConnectionFailed);
	connect(m_finder, &HolyricsFinder::scanProgress, this,
		&HolyricsDialog::onScanProgress);
	connect(m_finder, &HolyricsFinder::scanComplete, this,
		&HolyricsDialog::onScanComplete);
}

HolyricsDialog::~HolyricsDialog() {}

void HolyricsDialog::setupUI()
{
	QVBoxLayout *mainLayout = new QVBoxLayout(this);

	QGroupBox *connectionGroup = new QGroupBox("Connection", this);
	QVBoxLayout *connectionLayout = new QVBoxLayout(connectionGroup);

	QLabel *ipLabel = new QLabel("Holyrics IP Address:", this);
	connectionLayout->addWidget(ipLabel);

	m_ipCombo = new QComboBox(this);
	m_ipCombo->setEditable(true);
	m_ipCombo->setInsertPolicy(QComboBox::NoInsert);
	m_ipCombo->lineEdit()->setPlaceholderText("192.168.0.124");
	
	QRegularExpression ipRegex(
		"^(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}"
		"(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$");
	QRegularExpressionValidator *ipValidator =
		new QRegularExpressionValidator(ipRegex, this);
	m_ipCombo->lineEdit()->setValidator(ipValidator);

	connectionLayout->addWidget(m_ipCombo);

	QHBoxLayout *buttonLayout = new QHBoxLayout();
	
	m_testButton = new QPushButton("Test Connection", this);
	connect(m_testButton, &QPushButton::clicked, this,
		&HolyricsDialog::onTestConnection);
	buttonLayout->addWidget(m_testButton);

	m_scanButton = new QPushButton("Scan Network", this);
	connect(m_scanButton, &QPushButton::clicked, this,
		&HolyricsDialog::onScanNetwork);
	buttonLayout->addWidget(m_scanButton);

	connectionLayout->addLayout(buttonLayout);

	m_statusLabel = new QLabel("Ready", this);
	m_statusLabel->setWordWrap(true);
	connectionLayout->addWidget(m_statusLabel);

	m_progressBar = new QProgressBar(this);
	m_progressBar->setVisible(false);
	connectionLayout->addWidget(m_progressBar);

	mainLayout->addWidget(connectionGroup);

	QGroupBox *sourcesGroup = new QGroupBox("Holyrics Sources", this);
	QVBoxLayout *sourcesLayout = new QVBoxLayout(sourcesGroup);

	QLabel *sourcesLabel =
		new QLabel("The following sources will be created:", this);
	sourcesLayout->addWidget(sourcesLabel);

	m_sourcesList = new QListWidget(this);
	auto sources = HolyricsFinder::getSourceDefinitions();
	for (const auto &source : sources) {
		m_sourcesList->addItem(source.name);
	}
	sourcesLayout->addWidget(m_sourcesList);

	m_createButton = new QPushButton("Create Sources", this);
	m_createButton->setEnabled(false);
	connect(m_createButton, &QPushButton::clicked, this,
		&HolyricsDialog::onCreateSources);
	sourcesLayout->addWidget(m_createButton);

	mainLayout->addWidget(sourcesGroup);

	QHBoxLayout *closeLayout = new QHBoxLayout();
	closeLayout->addStretch();
	QPushButton *closeButton = new QPushButton("Close", this);
	connect(closeButton, &QPushButton::clicked, this, &QDialog::close);
	closeLayout->addWidget(closeButton);
	mainLayout->addLayout(closeLayout);
}

void HolyricsDialog::loadIpHistory()
{
	QStringList history = m_finder->getIpHistory();
	m_ipCombo->clear();
	m_ipCombo->addItems(history);
}

void HolyricsDialog::onTestConnection()
{
	QString ip = m_ipCombo->currentText().trimmed();
	if (ip.isEmpty()) {
		updateStatus("Please enter an IP address", true);
		return;
	}

	updateStatus("Testing connection to " + ip + "...");
	m_testButton->setEnabled(false);
	m_scanButton->setEnabled(false);
	m_createButton->setEnabled(false);

	m_finder->testConnection(ip);
}

void HolyricsDialog::onScanNetwork()
{
	QString ip = m_ipCombo->currentText().trimmed();
	if (ip.isEmpty()) {
		ip = "192.168.0.1";
	}

	updateStatus("Scanning network for Holyrics instances...");
	m_progressBar->setVisible(true);
	m_progressBar->setValue(0);
	m_testButton->setEnabled(false);
	m_scanButton->setEnabled(false);
	m_createButton->setEnabled(false);

	m_finder->scanNetwork(ip);
}

void HolyricsDialog::onCreateSources()
{
	QString ip = m_ipCombo->currentText().trimmed();
	if (ip.isEmpty()) {
		updateStatus("Please enter an IP address", true);
		return;
	}

	m_finder->createHolyricsSources(ip);
	updateStatus("Sources created successfully for " + ip);
	loadIpHistory();
}

void HolyricsDialog::onConnectionSuccess(const QString &ip)
{
	m_testButton->setEnabled(true);
	m_scanButton->setEnabled(true);
	m_createButton->setEnabled(true);

	updateStatus("? Connection successful to " + ip);
	
	m_ipCombo->setCurrentText(ip);
	
	if (m_ipCombo->findText(ip) == -1) {
		m_ipCombo->insertItem(0, ip);
	}
}

void HolyricsDialog::onConnectionFailed(const QString &ip)
{
	m_testButton->setEnabled(true);
	m_scanButton->setEnabled(true);
	m_createButton->setEnabled(false);

	updateStatus("? Connection failed to " + ip, true);
}

void HolyricsDialog::onScanProgress(int current, int total)
{
	m_progressBar->setMaximum(total);
	m_progressBar->setValue(current);
	updateStatus(QString("Scanning network... %1/%2").arg(current).arg(total));
}

void HolyricsDialog::onScanComplete()
{
	m_progressBar->setVisible(false);
	m_testButton->setEnabled(true);
	m_scanButton->setEnabled(true);
	updateStatus("Network scan complete");
}

void HolyricsDialog::updateStatus(const QString &message, bool isError)
{
	m_statusLabel->setText(message);
	
	if (isError) {
		m_statusLabel->setStyleSheet("QLabel { color: red; }");
	} else if (message.startsWith("?")) {
		m_statusLabel->setStyleSheet("QLabel { color: green; }");
	} else {
		m_statusLabel->setStyleSheet("");
	}
}
