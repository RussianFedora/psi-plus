/*
 * translateplugin.cpp - plugin
 * Copyright (C) 2009-2010  Kravtsov Nikolai
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

#include <QtGui>
#include <QApplication>
#include <QMap>
#include "psiplugin.h"
#include "shortcutaccessor.h"
#include "shortcutaccessinghost.h"
#include "optionaccessor.h"
#include "optionaccessinghost.h"
#include "activetabaccessor.h"
#include "activetabaccessinghost.h"
#include "plugininfoprovider.h"

#define constOld "oldsymbol"
#define constNew "newsymbol"
#define constShortCut "shortcut"
#define constNotTranslate "nottranslate"
#define constVersion "0.3.1"

class TranslatePlugin : public QObject, public PsiPlugin, public OptionAccessor, public ShortcutAccessor, public ActiveTabAccessor, public PluginInfoProvider
{
        Q_OBJECT
	Q_INTERFACES(PsiPlugin OptionAccessor ShortcutAccessor ActiveTabAccessor PluginInfoProvider)

public:
        TranslatePlugin();

        virtual QString name() const;
        virtual QString shortName() const;
        virtual QString version() const;
        virtual QWidget* options();
        virtual bool enable();
        virtual bool disable();

        virtual void applyOptions();
        virtual void restoreOptions();

        // OptionAccessor
        virtual void setOptionAccessingHost(OptionAccessingHost* host);
        virtual void optionChanged(const QString& option);
	
	// ShortcutsAccessor
	virtual void setShortcutAccessingHost(ShortcutAccessingHost* host);
	virtual void setShortcuts();
        //ActiveTabAccessor
        virtual void setActiveTabAccessingHost(ActiveTabAccessingHost* host);

	virtual QString pluginInfo();

private slots:
        void trans();
        void addToMap();
        void del();
        void grep();
        void onNewShortcutKey(QKeySequence);
        void changeItem(int,int);
        void storeItem(QTableWidgetItem*);
        void restoreMap();
private:
        bool enabled_;
        bool notTranslate;
        QMap<QString,QString> *map;
        QMap<QString,QString> *mapBakup;
        QTableWidget * table;
        QLineEdit *shortCutWidget;
        OptionAccessingHost* psiOptions;
	ShortcutAccessingHost* psiShortcuts;
        ActiveTabAccessingHost* activeTab;
        QString shortCut;
        QCheckBox *check_button;
        QString storage;

};

Q_EXPORT_PLUGIN(TranslatePlugin);

TranslatePlugin::TranslatePlugin()
{
        shortCut = "Alt+Ctrl+t";
        enabled_ = false;
        notTranslate = false;
        table = 0;
        psiOptions = 0;
        shortCutWidget = 0;
        check_button = 0;
        activeTab = 0;
        map = new QMap<QString, QString>();
        map->insert("~",QString::fromUtf8("Ё")); map->insert(QString::fromUtf8("Ё"),"~");
        map->insert("`",QString::fromUtf8("ё")); map->insert(QString::fromUtf8("ё"),"`");
        map->insert("#",QString::fromUtf8("№")); map->insert(QString::fromUtf8("№"),"#");
        map->insert("q",QString::fromUtf8("й")); map->insert(QString::fromUtf8("й"),"q");
        map->insert("w",QString::fromUtf8("ц")); map->insert(QString::fromUtf8("ц"),"w");
        map->insert("e",QString::fromUtf8("у")); map->insert(QString::fromUtf8("у"),"e");
        map->insert("r",QString::fromUtf8("к")); map->insert(QString::fromUtf8("к"),"r");
        map->insert("t",QString::fromUtf8("е")); map->insert(QString::fromUtf8("е"),"t");
        map->insert("y",QString::fromUtf8("н")); map->insert(QString::fromUtf8("н"),"y");
        map->insert("u",QString::fromUtf8("г")); map->insert(QString::fromUtf8("г"),"u");
        map->insert("i",QString::fromUtf8("ш")); map->insert(QString::fromUtf8("ш"),"i");
        map->insert("o",QString::fromUtf8("щ")); map->insert(QString::fromUtf8("щ"),"o");
        map->insert("p",QString::fromUtf8("з")); map->insert(QString::fromUtf8("з"),"p");
        map->insert("a",QString::fromUtf8("ф")); map->insert(QString::fromUtf8("ф"),"a");
        map->insert("s",QString::fromUtf8("ы")); map->insert(QString::fromUtf8("ы"),"s");
        map->insert("d",QString::fromUtf8("в")); map->insert(QString::fromUtf8("в"),"d");
        map->insert("f",QString::fromUtf8("а")); map->insert(QString::fromUtf8("а"),"f");
        map->insert("g",QString::fromUtf8("п")); map->insert(QString::fromUtf8("п"),"g");
        map->insert("h",QString::fromUtf8("р")); map->insert(QString::fromUtf8("р"),"h");
        map->insert("j",QString::fromUtf8("о")); map->insert(QString::fromUtf8("о"),"j");
        map->insert("k",QString::fromUtf8("л")); map->insert(QString::fromUtf8("л"),"k");
        map->insert("l",QString::fromUtf8("д")); map->insert(QString::fromUtf8("д"),"l");
        map->insert("z",QString::fromUtf8("я")); map->insert(QString::fromUtf8("я"),"z");
        map->insert("x",QString::fromUtf8("ч")); map->insert(QString::fromUtf8("ч"),"x");
        map->insert("c",QString::fromUtf8("с")); map->insert(QString::fromUtf8("с"),"c");
        map->insert("v",QString::fromUtf8("м")); map->insert(QString::fromUtf8("м"),"v");
        map->insert("b",QString::fromUtf8("и")); map->insert(QString::fromUtf8("и"),"b");
        map->insert("n",QString::fromUtf8("т")); map->insert(QString::fromUtf8("т"),"n");
        map->insert("m",QString::fromUtf8("ь")); map->insert(QString::fromUtf8("ь"),"m");
        map->insert("Q",QString::fromUtf8("Й")); map->insert(QString::fromUtf8("Й"),"Q");
        map->insert("W",QString::fromUtf8("Ц")); map->insert(QString::fromUtf8("Ц"),"W");
        map->insert("E",QString::fromUtf8("У")); map->insert(QString::fromUtf8("У"),"E");
        map->insert("R",QString::fromUtf8("К")); map->insert(QString::fromUtf8("К"),"R");
        map->insert("T",QString::fromUtf8("Е")); map->insert(QString::fromUtf8("Е"),"T");
        map->insert("Y",QString::fromUtf8("Н")); map->insert(QString::fromUtf8("Н"),"Y");
        map->insert("U",QString::fromUtf8("Г")); map->insert(QString::fromUtf8("Г"),"U");
        map->insert("I",QString::fromUtf8("Ш")); map->insert(QString::fromUtf8("Ш"),"I");
        map->insert("O",QString::fromUtf8("Щ")); map->insert(QString::fromUtf8("Щ"),"O");
        map->insert("P",QString::fromUtf8("З")); map->insert(QString::fromUtf8("З"),"P");
        map->insert("A",QString::fromUtf8("Ф")); map->insert(QString::fromUtf8("Ф"),"A");
        map->insert("S",QString::fromUtf8("Ы")); map->insert(QString::fromUtf8("Ы"),"S");
        map->insert("D",QString::fromUtf8("В")); map->insert(QString::fromUtf8("В"),"D");
        map->insert("F",QString::fromUtf8("А")); map->insert(QString::fromUtf8("А"),"F");
        map->insert("G",QString::fromUtf8("П")); map->insert(QString::fromUtf8("П"),"G");
        map->insert("H",QString::fromUtf8("Р")); map->insert(QString::fromUtf8("Р"),"H");
        map->insert("J",QString::fromUtf8("О")); map->insert(QString::fromUtf8("О"),"J");
        map->insert("K",QString::fromUtf8("Л")); map->insert(QString::fromUtf8("Л"),"K");
        map->insert("L",QString::fromUtf8("Д")); map->insert(QString::fromUtf8("Д"),"L");
        map->insert("Z",QString::fromUtf8("Я")); map->insert(QString::fromUtf8("Я"),"Z");
        map->insert("X",QString::fromUtf8("Ч")); map->insert(QString::fromUtf8("Ч"),"X");
        map->insert("C",QString::fromUtf8("С")); map->insert(QString::fromUtf8("С"),"C");
        map->insert("V",QString::fromUtf8("М")); map->insert(QString::fromUtf8("М"),"V");
        map->insert("B",QString::fromUtf8("И")); map->insert(QString::fromUtf8("И"),"B");
        map->insert("N",QString::fromUtf8("Т")); map->insert(QString::fromUtf8("Т"),"N");
        map->insert("M",QString::fromUtf8("Ь")); map->insert(QString::fromUtf8("Ь"),"M");
        map->insert("[",QString::fromUtf8("х")); map->insert(QString::fromUtf8("х"),"[");
        map->insert("{",QString::fromUtf8("Х")); map->insert(QString::fromUtf8("Х"),"{");
        map->insert("]",QString::fromUtf8("ъ")); map->insert(QString::fromUtf8("ъ"),"]");
        map->insert("}",QString::fromUtf8("Ъ")); map->insert(QString::fromUtf8("Ъ"),"}");
        map->insert("$",QString::fromUtf8(";")); map->insert(QString::fromUtf8(";"),"$");
        map->insert(";",QString::fromUtf8("ж")); map->insert(QString::fromUtf8("ж"),";");
        map->insert("^",QString::fromUtf8(":")); map->insert(QString::fromUtf8(":"),"^");
        map->insert(":",QString::fromUtf8("Ж")); map->insert(QString::fromUtf8("Ж"),":");
        map->insert("'",QString::fromUtf8("э")); map->insert(QString::fromUtf8("э"),"'");
        map->insert("@",QString::fromUtf8("\"")); map->insert(QString::fromUtf8("\""),"@");
        map->insert("\"",QString::fromUtf8("Э")); map->insert(QString::fromUtf8("Э"),"\"");
        map->insert("<",QString::fromUtf8("Б")); map->insert(QString::fromUtf8("Б"),"<");
        map->insert(">",QString::fromUtf8("Ю")); map->insert(QString::fromUtf8("Ю"),">");
        map->insert("&",QString::fromUtf8("?")); map->insert(QString::fromUtf8("?"),"&");
        map->insert("?",QString::fromUtf8(",")); map->insert(QString::fromUtf8(","),"?");
        map->insert(",",QString::fromUtf8("б")); map->insert(QString::fromUtf8("б"),",");
        map->insert("|",QString::fromUtf8("/")); map->insert(QString::fromUtf8("/"),"|");
        map->insert("/",QString::fromUtf8(".")); map->insert(QString::fromUtf8("."),"/");
        map->insert(".",QString::fromUtf8("ю")); map->insert(QString::fromUtf8("ю"),".");
        mapBakup = new QMap<QString,QString>(*map);
}

QString TranslatePlugin::name() const
{
        return "Translate Plugin";
}

QString TranslatePlugin::shortName() const
{
        return "Translate";
}

QString TranslatePlugin::version() const
{
        return constVersion;
}

QWidget* TranslatePlugin::options()
{
        if (!enabled_) {
                return 0;
        }
        QWidget *optionsWid = new QWidget();
        table = new QTableWidget(optionsWid);
        table->setColumnCount(2);
        QStringList header;
        header <<tr("from")<<tr("to");
        table->setHorizontalHeaderLabels(header);
        table->verticalHeader()->setVisible(false);
        table->setTextElideMode(Qt::ElideMiddle);
        table->setSelectionBehavior(QAbstractItemView::SelectRows);
        table->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        table->setEditTriggers(QAbstractItemView::DoubleClicked);
        table->verticalHeader()->setDefaultSectionSize(20);
        table->verticalHeader()->setMinimumSectionSize(20);
        table->horizontalHeader()->setDefaultSectionSize(50);
        table->horizontalHeader()->setMinimumSectionSize(20);
        table->setColumnWidth(0,50);
        table->setColumnWidth(1,50);
        table->setMaximumWidth(120);
        QHBoxLayout *hBox = new QHBoxLayout(optionsWid);
        QVBoxLayout *leftSide = new QVBoxLayout;
        leftSide->addWidget(table);
        QHBoxLayout *buttonLayout = new QHBoxLayout;
        QPushButton *addButton = new QPushButton(tr("Add"), optionsWid);
        QPushButton *delButton = new QPushButton(tr("Delete"), optionsWid);
        buttonLayout->addWidget(addButton);
        buttonLayout->addWidget(delButton);
        leftSide->addLayout(buttonLayout);
        hBox->addLayout(leftSide);
        QVBoxLayout *rightSide = new QVBoxLayout;
        rightSide->addWidget(new QLabel(tr("ShortCut:")),5,Qt::AlignTop);
        QHBoxLayout *shortBox = new QHBoxLayout;
        shortCutWidget = new QLineEdit(optionsWid);
        shortCutWidget->setFixedWidth(100);
        shortCutWidget->setText(shortCut);
        shortCutWidget->setDisabled(true);
        QPushButton * modShortCut = new QPushButton(tr("Modify"), optionsWid);
        shortBox->addWidget(shortCutWidget,0,Qt::AlignLeft);
        shortBox->addWidget(modShortCut,200,Qt::AlignLeft);
        rightSide->addLayout(shortBox,30);
        check_button = new QCheckBox(tr("Not translating \"Nickname:\""), optionsWid);
        check_button->setChecked(notTranslate);
        check_button->setProperty("isOption",true);
        rightSide->addWidget(check_button,30,Qt::AlignTop);
        QPushButton *restoreButton = new QPushButton(tr("Restore Defaults Settings"), optionsWid);
        restoreButton->setFixedWidth(220);
        rightSide->addWidget(restoreButton,30,Qt::AlignBottom);
        if (!map->isEmpty()) {
            foreach(QString symbol, map->keys()){
                table->insertRow(table->rowCount());
                table->setItem(table->rowCount()-1,0,new QTableWidgetItem(symbol));
                table->setItem(table->rowCount()-1,1,new QTableWidgetItem(map->value(symbol)));
            }
        }
        hBox->addLayout(rightSide);
        connect(delButton,SIGNAL(clicked()),this,SLOT(del()));
        connect(addButton,SIGNAL(clicked()),this,SLOT(addToMap()));
        connect(modShortCut,SIGNAL(clicked()),this,SLOT(grep()));
        connect(restoreButton,SIGNAL(clicked()),this,SLOT(restoreMap()));
        connect(table,SIGNAL(cellChanged(int,int)),this,SLOT(changeItem(int,int)));
        connect(table,SIGNAL(itemDoubleClicked(QTableWidgetItem*)),this,SLOT(storeItem(QTableWidgetItem*)));
        return optionsWid;
}

bool TranslatePlugin::enable()
{
        enabled_ = true;

        QVariant vShortCut(shortCut);
        vShortCut = psiOptions->getPluginOption(constShortCut);
        if (!vShortCut.isNull()) {
            shortCut = vShortCut.toString();
        }
        QVariant vNotTraslate(notTranslate);
        vNotTraslate = psiOptions->getPluginOption(constNotTranslate);
        if (!vNotTraslate.isNull()) {
            notTranslate = vNotTraslate.toBool();
        }
        psiShortcuts->connectShortcut(QKeySequence(shortCut),this, SLOT(trans()));
        QVariant vOldSymbols(QStringList(map->keys()));
        QVariant vNewSymbols(QStringList(map->values()));
        vOldSymbols = psiOptions->getPluginOption(constOld);
        vNewSymbols = psiOptions->getPluginOption(constNew);
        int iterator = 0;
        if (!vOldSymbols.isNull() && !vNewSymbols.isNull()) {
            map->clear();
            foreach(QString symbol, vOldSymbols.toStringList()){
                map->insert(symbol,vNewSymbols.toStringList().at(iterator++));
            }
        }

        return true;
}

bool TranslatePlugin::disable()
{
        enabled_ = false;
        psiShortcuts->disconnectShortcut(QKeySequence(shortCut),this, SLOT(trans()));
        return true;
}

void TranslatePlugin::trans(){
    QTextEdit * ed = activeTab->getEditBox();
    if (ed == 0){
        return;
    }
    QString toReverse = ed->textCursor().selectedText();
    bool isSelect = true;
    QString nick("");
    if (toReverse == ""){
       toReverse = ed->toPlainText();
       if (notTranslate){
          int index = toReverse.indexOf(":") + 1;
          nick = toReverse.left(index);
          toReverse = toReverse.right(toReverse.size() - index);
       }
       isSelect = false;
    }
    //запоминаем позицию курсора
    int pos = ed->textCursor().position();
    QString newString = "";
    foreach(QString symbol,toReverse){
        newString.append(map->value(symbol,symbol));
    }
    if (!isSelect){
        ed->setPlainText(nick + newString);
        //восстанавливаем позицию курсора
        QTextCursor c = ed->textCursor();
        c.setPosition( pos );
        ed->setTextCursor( c );
    }else{
        int end = ed->textCursor().selectionEnd();
        int start = ed->textCursor().selectionStart();
        ed->textCursor().clearSelection();
        ed->textCursor().insertText(newString);
        QTextCursor c = ed->textCursor();
        if ( pos == start) {
            c.setPosition(end);
            c.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor, end-start);
        } else {
            c.setPosition(start);
            c.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, end-start);
        }
        ed->setTextCursor( c );
    }
}

void TranslatePlugin::addToMap(){
    if (table != 0){
        if (table->currentRow() != -1){
            table->insertRow(table->currentRow());
        } else {
            table->insertRow(0);
        }
    }
}

void TranslatePlugin::setOptionAccessingHost(OptionAccessingHost* host)
{
        psiOptions = host;
}

void TranslatePlugin::optionChanged(const QString& option)
{
        Q_UNUSED(option);
}

void TranslatePlugin::setShortcutAccessingHost(ShortcutAccessingHost* host)
{
        psiShortcuts = host;
}

void TranslatePlugin::setShortcuts()
{
	if (enabled_) {
		psiShortcuts->connectShortcut(QKeySequence(shortCut), this, SLOT(trans()));
	}
}

void TranslatePlugin::applyOptions() {
        if (table == 0 || shortCutWidget == 0 || shortCutWidget->text() == "" || check_button == 0){
            return;
        }
        QVariant vShortCut(shortCutWidget->text());
        psiOptions->setPluginOption(constShortCut, vShortCut);
        psiShortcuts->disconnectShortcut(QKeySequence(shortCut), this, SLOT(trans()));
        shortCut = vShortCut.toString();
        psiShortcuts->connectShortcut(QKeySequence(shortCut), this, SLOT(trans()));
        QVariant vNotTranslate(check_button->isChecked());
        psiOptions->setPluginOption(constNotTranslate, vNotTranslate);
        notTranslate = vNotTranslate.toBool();

        map->clear();
        int count = table->rowCount();
        for (int row = 0 ; row < count; ++row){
            if (table->item(row,0)->text() != "" && table->item(row,1)->text() != ""){
                map->insert(table->item(row,0)->text().left(1),table->item(row,1)->text());
            }
        }
        QVariant vOldSymbols(QStringList(map->keys()));
        psiOptions->setPluginOption(constOld, vOldSymbols);
        QVariant vNewSymbols(QStringList(map->values()));
        psiOptions->setPluginOption(constNew, vNewSymbols);
}

void TranslatePlugin::restoreOptions() {
        if (table == 0 || shortCutWidget == 0 || check_button == 0){
            return;
        }
        QVariant vShort(shortCut);
        vShort = psiOptions->getPluginOption(constShortCut);
        if (!vShort.isNull()) {
            shortCutWidget->setText(vShort.toString());
        }
        QVariant vNotTranslate(notTranslate);
        vNotTranslate = psiOptions->getPluginOption(constNotTranslate);
        if (!vNotTranslate.isNull()) {
            check_button->setChecked(vNotTranslate.toBool());
        }
        QVariant vOldSymbols(QStringList(map->keys()));
        QVariant vNewSymbols(QStringList(map->values()));
        vOldSymbols = psiOptions->getPluginOption(constOld);
        vNewSymbols = psiOptions->getPluginOption(constNew);
        int iterator = 0;
        if (!vOldSymbols.isNull() && !vNewSymbols.isNull()) {
            foreach(QString symbol, vOldSymbols.toStringList()){
                table->insertRow(table->rowCount());
                table->setItem(table->rowCount()-1,0,new QTableWidgetItem(symbol));
                table->setItem(table->rowCount()-1,1,new QTableWidgetItem(vNewSymbols.toStringList().at(++iterator)));
            }
        }
}


void TranslatePlugin::del(){
    if (table->currentRow() == -1){
        return;
    }
    table->removeRow(table->currentRow());
}

void TranslatePlugin::grep(){
    psiShortcuts->requestNewShortcut(this, SLOT(onNewShortcutKey(QKeySequence)));
}

void TranslatePlugin::onNewShortcutKey(QKeySequence ks){
    shortCutWidget->setText(ks.toString(QKeySequence::NativeText));
}
void TranslatePlugin::changeItem(int row,int column){
    if (column == 0 && storage != ""){
    //если первая колонка, то её менять нельзя, возвращаем старое значение
        table->item(row,column)->setText(storage);
    } else {
        //иначе приравниваем ячейке значение первого символа
        if (table->item(row,column)->text() == ""){
             table->item(row,column)->setText(storage);
        } else {
            table->item(row,column)->setText(table->item(row,column)->text().left(1));
        }
    }
}

void TranslatePlugin::storeItem(QTableWidgetItem* item){
    storage = item->text();
}

void TranslatePlugin::restoreMap(){
    disconnect(table,SIGNAL(cellChanged(int,int)),this,SLOT(changeItem(int,int)));
    table->setRowCount(0);
    foreach(QString symbol, mapBakup->keys()){
        table->insertRow(table->rowCount());
        table->setItem(table->rowCount()-1,0,new QTableWidgetItem(symbol));
        table->setItem(table->rowCount()-1,1,new QTableWidgetItem(mapBakup->value(symbol)));
    }
    connect(table,SIGNAL(cellChanged(int,int)),this,SLOT(changeItem(int,int)));
}

void TranslatePlugin::setActiveTabAccessingHost(ActiveTabAccessingHost* host){
    activeTab = host;
}

QString TranslatePlugin::pluginInfo() {
	return tr("Author: ") +  "VampiRUS\n\n"
			+ trUtf8("This plugin allows you to convert selected text into another language.\n");
}

#include "translateplugin.moc"
