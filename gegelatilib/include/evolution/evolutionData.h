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
#include "rng/rng.h"
#include "util/stopwatch.h"

namespace Evolution {

    /**
     * \brief The data of an evolution: what the modules of an evolution produce and exchange.
     *
     * It is shared by all the modules of an evolution. Apart from the generation counters and the random number
     * generator, everything is a data identified by a name: individuals of the populations, parents, offspring,
     * metrics, fitness, and any data the user modules exchange. The type of a data is given when it is stored and
     * when it is read: the modules know the type of what they read and write, and a wrong type throws when it is
     * read. Data that only one module needs (e.g. its parameters) should stay inside that module.
     *
     * Each data is stored in an Entry that records the generation in which it was stored (see isUpToDate), so a reader
     * can tell whether a result is from the current generation. Modifying a data in place keeps its generation.
     * summary() lists the data with their type, size and generation.
     */
    struct EvolutionData {
        /// Generation number of the initialization: the generation 0 is the initial state, the first evolved generation is the 1.
        static constexpr uint64_t initGeneration = 0;

        /// Index of the generation currently running, initGeneration during the initialization.
        uint64_t generation = initGeneration;

        /// Number of generations already done since the initialization (the initialization is not counted).
        uint64_t nbGenerationsDone = 0;

        /// Random number generator of the evolution.
        RNG::RNG rng;

        /// Stopwatch of the evolution, started when the data are created (or when a new evolution starts, see Module::evolve). The modules and the log read their times from it.
        Stopwatch stopwatch;

        /**
         * \brief Constructor.
         */
        explicit EvolutionData() {}

        /**
         * \brief Add the data of another EvolutionData, e.g. the components of a module added to another one.
         *
         * A data already present under the same name is kept: the data of the other one is ignored. Different data must therefore
         * have different names.
         *
         * \param[in] other the merged data.
         * \throw std::runtime_error if both have a data of different types under the same name.
         */
        void mergeWith(const EvolutionData& other);

        /// Return the names of the data of the state.
        std::set<std::string> dataNames() const;

        /**
         * \brief Store a data, creating it or replacing the previous one, with the current generation.
         *
         * \tparam T type of the data, deduced from the value.
         * \param[in] name the name of the data.
         * \param[in] value the stored value, shared with the caller.
         */
        template <typename T> void set(const std::string& name, std::shared_ptr<T> value)
        {
            this->data[name] = std::make_shared<const Entry<T>>(std::move(value), this->generation);
        }

        /**
         * \brief Return the data stored under a name.
         *
         * \tparam T type of the data.
         * \param[in] name the name of the data.
         * \throw std::runtime_error if there is no data, or if it is not a T.
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
         * \brief Return true if a data is stored under the name.
         *
         * \param[in] name the name of the data.
         */
        bool has(const std::string& name) const
        {
            return this->data.find(name) != this->data.end();
        }

        /**
         * \brief Remove the data stored under the name, if any.
         *
         * \param[in] name the name of the data.
         */
        void erase(const std::string& name)
        {
            this->data.erase(name);
        }

        /**
         * \brief Return true if the data was stored during the current generation.
         *
         * \param[in] name the name of the data.
         * \return false if there is no data.
         */
        bool isUpToDate(const std::string& name) const
        {
            std::map<std::string, std::shared_ptr<const EntryBase>>::const_iterator it = this->data.find(name);
            return it != this->data.end() && it->second->generation() == this->generation;
        }

        /**
         * \brief Return a printable table of the data: name, type, size or value, and generation in which it was stored.
         *
         * Data stored during the current generation are "up to date", the others show their generation.
         */
        std::string summary() const;

      private:
        /// Data stored by the steps, by name.
        std::map<std::string, std::shared_ptr<const EntryBase>> data;

        /// Return the data with a name, checking its existence and type.
        template <typename T> const std::shared_ptr<T>& find(const std::string& name) const
        {
            std::map<std::string, std::shared_ptr<const EntryBase>>::const_iterator it = this->data.find(name);
            if(it == this->data.end()) {
                throw std::runtime_error("Evolution::EvolutionData::get: no data named " + name);
            }
            if(it->second->type() != std::type_index(typeid(T))) {
                throw std::runtime_error("Evolution::EvolutionData::get: data " + name + " is not a " + Data::demangle(typeid(T).name()) + " but a "
                                         + Data::demangle(it->second->type().name()));
            }
            return static_cast<const Entry<T>&>(*it->second).value();
        }
    };

}; // namespace Evolution

#endif // EVOLUTION_DATA_H
