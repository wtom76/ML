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
#include <neuro/Network/mfn.hpp>
#include <neuro/DataUtils/DataSeries.hpp>

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
		using GradTerms = typename Network::Outputs;
		using WeightGrads = typename Network::Weights;
		using BiasGrads = typename Network::Outputs;
		using WeightDeltas = typename Network::Weights;
		using BiasDeltas = typename Network::Outputs;
		using TeachingData = DataUtils::TeachingData;

	// data
	private:
		static constexpr double initial_learning_rate_ = 0.1;
		static constexpr double min_learning_rate_ = 1e-6;
		static constexpr double max_learning_rate_ = 50;
		static constexpr double inc_learning_rate_ = 1.2;
		static constexpr double dec_learning_rate_ = 0.5;

		std::unique_ptr<GradTerms> grad_terms_;
		std::unique_ptr<WeightGrads> dEdw_off_;
		std::unique_ptr<WeightGrads> prev_dEdw_off_;
		std::unique_ptr<BiasGrads> bias_dEdw_off_;
		std::unique_ptr<BiasGrads> prev_bias_dEdw_off_;
		std::unique_ptr<WeightDeltas> prev_weight_deltas_;
		std::unique_ptr<BiasDeltas> prev_bias_deltas_;

		std::random_device rd_;
		std::mt19937 rand_;
		TeachingData src_data_;
		TeachingData teaching_data_;
		TeachingData test_data_;

	// methods
	private:
		void _init(const typename Network::Config& cfg);
		void _updateGradients(Network& network, const std::vector<double>& targets);
		void _updateWeights(Network& network);
		void _updateBiases(Network& network);
		void _splitData(TeachingData& teaching_data, TeachingData& test_data);
		double _meanSqrError(Network& network, const TeachingData& test_data) const;
	public:
		explicit RProp(TeachingData&& src_data)
			: rand_(rd_())
			, src_data_(std::move(src_data))
		{}

		void teach(const typename Network::Config& cfg, Network& network, double min_err, bool& stop);
		void showTest(Network& network, const DataUtils::DataSeries& normalized_series) const;
	};

	//-----------------------------------------------------------------------------------------------------
	template <class Network>
	void RProp<Network>::_init(const typename Network::Config& cfg)
	{
		grad_terms_ = std::make_unique<GradTerms>(cfg, 0.);
		dEdw_off_ = std::make_unique<WeightGrads>(*grad_terms_, 0.);
		prev_dEdw_off_ = std::make_unique<WeightGrads>(*grad_terms_, 0.);
		bias_dEdw_off_ = std::make_unique<BiasGrads>(cfg, 0.);
		prev_bias_dEdw_off_ = std::make_unique<BiasGrads>(cfg, 0.);
		prev_weight_deltas_ = std::make_unique<WeightDeltas>(*grad_terms_, initial_learning_rate_);
		prev_bias_deltas_ = std::make_unique<BiasDeltas>(cfg, initial_learning_rate_);
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
	void RProp<Network>::_splitData(TeachingData& teaching_data, TeachingData& test_data)
	{
		std::shuffle(begin(src_data_), end(src_data_), rand_);
		auto teaching_e = begin(src_data_);
		std::advance(teaching_e, static_cast<size_t>(src_data_.size() * 0.8));
		teaching_data.assign(begin(src_data_), teaching_e);
		test_data.assign(teaching_e, end(src_data_));
	}
	//-----------------------------------------------------------------------------------------------------
	template <class Network>
	double RProp<Network>::_meanSqrError(Network& network, const TeachingData& test_data) const
	{
		if (test_data.empty())
		{
			return 0.;
		}

		double sqr_err_sum = 0.;
		for (const auto& sample : test_data)
		{
			std::copy(cbegin(sample.inputs_), cend(sample.inputs_), begin(network.inputLayer()));
			network.forward();
			auto target_i = cbegin(sample.targets_);
			for (auto omega : network.omegaLayer())
			{
				const double err = (omega - *target_i);
				sqr_err_sum += err * err;
				++target_i;
			}
		}
		return sqr_err_sum / test_data.size();
	}
	//-----------------------------------------------------------------------------------------------------
	template <class Network>
	void RProp<Network>::teach(const typename Network::Config& cfg, Network& network, double min_err, bool& stop)
	{
		Network best_network;

		_init(cfg);

		double err = std::numeric_limits<double>::max();
		double cur_min_err = std::numeric_limits<double>::max();

		do
		{
			__int64 epochs_left = 1000;

			while (--epochs_left && err > min_err && !stop)
			{
				_splitData(teaching_data_, test_data_);

				dEdw_off_->reset(0.);
				bias_dEdw_off_->reset(0.);

				for (const auto& sample : teaching_data_)
				{
					std::copy(cbegin(sample.inputs_), cend(sample.inputs_), begin(network.inputLayer()));
					network.forward();
					_updateGradients(network, sample.targets_);
				}

				_updateWeights(network);
				_updateBiases(network);
				err = _meanSqrError(network, test_data_);

				if (cur_min_err > err)
				{
					cur_min_err = err;
					best_network = network;
				}
			}
			std::cout << "**** current best " << cur_min_err << " ****\r\n";
			std::cout << best_network << std::endl;
			std::cout << "**** last " << err << " ****\r\n";
			std::cout << network << std::endl;
		}
		while (err > min_err && !stop);

		network = best_network;
	}
	//-----------------------------------------------------------------------------------------------------
	template <class Network>
	void RProp<Network>::showTest(Network& network,
		const DataUtils::DataSeries& normalized_series) const
	{
		std::cout << "test\r\n";
		size_t true_side_count = 0;
		size_t false_side_count = 0;
		for (const auto& sample : test_data_)
		{
			std::copy(cbegin(sample.inputs_), cend(sample.inputs_), begin(network.inputLayer()));
			network.forward();
			auto target_i = cbegin(sample.targets_);
			for (auto omega : network.omegaLayer())
			{
				const double err = (omega - *target_i);
				const double restored_omega = normalized_series.restore(omega);
				const double restored_target = normalized_series.restore(*target_i);

				std::cout
					<< "o: " << omega << "\tt: " << *target_i
					<< "\too: " << restored_omega << "\tot: " << restored_target
					<< "\te: " << err << "\tse: " << err * err << std::endl;
				++target_i;

				if (restored_omega * restored_target > 0. || restored_omega == 0. && restored_target == 0.)
				{
					++true_side_count;
				}
				else
				{
					++false_side_count;
				}
			}
		}
		std::cout << "true_side_count: " << true_side_count << "\tfalse_side_count: " << false_side_count << std::endl;
	}
}