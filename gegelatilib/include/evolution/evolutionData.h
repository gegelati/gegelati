#ifndef EVOLUTION_DATA_H
#define EVOLUTION_DATA_H

#include <cstdint>
#include <limits>
#include <map>
#include <memory>
#include <set>
#include <stdexcept>
#include <string>
#include <typeindex>
#include <typeinfo>
#include <utility>

#include "data/demangle.h"
#include "evolution/entry.h"
#include "evolution/key.h"
#include "rng/rng.h"

namespace Evolution {

    /**
     * \brief The data of an evolution: what the modules of an evolution produce and exchange.
     *
     * It is shared by all the modules of an evolution. Apart from the generation counters and the random number
     * generator, everything is a data identified by a typed Key: individuals of the populations,
     * parents, offspring, metrics, fitness, and any data the user modules exchange. The data are only accessed through
     * keys. Data that only one module needs (e.g. its parameters) should stay inside that module.
     *
     * Each data is stored in an Entry that records the generation in which it was stored (see isUpToDate), so a reader
     * can tell whether a result is from the current generation. Modifying a data in place keeps its generation.
     * summary() lists the data with their size and generation.
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

        /// Return the types of the data of the state, by name. The type of a data is the T of its Key<T>.
        std::map<std::string, std::type_index> dataTypes() const;

        /**
         * \brief Store a data, replacing the previous one, with the current generation.
         *
         * \tparam T type of the data, given by the key.
         * \param[in] key the key of the data.
         * \param[in] value the stored value, shared with the caller.
         */
        template <typename T> void set(const Key<T>& key, std::shared_ptr<T> value)
        {
            this->data[key.name()] = std::make_shared<const Entry<T>>(std::move(value), this->generation);
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
            return it != this->data.end() && it->second->generation() == this->generation;
        }

        /**
         * \brief Return a printable table of the data: name, size or value, and generation in which it was stored.
         *
         * Data stored during the current generation are "up to date", the others show their generation.
         */
        std::string summary() const;

      private:
        /// Data stored by the steps, by name of their key.
        std::map<std::string, std::shared_ptr<const EntryBase>> data;

        /// Return the data with a name, checking its existence and type.
        template <typename T> const std::shared_ptr<T>& find(const std::string& name) const
        {
            auto it = this->data.find(name);
            if(it == this->data.end()) {
                throw std::runtime_error("Evolution::EvolutionData::get: no data named " + name);
            }
            if(it->second->type() != std::type_index(typeid(T))) {
                throw std::runtime_error("Evolution::EvolutionData::get: data " + name + " is not a " + Data::demangle(typeid(T).name()));
            }
            return static_cast<const Entry<T>&>(*it->second).value();
        }
    };

}; // namespace Evolution

#endif // EVOLUTION_DATA_H
