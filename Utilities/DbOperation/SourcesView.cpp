#include "stdafx.h"
#include <QApplication>
#include <QMouseEvent>
#include <QMimeData>
#include <QDrag>
#include <Shared/DbAccess/ColumnPath.h>
#include "SourcesView.h"
#include "SourcesModel.h"
#include "TreeItem.h"

//----------------------------------------------------------------------------------------------------------
SourcesView::SourcesView(SourcesModel* model, QWidget* parent)
	: QTreeView(parent)
	, model_(model)
{
	assert(model_);
	setDragEnabled(true);
}
//----------------------------------------------------------------------------------------------------------
SourcesView::~SourcesView()
{
}
//----------------------------------------------------------------------------------------------------------
void SourcesView::mousePressEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton)
	{
		drag_start_point_ = event->pos();
	}
	QTreeView::mousePressEvent(event);
}
//----------------------------------------------------------------------------------------------------------
void SourcesView::mouseMoveEvent(QMouseEvent* event)
{
	if (!(event->buttons() & Qt::LeftButton))
	{
		return;
	}
	if ((event->pos() - drag_start_point_).manhattanLength() < QApplication::startDragDistance())
	{
		return;
	}
	const QModelIndex item_idx = indexAt(drag_start_point_);
	if (!item_idx.isValid())
	{
		return;
	}

	std::unique_ptr<QMimeData> mime_data = std::make_unique<QMimeData>();
	mime_data->setData(column_name_mime_type_name,
		QString::fromStdString(model_->buildPath(item_idx).to_string()).toUtf8());
	std::unique_ptr<QDrag> drag = std::make_unique<QDrag>(this);
	drag->setMimeData(mime_data.release());

	/*Qt::DropAction dropAction = */drag->exec(Qt::CopyAction | Qt::MoveAction, Qt::CopyAction);
}
//---------------------------------------------------------------------------------------------------------
void SourcesView::dragEnterEvent(QDragEnterEvent* event)
{
	if (event->source() == this)
	{
		event->setDropAction(Qt::MoveAction);
		event->accept();
	}
	else
	{
		event->ignore();
	}
}
//---------------------------------------------------------------------------------------------------------
void SourcesView::dragMoveEvent(QDragMoveEvent* event)
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
void SourcesView::dropEvent(QDropEvent* event)
{
	if (event->source() == this)
	{
		event->setDropAction(Qt::MoveAction);
		event->accept();
	}
	else
	{
		event->ignore();
	}
}
