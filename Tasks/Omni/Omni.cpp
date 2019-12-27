#include "stdafx.h"
#include "Omni.h"
#include "Normalizer.h"
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

	connect(ui_.action_normalize_all_, &QAction::triggered, this, &Omni::normalizeAll);
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
// TODO: load and store all non-norm cols at once
void Omni::normalizeAll()
{
	QApplication::setOverrideCursor(Qt::WaitCursor);

	int total_count = 0;
	int norm_count = 0;

	const Normalizer normalizer;
	std::vector<ColumnMetaData> infos = metadata_model_->columnInfos();

	for (auto& col_info : infos)
	{
		++total_count;
		if (!col_info.normalized_)
		{
			DataFrame col_data = db_->load_data("ready", col_info.table_, {col_info.column_});
			std::pair<double, double> min_max;
			if (normalizer.normalize(col_info.column_, col_data, min_max))
			{
				col_info.normalized_ = true;
				col_info.norm_min_ = min_max.first;
				col_info.norm_max_ = min_max.second;
				db_->store_column(col_info, col_data);
				++norm_count;
			}
		}
	}

	metadata_model_->load();

	QApplication::restoreOverrideCursor();
	QMessageBox::information(this, "Normalize all", QString("%1 normalized\n%2 processed").arg(norm_count).arg(total_count));
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
