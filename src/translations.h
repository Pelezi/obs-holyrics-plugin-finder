/*
Holyrics Finder Plugin
Copyright (C) 2024

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
*/

#pragma once

#include <QString>
#include <QLocale>

class Translations {
public:
	static QString get(const QString &key);
	static void setLanguage(const QString &lang);
	static QString getCurrentLanguage();

private:
	static QString s_currentLanguage;
};
