#include "stdafx.h"
#include "Omni.h"
#include "Correlations.h"
#include "CorrelationsDlg.h"
#include "CorrelationsModel.h"
#include "LearnMfnDlg.h"

//----------------------------------------------------------------------------------------------------------
Omni::Omni(QWidget* parent)
	: QMainWindow(parent)
	, db_(std::make_shared<DbAccess>())
{
	ui_.setupUi(this);

	_createMetadataView();

	connect(ui_.action_correlations_, &QAction::triggered, this, &Omni::showCorrelations);
	connect(ui_.action_mfn, &QAction::triggered, this, &Omni::showLearnMfn);
}
//----------------------------------------------------------------------------------------------------------
void Omni::_createMetadataView()
{
	metadata_model_ = std::make_unique<MetaDataModel>(*db_, this);
	ui_.meta_data_view_->setModel(metadata_model_.get());
	for (int i = 0; i < ui_.meta_data_view_->header()->count(); ++i)
	{
		ui_.meta_data_view_->resizeColumnToContents(i);
	}
}
//----------------------------------------------------------------------------------------------------------
void Omni::showCorrelations()
{
	QApplication::setOverrideCursor(Qt::WaitCursor);
	const std::vector<ColumnMetaData> infos = metadata_model_->columnInfos();
	CorrelationsModel* model = new CorrelationsModel(infos, *db_, this);
	CorrelationsDlg* wnd = new CorrelationsDlg(this);
	wnd->view()->setModel(model);
	wnd->show();
	QApplication::restoreOverrideCursor();
}
//----------------------------------------------------------------------------------------------------------
void Omni::showLearnMfn()
{
	QApplication::setOverrideCursor(Qt::WaitCursor);
	LearnMfnDlg* wnd = new LearnMfnDlg(metadata_model_->columnInfos(), db_, this);
	wnd->show();
	QApplication::restoreOverrideCursor();
}
