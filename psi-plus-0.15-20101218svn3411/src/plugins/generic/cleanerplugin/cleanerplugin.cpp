/*
 * cleanerplugin.cpp - plugin
 * Copyright (C) 2009-2010  Khryukin Evgeny
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include "cleanerplugin.h"
#include "cleaner.h"

#define constVersion "0.2.9"

#define constHeight "height"
#define constWidth "width"

Q_EXPORT_PLUGIN(CleanerPlugin);

CleanerPlugin::CleanerPlugin() {
    enabled = false;
    psiOptions = 0;
    appInfo = 0;
    iconHost = 0;
    cln = 0;
    height = 650;
    width = 900;
}

QString CleanerPlugin::name() const {
        return "Cleaner Plugin";
}

QString CleanerPlugin::shortName() const {
        return "cleaner";
}

QString CleanerPlugin::version() const {
        return constVersion;
}

bool CleanerPlugin::enable() {
    if(psiOptions) {
        enabled = true;
        height = psiOptions->getPluginOption(constHeight, QVariant(height)).toInt();
        width = psiOptions->getPluginOption(constWidth, QVariant(width)).toInt();
    }

    return enabled;
}

bool CleanerPlugin::disable() {
    if (cln) {
        delete(cln);
        cln = 0;
    }
    enabled = false;
    return true;
}

QWidget* CleanerPlugin::options() {
        if (!enabled) {
		return 0;
	}
        QWidget *options = new QWidget();
        QVBoxLayout *hbox= new QVBoxLayout(options);
        QPushButton *goButton = new QPushButton(tr("Launch Cleaner"));
	QHBoxLayout *h = new QHBoxLayout;
	h->addWidget(goButton);
	h->addStretch();
	hbox->addLayout(h);
        QLabel *wikiLink = new QLabel(tr("<a href=\"http://psi-plus.com/wiki/plugins#cleaner_plugin\">Wiki (Online)</a>"));
	wikiLink->setOpenExternalLinks(true);
        hbox->addStretch();
        hbox->addWidget(wikiLink);
        connect(goButton, SIGNAL(released()), SLOT(start()));
        return options;
}

void CleanerPlugin::setApplicationInfoAccessingHost(ApplicationInfoAccessingHost * host) {
    appInfo = host;
}

void CleanerPlugin::setIconFactoryAccessingHost(IconFactoryAccessingHost* host) {
    iconHost = host;
}

void CleanerPlugin::setOptionAccessingHost(OptionAccessingHost* host) {
    psiOptions = host;
}

void CleanerPlugin::start() {
    if(!enabled) return;

    if(!cln) {
        cln = new CleanerMainWindow(this);
        cln->resizeWindow(width, height);
        cln->showCleaner();
    } else
        cln->raise();
}

void CleanerPlugin::deleteCln() {
    height = cln->height();
    psiOptions->setPluginOption(constHeight, QVariant(height));
    width = cln->width();
    psiOptions->setPluginOption(constWidth, QVariant(width));
    delete(cln);
    cln = 0;
}

QString CleanerPlugin::pluginInfo() {
	return tr("Author: ") +  "Dealer_WeARE\n"
			+ tr("Email: ") + "wadealer@gmail.com\n\n"
			+ trUtf8("This plugin is designed to clear the avatar cache, saved local copies of vCards and history logs.\n"
			 "You can preview items before deleting them from your hard drive.");
}
