#ifndef EVALUATE_H
#define EVALUATE_H

#include <cstdint>
#include <map>
#include <memory>
#include <mutex>
#include <queue>
#include <set>
#include <vector>

#include "evaluation/problem.h"
#include "individual.h"
#include "metrics/metricMap.h"

namespace Evaluation {

    /**
     * \brief Evaluate individuals in a problem and return the metrics measured for each of them.
     *
     * This function calls the extractMetrics method of the problem for every individual.
     * The metrics of each individual are stored indexed by the individual ID.
     *
     * With several threads, each thread evaluates individuals in its own copy of the problem (see Problem::cloneUniquePtr),
     * the calling thread using the problem itself. A problem that cannot be copied is evaluated with a single thread.
     *
     * \param[in] individuals The individuals whose genotypes are evaluated (must be distinct).
     * \param[in] problem the problem in which individuals are evaluated.
     * \param[in] metricTemplate template of the metrics measured.
     * \param[in] nbIterations number of evaluations of each individual.
     * \param[in] generationNumber the integer number of the current generation.
     * \param[in] mode the mode of the problem during the evaluation.
     * \param[in] nbThreads maximum number of threads evaluating the individuals, zero and one being a single thread.
     * \return the metrics of each individual, indexed by individual ID.
     * \throw std::runtime_error if the template has no metric or if there is no individual.
     */
    std::map<uint64_t, std::unique_ptr<Metrics::MetricMap>> evaluateIndividuals(
        const std::vector<std::shared_ptr<const Individual>>& individuals,
        Problem& problem, std::shared_ptr<const Metrics::MetricMapTemplate> metricTemplate,
        size_t nbIterations, uint64_t generationNumber, Mode mode, size_t nbThreads = 1);

    /**
     * \brief Worker loop of evaluateIndividuals: evaluate the individuals popped from the queue until it is empty.
     *
     * \param[in,out] problem problem used by this worker to evaluate individuals.
     * \param[in] metricTemplate template of the metrics measured.
     * \param[in] hashes hashes identifying the evaluations.
     * \param[in] queueMutex mutex protecting the queue of individuals.
     * \param[in,out] queue queue of individuals still to evaluate.
     * \param[out] results map where the metrics of evaluated individuals are stored.
     * \param[in] resultsMutex mutex protecting the results map.
     */
    void evaluationWorker(Problem& problem, const std::shared_ptr<const Metrics::MetricMapTemplate>& metricTemplate,
                          const std::set<uint64_t>& hashes, std::mutex& queueMutex, std::queue<std::shared_ptr<const Individual>>& queue,
                          std::map<uint64_t, std::unique_ptr<Metrics::MetricMap>>& results, std::mutex& resultsMutex);

}; // namespace Evaluation

#endif // EVALUATE_H
