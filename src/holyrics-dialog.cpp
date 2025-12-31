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
#include <QIntValidator>
#include <QNetworkInterface>
#include <QHostAddress>

HolyricsDialog::HolyricsDialog(QWidget *parent, HolyricsFinder *finder)
	: QDialog(parent),
	  m_finder(finder)
{
	setWindowTitle("Holyrics Finder");
	setMinimumWidth(550);
	setMinimumHeight(400);

	setupUI();
	detectLocalIP();

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
	ipLayout->addWidget(new QLabel("Port:", this));

	m_portInput = new QSpinBox(this);
	m_portInput->setRange(1, 65535);
	m_portInput->setValue(7575);
	m_portInput->setButtonSymbols(QAbstractSpinBox::NoButtons);
	m_portInput->setAlignment(Qt::AlignCenter);
	m_portInput->setMaximumWidth(80);
	ipLayout->addWidget(m_portInput);

	ipLayout->addStretch();

	connectionLayout->addLayout(ipLayout);

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

void HolyricsDialog::detectLocalIP()
{
	QList<QHostAddress> addresses = QNetworkInterface::allAddresses();
	
	for (const QHostAddress &address : addresses) {
		if (address.protocol() == QAbstractSocket::IPv4Protocol && 
		    !address.isLoopback()) {
			QString ipStr = address.toString();
			QStringList parts = ipStr.split('.');
			
			if (parts.size() == 4) {
				m_octet1->setValue(parts[0].toInt());
				m_octet2->setValue(parts[1].toInt());
				m_octet3->setValue(parts[2].toInt());
				m_octet4->setValue(parts[3].toInt());
				return;
			}
		}
	}
	
	m_octet1->setValue(192);
	m_octet2->setValue(168);
	m_octet3->setValue(0);
	m_octet4->setValue(1);
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

	updateStatus(QString("Testing connection to %1:%2...").arg(ip).arg(port));
	m_testButton->setEnabled(false);
	m_scanButton->setEnabled(false);
	m_createButton->setEnabled(false);

	m_finder->testConnection(ip, port);
}

void HolyricsDialog::onScanNetwork()
{
	QString ip = getIpFromInputs();
	int port = getPortFromInput();

	updateStatus("Scanning network for Holyrics instances...");
	m_progressBar->setVisible(true);
	m_progressBar->setValue(0);
	m_testButton->setEnabled(false);
	m_scanButton->setEnabled(false);
	m_createButton->setEnabled(false);

	m_finder->scanNetwork(ip, port);
}

void HolyricsDialog::onCreateSources()
{
	QString ip = getIpFromInputs();
	int port = getPortFromInput();

	m_finder->createHolyricsSources(ip, port);
	updateStatus(QString("Sources created successfully for %1:%2").arg(ip).arg(port));
}

void HolyricsDialog::onConnectionSuccess(const QString &ip)
{
	m_testButton->setEnabled(true);
	m_scanButton->setEnabled(true);
	m_createButton->setEnabled(true);

	updateStatus("? Connection successful to " + ip);
	
	setIpToInputs(ip);
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
