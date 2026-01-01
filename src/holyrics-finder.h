/*
Holyrics Finder Plugin
Copyright (C) 2024

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
*/

#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QSettings>
#include <QList>

class HolyricsFinder : public QObject {
	Q_OBJECT

public:
	explicit HolyricsFinder(QObject *parent = nullptr);
	~HolyricsFinder();

	struct HolyricsSource {
		QString name;
		QString urlPath;
	};

	struct ConnectionInfo {
		QString ip;
		int port;
	};

	QStringList getIpHistory() const;
	QList<ConnectionInfo> getConnectionHistory() const;
	void addIpToHistory(const QString &ip);
	void addConnectionToHistory(const QString &ip, int port);
	void scanNetwork(const QString &baseIp, int port);
	void testConnection(const QString &ip, int port);
	void createHolyricsSources(const QString &ip, int port);
	void updateBrowserSourceUrl(const QString &name, const QString &url);
	void stopScanning();
	void logConnectionHistory() const;
	
	void prepareForShutdown();

	static QList<HolyricsSource> getSourceDefinitions();

signals:
	void connectionSuccess(const QString &ip);
	void connectionFailed(const QString &ip);
	void scanProgress(int current, int total);
	void scanComplete();

private slots:
	void onNetworkReply(QNetworkReply *reply);

private:
	QNetworkAccessManager *m_networkManager;
	QSettings *m_settings;
	int m_scanningCount;
	int m_scanningTotal;
	int m_currentPort;
	bool m_isShuttingDown;
	QList<QNetworkReply*> m_pendingReplies;
	bool m_scanFoundConnection;

	void createBrowserSource(const QString &name, const QString &url);
	bool isHolyricsResponse(const QString &response);
	void abortPendingRequests();
};
