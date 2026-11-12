#include "evaluation/evaluate.h"

#include <mutex>
#include <queue>
#include <set>
#include <stdexcept>
#include <thread>

// Worker loop evaluating the individuals popped from the queue until it is empty.
void Evaluation::evaluationWorker(Problem& problem, const std::shared_ptr<const Metrics::MetricMapTemplate>& metricTemplate,
                      const std::set<uint64_t>& hashes, std::mutex& queueMutex, std::queue<std::shared_ptr<const Individual>>& queue,
                      std::map<uint64_t, std::unique_ptr<Metrics::MetricMap>>& results, std::mutex& resultsMutex)
{
    while(true) {
        std::shared_ptr<const Individual> individual;
        { // Mutual exclusion zone: atomic job acquisition
            std::lock_guard<std::mutex> lock(queueMutex);
            if(queue.empty()) {
                break;
            }
            individual = queue.front();
            queue.pop();
        }

        std::unique_ptr<Metrics::MetricMap> localMetrics = std::make_unique<Metrics::MetricMap>(metricTemplate);
        problem.extractMetrics(*individual, *localMetrics, hashes);

        { // Store result Mutual exclusion zone
            std::lock_guard<std::mutex> lock(resultsMutex);
            results.insert(std::make_pair(individual->getIndividualID(), std::move(localMetrics)));
        }
    }
}

std::map<uint64_t, std::unique_ptr<Metrics::MetricMap>> Evaluation::evaluateIndividuals(
    const std::vector<std::shared_ptr<const Individual>>& individuals,
    Problem& problem, std::shared_ptr<const Metrics::MetricMapTemplate> metricTemplate,
    size_t nbIterations, uint64_t generationNumber, Mode mode, size_t nbThreads)
{
    if(metricTemplate->size() == 0 || individuals.empty()) {
        throw std::runtime_error("Evaluation::evaluateIndividuals: cannot evaluate with empty list of metrics or empty set of individuals");
    }

    const std::set<uint64_t> hashes = problem.computeEvaluationHashes(nbIterations, generationNumber, mode, problem.getProblemSeed());
    std::map<uint64_t, std::unique_ptr<Metrics::MetricMap>> results;

    // A single thread, or a problem that cannot be copied for the other threads.
    if(nbThreads <= 1 || !problem.isCopyable()) {
        for(const std::shared_ptr<const Individual>& individual : individuals) {
            std::unique_ptr<Metrics::MetricMap> localMetrics = std::make_unique<Metrics::MetricMap>(metricTemplate);
            problem.extractMetrics(*individual, *localMetrics, hashes);
            results.insert(std::make_pair(individual->getIndividualID(), std::move(localMetrics)));
        }
        return results;
    }

    std::queue<std::shared_ptr<const Individual>> queue;
    for(const std::shared_ptr<const Individual>& individual : individuals) {
        queue.push(individual);
    }

    // The calling thread uses the problem itself, the other threads a copy each.
    std::vector<std::unique_ptr<Problem>> cloneProblems;
    for(size_t idx = 0; idx < nbThreads - 1; idx++) {
        cloneProblems.push_back(problem.cloneUniquePtr());
    }

    std::mutex queueMutex;
    std::mutex resultsMutex;
    std::vector<std::thread> threads;
    for(size_t idx = 0; idx < nbThreads - 1; idx++) {
        threads.emplace_back(evaluationWorker, std::ref(*cloneProblems.at(idx)), std::cref(metricTemplate), std::cref(hashes),
                             std::ref(queueMutex), std::ref(queue), std::ref(results), std::ref(resultsMutex));
    }
    evaluationWorker(problem, metricTemplate, hashes, queueMutex, queue, results, resultsMutex);
    for(std::thread& thread : threads) {
        thread.join();
    }
    return results;
}
