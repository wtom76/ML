#pragma once
#include <array>
#include <QAbstractTableModel>
#include <QTimer>
#include "TrainLogger.h"

class TrainProgressModel;

//----------------------------------------------------------------------------------------------------------
/// class TrainProgressData
//----------------------------------------------------------------------------------------------------------
class TrainProgressData
	: public TrainLogger
	, public util::mt::Mutex
{
public:
	TrainProgressModel* owner_{nullptr};
	int			order_num_;		// in TrainProgressModel
	string		label_;
	ptrdiff_t	epochs_passed_{0};
	double		current_min_err_{0.};
	double		last_err_{0.};

public:
	TrainProgressData(int order_num, TrainProgressModel* owner)
		: order_num_{order_num}
		, owner_{owner}
	{}
	TrainLogger* next() override;
	// TrainLogger impl
	void begin_teach(string label) override;
	void set_best(double cur_min_err) override;
	void set_last(double err) override;
	void set_epoch(unsigned long long num) override;
	void end_teach() override;
	//~TrainLogger impl
};

//----------------------------------------------------------------------------------------------------------
/// class TrainProgressModel
//----------------------------------------------------------------------------------------------------------
class TrainProgressModel : public QAbstractTableModel
{
	Q_OBJECT;

// data
private:
	static constexpr int column_count_ = 4;

	std::array<QString, column_count_> col_names_;
	mutable util::mt::CriticalSection data_entries_mtx_;
	std::vector<unique_ptr<TrainProgressData>> data_;

	QTimer* timer_{nullptr};

// methods
public:
	TrainProgressModel(QObject* parent);
	~TrainProgressModel();

	// QAbstractTableModel impl
	int rowCount(const QModelIndex& parent = QModelIndex()) const override;
	int columnCount(const QModelIndex& parent = QModelIndex()) const override;
//	Qt::ItemFlags flags(const QModelIndex& index) const override;
	QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
//	bool setData(const QModelIndex& index, const QVariant& value, int role) override;
	QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
	//~QAbstractTableModel impl

	TrainLogger* create_entry();
	void clear();

public slots:
	void invalidate(int row_idx, int col_idx);
	void update_all();
};
