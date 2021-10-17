#include "stdafx.h"
#include <QDragEnterEvent>
#include <QMimeData>
#include <QMessageBox>
#include <Shared/DbAccess/MetaDataModel.h>
#include <Shared/DbAccess/OhlcDlg.h>
#include "MetaDataView.h"
#include "AddColumnDialog.h"
#include "NormalizationDlg.h"
#include "AdjustSplitsDlg.h"
#include "CreateFeaturesDlg.h"
#include "MakeTargetDlg.h"
#include "TargetOhlcSourceDlg.h"

//---------------------------------------------------------------------------------------------------------
MetaDataView::MetaDataView(MetaDataModel* model)
	: model_(model)
{
	setAcceptDrops(true);
	setDropIndicatorShown(true);
	hideColumn(0); // don't show the ID
	verticalHeader()->setDefaultSectionSize(16);
}
//---------------------------------------------------------------------------------------------------------
MetaDataView::~MetaDataView()
{}
//---------------------------------------------------------------------------------------------------------
void MetaDataView::dragEnterEvent(QDragEnterEvent* event)
{
	if (event->mimeData()->hasFormat(column_name_mime_type_name))
	{
		event->acceptProposedAction();
	}
	else
	{
		event->ignore();
	}
}
//---------------------------------------------------------------------------------------------------------
void MetaDataView::dragMoveEvent(QDragMoveEvent* event)
{
	if (event->mimeData()->hasFormat(column_name_mime_type_name))
	{
		if (event->source() == this)
		{
			event->setDropAction(Qt::MoveAction);
			event->accept();
		}
		else
		{
			event->acceptProposedAction();
		}
	}
	else
	{
		event->ignore();
	}
}
//---------------------------------------------------------------------------------------------------------
void MetaDataView::dropEvent(QDropEvent* event)
{
	const ColumnPath path = ColumnPath::from_string(QString::fromUtf8(
		event->mimeData()->data(column_name_mime_type_name)).toStdString());
	AddColumnDialog dlg(qApp->activeWindow());
	dlg.setColumn(path);
	if (dlg.exec() == QDialog::Accepted)
	{
		try
		{
			const int unit_id = dlg.selectedUnitId();
			if (unit_id < 0)
			{
				throw std::runtime_error("Unit is not selected");
			}
			model_->add_column(path, dlg.dest_column(), dlg.dest_table(), unit_id, dlg.is_target(), dlg.target_error());
		}
		catch (const std::exception& ex)
		{
			QMessageBox::critical(qApp->activeWindow(), "Failed to add column", QString::fromStdString(ex.what()));
		}
	}
	event->acceptProposedAction();
}
//----------------------------------------------------------------------------------------------------------
std::vector<std::pair<std::string, int>> MetaDataView::_series_names() const
{
	const std::vector<ColumnMetaData> infos = model_->columnInfos();
	std::vector<std::pair<std::string, int>> names;
	names.reserve(infos.size());
	int idx = 0;
	for (const ColumnMetaData& info : infos)
	{
		names.emplace_back(std::pair(info.column_, idx++));
	}
	return names;
}
//----------------------------------------------------------------------------------------------------------
void MetaDataView::delete_checked_columns()
{
	if (QMessageBox::question(this, "Deleting columns", "Confirm checked columns deletion", QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes)
	{
		model_->delete_checked_columns();
	}
}
//----------------------------------------------------------------------------------------------------------
void MetaDataView::normalize_checked_columns()
{
	NormalizationDlg dlg{this};
	if (dlg.exec() == QDialog::Accepted)
	{
		model_->normalize_checked_columns(dlg.method());
	}
}
//----------------------------------------------------------------------------------------------------------
void MetaDataView::normalize_all()
{
	model_->normalize_all();
}
//----------------------------------------------------------------------------------------------------------
void MetaDataView::make_target()
{
	QModelIndex idx = currentIndex();
	if (idx.isValid())
	{
		MakeTargetDlg dlg{this};
		if (dlg.exec() == QDialog::Accepted)
		{
			switch (dlg.type())
			{
			case MakeTargetDlg::Type::Delta:
				model_->make_target_delta(idx.row(), dlg.period());
				break;
			case MakeTargetDlg::Type::WinLoss:
				model_->make_target_winloss(idx.row(), dlg.win_loss_threshold());
				break;
			case MakeTargetDlg::Type::OHLC:
				_show_make_target_ohlc_dlg(dlg.win_loss_threshold());
				break;
			}
		}
	}
}
//----------------------------------------------------------------------------------------------------------
void MetaDataView::_show_make_target_ohlc_dlg(double wl_threshold) const
{
	TargetOhlcSourceDlg dlg{_series_names(), qApp->activeWindow()};
	if (dlg.exec() == QDialog::Accepted)
	{
		model_->make_target_ohlc({dlg.idx_o(), dlg.idx_h(), dlg.idx_l(), dlg.idx_c()}, wl_threshold);
	}
}
//----------------------------------------------------------------------------------------------------------
void MetaDataView::adjust_splits()
{
	QModelIndex idx = currentIndex();
	if (idx.isValid())
	{
		DataFrame df{model_->load_column(idx.row())};
		std::vector<Split> splits{detect_splits(df, 0)};
		AdjustSplitsDlg dlg(splits, this);
		if (dlg.exec() == QDialog::Accepted)
		{
			apply_splits(dlg.splits(), df, 0);
			model_->store_column(idx.row(), df);
			model_->load();
		}
	}
}
//----------------------------------------------------------------------------------------------------------
void MetaDataView::adjust_splits_ohlcv()
{
	unique_ptr<OhlcDlg> dlg{make_unique<OhlcDlg>()};
	util::config::load_or_create(*dlg, util::config::file_path("ohlc_dialog"));
	if (QDialog::Accepted == dlg->exec())
	{
		util::config::store(*dlg, util::config::file_path("ohlc_dialog"));

		DataFrame df{
			model_->db().load_data(
				dlg->schema(),
				dlg->table(),
				{dlg->open_col(), dlg->high_col(), dlg->low_col(), dlg->close_col(), dlg->close_col()}
			)
		};

		std::vector<Split> splits{detect_splits_ohlc(df)};
		AdjustSplitsDlg adjust_dlg(splits, this);
		if (adjust_dlg.exec() == QDialog::Accepted)
		{
			apply_splits_ohlc(adjust_dlg.splits(), df);
			for (int i = 0; i < df.col_count(); ++i)
			{
				model_->store_column(i, df);
			}
			model_->load();
		}
	}
}
//----------------------------------------------------------------------------------------------------------
void MetaDataView::create_features()
{
	QModelIndex idx = currentIndex();
	if (idx.isValid())
	{
		CreateFeaturesDlg dlg(QString::fromStdString(model_->col_meta(idx.row()).column_), this);
		if (dlg.exec() == QDialog::Accepted)
		{
			QApplication::setOverrideCursor(Qt::WaitCursor);
			for (int period = 1; period <= 5; ++period)
			{
				if (dlg.delta(period))
				{
					model_->make_feature_delta(idx.row(), period);
				}
			}
			model_->load();
			QApplication::restoreOverrideCursor();
			QMessageBox::information(qApp->activeWindow(), QString("Making feature"), QString("Done"));
		}
	}
}
