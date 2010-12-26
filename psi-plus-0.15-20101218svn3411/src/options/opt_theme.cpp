#include "opt_theme.h"

#include "ui_opt_theme.h"
#include "psioptions.h"
#include "psithememodel.h"
#include "psithemeviewdelegate.h"
#include "psithememanager.h"

class OptAppearanceThemeUI : public QWidget, public Ui::OptAppearanceTheme
{
public:
	OptAppearanceThemeUI() : QWidget() { setupUi(this); }
};



OptionsTabAppearanceThemes::OptionsTabAppearanceThemes(QObject *parent)
	: MetaOptionsTab(parent, "themes", "", tr("Themes"), tr("Configure themes"))
{
	foreach (PsiThemeProvider *provider,
			 PsiThemeManager::instance()->registeredProviders()) {
		addTab( new OptionsTabAppearanceTheme(this, provider) );
	}
}





//----------------------------------------------------------------------------
// OptionsTabAppearanceTheme
//----------------------------------------------------------------------------

OptionsTabAppearanceTheme::OptionsTabAppearanceTheme(QObject *parent,
													PsiThemeProvider *provider_)
	: OptionsTab(parent, qPrintable(provider_->type()), "",
				 tr(qPrintable(provider_->optionsName())),
				 tr(qPrintable(provider_->optionsDescription())))
	, w(0)
	, provider(provider_)
{

}

OptionsTabAppearanceTheme::~OptionsTabAppearanceTheme()
{

}

QWidget *OptionsTabAppearanceTheme::widget()
{
	if ( w )
		return 0;

	w = new OptAppearanceThemeUI();
	OptAppearanceThemeUI *d = (OptAppearanceThemeUI *)w;
	themesModel = new PsiThemeModel(this);
	themesModel->setType(provider->type());
	PsiThemeViewDelegate *vd = new PsiThemeViewDelegate(d->themeView);
	d->themeView->setItemDelegate(vd);
	d->themeView->setModel(themesModel);
	int sw = provider->screenshotWidth();
	if (sw) {
		d->themeView->setFixedWidth(sw);
	}
	connect(d->themeView->selectionModel(), SIGNAL(currentChanged(QModelIndex, QModelIndex)), SIGNAL(dataChanged()));

	return w;
}

void OptionsTabAppearanceTheme::applyOptions()
{
	if ( !w )
		return;

	OptAppearanceThemeUI *d = (OptAppearanceThemeUI *)w;
	//qDebug("save theme: %s", qPrintable(d->themeView->currentIndex().data(PsiThemeModel::IdRole).toString()));
	PsiOptions::instance()->setOption("options.ui.chat.theme", d->themeView->currentIndex().data(PsiThemeModel::IdRole));
}

void OptionsTabAppearanceTheme::restoreOptions()
{
	if ( !w )
		return;

	OptAppearanceThemeUI *d = (OptAppearanceThemeUI *)w;

	d->themeView->setCurrentIndex(themesModel->index( themesModel->themeRow(PsiOptions::instance()->getOption("options.ui.chat.theme").toString()) ));
}
