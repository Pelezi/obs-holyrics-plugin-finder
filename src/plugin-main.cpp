/*
Holyrics Finder Plugin
Copyright (C) 2024

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program. If not, see <https://www.gnu.org/licenses/>
*/

#include <obs-module.h>
#include <obs-frontend-api.h>
#include <plugin-support.h>
#include <QCoreApplication>
#include <QLocale>
#include "holyrics-finder.h"
#include "holyrics-dialog.h"
#include "translations.h"

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE(PLUGIN_NAME, "en-US")

const char *obs_module_name(void)
{
	return "Holyrics Finder";
}

const char *obs_module_description(void)
{
	return "Find and connect to Holyrics instances on your network";
}

const char *obs_module_author(void)
{
	return "Pelezi";
}

HolyricsFinder *g_finder = nullptr;
HolyricsDialog *g_dialog = nullptr;

bool obs_module_load(void)
{
	obs_log(LOG_INFO, "plugin loaded successfully (version %s)", PLUGIN_VERSION);

	g_finder = new HolyricsFinder();

	obs_frontend_add_event_callback(
		[](enum obs_frontend_event event, void *) {
			if (event == OBS_FRONTEND_EVENT_FINISHED_LOADING) {
				// Detect OBS language
				QString obsLang = QLocale().name();
				if (obsLang.startsWith("pt")) {
					Translations::setLanguage("pt-BR");
				} else {
					Translations::setLanguage("en");
				}

				g_dialog = new HolyricsDialog(
					(QWidget *)obs_frontend_get_main_window(),
					g_finder);

				// Get translated menu name
				QString menuName = Translations::get("menu.name");
				
				obs_frontend_add_tools_menu_item(
					menuName.toUtf8().constData(),
					[](void *) { g_dialog->show(); }, nullptr);
			}
		},
		nullptr);

	return true;
}

void obs_module_unload(void)
{
	obs_log(LOG_INFO, "[obs-holyrics-finder] plugin unloading...");

	// Set shutdown flag first
	if (g_finder) {
		g_finder->prepareForShutdown();
	}

	// Don't delete dialog - Qt is shutting down and any Qt calls can crash
	// Just set to nullptr and let the OS clean up the memory
	if (g_dialog) {
		g_dialog = nullptr;
	}

	// Don't delete finder - it has Qt objects (QNetworkAccessManager, QSettings)
	// Just set to nullptr and let the OS clean up the memory
	if (g_finder) {
		g_finder = nullptr;
	}

	obs_log(LOG_INFO, "[obs-holyrics-finder] plugin unloaded");
}
