#pragma once

#include <QAbstractTableModel>

class TrainProgressModel : public QAbstractTableModel
{
	Q_OBJECT

public:
	TrainProgressModel(QObject *parent);
	~TrainProgressModel();
};
