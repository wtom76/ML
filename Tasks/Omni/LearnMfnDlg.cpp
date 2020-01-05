#include "stdafx.h"
#include <algorithm>
#include <Shared/DbAccess/ColumnMetaData.h>
#include <Shared/DbAccess/DbAccess.h>
#include <Shared/Data/Preparation.hpp>
#include "LearnMfnDlg.h"

using namespace std;
using namespace wtom::ml;

//----------------------------------------------------------------------------------------------------------
LearnMfnDlg::LearnGuiMode::LearnGuiMode(Ui::LearnMfnDlg& ui)
	: ui_(ui)
{
	ui_.target_->setEnabled(false);
}
//----------------------------------------------------------------------------------------------------------
LearnMfnDlg::LearnGuiMode::~LearnGuiMode()
{
	ui_.target_->setEnabled(true);
}

//----------------------------------------------------------------------------------------------------------
LearnMfnDlg::LearnMfnDlg(std::vector<ColumnMetaData>&& infos, std::shared_ptr<DbAccess> db, QWidget* parent)
	: QDialog{parent}
	, db_{db}
	, col_infos_{std::move(infos)}
{
	ui_.setupUi(this);
	size_t idx = 0;
	for (const auto& col_info : col_infos_)
	{
		ui_.target_->addItem(QString::fromStdString(col_info.column_), QVariant{idx});
		++idx;
	}

	connect(ui_.learn_btn_,			&QPushButton::clicked, this, &LearnMfnDlg::slot_learn);
	connect(ui_.stop_btn_,			&QPushButton::clicked, this, &LearnMfnDlg::slot_stop);
	connect(ui_.save_network_btn_,	&QPushButton::clicked, this, &LearnMfnDlg::slot_save_network);

	connect(this, SIGNAL(begin_teach),		this, SLOT(slot_begin_teach),			Qt::QueuedConnection);
	connect(this, SIGNAL(set_best(double)),	this, SLOT(slot_set_best(double)),		Qt::QueuedConnection);
	connect(this, SIGNAL(set_last(double)),	this, SLOT(slot_set_last(double)),		Qt::QueuedConnection);
	connect(this, SIGNAL(set_epoch(unsigned long long)), this, SLOT(slot_set_epoch(unsigned long long)),	Qt::QueuedConnection);
	connect(this, SIGNAL(end_teach),		this, SLOT(slot_end_teach),				Qt::QueuedConnection);
	connect(this, SIGNAL(begin_test),		this, SLOT(slot_begin_test),			Qt::QueuedConnection);
	connect(this, SIGNAL(add_sample_result(double, double)), this, SLOT(slot_add_sample_result(double, double)), Qt::QueuedConnection);
	connect(this, SIGNAL(end_test(unsigned long long, unsigned long long)), this, SLOT(slot_end_test(unsigned long long, unsigned long long)), Qt::QueuedConnection);
}

//----------------------------------------------------------------------------------------------------------
LearnMfnDlg::~LearnMfnDlg()
{
	if (learn_fut_.valid())
	{
		stop_flag_ = true;
		learn_fut_.get();
	}
}
//----------------------------------------------------------------------------------------------------------
const ColumnMetaData* LearnMfnDlg::_target_column() const
{
	const QVariant current_idx = ui_.target_->currentData();
	if (current_idx.isValid())
	{
		return &col_infos_[current_idx.toInt()];
	}
	return nullptr;
}
//----------------------------------------------------------------------------------------------------------
vector<string> LearnMfnDlg::_input_names(const ColumnMetaData& target_column) const
{
	vector<string> input_names;
	input_names.reserve(col_infos_.size() - 1);
	for (const auto& col_info : col_infos_)
	{
		assert(col_info.table_ == target_column.table_);
		if (col_info.column_ != target_column.column_)
		{
			input_names.emplace_back(col_info.column_);
		}
	}
	return input_names;
}
//----------------------------------------------------------------------------------------------------------
/// 1. Calculate linear change shifted one step backward
bool LearnMfnDlg::_fill_target(const std::vector<double>& src, std::vector<double>& dst) const
{
	if (src.size() <= 1)
	{
		return false;
	}
	dst.resize(src.size());
	auto src_i = cbegin(src);
	auto src_e = cend(src);
	double prev_src = *src_i++;
	auto dst_i = begin(dst);
	double dst_max = std::numeric_limits<double>::min();
	double dst_min = std::numeric_limits<double>::max();
	for (; src_i != src_e; ++src_i, ++dst_i)
	{
		*dst_i = *src_i - prev_src;
		prev_src = *src_i;
		if (*dst_i < dst_min)
		{
			dst_min = *dst_i;
		}
		if (*dst_i > dst_max)
		{
			dst_max = *dst_i;
		}
	}
	assert(dst_max <= 1.);
	assert(dst_min >= -1.);
	return true;
}
//----------------------------------------------------------------------------------------------------------
/// 1. Load data frame from DB
/// 2. Fill all non-target-source lacunes by fill_forward policy
/// 3. Drop all rows where target source is NaN
std::pair<DataFrame, DataView> LearnMfnDlg::_prepare_data(const string& schema, const string& table, const string& target_name,
	const vector<string>& input_names) const
{
	constexpr size_t max_tolerated_gap = std::numeric_limits<size_t>::max();
	// 1.
	vector<string> load_names = input_names;
	load_names.emplace_back(target_name);
	DataFrame df = db_->load_data(schema, table, load_names);
	DataView dv{df};
	// 2.
	for (const string& col_name : load_names)
	{
		if (col_name != target_name)
		{
			DataFrame::series_t* series = df.series(col_name);
			if (series)
			{
				data::fill_missing_forward(*series, max_tolerated_gap);
			}
		}
	}
	// 3.
	dv.delete_rows_with_nan();
	return {df, dv};
}
//----------------------------------------------------------------------------------------------------------
void LearnMfnDlg::slot_learn()
{
	if (learn_fut_.valid())
	{
		stop_flag_ = true;
		try
		{
			learn_fut_.get();
		}
		catch (const exception&)
		{}
	}
	stop_flag_ = false;

	LearnGuiMode gui_mode(ui_);

	if (col_infos_.size() <= 1)
	{
		return;
	}

	const ColumnMetaData* target_column = _target_column();
	if (!target_column)
	{
		return;
	}
	if (!target_column->normalized_)
	{
		QMessageBox::warning(this, "Learning is rejected", QString("'%1' should be normalized").arg(target_column->column_.c_str()));
		return;
	}

	const double target_error = ui_.target_error_->text().toDouble();
	if (!target_error)
	{
		return;
	}

	const vector<string> input_names = _input_names(*target_column);
	std::pair<DataFrame, DataView> dfv = _prepare_data("ready"s, target_column->table_, target_column->column_, input_names);
	const vector<string> target_names{target_column->column_};

	wtom::ml::neuro::net::Config mfn_cfg{{col_infos_.size() - 1, 2 * (col_infos_.size() - 1), 1}};
	mfn_ = std::make_unique<Network>(mfn_cfg);
	teacher_ = std::make_unique<Teacher>(std::move(dfv), input_names, target_names);

	learn_fut_ = async(launch::async, [this, mfn_cfg, target_error]()
		{
			teacher_->teach(mfn_cfg, *this->mfn_, target_error, *this);
			teacher_->show_test(*this->mfn_, *this);
		}
	);
}
//----------------------------------------------------------------------------------------------------------
void LearnMfnDlg::slot_stop()
{
	stop_flag_ = true;
}
//----------------------------------------------------------------------------------------------------------
void LearnMfnDlg::slot_save_network()
{
}
//----------------------------------------------------------------------------------------------------------
void LearnMfnDlg::slot_begin_teach()
{
}
//----------------------------------------------------------------------------------------------------------
void LearnMfnDlg::slot_set_best(double cur_min_err)
{
	ui_.cur_min_err_->setText(QString::number(cur_min_err, 'f', 6));
}
//----------------------------------------------------------------------------------------------------------
void LearnMfnDlg::slot_set_last(double err)
{
	ui_.last_error_->setText(QString::number(err, 'f', 6));
}
//----------------------------------------------------------------------------------------------------------
void LearnMfnDlg::slot_set_epoch(unsigned long long num)
{
	ui_.epochs_passed_->setText(QString::number(num));
}
//----------------------------------------------------------------------------------------------------------
void LearnMfnDlg::slot_end_teach()
{
}
//----------------------------------------------------------------------------------------------------------
void LearnMfnDlg::slot_begin_test()
{
}
//----------------------------------------------------------------------------------------------------------
void LearnMfnDlg::slot_add_sample_result(double /*omega*/, double /*target*/)
{
}
//----------------------------------------------------------------------------------------------------------
void LearnMfnDlg::slot_end_test(unsigned long long true_count, unsigned long long false_count)
{
	ui_.test_result_->setText(QString("Hits/Misses: ") + QString::number(true_count) +
		QString("/") + QString::number(false_count));
}
