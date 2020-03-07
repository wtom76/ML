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

	std::vector<std::pair<std::string, int>> _series_names() const;
	void _show_make_target_ohlc_dlg(double wl_threshold) const;

public:
	explicit MetaDataView(MetaDataModel* model);
	virtual ~MetaDataView();

public slots:
	void delete_column();
	void normalize_column();
	void normalize_all();
	void make_target();
	void adjust_splits();
	void create_features();
};
