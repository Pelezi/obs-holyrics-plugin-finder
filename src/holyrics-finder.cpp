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
#include <QUrl>
#include <QNetworkRequest>

HolyricsFinder::HolyricsFinder(QObject *parent)
	: QObject(parent),
	  m_networkManager(new QNetworkAccessManager(this)),
	  m_settings(new QSettings("OBS", "HolyricsFinder")),
	  m_scanningCount(0),
	  m_scanningTotal(0),
	  m_currentPort(7575)
{
	connect(m_networkManager, &QNetworkAccessManager::finished, this,
		&HolyricsFinder::onNetworkReply);
}

HolyricsFinder::~HolyricsFinder()
{
	// Abort all pending network requests before destruction
	if (m_networkManager) {
		disconnect(m_networkManager, nullptr, this, nullptr);
		m_networkManager->deleteLater();
		m_networkManager = nullptr;
	}
	
	if (m_settings) {
		delete m_settings;
		m_settings = nullptr;
	}
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

void HolyricsFinder::scanNetwork(const QString &baseIp, int port)
{
	QStringList parts = baseIp.split('.');
	if (parts.size() != 4) {
		obs_log(LOG_WARNING, "Invalid IP format for scanning: %s",
			baseIp.toUtf8().constData());
		return;
	}

	m_scanningCount = 0;
	m_scanningTotal = 254;
	m_currentPort = port;

	QString base = QString("%1.%2.%3.").arg(parts[0], parts[1], parts[2]);

	for (int i = 1; i <= 254; ++i) {
		QString ip = base + QString::number(i);
		testConnection(ip, port);
	}
}

void HolyricsFinder::testConnection(const QString &ip, int port)
{
	QUrl qurl(QString("http://%1:%2/").arg(ip).arg(port));
	QNetworkRequest request;
	request.setUrl(qurl);
	request.setAttribute(QNetworkRequest::Attribute::User, QVariant(ip));

	m_networkManager->get(request);
}

void HolyricsFinder::onNetworkReply(QNetworkReply *reply)
{
	reply->deleteLater();

	QString ip = reply->request().attribute(QNetworkRequest::User).toString();

	if (m_scanningTotal > 0) {
		m_scanningCount++;
		emit scanProgress(m_scanningCount, m_scanningTotal);

		if (m_scanningCount >= m_scanningTotal) {
			m_scanningCount = 0;
			m_scanningTotal = 0;
			emit scanComplete();
		}
	}

	if (reply->error() == QNetworkReply::NoError) {
		QString response = reply->readAll();
		
		if (isHolyricsResponse(response) || response.contains("holyrics", Qt::CaseInsensitive)) {
			obs_log(LOG_INFO, "Holyrics found at: %s",
				ip.toUtf8().constData());
			emit connectionSuccess(ip);
		} else if (!ip.isEmpty() && m_scanningTotal == 0) {
			emit connectionFailed(ip);
		}
	} else if (!ip.isEmpty() && m_scanningTotal == 0) {
		emit connectionFailed(ip);
	}
}

bool HolyricsFinder::isHolyricsResponse(const QString &response)
{
	return response.contains("holyrics", Qt::CaseInsensitive) ||
	       response.contains("stage-view", Qt::CaseInsensitive);
}

void HolyricsFinder::createHolyricsSources(const QString &ip, int port)
{
	addIpToHistory(ip);

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
