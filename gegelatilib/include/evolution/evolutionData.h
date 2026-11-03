#ifndef EVOLUTION_DATA_H
#define EVOLUTION_DATA_H

#include <any>
#include <cstdint>
#include <functional>
#include <limits>
#include <map>
#include <memory>
#include <set>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

#include "data/demangle.h"
#include "evolution/key.h"
#include "individual.h"
#include "metrics/metricMap.h"
#include "rng/rng.h"

namespace Evolution {

    /**
     * \brief Create an index of individuals by their ID.
     *
     * \param[in] individuals the indexed individuals.
     */
    std::unordered_map<uint64_t, std::shared_ptr<const Individual>> makeIndex(const std::vector<std::shared_ptr<const Individual>>& individuals);

    /**
     * \brief The data of an evolution: what the modules of an evolution produce and exchange.
     *
     * It is shared by all the modules of an evolution. Apart from the generation counters and the random number
     * generator, everything is a data identified by a typed Key: individuals of the populations,
     * parents, offspring, metrics, fitness, and any data the user modules exchange. The data are only accessed through
     * keys. Data that only one module needs (e.g. its parameters) should stay inside that module.
     *
     * Each data records the generation in which it was stored (see isUpToDate), so a reader can tell whether a
     * result is from the current generation. Modifying a data in place keeps its generation. summary() lists
     * the data with their size and generation.
     */
    struct EvolutionData {
        /// Generation number used while the initialization steps are running.
        static constexpr uint64_t initGeneration = std::numeric_limits<uint64_t>::max();

        /// Index of the generation currently running, or initGeneration during the initialization.
        uint64_t generation = initGeneration;

        /// Number of generations already done since the initialization.
        uint64_t nbGenerationsDone = 0;

        /// Random number generator of the evolution.
        RNG::RNG rng;

        /// Identifiers of the steps whose initialization was run on this state (see Module::initialize).
        std::set<uint64_t> initializedSteps;

        /**
         * \brief Constructor.
         *
         * \param[in] seed the seed of the random number generator.
         */
        explicit EvolutionData(uint64_t seed = 0) { this->rng.setSeed(seed); }

        /**
         * \brief Empty the state to start a new evolution, data included.
         *
         * \param[in] seed the seed of the random number generator.
         */
        void reset(uint64_t seed);

        /// Return the names of the data of the state.
        std::set<std::string> dataNames() const;

        /**
         * \brief Return the individuals stored under a key, as individuals that cannot be modified.
         *
         * \param[in] key the key of the individuals (e.g. the individuals of a population).
         * \return the individuals, empty if there is no data.
         */
        std::vector<std::shared_ptr<const Individual>> getIndividuals(const Key<std::vector<std::shared_ptr<Individual>>>& key) const;

        /**
         * \brief Return the individuals of several lists, concatenated in the order of the keys.
         *
         * \param[in] keys the keys of the individuals.
         */
        std::vector<std::shared_ptr<const Individual>> getIndividuals(const std::vector<Key<std::vector<std::shared_ptr<Individual>>>>& keys) const;

        /**
         * \brief Return the best individual of a list according to a fitness.
         *
         * The best individual has the highest fitness, then the lowest ID.
         * The fitness may be older than the current generation (e.g. a periodic validation), see isUpToDate.
         *
         * \param[in] individuals key of the individuals ranked.
         * \param[in] fitness key of the fitness used.
         * \return the best individual, or nullptr if there is no such fitness or individuals.
         */
        std::shared_ptr<const Individual> bestIndividual(const Key<std::vector<std::shared_ptr<Individual>>>& individuals,
                                                         const Key<std::map<uint64_t, double>>& fitness) const;

        /**
         * \brief Store a data, replacing the previous one, with the current generation.
         *
         * \tparam T type of the data, given by the key.
         * \param[in] key the key of the data.
         * \param[in] value the stored value, shared with the caller.
         */
        template <typename T> void set(const Key<T>& key, std::shared_ptr<T> value)
        {
            const std::shared_ptr<T> shared = std::move(value);
            this->data[key.name()] = Entry{shared, this->generation, [shared] { return (shared == nullptr) ? std::string("null") : describeValue(*shared, 0); }};
        }

        /**
         * \brief Return the data stored under a key.
         *
         * \tparam T type of the data, given by the key.
         * \param[in] key the key of the data.
         * \throw std::runtime_error if there is no data.
         */
        template <typename T> std::shared_ptr<T> get(const Key<T>& key)
        {
            return this->find<T>(key.name());
        }

        /// See get.
        template <typename T> std::shared_ptr<const T> get(const Key<T>& key) const
        {
            return this->find<T>(key.name());
        }

        /**
         * \brief Return the data stored under a key, or null if there is none.
         *
         * \tparam T type of the data, given by the key.
         * \param[in] key the key of the data.
         */
        template <typename T> std::shared_ptr<T> getIfAny(const Key<T>& key)
        {
            return this->has(key) ? this->find<T>(key.name()) : nullptr;
        }

        /// See getIfAny.
        template <typename T> std::shared_ptr<const T> getIfAny(const Key<T>& key) const
        {
            return this->has(key) ? this->find<T>(key.name()) : nullptr;
        }

        /**
         * \brief Return true if a data is stored under the key.
         *
         * \param[in] key the key of the data.
         */
        template <typename T> bool has(const Key<T>& key) const
        {
            return this->data.find(key.name()) != this->data.end();
        }

        /**
         * \brief Remove the data stored under the key, if any.
         *
         * \param[in] key the key of the data.
         */
        template <typename T> void erase(const Key<T>& key)
        {
            this->data.erase(key.name());
        }

        /**
         * \brief Return true if the data was stored during the current generation.
         *
         * \param[in] key the key of the data.
         * \return false if there is no data.
         */
        template <typename T> bool isUpToDate(const Key<T>& key) const
        {
            auto it = this->data.find(key.name());
            return it != this->data.end() && it->second.generation == this->generation;
        }

        /**
         * \brief Return a printable table of the data: name, size or value, and generation in which it was stored.
         *
         * Data stored during the current generation are "up to date", the others show their generation.
         */
        std::string summary() const;

      private:
        /// Data stored by the steps, with the generation in which it was stored.
        struct Entry {
            /// The value, a std::shared_ptr<T>.
            std::any value;

            /// Generation in which the value was stored.
            uint64_t generation;

            /// Describe the current content of the value (its size, or its value for a number) for summaries.
            std::function<std::string()> describe;
        };

        /// Describe a value that has a size, e.g. a list or a map.
        template <typename T> static auto describeValue(const T& value, int) -> decltype(value.size(), std::string())
        {
            return std::to_string(value.size()) + " elements";
        }

        /// Describe any other value: its content if it is a number, nothing otherwise.
        template <typename T> static std::string describeValue(const T& value, long)
        {
            if constexpr(std::is_arithmetic_v<T>) {
                return std::to_string(value);
            } else {
                (void)value;
                return "";
            }
        }

        /// Data stored by the steps, by name of their key.
        std::map<std::string, Entry> data;

        /// Return the data with a name, checking its existence and type.
        template <typename T> const std::shared_ptr<T>& find(const std::string& name) const
        {
            auto it = this->data.find(name);
            if(it == this->data.end()) {
                throw std::runtime_error("Evolution::EvolutionData::get: no data named " + name);
            }
            const std::shared_ptr<T>* value = std::any_cast<std::shared_ptr<T>>(&it->second.value);
            if(value == nullptr) {
                throw std::runtime_error("Evolution::EvolutionData::get: data " + name + " is not a " + Data::demangle(typeid(T).name()));
            }
            return *value;
        }
    };

}; // namespace Evolution

#endif // EVOLUTION_DATA_H
