#pragma once
#include <QTableView>

class MetaDataModel;

//---------------------------------------------------------------------------------------------------------
// class MetaDataView
//---------------------------------------------------------------------------------------------------------
class MetaDataView : public QTableView
{
	MetaDataModel* model_ = nullptr;

protected:
	void dragEnterEvent(QDragEnterEvent* event) override;
	void dragMoveEvent(QDragMoveEvent* event) override;
	void dropEvent(QDropEvent* event) override;

public:
	explicit MetaDataView(MetaDataModel* model);
	virtual ~MetaDataView();

public slots:
	void deleteColumn();
};
