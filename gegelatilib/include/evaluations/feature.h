

#ifndef EVALUATIONS_FEATURE_H
#define EVALUATIONS_FEATURE_H

#include <typeindex>
#include <set>
#include <map>

#include "data/dataValue.h"
#include "data/hash.h"


/// For include
class Individual;

namespace Evaluations {

    class Problem;

    /**
     * \brief Abstract class to extract any metrics from either the individual or the environment during an evaluation run.
     */
    class Feature
    {
      protected:

        /**
         * \brief map of features measured. 
         * 
         * Each feature is saved as a dataValue, with a unique hash key.
         */
        std::map<size_t, Data::DataValue> features;

      public:
        /**
         * \brief Default constructor
         */
        Feature() {};
        
        /**
         * \brief Return the keys of the current features map
         */
        virtual std::set<size_t> getKeys() const;

        /**
         * \brief Return if the feature possess the required key.
         */
        virtual bool hasKey(size_t key) const;

        /**
         * \brief return the feature at the required key.
         * 
         * \throw if the key is not in the map
         */
        virtual const Data::DataValue& getFeatureAt(size_t key) const;

        /**
         * \brief return the number of features measured
         */
        virtual size_t size() const;

        /**
         * \brief Add the features extracted
         * 
         * If both "this" and "other" contains a same key, the feature of "other" override "this"
         */
        virtual void merge(Feature& other);

        /**
         * \brief Extract metrics from the individual in the problem.
         *
         * This method is called at every step of the evaluation, before execution.
         *
         * \param[in] hash unique hash identifying the evaluation.
         * \param[in] individual the individual performing a step.
         * \param[in] inputs inputs of the problem
         * \param[in] problem the problem in which the individual is evaluated.
         */
        virtual void extractBeforeExecution(
            const uint64_t& hash,
            const Individual& individual,
            const std::vector<Data::DataView>& inputs,
            const Problem& problem) {
            /* Empty because sub-class does not need to inherrit from it.*/
        };

        /**
         * \brief Extract metrics from the individual in the problem.
         *
         * This method is called at every step of the evaluation, after execution.
         *
         * \param[in] hash unique hash identifying the evaluation.
         * \param[in] individual the individual performing a step.
         * \param[in] output the output of the individual execution individual.
         * \param[in] problem the problem in which the individual is evaluated.
         */
        virtual void extractAfterExecution(
            const uint64_t& hash,
            const Individual& individual, 
            const Data::DataValue& output,
            const Problem& problem) {
            /* Empty because sub-class does not need to inherrit from it.*/
        };

        /**
         * \brief Print the content of the metric.
         */
        virtual std::string toString(std::string prefix = "") const = 0;

        /**
         * \brief Compute the hash of the feature. Default is computed from std::type_index.
         */
        virtual uint64_t hash() const noexcept;
    };
        
    /**
     * \brief operator for printing
     */
    inline std::ostream& operator<<(std::ostream& os, const Feature& feature) {
        return os << feature.toString();
    }


}; // namespace Evaluation

#endif // EVALUATIONS_FEATURE_H