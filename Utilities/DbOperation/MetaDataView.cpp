#include "stdafx.h"
#include <Shared/DbAccess/MetaDataModel.h>
#include "MetaDataView.h"
#include "AddColumnDialog.h"
#include "NormalizationDlg.h"
#include "AdjustSplitsDlg.h"
#include "CreateFeaturesDlg.h"

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
			model_->add_column(path, unit_id);
		}
		catch (const std::exception& ex)
		{
			QMessageBox::critical(qApp->activeWindow(), "Failed to add column", QString::fromStdString(ex.what()));
		}
	}
	event->acceptProposedAction();
}
//----------------------------------------------------------------------------------------------------------
void MetaDataView::delete_column()
{
	QModelIndex idx = currentIndex();
	if (idx.isValid() &&
		QMessageBox::question(this, "Deleting column", "Confirm column deletion", QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes)
	{
		model_->delete_column(idx.row());
	}
}
//----------------------------------------------------------------------------------------------------------
void MetaDataView::normalize_column()
{
	QModelIndex idx = currentIndex();
	if (idx.isValid())
	{
		NormalizationDlg dlg{this};
		if (dlg.exec() == QDialog::Accepted)
		{
			model_->normalize_column(dlg.method(), idx.row());
		}
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
		model_->make_target(idx.row());
	}
}
//----------------------------------------------------------------------------------------------------------
void MetaDataView::adjust_splits()
{
	QModelIndex idx = currentIndex();
	if (idx.isValid())
	{
		DataFrame df = model_->load_column(idx.row());
		std::vector<Split> splits = detect_splits(df, 0);
		AdjustSplitsDlg dlg(splits, this);
		if (dlg.exec() == QDialog::Accepted)
		{
			apply_splits(dlg.splits(), df, 0);
			model_->store_column(idx.row(), df);
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
