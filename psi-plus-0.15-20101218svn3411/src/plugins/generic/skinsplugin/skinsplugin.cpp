/*
 * skinsplugin.cpp - plugin
 * Copyright (C) 2010  Khryukin Evgeny
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

#include <QFileDialog>
#include <QMessageBox>

#include "psiplugin.h"
#include "applicationinfoaccessor.h"
#include "applicationinfoaccessinghost.h"
#include "optionaccessor.h"
#include "optionaccessinghost.h"
#include "plugininfoprovider.h"

#include "ui_skinsplugin.h"
#include "skin.h"
#include "optionsparser.h"


#define cVer "0.3.0"


class SkinsPlugin: public QObject, public PsiPlugin, public ApplicationInfoAccessor, public OptionAccessor, public PluginInfoProvider
{
	Q_OBJECT
	Q_INTERFACES(PsiPlugin OptionAccessor ApplicationInfoAccessor PluginInfoProvider)

public:
        SkinsPlugin();
	virtual QString name() const;
	virtual QString shortName() const;
	virtual QString version() const;
        virtual QWidget* options();
	virtual bool enable();
        virtual bool disable();
        virtual void applyOptions() {};
        virtual void restoreOptions();
        virtual void setApplicationInfoAccessingHost(ApplicationInfoAccessingHost* host);
        virtual void setOptionAccessingHost(OptionAccessingHost* host);
        virtual void optionChanged(const QString& /*option*/) {};
	virtual QString pluginInfo();

private:
        bool enabled;
        OptionAccessingHost* psiOptions; 
        ApplicationInfoAccessingHost *appInfo;
	Ui::SkinsPlugin ui_;
        QStringList skins_;
        void findSkins(QString dir);
	QDomDocument createSkinDocument(QDomElement sampleDoc, QString name = "",
					QString author = "", QString version = "", QString path = "");
        bool validateOption(QString option);
        void appendSkin(QString fileName);
	QPointer<QWidget> optionsWidget;


private slots:
        void updateSkins();
        void loadPreview();
        void updateButtonPressed();
        void openButtonPressed();
        void enableButton();
        void createSkin(QString name, QString author, QString version);
        void getSkinName();
        void applySkin();
        void removeSkin();
        void overwrite();

    };

Q_EXPORT_PLUGIN(SkinsPlugin);

SkinsPlugin::SkinsPlugin() {
        enabled = false;        
        appInfo = 0;
        psiOptions = 0;
        QTextCodec *codec = QTextCodec::codecForName("UTF-8");
        QTextCodec::setCodecForLocale(codec);

    }

QString SkinsPlugin::name() const {
        return "Skins Plugin";
    }

QString SkinsPlugin::shortName() const {
        return "skins";
}

QString SkinsPlugin::version() const {
        return cVer;
}

bool SkinsPlugin::enable() {
    if(psiOptions) {
        enabled = true;
    }
    return enabled;
}

bool SkinsPlugin::disable() {
	if(optionsWidget){
		delete(ui_.lw_skins);
	}
        enabled = false;
	return true;
}

QWidget* SkinsPlugin::options() {
    if(!enabled) {
        return 0;
    }

    optionsWidget = new QWidget();

    ui_.setupUi(optionsWidget);
    skins_.clear();
    updateSkins();

    ui_.pb_apply->setEnabled(false);
    ui_.pb_remove->setEnabled(false);
    ui_.pb_save->setEnabled(false);

    ui_.lbl_wiki->setText(tr("<a href=\"http://psi-plus.com/wiki/plugins#skins_plugin\">Wiki (Online)</a>"));
    ui_.lbl_wiki->setOpenExternalLinks(true);

    connect(ui_.pb_update, SIGNAL(released()), this, SLOT(updateSkins()));
    connect(ui_.pb_preview, SIGNAL(released()), this, SLOT(loadPreview()));
    connect(ui_.pb_update, SIGNAL(released()), this, SLOT(updateButtonPressed()));
    connect(ui_.pb_open, SIGNAL(released()), this, SLOT(openButtonPressed()));
    connect(ui_.pb_apply, SIGNAL(released()), this, SLOT(applySkin()));
    connect(ui_.pb_create, SIGNAL(released()), this, SLOT(getSkinName()));
    connect(ui_.pb_remove, SIGNAL(released()), this, SLOT(removeSkin()));
    connect(ui_.pb_save, SIGNAL(released()), this, SLOT(overwrite()));
    connect(ui_.lw_skins, SIGNAL(currentRowChanged(int)), this, SLOT(enableButton()));
    connect(ui_.lw_skins, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(loadPreview()));

    ui_.cb_hack->setVisible(false); //Hack, to enable Apply button

    return optionsWidget;
}

void SkinsPlugin::restoreOptions() {

}

void SkinsPlugin::setApplicationInfoAccessingHost(ApplicationInfoAccessingHost* host) {
     appInfo = host;
 }

void SkinsPlugin::setOptionAccessingHost(OptionAccessingHost* host) {
    psiOptions = host;
}

void SkinsPlugin::updateSkins() {
    QStringList dirs;
    dirs << appInfo->appHomeDir()
         << appInfo->appResourcesDir() + "/skins"
         << appInfo->appHomeDir() + "/skins";

    foreach(QString dirName, dirs) {
        findSkins(dirName);
    }
}

void SkinsPlugin::findSkins(QString path) {
    if(!ui_.lw_skins) return;

    QDir dir(path);
    foreach (QString filename, dir.entryList(QDir::Files)) {
        if(filename.endsWith(".skn", Qt::CaseInsensitive)) {
            QString file = dir.absolutePath() + QString("/") + filename;
            if(skins_.contains(file))
                continue;

            skins_.append(file);
            Skin *newItem = new Skin(ui_.lw_skins);
            newItem->setFile(file);
            newItem->setText(newItem->name());
        }
    }

    foreach (QString subDir, dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
        findSkins(path + QDir::separator() + subDir);
    }    
}

void SkinsPlugin::updateButtonPressed() {
    if(!ui_.lw_skins) return;

    updateSkins();

   // ui_.pb_apply->setEnabled(false);
}

void SkinsPlugin::loadPreview()
{
    Skin *skin = (Skin*)ui_.lw_skins->currentItem();
    if(!skin)
        return;

    Previewer *prev = new Previewer(skin);
    if(prev->loadSkinInformation()) {
        connect(prev, SIGNAL(applySkin()), this, SLOT(applySkin()));
        prev->show();
    } else
        delete(prev);
}

void SkinsPlugin::openButtonPressed() {
    QString fileName = QFileDialog::getOpenFileName(0,tr("Choose a skin file"), appInfo->appHomeDir(), tr("*.skn"));
    if(fileName.isEmpty()) return;

    if(skins_.contains(fileName))
        return;

    appendSkin(fileName);
}

void SkinsPlugin::appendSkin(QString fileName) {
    if(!ui_.lw_skins) return;

    skins_.append(fileName);
    Skin *newItem = new Skin(ui_.lw_skins);
    newItem->setFile(fileName);
    newItem->setText(newItem->name());
}

void SkinsPlugin::enableButton() {
    ui_.pb_apply->setEnabled(true);
    ui_.pb_remove->setEnabled(true);
    ui_.pb_save->setEnabled(true);
}

void SkinsPlugin::getSkinName() {
    QString name, author, version;
    Skin *skin = (Skin*)ui_.lw_skins->currentItem();
    if(skin) {
	QFile file(skin->filePass());
        QDomDocument doc;
	if(doc.setContent(&file)) {
            QDomElement elem = doc.documentElement();
            if(elem.tagName() == "skin") {
                author = elem.attribute("author");
                version = elem.attribute("version");
                name = elem.attribute("name");
            }
        }
    }
    GetSkinName *getName = new GetSkinName(name, author, version);
    connect(getName, SIGNAL(ok(QString,QString,QString)), this, SLOT(createSkin(QString,QString,QString)));
    getName->show();
}

void SkinsPlugin::createSkin(QString name, QString author, QString version) {
    QFile file(":/skinsplugin/defskin.skn");
    QDomDocument doc, newDoc;
    if(!doc.setContent(&file)) {
        QMessageBox::warning(0, tr("Create Skin"), tr("Unknown error!"));
        return;
    }
    QDomElement elem = doc.documentElement();
    if(elem.tagName() != "skin") {
        QMessageBox::warning(0, tr("Create Skin"), tr("Unknown error!"));
        return;
    }

    QString fileName = QFileDialog::getSaveFileName(0,tr("Save a skin file"), appInfo->appHomeDir() + QString("/%1_%2").arg(name, version), tr("*.skn"));
    if(fileName.isEmpty())
        return;
    if(fileName.right(4) != ".skn")
        fileName.append(".skn");

    QString skinPath = fileName;
    skinPath.chop(skinPath.size() - skinPath.lastIndexOf("/"));
    newDoc = createSkinDocument(elem, name, author, version, skinPath);

    QFile saveFile(fileName);

    if(saveFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        QTextStream str(&saveFile);
        int indent = 4;
        str.setGenerateByteOrderMark(false);
        newDoc.save(str, indent);
        if(!skins_.contains(fileName))
            appendSkin(fileName);
    } else {
        QMessageBox::warning(0, tr("Create Skin"), tr("Can't save skin!"));
    }
}

void SkinsPlugin::applySkin() {
    Skin *skin = (Skin*)ui_.lw_skins->currentItem();
    if(!skin)
        return;

    QFile file(skin->filePass());
    QDomDocument doc;
    if(!doc.setContent(&file)) {
        QMessageBox::warning(0, tr("Apply Skin"), tr("Unknown error!"));
        return;
    }
    QDomElement elem = doc.documentElement();
    if(elem.tagName() != "skin") {
        QMessageBox::warning(0, tr("Apply Skin"), tr("Unknown error!"));
        return;
    }

    bool backup = ui_.cb_backup->isChecked();
    QString fileName;
    if(backup && skin->name().left(11) != "backupSkin_") {
        QDomDocument backUp = createSkinDocument(elem, "backup", "SkinsPlugin", "0");

        QDir skinsDir(appInfo->appHomeDir() + QString("/skins"));
        if(!skinsDir.exists())
            skinsDir.mkdir(appInfo->appHomeDir() + QString("/skins"));

        fileName = skinsDir.absolutePath() + "/backupSkin_" + QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss") + ".skn";
        QFile backUpFile(fileName);
	if(backUpFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            QTextStream str(&backUpFile);
            int indent = 4;
            str.setGenerateByteOrderMark(false);
            backUp.save(str, indent);
            appendSkin(fileName);
        } else {
            QMessageBox::warning(0, tr("Apply Skin"), tr("Can't save the backup skin!"));
            return;
        }
    }

    QDomElement options = elem.firstChildElement("options");
    QDomNode optionNode = options.firstChild();
    while(!optionNode.isNull()) {
        QDomElement optionElem = optionNode.toElement();
	QString oldPath = elem.attribute("path");
        QString optionName = optionElem.tagName();
        if(validateOption(optionName)) {
            QVariant optionValue = OptionsParser::instance()->elementToVariant(optionElem);
	    if(!oldPath.isEmpty() && optionValue.type() == QVariant::String) {
		    QString str = optionValue.toString();
		    str.replace(oldPath, skin->skinFolder());
		    optionValue = str;
	    }
            psiOptions->setGlobalOption(optionName, optionValue);
        }
        optionNode = optionNode.nextSibling();
    }
    QString mes = QString("Skin %1 successfully applied!\n"
                  "Some changes may only have full effect upon restart!").arg(elem.attribute("name"));
    if(backup) {
        mes += QString("\nBakup skin saved to %2").arg(fileName);
    }

    QMessageBox::information(0, tr("Apply Skin"), mes);

    ui_.cb_hack->toggle(); //enable Apply button
}

QDomDocument SkinsPlugin::createSkinDocument(QDomElement elem, QString name, QString author, QString version, QString path)
{
    QDomDocument newDoc;

    QDomElement newElem = newDoc.createElement("skin");
    newElem.setAttribute("name", name);
    newElem.setAttribute("author", author);
    newElem.setAttribute("version", version);
    newElem.setAttribute("path", path);
    QDomElement newOptions = newDoc.createElement("options");

    QDomElement options = elem.firstChildElement("options");
    QDomNode optionNode = options.firstChild();
    while(!optionNode.isNull()) {
        QString optionName = optionNode.toElement().tagName();
        if(validateOption(optionName)) {
            QVariant optionValue = psiOptions->getGlobalOption(optionName);
            QDomElement newOption = newDoc.createElement(optionName);
            OptionsParser::instance()->variantToElement(optionValue, newOption);
            newOptions.appendChild(newOption);
        }
        optionNode = optionNode.nextSibling();
    }

    newElem.appendChild(newOptions);
    newDoc.appendChild(newElem);

    return newDoc;
}

bool SkinsPlugin::validateOption(QString optionName)
{
    bool b =(optionName.contains("options.ui.") || optionName.contains("options.iconsets.")) &&
            !optionName.contains("notifications.send-receipts") &&
            !optionName.contains("spell-check.enabled") &&
            !optionName.contains("service-discovery");

    return b;
}

void SkinsPlugin::overwrite() {
    int ret = QMessageBox::question(0, tr("Overwrite selected skin"),
                                tr("Are You Sure?"),
                                QMessageBox::Ok  | QMessageBox::Cancel);
    if(ret == QMessageBox::Cancel) return;

    Skin *skin = (Skin*)ui_.lw_skins->currentItem();
    if(!skin)
        return;

    QFile file(skin->filePass());
    QDomDocument doc;
    if(!doc.setContent(&file)) {
        QMessageBox::warning(0, tr("Overwrite Skin"), tr("Unknown error!"));
        return;
    }
    QDomElement elem = doc.documentElement();
    if(elem.tagName() != "skin") {
        QMessageBox::warning(0, tr("Overwrite Skin"), tr("Unknown error!"));
        return;
    }

    QDomDocument overwriteDoc = createSkinDocument(elem, elem.attribute("name"), elem.attribute("author"),
					     elem.attribute("version"), skin->skinFolder());
    if(file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
	    QTextStream str(&file);
            int indent = 4;
            str.setGenerateByteOrderMark(false);
	    overwriteDoc.save(str, indent);
        } else {
            QMessageBox::warning(0, tr("Overwrite Skin"), tr("Can't save the skin!"));
            return;
        }
}

void SkinsPlugin::removeSkin() {
    Skin *skin = (Skin*)ui_.lw_skins->currentItem();
    if(!skin)
        return;

    int ret = QMessageBox::question(0, tr("Delete skin"),
                                tr("Are You Sure?"),
                                QMessageBox::Ok  | QMessageBox::Cancel);
    if(ret == QMessageBox::Cancel) return;

    QString filePass = skin->filePass();
    QFile file(filePass);
    if(file.open(QIODevice::ReadWrite)) {
        ui_.lw_skins->removeItemWidget(skin);
        delete(skin);
        file.remove();
        skins_.removeAt(skins_.indexOf(filePass));
    }
}

QString SkinsPlugin::pluginInfo() {
	return tr("Author: ") +  "Dealer_WeARE\n"
			+ tr("Email: ") + "wadealer@gmail.com\n\n"
			+ trUtf8("This plugin is designed to create, store and apply skins to Psi+.\n"
			 "Skin - a set of custom settings.\n"
			 "To download a new skin, create a folder named skins in the PsiData directory and put the new skin in it. You can also just open a skin file.\n"
			 "Each skin must be in a separate directory. You can also add a screenshot to the skin file.\n"
			 "In most cases, to be sure that the skin is applied correctly, you must perform a sequence of actions:\n"
			  "1. Apply the skin\n"
			   "2. Restart the application\n"
			   "3. Apply the same skin again\n"
			 "This allows all settings (icons, toolbar layout) to be picked up correctly. ");
}

#include "skinsplugin.moc"
