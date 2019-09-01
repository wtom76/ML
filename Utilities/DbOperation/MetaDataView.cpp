#include "stdafx.h"
#include <Shared/DbAccess/MetaDataModel.h>
#include "MetaDataView.h"
#include "AddColumnDialog.h"

//---------------------------------------------------------------------------------------------------------
MetaDataView::MetaDataView(MetaDataModel* model)
	: model_(model)
{
	setAcceptDrops(true);
	setDropIndicatorShown(true);
	hideColumn(0); // don't show the ID
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
	const ColumnPath path = ColumnPath::fromString(QString::fromUtf8(
		event->mimeData()->data(column_name_mime_type_name)));
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
			model_->addColumn(path, unit_id);
		}
		catch (const std::exception& ex)
		{
			QMessageBox::critical(qApp->activeWindow(), "Failed to add column", QString::fromStdString(ex.what()));
		}
	}
	event->acceptProposedAction();
}
//----------------------------------------------------------------------------------------------------------
void MetaDataView::deleteColumn()
{
	QModelIndex idx = currentIndex();
	if (idx.isValid() &&
		QMessageBox::question(this, "Deleting column", "Confirm column deletion", QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes)
	{
		model_->deleteColumn(idx.row());
	}
}
