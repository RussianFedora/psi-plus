#include "opt_application.h"
#include "common.h"
#include "iconwidget.h"
#include "psioptions.h"
#include "proxy.h"
#include "translationmanager.h"
#include "varlist.h"
#include "applicationinfo.h"

#include <QCheckBox>
#include <QComboBox>
#include <QLineEdit>
#include "qsettings.h"
#include <QList>
#include <QMessageBox>

#include "ui_opt_application.h"

class OptApplicationUI : public QWidget, public Ui::OptApplication
{
public:
	OptApplicationUI() : QWidget() { setupUi(this); }
};

//----------------------------------------------------------------------------
// OptionsTabApplication
//----------------------------------------------------------------------------

OptionsTabApplication::OptionsTabApplication(QObject *parent)
: OptionsTab(parent, "application", "", tr("Application"), tr("General application options"), "psi/logo_16")
{
	w = 0;
}

OptionsTabApplication::~OptionsTabApplication()
{
}

QWidget *OptionsTabApplication::widget()
{
	if ( w )
		return 0;

	w = new OptApplicationUI();
	OptApplicationUI *d = (OptApplicationUI *)w;

	d->ck_keepSizes->setWhatsThis(
		tr("Makes Psi remember window size and positions for chats and messages."
		"  If disabled, the windows will always appear in their default positions and sizes."));
	d->ck_winDecor->setWhatsThis(
		tr("Set or remove window decorations for roster and chats."
		" For chats there are special header with close, hide and maximize buttons"));
	// docklet
	d->ck_docklet->setWhatsThis(
		tr("Makes Psi use a docklet icon, also known as system tray icon."));
	d->ck_dockDCstyle->setWhatsThis(
		tr("Normally, single-clicking on the Psi docklet icon brings the main window to"
		" the foreground.  Check this option if you would rather use a double-click."));
	d->ck_dockHideMW->setWhatsThis(
		tr("Starts Psi with only the docklet icon visible."));
	d->ck_dockToolMW->setWhatsThis(
		tr("Prevents Psi from taking up a slot on the taskbar and makes the main "
		"window use a small titlebar."));

#ifdef Q_WS_MAC
	d->gb_docklet->hide();
#endif

	if (!haveAutoUpdater_) {
		d->ck_autoUpdate->hide();
	}

	//Proxy

	ProxyChooser *pc = ProxyManager::instance()->createProxyChooser(w);
	d->gb_proxy->layout()->addWidget(ProxyManager::instance()->proxyForObject()->getComboBox(pc, w));
	d->gb_proxy->layout()->addWidget(pc);


	connect(d->le_dtPort, SIGNAL(textChanged(QString)), this, SLOT(updatePortLabel()));
	connect(d->ck_docklet, SIGNAL(stateChanged(int)), this, SLOT(doEnableQuitOnClose(int)));

	return w;
}

void OptionsTabApplication::setHaveAutoUpdater(bool b) 
{
	haveAutoUpdater_ = b;
}

void OptionsTabApplication::applyOptions()
{
	if ( !w )
		return;

	OptApplicationUI *d = (OptApplicationUI *)w;

	PsiOptions::instance()->setOption("options.ui.remember-window-sizes",d->ck_keepSizes->isChecked());
	PsiOptions::instance()->setOption("options.ui.contactlist.quit-on-close", d->ck_quitOnClose->isChecked());
	PsiOptions::instance()->setOption("options.ui.decorate-windows", d->ck_winDecor->isChecked());

	// Auto-update
	PsiOptions::instance()->setOption("options.auto-update.check-on-startup", d->ck_autoUpdate->isChecked());
	
	// docklet
	PsiOptions::instance()->setOption("options.ui.systemtray.enable", d->ck_docklet->isChecked());
	PsiOptions::instance()->setOption("options.ui.systemtray.use-double-click", d->ck_dockDCstyle->isChecked());
	PsiOptions::instance()->setOption("options.contactlist.hide-on-start", d->ck_dockHideMW->isChecked());
	PsiOptions::instance()->setOption("options.contactlist.use-toolwindow", d->ck_dockToolMW->isChecked());

	// data transfer
	PsiOptions::instance()->setOption("options.p2p.bytestreams.listen-port", d->le_dtPort->text().toInt());
	PsiOptions::instance()->setOption("options.p2p.bytestreams.external-address", d->le_dtExternal->text().trimmed());

	//Proxy
	ProxyManager::instance()->proxyForObject()->save();

	// Language
	QString curLang = TranslationManager::instance()->currentLanguage();
	QString lang = d->cb_lang->currentText();
	QString itemData;
	foreach(VarListItem it, TranslationManager::instance()->availableTranslations() ) {
		if(it.data() == lang) {
			itemData = it.key();
			break;
		}
	}
	if(curLang != itemData && !itemData.isEmpty()) {
		TranslationManager::instance()->loadTranslation(itemData);
		QMessageBox::information(0, tr("Information"), tr("Some of the options you changed will only have full effect upon restart."));
	}
	QSettings s(ApplicationInfo::homeDir() + "/psirc", QSettings::IniFormat);
	s.setValue("last_lang", itemData);
}

void OptionsTabApplication::restoreOptions()
{
	if ( !w )
		return;

	OptApplicationUI *d = (OptApplicationUI *)w;

	d->ck_keepSizes->setChecked( PsiOptions::instance()->getOption("options.ui.remember-window-sizes").toBool() );
	d->ck_autoUpdate->setChecked(PsiOptions::instance()->getOption("options.auto-update.check-on-startup").toBool());
	d->ck_quitOnClose->setChecked(PsiOptions::instance()->getOption("options.ui.contactlist.quit-on-close").toBool());
	d->ck_winDecor->setChecked(PsiOptions::instance()->getOption("options.ui.decorate-windows").toBool());

	// docklet
	d->ck_docklet->setChecked( PsiOptions::instance()->getOption("options.ui.systemtray.enable").toBool() );
	d->ck_dockDCstyle->setChecked( PsiOptions::instance()->getOption("options.ui.systemtray.use-double-click").toBool() );
	d->ck_dockHideMW->setChecked( PsiOptions::instance()->getOption("options.contactlist.hide-on-start").toBool() );
	d->ck_dockToolMW->setChecked( PsiOptions::instance()->getOption("options.contactlist.use-toolwindow").toBool() );
	doEnableQuitOnClose(d->ck_docklet->isChecked()?1:0);

	// data transfer
	d->le_dtPort->setText( QString::number(PsiOptions::instance()->getOption("options.p2p.bytestreams.listen-port").toInt()) );
	d->le_dtExternal->setText( PsiOptions::instance()->getOption("options.p2p.bytestreams.external-address").toString() );

	// Language
	VarList vList = TranslationManager::instance()->availableTranslations();
	QStringList lang = vList.varsToStringList();
	d->cb_lang->addItem(tr("Default"));
	foreach(QString item, lang) {
		d->cb_lang->addItem(vList.get(item));
	}
	QString curLang = TranslationManager::instance()->currentLanguage();
	QSettings s(ApplicationInfo::homeDir() + "/psirc", QSettings::IniFormat);
	QString curL = s.value("last_lang", "").toString();
	if (curL.isEmpty())
		d->cb_lang->setCurrentIndex( 0 );
	else if(!curLang.isEmpty() && lang.contains(curLang) )
		d->cb_lang->setCurrentIndex( d->cb_lang->findText(vList.get(curLang)) );
}

void OptionsTabApplication::doEnableQuitOnClose(int state)
{
	OptApplicationUI *d = (OptApplicationUI *)w;
	d->ck_quitOnClose->setEnabled(state>0);
}

void OptionsTabApplication::updatePortLabel()
{
	if ( !w )
		return;

	OptApplicationUI *d = (OptApplicationUI *)w;

	if ( d->le_dtPort->text().isEmpty() ) {
		d->label->clear();
		return;
	}

	int port = d->le_dtPort->text().toInt();
	if ( port < 0 || port > 65532 ) {
		d->label->clear();
		return;
	}

	if ( port == 0 ) {
		d->label->setText(tr("(TCP: Disabled, UDP: Auto)"));
	}
	else {
		d->label->setText(tr("(TCP: %1, UDP: %1-%2)").arg( port ).arg( port + 3 ));
	}
}
