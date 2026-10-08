

#include "evaluation/parallelEvaluator.h"

void Evaluation::ParallelEvaluator::slaveEvalJobThread(
            Problem& problem, std::shared_ptr<const Metrics::MetricMapTemplate> metricTemplate, const std::set<uint64_t>& hashes, 
            std::mutex& individualsToProcessMutex,
            std::queue<std::shared_ptr<const Individual>>& individualsQueue, 
            std::map<std::shared_ptr<const Individual>, std::unique_ptr<Metrics::MetricMap>, SharedLess<Individual>>& results,
            std::mutex& resultsMutex) const
{
    
    // Pop a job and process it
    while (true) {
    
        bool doProcess = false;
        std::shared_ptr<const Individual> individualToProcess;

        { // Mutual exclusion zone: atomic job acquisition + engine creation
            std::lock_guard<std::mutex> lock(individualsToProcessMutex);
            if (!individualsQueue.empty()) {
                individualToProcess = individualsQueue.front();
                individualsQueue.pop();
                doProcess = true;
            }
        } // End of mutual exclusion zone

        // Exit if no more jobs
        if (!doProcess) {
            break;
        }

        std::unique_ptr<Metrics::MetricMap> localMetrics = std::make_unique<Metrics::MetricMap>(metricTemplate);
        problem.extractMetrics(*individualToProcess, *localMetrics, hashes);

        
        { // Store result Mutual exclusion zone
            std::lock_guard<std::mutex> lock(resultsMutex);
            results.insert(std::make_pair(individualToProcess, std::move(localMetrics)));
        }
    }
}

std::map<std::shared_ptr<const Individual>, std::unique_ptr<Metrics::MetricMap>, SharedLess<Individual>> Evaluation::ParallelEvaluator::evaluateIndividuals(
            const std::set<std::shared_ptr<const Individual>, SharedLess<Individual>>& individuals, 
            Problem& problem, std::shared_ptr<const Metrics::MetricMapTemplate> metricTemplate,
            size_t nbIterations, uint64_t generationNumber,
            Mode mode) const
{
    if(!problem.isCopyable() || this->maxNbThreads == 1) {
        return Evaluation::Evaluator::evaluateIndividuals(individuals, problem, metricTemplate, nbIterations, generationNumber, mode);
    }

    if(metricTemplate->size() == 0 || individuals.empty()) {
        throw std::runtime_error("Evaluation::Evaluator::evaluateIndividuals: cannot evaluate with empty list of metrics or empty set of individuals");
    }

    // Define hashes
    std::set<uint64_t> hashes = this->computeEvaluationHashes(
        nbIterations, generationNumber, mode, problem.getProblemSeed(), problem.maxHash());

    // Create a copy of Individuals in a queue structure
    std::queue<std::shared_ptr<const Individual>> individualsQueue;
    for (const std::shared_ptr<const Individual>& indiv : individuals) {
        individualsQueue.push(indiv);
    }

    std::vector<std::unique_ptr<Problem>> cloneProblems;
    for(size_t idx = 0; idx < this->maxNbThreads; idx++) {
        cloneProblems.push_back(problem.cloneUniquePtr());
    }

    
    std::map<std::shared_ptr<const Individual>, std::unique_ptr<Metrics::MetricMap>, SharedLess<Individual>> results;
    std::mutex resultsMutex;
    std::mutex individualsToProcessMutex;

    // Create the threads
    std::vector<std::thread> threads;
    for (auto i = 0; i < (this->maxNbThreads - 1); i++) {
        threads.emplace_back(std::thread(
            &ParallelEvaluator::slaveEvalJobThread, this, std::ref(*cloneProblems.at(i)),
            std::ref(metricTemplate), std::ref(hashes), std::ref(individualsToProcessMutex), std::ref(individualsQueue),
            std::ref(results), std::ref(resultsMutex)));
    }

    // Work in the main thread also, using the main environment
    this->slaveEvalJobThread(problem, metricTemplate, hashes, individualsToProcessMutex,
                             individualsQueue, results,
                             resultsMutex);
                    
    // Join the threads
    for (auto& thread : threads) {
        thread.join();
    }

    return results;
}