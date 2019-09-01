#include "stdafx.h"
#include "Omni.h"
#include "Normalizer.h"

//----------------------------------------------------------------------------------------------------------
Omni::Omni(QWidget* parent)
	: QMainWindow(parent)
	, db_(std::make_unique<DbAccess>())
{
	ui_.setupUi(this);

	_createMetadataView();

	connect(ui_.action_normalize_all_, &QAction::triggered, this, &Omni::normalizeAll);
}
//----------------------------------------------------------------------------------------------------------
void Omni::_createMetadataView()
{
	metadata_model_ = std::make_unique<MetaDataModel>(*db_, this);
	ui_.meta_data_view_->setModel(metadata_model_.get());
	//setCentralWidget(ui_.meta_data_view_);
}
//----------------------------------------------------------------------------------------------------------
void Omni::normalizeAll()
{
	QApplication::setOverrideCursor(Qt::WaitCursor);

	int total_count = 0;
	int norm_count = 0;

	const Normalizer normalizer;
	std::vector<ColumnInfo> infos = metadata_model_->columnInfos();

	for (auto& col_info : infos)
	{
		++total_count;
		if (!col_info.normalized_)
		{
			ColumnData col_data = db_->loadColumnData("ready", col_info.table_, col_info.column_);
			std::pair<double, double> min_max;
			if (normalizer.normalize(col_data, min_max))
			{
				col_info.normalized_ = true;
				col_info.norm_min_ = min_max.first;
				col_info.norm_max_ = min_max.second;
				db_->storeColumnData(col_info, col_data);
				++norm_count;
			}
		}
	}

	metadata_model_->load();

	QApplication::restoreOverrideCursor();
	QMessageBox::information(this, "Normalize all", QString("%1 normalized\n%2 processed").arg(norm_count).arg(total_count));
}
