#pragma once

#include <vector>
#include <memory>
#include <iterator>
#include <algorithm>
#include <cassert>
#include <iostream>
#include <iomanip>
#include <cmath>
#include <random>
#include <Shared/Neuro/activation.hpp>
#include <Shared/LibIncludes/IncludeJson.h>

namespace wtom::ml::neuro::net
{
	//-----------------------------------------------------------------------------------------------------
	// class Config
	//-----------------------------------------------------------------------------------------------------
	class Config
	{
		std::vector<size_t> layers_sizes_;

	public:
		static constexpr double initial_weight_min_ = -0.1; //0.01;
		static constexpr double initial_weight_max_ = 0.1;

		explicit Config(const std::vector<size_t>& layers_sizes)
			: layers_sizes_(layers_sizes)
		{
			assert(layers_sizes_.size() >= 2);
		}

		const std::vector<size_t>& layersSizes() const noexcept { return layers_sizes_; }
	};
	//-----------------------------------------------------------------------------------------------------
	// class MultilayerFeedforward
	//-----------------------------------------------------------------------------------------------------
	class MultilayerFeedforward
	{
	// types
	public:
		using Config = wtom::ml::neuro::net::Config;

		struct Outputs
		{
		private:
			static constexpr size_t hidden_func_count_ = 3;

			activation::Identity	identity_func_;
			activation::Sigma		sigma_func_;
			activation::Tanh		tanh_func_;
			activation::Gaussian	gaussian_func_;
		public:
			std::vector<std::vector<double>> layers_;
			std::vector<std::vector<activation::Function*>> functions_;

			Outputs() = default;
			Outputs(const Config& cfg, double def);
			void reset(double val);
		};
		struct Biases
		{
			std::vector<std::vector<double>> layers_;

			Biases() = default;
			Biases(const Outputs& cfg, double rand_min, double rand_max);
			void reset(double val);
		};
		struct Weights
		{
			using Dest = std::vector<double>; // from one source node to all destination nodes
			using SourceToDestMatrix = std::vector<Dest>;

			std::vector<SourceToDestMatrix> layers_;

			Weights() = default;
			Weights(const Outputs& cfg, double def);
			Weights(const Outputs& cfg, double rand_min, double rand_max);
			void reset(double val);
		};

	// data
	private:
		Outputs	outputs_;
		Biases	biases_;
		Weights	weights_;

	// methods
	public:
		MultilayerFeedforward() = default;
		explicit MultilayerFeedforward(const Config& cfg);
		~MultilayerFeedforward();

		void construct(const Config& cfg);

		std::vector<double>& inputLayer() { return outputs_.layers_.front(); }
		const std::vector<double>& omegaLayer() const noexcept { return outputs_.layers_.back(); }
		double output(size_t layer, size_t node) const noexcept { return outputs_.layers_[layer][node]; }
		const std::vector<std::vector<double>>& outputs() const noexcept { return outputs_.layers_; }
		const std::vector<std::vector<activation::Function*>>& functions() const noexcept { return outputs_.functions_; }
		std::vector<std::vector<double>>& biases() noexcept { return biases_.layers_; }
		const std::vector<std::vector<double>>& biases() const noexcept { return biases_.layers_; }
		double weight(size_t layer, size_t src_node, size_t dst_node) const noexcept { return weights_.layers_[layer][src_node][dst_node]; }
		const std::vector<Weights::SourceToDestMatrix>& weights() const noexcept { return weights_.layers_; }
		std::vector<Weights::SourceToDestMatrix>& weights() noexcept { return weights_.layers_; }
		void forward();

		friend std::ostream& operator << (std::ostream& strm, const MultilayerFeedforward& network);
		friend std::istream& operator >> (std::istream& strm, MultilayerFeedforward& network);
	};

	//-----------------------------------------------------------------------------------------------------
	inline MultilayerFeedforward::Outputs::Outputs(const Config& cfg, double initial_value)
		: layers_(cfg.layersSizes().size())
		, functions_(cfg.layersSizes().size() - 1)
	{
		layers_.front().resize(cfg.layersSizes().front(), initial_value);
		layers_.back().resize(cfg.layersSizes().back(), initial_value);
		functions_.back().resize(cfg.layersSizes().back(), &tanh_func_);
		for (size_t i = 1; i < cfg.layersSizes().size() - 1; ++i)
		{
			layers_[i].resize(cfg.layersSizes()[i] * hidden_func_count_, initial_value);
			functions_[i - 1].resize(cfg.layersSizes()[i] * hidden_func_count_, nullptr);
			for (size_t aidx = 0; aidx < cfg.layersSizes()[i]; ++aidx)
			{
				functions_[i - 1][aidx] = &identity_func_;
				functions_[i - 1][aidx + cfg.layersSizes()[i]] = &sigma_func_;
				functions_[i - 1][aidx + (cfg.layersSizes()[i] << 1)] = &gaussian_func_;
			}
		}
	}
	//-----------------------------------------------------------------------------------------------------
	inline MultilayerFeedforward::~MultilayerFeedforward()
	{
	}
	//-----------------------------------------------------------------------------------------------------
	inline void MultilayerFeedforward::Outputs::reset(double val)
	{
		for (auto& layer : layers_)
		{
			std::fill(std::begin(layer), std::end(layer), val);
		}
	}
	//-----------------------------------------------------------------------------------------------------
	inline MultilayerFeedforward::Biases::Biases(const Outputs& cfg, double rand_min, double rand_max)
		: layers_(cfg.layers_.size())
	{
		std::random_device rd;
		std::mt19937 rand(rd());
		const double rand_factor = (rand_max - rand_min) / (rand.max() - rand.min());

		layers_.front().resize(cfg.layers_.front().size(), 0.); // input layer isn't used for now
		for (size_t i = 1; i < cfg.layers_.size(); ++i)
		{
			layers_[i].resize(cfg.layers_[i].size()); // 1 chunk for identity and 1 chunk for tanh
			for (auto& hl_val : layers_[i])
			{
				hl_val = rand_min + (rand() - rand.min()) * rand_factor;
			}
		}
	}
	//-----------------------------------------------------------------------------------------------------
	inline void MultilayerFeedforward::Biases::reset(double val)
	{
		for (auto& layer : layers_)
		{
			std::fill(std::begin(layer), std::end(layer), val);
		}
	}
	//-----------------------------------------------------------------------------------------------------
	inline MultilayerFeedforward::Weights::Weights(const Outputs& cfg, double def)
		: layers_(cfg.layers_.size() - 1)
	{
		auto hls = std::cbegin(cfg.layers_);
		auto dest_hls = hls;
		++dest_hls;
		std::for_each(std::begin(layers_), std::end(layers_),
			[&hls, &dest_hls, def](auto& hl)
			{
				hl.resize((hls++)->size(), Dest((dest_hls++)->size(), def));
			}
		);
	}
	//-----------------------------------------------------------------------------------------------------
	inline MultilayerFeedforward::Weights::Weights(const Outputs& cfg, double rand_min, double rand_max)
		: layers_(cfg.layers_.size() - 1)
	{
		std::random_device rd;
		std::mt19937 rand(rd());
		const double rand_factor = (rand_max - rand_min) / (rand.max() - rand.min());

		auto hls = std::cbegin(cfg.layers_);
		auto dest_hls = hls;
		++dest_hls;
		std::for_each(std::begin(layers_), std::end(layers_),
			[&hls, &dest_hls, &rand, rand_factor, rand_min](auto& hl)
			{
				hl.resize((hls++)->size(), Dest((dest_hls++)->size()));
				for (auto& hl_val : hl)
				{
					for (auto& dest_val : hl_val)
					{
						dest_val = rand_min + (rand() - rand.min()) * rand_factor;
					}
				}
			}
		);
	}
	//-----------------------------------------------------------------------------------------------------
	inline void MultilayerFeedforward::Weights::reset(double val)
	{
		for (auto& layer : layers_)
		{
			for (auto& dest : layer)
			{
				std::fill(std::begin(dest), std::end(dest), val);
			}
		}
	}
	//-----------------------------------------------------------------------------------------------------
	inline MultilayerFeedforward::MultilayerFeedforward(const Config& cfg)
		: outputs_(cfg, 0.)
		, biases_(outputs_, cfg.initial_weight_min_, cfg.initial_weight_max_)
		, weights_(outputs_, cfg.initial_weight_min_, cfg.initial_weight_max_)
	{
	}
	//-----------------------------------------------------------------------------------------------------
	inline void MultilayerFeedforward::forward()
	{
		auto src_layer_i = std::begin(outputs_.layers_);
		auto func_layer_i = std::begin(outputs_.functions_); // activation functions for dest layer
		auto dst_layer_i = src_layer_i;
		++dst_layer_i;
		auto dst_layer_e = std::end(outputs_.layers_);

		auto weights_layer_i = std::begin(weights_.layers_);
		auto bias_layer_i = std::begin(biases_.layers_);
		++bias_layer_i;

		for (; dst_layer_i != dst_layer_e;
			++src_layer_i, ++dst_layer_i, ++weights_layer_i, ++bias_layer_i, ++func_layer_i)
		{
			std::vector<double>& src_outputs = *src_layer_i;
			std::vector<double>& dst_outputs = *dst_layer_i;
			std::vector<double>& biases = *bias_layer_i;
			std::vector<activation::Function*>& funcs = *func_layer_i;
			const Weights::SourceToDestMatrix& weights = *weights_layer_i;

			for (size_t dst_idx = 0; dst_idx < dst_outputs.size(); ++dst_idx)
			{
				double net = 0.;
				for (size_t src_idx = 0; src_idx < src_outputs.size(); ++src_idx)
				{
					net += src_outputs[src_idx] * weights[src_idx][dst_idx];
				}
				net += biases[dst_idx];
				dst_outputs[dst_idx] = funcs[dst_idx]->activation(net);
			}
		}
	}
	////---------------------------------------------------------------------------------------------------------
	//inline void to_json(json& j, const MultilayerFeedforward::Biases& src)
	//{
	//	j = json{src.layers_};
	//}
	////---------------------------------------------------------------------------------------------------------
	//inline void from_json(const json& j, MultilayerFeedforward::Biases& dst)
	//{
	//	dst.layers_ = j.get<decltype(dst.layers_)>();
	//}
	////---------------------------------------------------------------------------------------------------------
	//inline void to_json(json& j, const MultilayerFeedforward::Weights& src)
	//{
	//	j = json{src.layers_};
	//}
	////---------------------------------------------------------------------------------------------------------
	//inline void from_json(const json& j, MultilayerFeedforward::Weights& dst)
	//{
	//	dst.layers_ = j.get<decltype(dst.layers_)>();
	//}
	//---------------------------------------------------------------------------------------------------------
	inline void to_json(json& j, const MultilayerFeedforward& src)
	{
		j = json{
			{ "biases", src.biases() },
			{ "weights", src.weights() }
		};
	}
	//---------------------------------------------------------------------------------------------------------
	inline void from_json(const json& j, MultilayerFeedforward& dst)
	{
		dst.biases() = j.get<std::vector<std::vector<double>>>();
		dst.weights() = j.get<std::vector<MultilayerFeedforward::Weights::SourceToDestMatrix>>();
	}
	//-----------------------------------------------------------------------------------------------------
	inline std::ostream& operator << (std::ostream& strm, const MultilayerFeedforward& network)
	{
		//strm << std::setprecision(12) << std::fixed;

		strm << "biases\r\n";
		for (auto& bias_layer : network.biases_.layers_)
		{
			for (auto& bias : bias_layer)
			{
				strm << "\t\t" << bias;
			}
			strm << "\r\n";
		}

		size_t layer_idx = 0;
		for (auto& layer : network.weights_.layers_)
		{
			strm << "weights(src\\dst) " << layer_idx << "-" << layer_idx + 1 << "\r\n";
			for (auto& src : layer)
			{
				for (auto& dst : src)
				{
					strm << "\t\t" << dst;
				}
				strm << "\r\n";
			}
			++layer_idx;
		}
		return strm;
	}
	//-----------------------------------------------------------------------------------------------------
	inline std::istream& operator >> (std::istream& strm, MultilayerFeedforward& network)
	{
		// biases
		for (auto& bias_layer : network.biases_.layers_)
		{
			for (auto& bias : bias_layer)
			{
				strm >> bias;
			}
		}

		// weights
		for (auto& layer : network.weights_.layers_)
		{
			for (auto& src : layer)
			{
				for (auto& dst : src)
				{
					strm >> dst;
				}
			}
		}
		return strm;
	}
}