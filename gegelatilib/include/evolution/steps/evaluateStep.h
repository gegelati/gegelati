#ifndef EVOLUTION_EVALUATE_STEP_H
#define EVOLUTION_EVALUATE_STEP_H

#include <memory>
#include <set>
#include <string>
#include <vector>

#include "evaluation/evaluate.h"
#include "evaluation/problem.h"
#include "individual.h"
#include "evolution/module.h"
#include "metrics/metricMap.h"

namespace Evolution {

    /**
     * \brief Step evaluating individuals in a problem.
     */
    class EvaluateStep : public Module {
        protected:
            /// Names of the lists of evaluated individuals.
            std::vector<std::string> evaluated;

            /// Name of the measured metrics.
            std::string metrics;

            /// Problem in which the individuals are evaluated.
            std::shared_ptr<Evaluation::Problem> problem;

            /// Metrics measured during the evaluations.
            std::shared_ptr<const Metrics::MetricMapTemplate> metricTemplate;

            /// Number of evaluations of each individual.
            size_t nbIterations;

            /// Evaluation mode, given to the problem.
            Evaluation::Mode mode;

            /// Maximum number of threads evaluating the individuals.
            size_t nbThreads;

        public:
            /**
             * \brief Constructor.
             *
             * \param[in] evaluated names of the lists of evaluated individuals (list of individuals or list of mutable individuals).
             * \param[in] metrics name of the measured metrics, a metrics map.
             * \param[in] problem problem in which the individuals are evaluated.
             * \param[in] metricTemplate metrics measured during the evaluations.
             * \param[in] nbIterations number of evaluations of each individual.
             * \param[in] mode evaluation mode, given to the problem.
             * \param[in] nbThreads maximum number of threads evaluating the individuals (see Evaluation::evaluateIndividuals).
             * \param[in] name name of the step (e.g. "ValidationEvaluation").
             * \throw std::runtime_error if the problem is null.
             */
            EvaluateStep(std::vector<std::string> evaluated, std::string metrics, std::shared_ptr<Evaluation::Problem> problem,
                         std::shared_ptr<const Metrics::MetricMapTemplate> metricTemplate, size_t nbIterations,
                         Evaluation::Mode mode = Evaluation::Mode::TRAINING, size_t nbThreads = 1, std::string name = "Evaluate");

            /// Return the names of the lists of evaluated individuals.
            const std::vector<std::string>& getEvaluated() const { return this->evaluated; }

            /// Return the name of the measured metrics.
            const std::string& getMetrics() const { return this->metrics; }

            /// Return the problem in which the individuals are evaluated.
            std::shared_ptr<Evaluation::Problem> getProblem() const { return this->problem; }

            /// Return the maximum number of threads evaluating the individuals.
            size_t getNbThreads() const { return this->nbThreads; }

            /// Return the metrics measured during the evaluations.
            std::shared_ptr<const Metrics::MetricMapTemplate> getMetricTemplate() const { return this->metricTemplate; }

            /// Return the number of evaluations of each individual.
            size_t getNbIterations() const { return this->nbIterations; }

            /// Return the evaluation mode.
            Evaluation::Mode getMode() const { return this->mode; }

            /// Inherited from Module
            virtual std::string details() const override;
            /// Inherited from Module
            virtual void apply(EvolutionData& state) override;
    };

}; // namespace Evolution

#endif // EVOLUTION_EVALUATE_STEP_H
