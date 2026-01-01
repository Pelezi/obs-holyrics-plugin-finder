/*
Holyrics Finder Plugin
Copyright (C) 2024

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
*/

#include "holyrics-finder.h"
#include <obs-module.h>
#include <obs-frontend-api.h>
#include <plugin-support.h>
#include <obs-data.h>
#include <QUrl>
#include <QNetworkRequest>
#include <QMainWindow>
#include <QDockWidget>
#include <QMetaProperty>
#include <QMessageBox>
#include <QRegularExpression>
#include <QUuid>
#include <QSet>

HolyricsFinder::HolyricsFinder(QObject *parent)
	: QObject(parent),
	  m_networkManager(new QNetworkAccessManager(this)),
	  m_settings(new QSettings("OBS", "HolyricsFinder")),
	  m_scanningCount(0),
	  m_scanningTotal(0),
	  m_currentPort(80),
	  m_isShuttingDown(false),
	  m_scanFoundConnection(false)
{
	connect(m_networkManager, &QNetworkAccessManager::finished, this,
		&HolyricsFinder::onNetworkReply);
	
	logConnectionHistory();
}

HolyricsFinder::~HolyricsFinder()
{
	obs_log(LOG_INFO, "[HolyricsFinder] Destructor called");
	
	m_isShuttingDown = true;
	
	// Don't delete Qt objects during shutdown - Qt's event system is shutting down
	// and any Qt calls can crash. Just set to nullptr and let the OS clean up.
	m_networkManager = nullptr;
	m_settings = nullptr;
	
	obs_log(LOG_INFO, "[HolyricsFinder] Destructor complete");
}

QList<HolyricsFinder::HolyricsSource> HolyricsFinder::getSourceDefinitions()
{
	return {
		{"Holyrics - Text", "/stage-view/text"},
		{"Holyrics - Text 2", "/stage-view/text-2"},
		{"Holyrics - Text Aux Control", "/stage-view/text/aux-control"},
		{"Holyrics - Widescreen", "/stage-view/widescreen"}
	};
}

QStringList HolyricsFinder::getIpHistory() const
{
	return m_settings->value("ipHistory", QStringList()).toStringList();
}

QList<HolyricsFinder::ConnectionInfo> HolyricsFinder::getConnectionHistory() const
{
	QList<ConnectionInfo> history;
	QStringList connections = m_settings->value("connectionHistory", QStringList()).toStringList();
	
	for (const QString &connStr : connections) {
		QStringList parts = connStr.split(':');
		if (parts.size() == 2) {
			ConnectionInfo info;
			info.ip = parts[0];
			info.port = parts[1].toInt();
			history.append(info);
		}
	}
	
	return history;
}

void HolyricsFinder::addIpToHistory(const QString &ip)
{
	QStringList history = getIpHistory();
	
	history.removeAll(ip);
	history.prepend(ip);
	
	while (history.size() > 10) {
		history.removeLast();
	}
	
	m_settings->setValue("ipHistory", history);
	m_settings->sync();
}

void HolyricsFinder::addConnectionToHistory(const QString &ip, int port)
{
	QString connStr = QString("%1:%2").arg(ip).arg(port);
	QStringList history = m_settings->value("connectionHistory", QStringList()).toStringList();
	
	history.removeAll(connStr);
	history.prepend(connStr);
	
	while (history.size() > 10) {
		history.removeLast();
	}
	
	m_settings->setValue("connectionHistory", history);
	m_settings->sync();
	
	obs_log(LOG_INFO, "[HolyricsFinder] Added connection to history: %s:%d",
		ip.toUtf8().constData(), port);
	
	addIpToHistory(ip);
	
	logConnectionHistory();
}

void HolyricsFinder::scanNetwork(const QString &baseIp, int port)
{
	QStringList parts = baseIp.split('.');
	if (parts.size() != 4) {
		obs_log(LOG_WARNING, "Invalid IP format for scanning: %s",
			baseIp.toUtf8().constData());
		return;
	}

	abortPendingRequests();
	
	m_currentPort = port;
	m_scanFoundConnection = false;

	QList<ConnectionInfo> history = getConnectionHistory();
	QSet<QString> historyIps;
	int historyTestCount = 0;
	
	for (const ConnectionInfo &conn : history) {
		if (conn.port == port) {
			historyIps.insert(conn.ip);
			historyTestCount++;
		}
	}

	QString base = QString("%1.%2.%3.").arg(parts[0], parts[1], parts[2]);
	QStringList ipsToScan;
	
	for (const ConnectionInfo &conn : history) {
		if (conn.port == port) {
			ipsToScan.prepend(conn.ip);
		}
	}
	
	for (int i = 1; i <= 254; ++i) {
		QString ip = base + QString::number(i);
		if (!historyIps.contains(ip)) {
			ipsToScan.append(ip);
		}
	}
	
	m_scanningCount = 0;
	m_scanningTotal = ipsToScan.size();
	
	if (historyTestCount > 0) {
		obs_log(LOG_INFO, "Testing %d connection(s) from history first, then %d other IPs",
			historyTestCount, m_scanningTotal - historyTestCount);
	}
	
	for (const QString &ip : ipsToScan) {
		testConnection(ip, port);
	}
}

void HolyricsFinder::testConnection(const QString &ip, int port)
{
	if (m_scanFoundConnection && m_scanningTotal > 0) {
		return;
	}
	
	QUrl qurl(QString("http://%1:%2/").arg(ip).arg(port));
	QNetworkRequest request;
	request.setUrl(qurl);
	request.setAttribute(QNetworkRequest::Attribute::User, QVariant(ip));
	request.setTransferTimeout(2000);

	QNetworkReply *reply = m_networkManager->get(request);
	if (m_scanningTotal > 0) {
		m_pendingReplies.append(reply);
	}
}

void HolyricsFinder::onNetworkReply(QNetworkReply *reply)
{
	m_pendingReplies.removeOne(reply);
	reply->deleteLater();

	QString ip = reply->request().attribute(QNetworkRequest::User).toString();

	bool wasScanning = (m_scanningTotal > 0);
	
	if (wasScanning) {
		m_scanningCount++;
		emit scanProgress(m_scanningCount, m_scanningTotal);
	}

	if (reply->error() == QNetworkReply::NoError) {
		QString response = reply->readAll();
		
		if (isHolyricsResponse(response) || response.contains("holyrics", Qt::CaseInsensitive)) {
			obs_log(LOG_INFO, "Holyrics found at: %s",
				ip.toUtf8().constData());
			
			int port = reply->request().url().port();
			addConnectionToHistory(ip, port);
			
			if (wasScanning) {
				m_scanFoundConnection = true;
				m_scanningCount = 0;
				m_scanningTotal = 0;
				abortPendingRequests();
				emit scanComplete();
			}
			
			emit connectionSuccess(ip);
			return;
		} else if (!wasScanning) {
			emit connectionFailed(ip);
		}
	} else if (!wasScanning) {
		emit connectionFailed(ip);
	}
	
	if (wasScanning && m_scanningTotal > 0 && m_scanningCount >= m_scanningTotal) {
		m_scanningCount = 0;
		m_scanningTotal = 0;
		abortPendingRequests();
		emit scanComplete();
	}
}

bool HolyricsFinder::isHolyricsResponse(const QString &response)
{
	return response.contains("holyrics", Qt::CaseInsensitive) ||
	       response.contains("stage-view", Qt::CaseInsensitive);
}

void HolyricsFinder::createHolyricsSources(const QString &ip, int port)
{
	addConnectionToHistory(ip, port);

	auto sources = getSourceDefinitions();
	for (const auto &source : sources) {
		QString url = QString("http://%1:%2%3").arg(ip).arg(port).arg(source.urlPath);
		createBrowserSource(source.name, url);
	}

	obs_log(LOG_INFO, "Created %d Holyrics sources for IP: %s:%d",
		sources.size(), ip.toUtf8().constData(), port);
}

void HolyricsFinder::createBrowserSource(const QString &name, const QString &url)
{
	obs_source_t *currentSceneSource = obs_frontend_get_current_scene();
	if (!currentSceneSource) {
		obs_log(LOG_WARNING, "No current scene available");
		return;
	}

	obs_scene_t *currentScene = obs_scene_from_source(currentSceneSource);
	if (!currentScene) {
		obs_log(LOG_WARNING, "Could not get scene from source");
		obs_source_release(currentSceneSource);
		return;
	}

	obs_data_t *settings = obs_data_create();
	obs_data_set_string(settings, "url", url.toUtf8().constData());
	obs_data_set_int(settings, "width", 1920);
	obs_data_set_int(settings, "height", 1080);
	obs_data_set_bool(settings, "shutdown", true);

	obs_source_t *source = obs_source_create("browser_source",
						 name.toUtf8().constData(),
						 settings, nullptr);

	if (source) {
		obs_scene_add(currentScene, source);
		obs_source_release(source);
		obs_log(LOG_INFO, "Created source: %s with URL: %s",
			name.toUtf8().constData(),
			url.toUtf8().constData());
	} else {
		obs_log(LOG_ERROR, "Failed to create browser source: %s",
			name.toUtf8().constData());
	}

	obs_data_release(settings);
	obs_source_release(currentSceneSource);
}

void HolyricsFinder::updateBrowserSourceUrl(const QString &name, const QString &url)
{
	obs_source_t *source = obs_get_source_by_name(name.toUtf8().constData());
	if (!source) {
		obs_log(LOG_WARNING, "Source not found: %s", name.toUtf8().constData());
		return;
	}

	obs_data_t *settings = obs_source_get_settings(source);
	obs_data_set_string(settings, "url", url.toUtf8().constData());
	obs_source_update(source, settings);
	
	obs_log(LOG_INFO, "Updated source: %s with URL: %s",
		name.toUtf8().constData(),
		url.toUtf8().constData());

	obs_data_release(settings);
	obs_source_release(source);
}

void HolyricsFinder::prepareForShutdown()
{
	obs_log(LOG_INFO, "[HolyricsFinder] Preparing for shutdown");
	m_isShuttingDown = true;
}

void HolyricsFinder::stopScanning()
{
	if (m_scanningTotal > 0) {
		obs_log(LOG_INFO, "Stopping network scan");
		m_scanningCount = 0;
		m_scanningTotal = 0;
		m_scanFoundConnection = false;
		abortPendingRequests();
		emit scanComplete();
	}
}

void HolyricsFinder::abortPendingRequests()
{
	for (QNetworkReply *reply : m_pendingReplies) {
		if (reply && reply->isRunning()) {
			reply->abort();
		}
	}
	m_pendingReplies.clear();
}

void HolyricsFinder::logConnectionHistory() const
{
	QList<ConnectionInfo> history = getConnectionHistory();
	
	if (history.isEmpty()) {
		obs_log(LOG_INFO, "[HolyricsFinder] Connection history is empty");
		return;
	}
	
	obs_log(LOG_INFO, "[HolyricsFinder] Connection history (%d entries):", history.size());
	for (int i = 0; i < history.size(); ++i) {
		obs_log(LOG_INFO, "  [%d] %s:%d", i + 1,
			history[i].ip.toUtf8().constData(), history[i].port);
	}
}
