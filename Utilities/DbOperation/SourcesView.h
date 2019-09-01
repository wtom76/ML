#pragma once

#include <QTreeView>

class SourcesModel;

//----------------------------------------------------------------------------------------------------------
// class SourcesView
//----------------------------------------------------------------------------------------------------------
class SourcesView : public QTreeView
{
	Q_OBJECT;

	SourcesModel* model_ = nullptr;
	QPoint drag_start_point_;

public:
	SourcesView(SourcesModel* model, QWidget* parent = nullptr);
	~SourcesView();

protected:
	void mousePressEvent(QMouseEvent* event) override;
	void mouseMoveEvent(QMouseEvent *event) override;
	void dragEnterEvent(QDragEnterEvent* event) override;
	void dragMoveEvent(QDragMoveEvent* event) override;
	void dropEvent(QDropEvent* event) override;
};
