#pragma once

#include <vector>
#include <memory>
#include <iterator>
#include <algorithm>
#include <numeric>
#include <cassert>
#include <random>
#include <iostream>
#include <limits>
#include <Shared/DbAccess/DataFrame.h>

//-----------------------------------------------------------------------------------------------------
/// ProgessView should provide
///  bool stop_requested() const noexcept { return stop_flag_; }
///  void begin_teach();
///  void set_best(double cur_min_err);
///  void set_last(double err);
///  void end_teach();
///  void begin_teach();
///  void add_sample_result(double omega, double target_i);
///  void end_test(double true_count, double false_count);
///  void begin_teach();

namespace Neuro::Learn
{
	//-----------------------------------------------------------------------------------------------------
	// class RProp
	//-----------------------------------------------------------------------------------------------------
	template <class Network>
	class RProp
	{
	// types
	public:
		using GradTerms		= typename Network::Outputs;
		using WeightGrads	= typename Network::Weights;
		using BiasGrads		= typename Network::Outputs;
		using WeightDeltas	= typename Network::Weights;
		using BiasDeltas	= typename Network::Outputs;

	private:
		struct SampleFiller
		{
			std::vector<const DataColumn*> inputs_;
			std::vector<const DataColumn*> targets_;

			explicit SampleFiller(const DataFrame& src_data,
				const std::vector<std::string>& in_names, const std::vector<std::string>& target_names);
			void fill(std::ptrdiff_t row, std::vector<double>& inputs, std::vector<double>& targets) const;
		};

	// data
	private:
		static constexpr double initial_learning_rate_ = 0.1;
		static constexpr double min_learning_rate_ = 1e-6;
		static constexpr double max_learning_rate_ = 50;
		static constexpr double inc_learning_rate_ = 1.2;
		static constexpr double dec_learning_rate_ = 0.5;

		std::unique_ptr<GradTerms>		grad_terms_;
		std::unique_ptr<WeightGrads>	dEdw_off_;
		std::unique_ptr<WeightGrads>	prev_dEdw_off_;
		std::unique_ptr<BiasGrads>		bias_dEdw_off_;
		std::unique_ptr<BiasGrads>		prev_bias_dEdw_off_;
		std::unique_ptr<WeightDeltas>	prev_weight_deltas_;
		std::unique_ptr<BiasDeltas>		prev_bias_deltas_;

		std::random_device				rd_;
		std::mt19937					rand_;
		DataFrame						src_data_;
		std::vector<std::ptrdiff_t>		teaching_set_;
		std::vector<std::ptrdiff_t>		test_set_;
		SampleFiller					sample_filler_;
		std::vector<double>				sample_targets_;

	// methods
	private:
		void _init(const typename Network::Config& cfg);
		void _updateGradients(Network& network, const std::vector<double>& targets);
		void _updateWeights(Network& network);
		void _updateBiases(Network& network);
		void _splitData();
		double _meanSqrError(Network& network);
	public:
		RProp(DataFrame&& src_data,
			const std::vector<std::string>& in_names,
			const std::vector<std::string>& target_names)
			: rand_{rd_()}
			, src_data_{std::move(src_data)}
			, sample_filler_{src_data_, in_names, target_names}
			, sample_targets_(target_names.size(), 0.)
		{}

		template <class ProgressView>
		void teach(const typename Network::Config& cfg, Network& network, double min_err, ProgressView& progress_view);
		template <class ProgressView>
		void show_test(Network& network, ProgressView& progress_view);
	};
	//-----------------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------------
	template <class Network>
	RProp<Network>::SampleFiller::SampleFiller(const DataFrame& src_data,
		const std::vector<std::string>& in_names,
		const std::vector<std::string>& target_names)
		: inputs_(in_names.size(), nullptr)
		, targets_(target_names.size(), nullptr)
	{
		for (size_t i = 0; i != in_names.size(); ++i)
		{
			inputs_[i] = &src_data.get_column<double>(in_names[i].c_str());
			assert(inputs_[i]);
		}
		for (size_t i = 0; i != target_names.size(); ++i)
		{
			targets_[i] = &src_data.get_column<double>(target_names[i].c_str());
			assert(targets_[i]);
		}
	}
	//-----------------------------------------------------------------------------------------------------
	template <class Network>
	void RProp<Network>::SampleFiller::fill(std::ptrdiff_t row, std::vector<double>& inputs, std::vector<double>& targets) const
	{
		auto input_i = std::begin(inputs);
		for (const auto src_input : inputs_)
		{
			++*input_i = (*src_input)[row];
		}
		auto target_i = std::begin(targets);
		for (const auto src_target : targets_)
		{
			++*target_i = (*src_target)[row];
		}
	}
	//-----------------------------------------------------------------------------------------------------
	template <class Network>
	void RProp<Network>::_init(const typename Network::Config& cfg)
	{
		grad_terms_			= std::make_unique<GradTerms>(cfg, 0.);
		dEdw_off_			= std::make_unique<WeightGrads>(*grad_terms_, 0.);
		prev_dEdw_off_		= std::make_unique<WeightGrads>(*grad_terms_, 0.);
		bias_dEdw_off_		= std::make_unique<BiasGrads>(cfg, 0.);
		prev_bias_dEdw_off_	= std::make_unique<BiasGrads>(cfg, 0.);
		prev_weight_deltas_	= std::make_unique<WeightDeltas>(*grad_terms_, initial_learning_rate_);
		prev_bias_deltas_	= std::make_unique<BiasDeltas>(cfg, initial_learning_rate_);
	}
	//-----------------------------------------------------------------------------------------------------
	// 1. calc gradient terms for omega layer
	// 2. calc gradient terms for hidden layers
	// 3. calc bias gradients for omega layer and hidden layers
	// 4. calc weight gradients for omega layer and hidden layers
	template <class Network>
	void RProp<Network>::_updateGradients(Network& network, const std::vector<double>& targets)
	{
		const size_t layer_count = network.outputs().size();
		assert(layer_count >= 3); // input, n * hidden, omega

		__int64 layer_idx = layer_count - 1;
		// 1.
		{
			auto& grad_terms = grad_terms_->layers_.back();
			auto& outputs = network.outputs().back();
			auto& functions = network.functions().back();
			for (size_t node_idx = 0; node_idx < grad_terms.size(); ++node_idx)
			{
				grad_terms[node_idx] =
					functions[node_idx]->derivative(outputs[node_idx]) * (outputs[node_idx] - targets[node_idx]);
			}
		}
		// 2.
		for (--layer_idx; layer_idx >= 1; --layer_idx)
		{
			auto& grad_terms = grad_terms_->layers_[layer_idx];
			auto& prev_grad_terms = grad_terms_->layers_[layer_idx + 1];
			auto& weights = network.weights()[layer_idx];
			auto& outputs = network.outputs()[layer_idx];
			auto& functions = network.functions()[layer_idx - 1];
			for (size_t src_node_idx = 0; src_node_idx < grad_terms.size(); ++src_node_idx)
			{
				double grad_sum = 0.;
				for (size_t dst_node_idx = 0; dst_node_idx < prev_grad_terms.size(); ++dst_node_idx)
				{
					grad_sum += prev_grad_terms[dst_node_idx] * weights[src_node_idx][dst_node_idx];
				}
				grad_terms[src_node_idx] = functions[src_node_idx]->derivative(outputs[src_node_idx]) * grad_sum;
			}
		}

		// 3.
		for (layer_idx = layer_count - 1; layer_idx > 0; --layer_idx)
		{
			auto& bias_grads = bias_dEdw_off_->layers_[layer_idx];
			auto& grad_terms = grad_terms_->layers_[layer_idx];
			for (size_t node_idx = 0; node_idx < bias_grads.size(); ++node_idx)
			{
				bias_grads[node_idx] += grad_terms[node_idx];
			}
		}
		// 4.
		for (layer_idx = layer_count - 2; layer_idx >= 0; --layer_idx)
		{
			auto& dEdw_off = dEdw_off_->layers_[layer_idx];
			auto& outputs = network.outputs()[layer_idx];
			auto& grad_terms = grad_terms_->layers_[layer_idx + 1];
			for (size_t src_node_idx = 0; src_node_idx < dEdw_off.size(); ++src_node_idx)
			{
				for (size_t dst_node_idx = 0; dst_node_idx < dEdw_off[src_node_idx].size();
					++dst_node_idx)
				{
					dEdw_off[src_node_idx][dst_node_idx]
						+= grad_terms[dst_node_idx] * outputs[src_node_idx];
				}
			}
		}
	}
	//-----------------------------------------------------------------------------------------------------
	template <class Network>
	void RProp<Network>::_updateWeights(Network& network)
	{
		const size_t layer_count = network.outputs().size();
		assert(layer_count >= 3); // input, n * hidden, omega

		for (size_t layer_idx = 0; layer_idx < layer_count - 1; ++layer_idx)
		{
			auto& weights = network.weights()[layer_idx];
			auto& prev_weight_deltas = prev_weight_deltas_->layers_[layer_idx];
			auto& dEdw_off = dEdw_off_->layers_[layer_idx];
			auto& prev_dEdw_off = prev_dEdw_off_->layers_[layer_idx];

			for (size_t src_node_idx = 0; src_node_idx < weights.size(); ++src_node_idx)
			{
				for (size_t dst_node_idx = 0; dst_node_idx < weights[src_node_idx].size(); ++dst_node_idx)
				{
					const double prev_weight_delta = prev_weight_deltas[src_node_idx][dst_node_idx];
					double weight_delta = 0.;
					const double weight_grad_prod = prev_dEdw_off[src_node_idx][dst_node_idx] * dEdw_off[src_node_idx][dst_node_idx];

					if (weight_grad_prod > 0.)
					{
						weight_delta = std::min(prev_weight_delta * inc_learning_rate_, max_learning_rate_);
						weights[src_node_idx][dst_node_idx] -= std::copysign(weight_delta, dEdw_off[src_node_idx][dst_node_idx]);
					}
					else if (weight_grad_prod < 0.)
					{
						weight_delta = std::max(prev_weight_delta * dec_learning_rate_, min_learning_rate_);
						weights[src_node_idx][dst_node_idx] -= prev_weight_deltas[src_node_idx][dst_node_idx];
						dEdw_off[src_node_idx][dst_node_idx] = 0.;
					}
					else
					{
						weight_delta = prev_weight_delta;
						weights[src_node_idx][dst_node_idx] -= std::copysign(weight_delta, dEdw_off[src_node_idx][dst_node_idx]);
					}
					assert(weight_delta >= 0.);
					prev_weight_deltas[src_node_idx][dst_node_idx] = weight_delta;
					prev_dEdw_off[src_node_idx][dst_node_idx] = dEdw_off[src_node_idx][dst_node_idx];
				}
			}
		}
	}
	//-----------------------------------------------------------------------------------------------------
	template <class Network>
	void RProp<Network>::_updateBiases(Network& network)
	{
		const size_t layer_count = network.outputs().size();
		assert(layer_count >= 3); // input, n * hidden, omega

		for (size_t layer_idx = 1; layer_idx < layer_count; ++layer_idx)
		{
			auto& biases = network.biases()[layer_idx];
			auto& prev_bias_deltas = prev_bias_deltas_->layers_[layer_idx];
			auto& bias_dEdw_off = bias_dEdw_off_->layers_[layer_idx];
			auto& prev_bias_dEdw_off = prev_bias_dEdw_off_->layers_[layer_idx];

			for (size_t node_idx = 0; node_idx < biases.size(); ++node_idx)
			{
				const double prev_weight_delta = prev_bias_deltas[node_idx];
				double delta = 0.;
				const double weight_grad_prod = prev_bias_dEdw_off[node_idx] * bias_dEdw_off[node_idx];

				if (weight_grad_prod > 0.)
				{
					delta = std::min(prev_weight_delta * inc_learning_rate_, max_learning_rate_);
					biases[node_idx] -= std::copysign(delta, bias_dEdw_off[node_idx]);
				}
				else if (weight_grad_prod < 0.)
				{
					delta = std::max(prev_weight_delta * dec_learning_rate_, min_learning_rate_);
					biases[node_idx] -= prev_bias_deltas[node_idx];
					bias_dEdw_off[node_idx] = 0.;
				}
				else
				{
					delta = prev_weight_delta;
					biases[node_idx] -= std::copysign(delta, bias_dEdw_off[node_idx]);
				}
				assert(delta >= 0.);
				prev_bias_deltas[node_idx] = delta;
				prev_bias_dEdw_off[node_idx] = bias_dEdw_off[node_idx];
			}
		}
	}
	//-----------------------------------------------------------------------------------------------------
	template <class Network>
	void RProp<Network>::_splitData()
	{
		constexpr double train_frac = 0.8;

		teaching_set_.clear();
		test_set_.clear();

		const size_t src_size = src_data_.shape().first;
		const size_t teaching_size = src_size * train_frac;
		if (!teaching_size || teaching_size > src_size)
		{
			return;
		}
		teaching_set_.reserve(teaching_size);
		test_set_.reserve(src_size - teaching_size);
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_int_distribution<size_t> distribution(0, src_size - 1);

		for (size_t i = 0; i < src_size; ++i)
		{
			if (distribution(gen) < teaching_size)
			{
				teaching_set_.push_back(i);
			}
			else
			{
				test_set_.push_back(i);
			}
		}
	}
	//-----------------------------------------------------------------------------------------------------
	template <class Network>
	double RProp<Network>::_meanSqrError(Network& network)
	{
		if (test_set_.empty())
		{
			return 0.;
		}

		double sqr_err_sum = 0.;
		for (const size_t row : test_set_)
		{
			sample_filler_.fill(row, network.inputLayer(), sample_targets_);
			network.forward();
			auto target_i = cbegin(sample_targets_);
			for (auto omega : network.omegaLayer())
			{
				const double err = (omega - *target_i);
				sqr_err_sum += err * err;
				++target_i;
			}
		}
		return sqr_err_sum / test_set_.size();
	}
	//-----------------------------------------------------------------------------------------------------
	template <class Network>
	template <class ProgressView>
	void RProp<Network>::teach(const typename Network::Config& cfg, Network& network, double min_err, ProgressView& progress_view)
	{
		constexpr __int64 epochs_max = 10000;

		progress_view.begin_teach();

		Network best_network;

		_init(cfg);

		double err = std::numeric_limits<double>::max();
		double cur_min_err = std::numeric_limits<double>::max();

		__int64 epochs_left = epochs_max;

		while (epochs_left-- && err > min_err && !progress_view.stop_requested())
		{
			_splitData();

			dEdw_off_->reset(0.);
			bias_dEdw_off_->reset(0.);

			const size_t teaching_size = teaching_set_.size();
			for (size_t row : teaching_set_)
			{
				sample_filler_.fill(row, network.inputLayer(), sample_targets_);
				network.forward();
				_updateGradients(network, sample_targets_);
			}
			_updateWeights(network);
			_updateBiases(network);
			err = _meanSqrError(network);

			if (cur_min_err > err)
			{
				cur_min_err = err;
				best_network = network;
			}

			progress_view.set_best(cur_min_err);
			progress_view.set_last(err);
			progress_view.set_epoch(epochs_max - epochs_left);
		}

		network = best_network;

		progress_view.end_teach();
	}
	//-----------------------------------------------------------------------------------------------------
	template <class Network>
	template <class ProgressView>
	void RProp<Network>::show_test(Network& network, ProgressView& progress_view)
	{
		progress_view.begin_test();

		size_t true_count = 0;
		size_t false_count = 0;

		for (size_t row : test_set_)
		{
			sample_filler_.fill(row, network.inputLayer(), sample_targets_);
			network.forward();
			auto target_i = cbegin(sample_targets_);
			for (auto omega : network.omegaLayer())
			{
				progress_view.add_sample_result(omega, *target_i);
				//if (omega * *target_i > 0. || omega == 0. && *target_i == 0.)
				if (omega * *target_i > 0.)
				{
					++true_count;
				}
				else if (omega * *target_i < 0.)
				{
					++false_count;
				}
				++target_i;
			}
		}
		progress_view.end_test(true_count, false_count);
	}
}