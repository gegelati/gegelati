#ifndef EVOLUTION_STATE_H
#define EVOLUTION_STATE_H

#include <any>
#include <cstdint>
#include <limits>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "data/demangle.h"
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
     * \brief Everything an evolution knows at a given time.
     *
     * The state is shared by all the steps of a Module. Apart from the generation counters, the random number
     * generator and the step times, everything is an entry stored under a name: individuals of the populations, parents,
     * offspring, metrics, fitness, and any data the user steps exchange. Data that only one step needs (e.g. its
     * parameters) should stay inside that step.
     *
     * Each entry records the generation in which it was stored (see isUpToDate), so a reader can tell whether a
     * result is from the current generation. Modifying an entry in place keeps its generation.
     */
    struct EvolutionState {
        /// Generation number used while the initialization steps are running.
        static constexpr uint64_t initGeneration = std::numeric_limits<uint64_t>::max();

        /// Index of the generation currently running, or initGeneration during the initialization.
        uint64_t generation = initGeneration;

        /// Number of generations already done since the initialization.
        uint64_t nbGenerationsDone = 0;

        /// Time in seconds since the beginning of the evolution, updated before and after each step.
        double elapsedSeconds = 0.0;

        /// Random number generator of the evolution.
        RNG::RNG rng;

        /// Name and duration in seconds of the steps already run in the current generation.
        std::vector<std::pair<std::string, double>> stepTimes;

        /**
         * \brief Empty the state to start a new evolution, entries included.
         *
         * \param[in] seed the seed of the random number generator.
         */
        void reset(uint64_t seed);

        /**
         * \brief Return the individuals stored under a name, either a list of individuals or a list of mutable individuals.
         *
         * \param[in] name the name of the individuals (e.g. population->individuals()).
         * \return the individuals, empty if there is no entry.
         * \throw std::runtime_error if the entry is not a list of individuals.
         */
        std::vector<std::shared_ptr<const Individual>> getIndividuals(const std::string& name) const;

        /**
         * \brief Return the best individual of a list according to a fitness.
         *
         * Individuals are ranked as the selectors do: highest fitness first, then lowest ID.
         * The fitness may be older than the current generation (e.g. a periodic validation), see isUpToDate.
         *
         * \param[in] individuals name of the individuals ranked.
         * \param[in] fitness name of the fitness used.
         * \return the best individual, or nullptr if there is no such fitness or individuals.
         */
        std::shared_ptr<const Individual> bestIndividual(const std::string& individuals, const std::string& fitness) const;

        /**
         * \brief Store an entry, replacing the previous one, with the current generation.
         *
         * \param[in] name the name of the entry.
         * \param[in] value the stored value, shared with the caller.
         */
        template <typename T> void set(const std::string& name, std::shared_ptr<T> value)
        {
            this->entries[name] = Entry{std::move(value), this->generation};
        }

        /**
         * \brief Return the entry stored under a name.
         *
         * \param[in] name the name of the entry.
         * \throw std::runtime_error if there is no entry, or if it is not a T.
         */
        template <typename T> std::shared_ptr<T> get(const std::string& name)
        {
            return this->find<T>(name);
        }

        /// See get.
        template <typename T> std::shared_ptr<const T> get(const std::string& name) const
        {
            return this->find<T>(name);
        }

        /**
         * \brief Return the entry stored under a name, or null if there is none.
         *
         * \param[in] name the name of the entry.
         * \throw std::runtime_error if the entry is not a T.
         */
        template <typename T> std::shared_ptr<T> getIfAny(const std::string& name)
        {
            return this->has(name) ? this->find<T>(name) : nullptr;
        }

        /// See getIfAny.
        template <typename T> std::shared_ptr<const T> getIfAny(const std::string& name) const
        {
            return this->has(name) ? this->find<T>(name) : nullptr;
        }

        /**
         * \brief Return true if an entry is stored under the name.
         *
         * \param[in] name the name of the entry.
         */
        bool has(const std::string& name) const;

        /**
         * \brief Remove the entry stored under the name, if any.
         *
         * \param[in] name the name of the entry.
         */
        void erase(const std::string& name);

        /**
         * \brief Return true if the entry was stored during the current generation.
         *
         * \param[in] name the name of the entry.
         * \return false if there is no entry.
         */
        bool isUpToDate(const std::string& name) const;

      private:
        /// Entry stored by the steps.
        struct Entry {
            /// The value, a std::shared_ptr<T>.
            std::any value;

            /// Generation in which the value was stored.
            uint64_t generation;
        };

        /// Entries stored by the steps.
        std::map<std::string, Entry> entries;

        /// Return the entry with a name, checking its existence and type.
        template <typename T> const std::shared_ptr<T>& find(const std::string& name) const
        {
            auto it = this->entries.find(name);
            if(it == this->entries.end()) {
                throw std::runtime_error("Evolution::EvolutionState::get: no entry named " + name);
            }
            const std::shared_ptr<T>* value = std::any_cast<std::shared_ptr<T>>(&it->second.value);
            if(value == nullptr) {
                throw std::runtime_error("Evolution::EvolutionState::get: entry " + name + " is not a " + Data::demangle(typeid(T).name()));
            }
            return *value;
        }
    };

}; // namespace Evolution

#endif // EVOLUTION_STATE_H
