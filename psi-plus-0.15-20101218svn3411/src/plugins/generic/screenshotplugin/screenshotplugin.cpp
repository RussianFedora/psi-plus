/*
 * screenshotplugin.cpp - plugin
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

#include "psiplugin.h"
#include "optionaccessor.h"
#include "optionaccessinghost.h"
#include "shortcutaccessor.h"
#include "shortcutaccessinghost.h"
#include "plugininfoprovider.h"
#include "iconfactoryaccessinghost.h"
#include "iconfactoryaccessor.h"
#include "menuaccessor.h"
#include "applicationinfoaccessinghost.h"
#include "applicationinfoaccessor.h"

#include "screenshot.h"
#include "server.h"
#include "editserverdlg.h"
#include "ui_options.h"

#define cVersion "0.4.4"

#define constShortCut "shortCut"
#define constFormat "format"
#define constFileName "fileName"
#define constServerList "serverlist"

const QString imageShack = "ImageShack.us&split&http://load.imageshack.us/&split&&split&&split&uploadtype=image&split&fileupload&split&(?:<div class=\"listbox\">.*)(?:<label><a href=\")(.*imageshack.*)(?:\"\\sonClick.*>.*</a></label>)&split&true";
const QString radikal = "Radikal.ru&split&http://www.radikal.ru/action.aspx&split&&split&&split&upload=yes&split&F&split&<input\\s+id=\"input_link_1\"\\s+value=\"([^\"]+)\"&split&true";
const QString pixacadem = "Pix.Academ.org&split&http://pix.academ.org/&split&&split&&split&action=upload_image&split&image&split&<div id='link'><a href=\"(http[^\"]+)\"&split&true";
const QString kachalka = "Kachalka.com&split&http://www.kachalka.com/upload.php&split&&split&&split&&split&userfile[]&split&name=\"option\" value=\"(http://www.kachalka.com/[^\"]+)\" /></td>&split&true";

class ScreenshotPlugin : public QObject, public PsiPlugin, public OptionAccessor, public ShortcutAccessor, public PluginInfoProvider,
		public IconFactoryAccessor, public MenuAccessor, public ApplicationInfoAccessor
{
	Q_OBJECT
	Q_INTERFACES(PsiPlugin OptionAccessor ShortcutAccessor PluginInfoProvider IconFactoryAccessor MenuAccessor ApplicationInfoAccessor)

public:
	ScreenshotPlugin();

	virtual QString name() const;
	virtual QString shortName() const;
	virtual QString version() const;
	virtual QWidget* options();
	virtual bool enable();
	virtual bool disable();
	virtual void setOptionAccessingHost(OptionAccessingHost* host);
	virtual void optionChanged(const QString& /*option*/) {};
	virtual void setShortcutAccessingHost(ShortcutAccessingHost* host);
	virtual void setIconFactoryAccessingHost(IconFactoryAccessingHost* host);
	virtual void setApplicationInfoAccessingHost(ApplicationInfoAccessingHost* host);
	virtual QList < QVariantHash >* getAccountMenuParam();
	virtual QList < QVariantHash >* getContactMenuParam();
	virtual void setShortcuts();

	virtual void applyOptions();
	virtual void restoreOptions();

	virtual QString pluginInfo();

private:
	bool enabled_;
	QPointer<Screenshot> screenshot;
	OptionAccessingHost* psiOptions;
	ShortcutAccessingHost* psiShortcuts;
	IconFactoryAccessingHost* icoHost;
	ApplicationInfoAccessingHost* appInfo;

	QString shortCut;
	QString format;
	QString fileName;
	QStringList servers;

	QPointer<QWidget> optionsWid;

	Ui::OptionsWidget ui_;

	bool isListContainsServer(QString server);

private slots:
	void requstNewShortcut();
	void onNewShortcut(QKeySequence);
	void onShortCutActivated();
	void addServer();
	void delServer();
	void editServer();
	void addNewServer(QString);
	void editCurrentServer(QString);
	void openImage();
	void applyButtonActivate();
	void setOption(QString option, QVariant value);
    };

Q_EXPORT_PLUGIN(ScreenshotPlugin);

ScreenshotPlugin::ScreenshotPlugin()
{
	enabled_ = false;
	psiOptions = 0;
	psiShortcuts = 0;
	icoHost = 0;

	shortCut = "Alt+Ctrl+p";
	format = "jpg";
	fileName = "pic-yyyyMMdd-hhmmss";

}

QString ScreenshotPlugin::name() const
{
	return "Screenshot Plugin";
}

QString ScreenshotPlugin::shortName() const
{
	return "Screenshot";
}

QString ScreenshotPlugin::version() const
{
	return cVersion;
}

QWidget* ScreenshotPlugin::options()
{
	if (!enabled_) {
		return 0;
	}
	optionsWid = new QWidget();

	ui_.setupUi(optionsWid);
	ui_.cb_hack->setVisible(false);

	restoreOptions();

	connect(ui_.pb_modify, SIGNAL(clicked()), this, SLOT(requstNewShortcut()));
	connect(ui_.pb_add, SIGNAL(clicked()), this, SLOT(addServer()));
	connect(ui_.pb_del, SIGNAL(clicked()), this, SLOT(delServer()));
	connect(ui_.pb_edit, SIGNAL(clicked()), this, SLOT(editServer()));
	connect(ui_.lw_servers, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(editServer()));
	connect(ui_.lw_servers, SIGNAL(currentRowChanged(int)), this, SLOT(applyButtonActivate()));

	return optionsWid;
}

bool ScreenshotPlugin::enable(){
	QFile file(":/screenshotplugin/screenshot.png");
	if ( file.open(QIODevice::ReadOnly) ) {
		QByteArray image = file.readAll();
		icoHost->addIcon("screenshotplugin/screenshot",image);
		file.close();
	} else {
		enabled_ = false;
		return enabled_;
	}
	if(psiOptions) {
		enabled_ = true;
		shortCut = psiOptions->getPluginOption(constShortCut, QVariant(shortCut)).toString();
		format = psiOptions->getPluginOption(constFormat, QVariant(format)).toString();
		fileName = psiOptions->getPluginOption(constFileName, QVariant(fileName)).toString();
		servers = psiOptions->getPluginOption(constServerList, QVariant(servers)).toStringList();

		if(!isListContainsServer(imageShack))
			servers.append(imageShack);
		if(!isListContainsServer(pixacadem))
			servers.append(pixacadem);
		if(!isListContainsServer(radikal))
			servers.append(radikal);
		if(!isListContainsServer(kachalka))
			servers.append(kachalka);
	}

	appInfo->getProxyFor(name()); //init proxy settings
	return enabled_;
}

bool ScreenshotPlugin::disable() {
	psiShortcuts->disconnectShortcut(QKeySequence(shortCut), this,  SLOT(onShortCutActivated()));	
	if (screenshot) {		
		delete(screenshot);
	}
	if(optionsWid)
		delete(ui_.lw_servers);
	enabled_ = false;
	return true;
}

void ScreenshotPlugin::setOptionAccessingHost(OptionAccessingHost* host)
{
	psiOptions = host;
}

void ScreenshotPlugin::setShortcutAccessingHost(ShortcutAccessingHost* host)
{
	psiShortcuts = host;
}

void ScreenshotPlugin::setIconFactoryAccessingHost(IconFactoryAccessingHost* host) {
	icoHost = host;
}

void ScreenshotPlugin::setApplicationInfoAccessingHost(ApplicationInfoAccessingHost *host) {
	appInfo = host;
}

void ScreenshotPlugin::setShortcuts() {
	psiShortcuts->connectShortcut(QKeySequence(shortCut), this, SLOT(onShortCutActivated()));
}

QList < QVariantHash >* ScreenshotPlugin::getAccountMenuParam() {
		QVariantHash hash;
		hash["icon"] = QVariant(QString("screenshotplugin/screenshot"));
		hash["name"] = QVariant(tr("Upload Image"));
		hash["reciver"] = qVariantFromValue(qobject_cast<QObject *>(this));
		hash["slot"] = QVariant(SLOT(openImage()));
		QList< QVariantHash > * l = new QList< QVariantHash >();
		l->push_back(hash);
		return l;
}

QList < QVariantHash >* ScreenshotPlugin::getContactMenuParam() {
	return 0;
}

void ScreenshotPlugin::openImage() {
	if(!enabled_) return;

	if(!screenshot) {
		screenshot = new Screenshot(icoHost, fileName, format);
		screenshot->setServersList(servers);
		screenshot->init(psiOptions->getPluginOption(constPenWidth, QVariant(2)).toInt(),
				 psiOptions->getPluginOption(constColor, QVariant("#fffff")).toString(),
				 psiOptions->getPluginOption(constFont, QVariant("Sans Serif,18,-1,5,50,0,0,0,0,0")).toString());
		Proxy prx = appInfo->getProxyFor(name());
		screenshot->setProxy(prx.host,prx.port,prx.user,prx.pass,prx.type);

		connect(screenshot, SIGNAL(settingsChanged(QString, QVariant)), SLOT(setOption(QString,QVariant)));
	}

	screenshot->openImage();
	screenshot->show();
}

void ScreenshotPlugin::applyOptions() {
	shortCut = ui_.le_shortcut->text();
	psiOptions->setPluginOption(constShortCut, QVariant(shortCut));

	format = ui_.cb_format->currentText();
	psiOptions->setPluginOption(constFormat, QVariant(format));

	fileName = ui_.le_filename->text();
	psiOptions->setPluginOption(constFileName, QVariant(fileName));

	servers.clear();
	for(int i = 0; i < ui_.lw_servers->count(); i++) {
		Server *s = (Server *)ui_.lw_servers->item(i);
		servers.append(s->settingsToString());
	}
	psiOptions->setPluginOption(constServerList, QVariant(servers));

	psiShortcuts->disconnectShortcut(QKeySequence(shortCut), this,  SLOT(onShortCutActivated()));
	setShortcuts();
}

void ScreenshotPlugin::restoreOptions() {
	QStringList l = QStringList() << "jpg" << "png";
	ui_.cb_format->addItems(l);
	int index = ui_.cb_format->findText(format);
	if(index != -1)
		ui_.cb_format->setCurrentIndex(index);
	ui_.le_filename->setText(fileName);
	ui_.le_shortcut->setText(shortCut);
	foreach(QString settings, servers) {
		Server *s = new Server(ui_.lw_servers);
		s->setFromString(settings);
		s->setText(s->displayName());
	}
}

void ScreenshotPlugin::requstNewShortcut() {
	psiShortcuts->requestNewShortcut(this, SLOT(onNewShortcut(QKeySequence)));
}

void ScreenshotPlugin::onNewShortcut(QKeySequence ks) {
	ui_.le_shortcut->setText(ks.toString(QKeySequence::NativeText));
}

void ScreenshotPlugin::onShortCutActivated() {
	if(!enabled_) return;

	if(!screenshot) {
		screenshot = new Screenshot(icoHost, fileName, format);
		screenshot->setServersList(servers);
		screenshot->init(psiOptions->getPluginOption(constPenWidth, QVariant(2)).toInt(),
				 psiOptions->getPluginOption(constColor, QVariant("#fffff")).toString(),
				 psiOptions->getPluginOption(constFont, QVariant("Sans Serif,18,-1,5,50,0,0,0,0,0")).toString());
		Proxy prx = appInfo->getProxyFor(name());
		screenshot->setProxy(prx.host,prx.port,prx.user,prx.pass,prx.type);

		connect(screenshot, SIGNAL(settingsChanged(QString, QVariant)), SLOT(setOption(QString,QVariant)));
	}

	screenshot->shootScreen();
}

void ScreenshotPlugin::addServer() {
	EditServerDlg *esd = new EditServerDlg();
	connect(esd, SIGNAL(okPressed(QString)), this, SLOT(addNewServer(QString)));
	esd->show();
}

void ScreenshotPlugin::delServer() {
	Server *s = (Server*)ui_.lw_servers->currentItem();
	if(!s) return;
	ui_.lw_servers->removeItemWidget(s);
	delete(s);
	applyButtonActivate();
}

void ScreenshotPlugin::editServer() {
	Server *s = (Server*)ui_.lw_servers->currentItem();
	if(!s) return;
	EditServerDlg *esd = new EditServerDlg();
	connect(esd, SIGNAL(okPressed(QString)), this, SLOT(editCurrentServer(QString)));
	esd->setSettings(s->settingsToString());
	esd->show();
}

void ScreenshotPlugin::addNewServer(QString settings) {
	Server *s = new Server(ui_.lw_servers);
	s->setFromString(settings);
	s->setText(s->displayName());

	applyButtonActivate();
}

void ScreenshotPlugin::editCurrentServer(QString settings) {
	Server *s = (Server*)ui_.lw_servers->currentItem();
	s->setFromString(settings);

	applyButtonActivate();
}

bool ScreenshotPlugin::isListContainsServer(QString server) {
	foreach(QString serv, servers) {
		if(serv.split(Server::splitString()).first() == server.split(Server::splitString()).first())
			return true;
	}
	return false;
}

void ScreenshotPlugin::applyButtonActivate()  {
	ui_.cb_hack->toggle();
}

void ScreenshotPlugin::setOption(QString option, QVariant value) {
	psiOptions->setPluginOption(option, value);
}

QString ScreenshotPlugin::pluginInfo() {
	return tr("Authors: ") +  "C.H., Dealer_WeARE\n\n"
			+ trUtf8("This plugin allows you to make screenshots and save them to your hard drive or upload them to an FTP or HTTP server.\n"
			 "The plugin has the following settings:\n"
			 "* Shortcut -- hotkey to make the screenshot (by default, Ctrl+Alt+P)\n"
			 "* Format -- the file format in which the screenshot will be stored (default: .jpg)\n"
			 "* File Name -- format of the filename (default: pic-yyyyMMdd-hhmmss, where yyyyMMdd=YYYYMMDD, and hhmmss are current date in the format yearmonthday-hourminutesecond)\n"
		     "The address of FTP server is specified as ftp://ftp.domain.tld/path1/path2");
}

#include "screenshotplugin.moc"
