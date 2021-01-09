#pragma once

#include <QDialog>
#include <future>
#include <Shared/Neuro/mfn.hpp>
#include <Shared/Neuro/rprop.hpp>
#include "ui_LearnMfnDlg.h"

struct ColumnMetaData;
class DbAccess;

//----------------------------------------------------------------------------------------------------------
/// class LearnMfnDlg
//----------------------------------------------------------------------------------------------------------
class LearnMfnDlg : public QDialog
{
	Q_OBJECT;

private:
	using Network = wtom::ml::neuro::net::MultilayerFeedforward;
	using Teacher = wtom::ml::neuro::learn::RProp<Network>;

private:
	Ui::LearnMfnDlg				ui_;
	std::shared_ptr<DbAccess>	db_;
	std::vector<ColumnMetaData>	col_infos_;
	std::atomic_bool			stop_flag_ = false;
	std::unique_ptr<Network>	mfn_;
	std::unique_ptr<Teacher>	teacher_;
	std::future<void>			learn_fut_;

	class LearnGuiMode
	{
		Ui::LearnMfnDlg& ui_;
	public:
		LearnGuiMode(Ui::LearnMfnDlg& ui);
		~LearnGuiMode();
	};

private:
	const ColumnMetaData* _target_column() const;
	std::vector<std::string> _input_names(const ColumnMetaData& target_column) const;
	bool _fill_target(const std::vector<double>& src, std::vector<double>& dst) const;
	std::pair<DataFrame, DataView> _prepare_data(const std::string& schema, const std::string& table,
		const std::string& target_name, const std::vector<std::string>& input_names) const;

public:
	LearnMfnDlg(std::vector<ColumnMetaData>&& infos, std::shared_ptr<DbAccess> db, QWidget* parent);
	~LearnMfnDlg();

private slots:
	void slot_learn();
	void slot_stop();
	void slot_save_network();
	void slot_begin_teach();
	void slot_set_best(double cur_min_err);
	void slot_set_last(double err);
	void slot_set_epoch(unsigned long long num);
	void slot_end_teach();
	void slot_begin_test();
	void slot_add_sample_result(double omega, double target_i);
	void slot_end_test(unsigned long long true_count, unsigned long long false_count);

public:
	bool stop_requested() const noexcept { return stop_flag_; }

signals:
	void begin_teach();
	void set_best(double cur_min_err);
	void set_last(double err);
	void set_epoch(unsigned long long num);
	void end_teach();
	void begin_test();
	void add_sample_result(double omega, double target);
	void end_test(unsigned long long true_count, unsigned long long false_count);
};
