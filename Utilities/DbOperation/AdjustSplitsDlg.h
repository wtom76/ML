#pragma once

#include <array>
#include <vector>
#include <chrono>
#include <QDialog>
#include <Shared/Utility/types.hpp>
#include "ui_AdjustSplitsDlg.h"

//----------------------------------------------------------------------------------------------------------
// struct Split
//----------------------------------------------------------------------------------------------------------
struct Split
{
	std::chrono::system_clock::time_point time_;	// first date/time of splitted price
	long den_ = 0;									// denominator

	Split() = default;
	Split(std::chrono::system_clock::time_point time, long den) : time_{time}, den_{den}
	{}
};

//----------------------------------------------------------------------------------------------------------
std::vector<Split> detect_splits(const DataFrame& df, size_t col_idx);
void apply_splits(const std::vector<Split>& splits, DataFrame& df, size_t col_idx);
//----------------------------------------------------------------------------------------------------------
// class SplitsModel
//----------------------------------------------------------------------------------------------------------
class SplitsModel : public QAbstractTableModel
{
	Q_OBJECT;

// data
private:
	std::vector<Split> data_;
	static constexpr int column_count_ = 2;
	std::array<QString, column_count_> col_names_;

// methods
public:
	explicit SplitsModel(const std::vector<Split>& data, QObject* parent);

	// QAbstractTableModel impl
	int rowCount(const QModelIndex& parent = QModelIndex()) const override;
	int columnCount(const QModelIndex& parent = QModelIndex()) const override;
	QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
	QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
	Qt::ItemFlags flags(const QModelIndex& index) const override;
	bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
	//~QAbstractTableModel impl

	void set(const std::vector<Split>& data);
	const std::vector<Split>& splits() const noexcept { return data_; }
};

//---------------------------------------------------------------------------------------------------------
/// class AdjustSplitsDlg
//----------------------------------------------------------------------------------------------------------
class AdjustSplitsDlg : public QDialog
{
	Q_OBJECT;

private:
	Ui::AdjustSplitsDlg ui_;
	std::unique_ptr<SplitsModel> model_;

public:
	AdjustSplitsDlg(const std::vector<Split>& data, QWidget* parent = nullptr);
	~AdjustSplitsDlg();

	const std::vector<Split>& splits() const noexcept { return model_->splits(); }
};
