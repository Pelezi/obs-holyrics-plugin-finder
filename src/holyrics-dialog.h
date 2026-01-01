/*
Holyrics Finder Plugin
Copyright (C) 2024

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
*/

#pragma once

#include <QDialog>
#include <QLineEdit>
#include <QSpinBox>
#include <QPushButton>
#include <QLabel>
#include <QProgressBar>
#include <QListWidget>

class HolyricsFinder;

class HolyricsDialog : public QDialog {
	Q_OBJECT

public:
	explicit HolyricsDialog(QWidget *parent, HolyricsFinder *finder);
	~HolyricsDialog();

private slots:
	void onTestConnection();
	void onScanNetwork();
	void onUpdateSources();
	void onConnectionSuccess(const QString &ip);
	void onConnectionFailed(const QString &ip);
	void onScanProgress(int current, int total);
	void onScanComplete();
	void refreshSourcesList();

private:
	HolyricsFinder *m_finder;

	QSpinBox *m_octet1;
	QSpinBox *m_octet2;
	QSpinBox *m_octet3;
	QSpinBox *m_octet4;
	QSpinBox *m_portInput;
	QPushButton *m_testButton;
	QPushButton *m_scanButton;
	QPushButton *m_updateButton;
	QPushButton *m_copyIpButton;
	QLabel *m_statusLabel;
	QProgressBar *m_progressBar;
	QListWidget *m_sourcesList;

	void setupUI();
	void detectLocalIP();
	QString getIpFromInputs() const;
	int getPortFromInput() const;
	void setIpToInputs(const QString &ip);
	void updateStatus(const QString &message, bool isError = false);
};
