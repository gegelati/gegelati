

#ifndef METRIC_H
#define METRIC_H

#include <typeindex>
#include <set>
#include <map>

#include "data/dataValue.h"
#include "data/hash.h"


/// For include
class Individual;

/// For include
namespace Evaluation {
    class Problem;
}

namespace Metrics {


    /**
     * \brief Abstract class to extract any metrics from either the individual or the environment during an evaluation run.
     */
    class Metric
    {
      protected:

      public:
        /**
         * \brief Default constructor
         */
        Metric() {};

        /**
         * \brief Clone the metric object with an empty metrics map.
         */
        virtual std::unique_ptr<Metric> cloneEmptyPtr() const = 0;

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
            const std::vector<Data::DataValue>& inputs,
            const Evaluation::Problem& problem) {
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
            const Evaluation::Problem& problem) {
            /* Empty because sub-class does not need to inherrit from it.*/
        };

        /**
         * \brief Extract metrics from the individual in the learning environment.
         *
         * This method is called at every step of the environment evaluation.
         *
         * \param[in] hash unique hash identifying the evaluation.
         * \param[in] individual the individual performing a step.
         * \param[in] problem the problem in which the individual is evaluated.
         */
        virtual void extractionEnd(
            const uint64_t& hash,
            const Individual& individual,
            const Evaluation::Problem& problem) {
            /* Empty because sub-class does not need to inherrit from it.*/    
        };

        /**
         * \brief Print the content of the metric.
         */
        virtual std::string toString(std::string prefix = "") const = 0;

        /**
         * \brief Compute the hash of the metric. Default is computed from std::type_index.
         */
        virtual uint64_t hash() const noexcept;
    };

        
    /**
     * \brief operator for printing
     */
    inline std::ostream& operator<<(std::ostream& os, const Metric& metric) {
        return os << metric.toString();
    }


}; // namespace Evaluation

#endif // METRIC_H