#ifndef DATA_VALUE_GENERATOR_H
#define DATA_VALUE_GENERATOR_H

#include <numeric>
#include "data/dataValue.h"

#include "mutator/rng.h"

namespace Dimensions {
    /**
     * \brief Abstract class representing a DataValueGenerator.
     * 
     * Contains a pure virtual method sample, which samples a DataValue based on the generator.
     */
    class DataValueGenerator {

        public:
            /**
             * \brief Sample method creating a DataValue based on the random number generator.
             * 
             * The method is purpusely not const, as the generator may have internal state that is modified during sampling.
             */
            virtual Data::DataValue sample(RNG::RNG& rng) = 0;

            /**
             * \brief dupplicate the current DataValueGenerator as a unique pointer
             */
            virtual std::unique_ptr<DataValueGenerator> cloneUniquePtr() const = 0;
    };

    /**
     * \brief Class generating a DataValue of numeric type T sampled unfiromly within a specified range.
     */
    template <typename T>
    class NumericUniformGenerator : public DataValueGenerator {
        private:
            T min;
            T max;

        public:
            /**
             * \brief Constructor for NumericUniformGenerator.
             * 
             * \param[in] min the lower bound for the generated values.
             * \param[in] max the upper bound for the generated values.
             */
            NumericUniformGenerator(T min, T max) : min(min), max(max) {}

            /**
             * \brief Sample method creating a DataValue of type T within the specified range.
             */
            virtual Data::DataValue sample(RNG::RNG& rng) override {
                return Data::DataValue::scalar<T>(rng.uniformSample<T>(min, max));
            }

            /// @brief Inherrit from DataValueGenerator class 
            virtual std::unique_ptr<DataValueGenerator> cloneUniquePtr() const override {
                return std::make_unique<NumericUniformGenerator<T>>(this->min, this->max);
            }
    };

    /**
     * \brief class generating a DataValue of any type T sampled uniformly from a list of possible values.
     * 
     * All values must have the same type T
     */
    template <typename T>
    class ListUniformGenerator : public DataValueGenerator {
        protected:
            /// \brief Possible values to sample
            const std::vector<T>& values;

        public:
            /**
             * \brief Constructor for ListUniformGenerator.
             * 
             * \param[in] values a vector of references to the possible values to sample from.
             */
            ListUniformGenerator(const std::vector<T>& values) : values(values) {}

            /**
             * \brief Sample method creating a DataValue of type T from the list of possible values.
             */
            virtual Data::DataValue sample(RNG::RNG& rng) override {
                if(this->values.size() == 0) {
                    throw std::runtime_error("Dimensions::ListUniformGenerator::sample: Sampling value failed because the vector is empty.");
                }
                return Data::DataValue::scalar<T>(values[rng.uniformSample<size_t>(0, values.size() - 1)]);
            }

            /// @brief Inherrit from DataValueGenerator class 
            virtual std::unique_ptr<DataValueGenerator> cloneUniquePtr() const override {
                return std::make_unique<ListUniformGenerator<T>>(values);
            }
    };

    /**
     * \brief Class storing multiple DataValueGenerator stored as unique pointer, alongside a weight.
     * 
     * The sample chooses a dataValueGenerator based on the weights and sample from it.
     */
    class MultiGenerator : public DataValueGenerator {
        protected: 
            /// \brief vector of the generators
            std::vector<std::unique_ptr<DataValueGenerator>> generators;

            /// \brief weights of generators
            std::vector<double> weights;

        public:

            /// @brief Default empty constructor
            MultiGenerator() = default;

            /**
             * \brief constructor with predefined list of generators and weights
             */
            MultiGenerator(const std::vector<std::reference_wrapper<const DataValueGenerator>>& generators, const std::vector<double>& weights) : weights{weights} {
                if(generators.size() != weights.size()) {
                    throw std::runtime_error("MultiGenerator: Generators and weights should be the same");
                }

                for(size_t idx = 0; idx < generators.size(); idx++) {
                    if(weights.at(idx) < 0.0) {
                        throw std::runtime_error("MultiGenerator: Weights should be positive.");  
                    }
                    this->generators.push_back(generators.at(idx).get().cloneUniquePtr());
                }
            }
            
            /**
             * \brief Add a generator to the list of generators alongside a positive weight.
             * 
             * \param[in] generator The generator to add
             * \param[in] weight The weight associated to the generator. default is 1.0
             */
            void addGenerator(const DataValueGenerator& generator, double weight = 1.0) {
                if(weight < 0) {
                    throw std::runtime_error("MultiGenerator:addGenerator: Weight should be positive.");  
                }
                this->generators.push_back(generator.cloneUniquePtr());
                weights.push_back(weight);
            }

            /**
             * \brief Sample a value by randomly selecting a generator based on the weights.
             */
            virtual Data::DataValue sample(RNG::RNG& rng) override {
                const double totalWeight = std::accumulate(weights.begin(), weights.end(), 0.0);

                const double randomValue =
                    rng.uniformSample<double>(0.0, totalWeight);

                double cumulativeWeight = 0.0;
                size_t index = 0;
                for(;cumulativeWeight < randomValue; index++) {
                    cumulativeWeight += weights[index];
                }
                return generators.at(index - 1)->sample(rng);
            }

            
            /// @brief Inherrit from DataValueGenerator class 
            virtual std::unique_ptr<DataValueGenerator> cloneUniquePtr() const override {
                std::unique_ptr<MultiGenerator> clone = std::make_unique<MultiGenerator>();
                for(size_t idx = 0; idx < this->generators.size(); idx++) {
                    clone->addGenerator(*this->generators.at(idx), this->weights.at(idx));
                }
                return std::move(clone);
            }
    };
};

#endif // DATA_VALUE_GENERATOR_H