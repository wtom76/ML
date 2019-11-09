#pragma once

#include <QAbstractTableModel>
#include <Shared/DbAccess/ColumnMetaData.h>
#include <Shared/DbAccess/DbAccess.h>
#include "Correlations.h"

//----------------------------------------------------------------------------------------------------------
// class CorrelationsModel
//----------------------------------------------------------------------------------------------------------
class CorrelationsModel : public QAbstractTableModel
{
	Q_OBJECT;

// data
private:
	Correlations data_;
	std::vector<ColumnMetaData> col_infos_;
	DbAccess& db_;

// methods
private:
	void _load();

public:
	CorrelationsModel(const std::vector<ColumnMetaData>& col_infos, DbAccess& db, QObject* parent);
	~CorrelationsModel();

	// QAbstractTableModel impl
	int rowCount(const QModelIndex& parent = QModelIndex()) const override;
	int columnCount(const QModelIndex& parent = QModelIndex()) const override;
	QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
	QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
	//~QAbstractTableModel impl
};
