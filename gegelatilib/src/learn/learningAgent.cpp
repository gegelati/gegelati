/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2019 - 2025) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2019 - 2022)
 * Nicolas Sourbier <nsourbie@insa-rennes.fr> (2019 - 2020)
 * Pierre-Yves Le Rolland-Raumer <plerolla@insa-rennes.fr> (2020)
 * Quentin Vacher <qvacher@insa-rennes.fr> (2023 - 2025)
 *
 * GEGELATI is an open-source reinforcement learning framework for training
 * artificial intelligence based on Tangled Program Graphs (TPGs).
 *
 * This software is governed by the CeCILL-C license under French law and
 * abiding by the rules of distribution of free software. You can use,
 * modify and/ or redistribute the software under the terms of the CeCILL-C
 * license as circulated by CEA, CNRS and INRIA at the following URL
 * "http://www.cecill.info".
 *
 * As a counterpart to the access to the source code and rights to copy,
 * modify and redistribute granted by the license, users are provided only
 * with a limited warranty and the software's author, the holder of the
 * economic rights, and the successive licensors have only limited
 * liability.
 *
 * In this respect, the user's attention is drawn to the risks associated
 * with loading, using, modifying and/or developing or reproducing the
 * software by the user in light of its specific status of free software,
 * that may mean that it is complicated to manipulate, and that also
 * therefore means that it is reserved for developers and experienced
 * professionals having in-depth computer knowledge. Users are therefore
 * encouraged to load and test the software's suitability as regards their
 * requirements in conditions enabling the security of their systems and/or
 * data to be ensured and, more generally, to use and operate it in the
 * same conditions as regards security.
 *
 * The fact that you are presently reading this means that you have had
 * knowledge of the CeCILL-C license and that you accept its terms.
 */

#include <inttypes.h>
#include <queue>
#include <unordered_set>

#include "data/hash.h"
#include "learn/evaluationResult.h"
#include "mutator/rng.h"

#include "learn/learningAgent.h"

#include "algorithm/tpg/tpgJob.h"
#include "algorithm/species/speciesAlgorithm.h"

void Learn::LearningAgent::setCurrentAlgorithm(Algorithm::Algorithm* algorithm)
{
    if(algorithm == nullptr){
        throw std::runtime_error("LearningAgent::setCurrentAlgorithm: given algorithm is a null pointer.");
    }
    if(std::find(this->algorithms.begin(), this->algorithms.end(), *algorithm) == this->algorithms.end()){
        throw std::runtime_error("LearningAgent::setCurrentAlgorithm: given algorithm is not part of the learning agent algorithms.");
    }

    this->currentExecutedAlgorithm = algorithm;
}

void Learn::LearningAgent::addAlgorithm(Algorithm::Algorithm& algorithm)
{
    this->algorithms.push_back(algorithm);
}


Algorithm::Algorithm& Learn::LearningAgent::getAlgorithm(const Algorithm::Algorithm& algorithm)
{
    
    auto iterator = std::find_if(this->algorithms.begin(), this->algorithms.end(),
        [&algorithm](Algorithm::Algorithm& algo){
            return algo == algorithm;
        });
    if(iterator == this->algorithms.end() || (*iterator).get().getAlgorithmID() != algorithm.getAlgorithmID()){
        throw std::invalid_argument("LearningAgent::getAlgorithm: the given algorithm is not managed by this learning agent.");
    }

    return *iterator;
}

EvoGraph::Graph& Learn::LearningAgent::getGraph()
{
    return *this->graph;
}

std::vector<std::reference_wrapper<const Algorithm::Algorithm>> Learn::LearningAgent::cGetAlgorithms() const
{
    std::vector<std::reference_wrapper<const Algorithm::Algorithm>> result;
    for(const Algorithm::Algorithm& algorithm : this->algorithms) {
        result.push_back(algorithm);
    }
    return result;
}

const std::vector<std::reference_wrapper<Algorithm::Algorithm>>& Learn::LearningAgent::getAlgorithms()
{
    return this->algorithms;
}

Algorithm::Algorithm& Learn::LearningAgent::getAlgorithmAt(size_t idx)
{
    return this->algorithms.at(idx);
}

RNG::RNG& Learn::LearningAgent::getRNG()
{
    return this->rng;
}

void Learn::LearningAgent::init(uint64_t seed)
{
    // Initialize Randomness
    this->rng.setSeed(seed);

    if(this->algorithms.empty()){
        throw std::runtime_error("LearningAgent::init: No algorithm to init.");
    }

    for(Algorithm::Algorithm& algorithm: algorithms){
        algorithm.initAlgorithm(this->rng, *this->learningEnvironment.getActions(), this->learningEnvironment.getDataSources(), this->graph);
        algorithm.initPopulation(this->rng);
    }
}

void Learn::LearningAgent::addLogger(Log::LALogger& logger)
{
    logger.doValidation = this->params.doValidation;
    logger.useUtility = this->learningEnvironment.isUsingUtility();
    // logs for example the headers of the columns the logger will print
    loggers.push_back(std::reference_wrapper<Log::LALogger>(logger));
}


std::shared_ptr<Learn::EvaluationResult> Learn::LearningAgent::evaluateJob(
    Algorithm::ExecutionEngine& execEngine, const Algorithm::Job& job, uint64_t generationNumber,
    Learn::LearningMode mode, LearningEnvironment& le) const
{
    if(this->currentExecutedAlgorithm == nullptr){
        throw std::runtime_error("LearningAgent::evaluateJob: currentExecutedAlgorithm is not set.");
    }

    // Get the current agent and the current algorithm
    const Algorithm::Agent& agent = job.getAgent();
    const Selector::Selector& selector = this->currentExecutedAlgorithm->getSelector();

    // Skip the agent evaluation process if enough evaluations were already
    // performed. In the evaluation mode only.
    std::shared_ptr<Learn::EvaluationResult> previousEval;
    if (mode == LearningMode::TRAINING &&
        selector.isAgentEvalSkipped(job.getAgent(), previousEval)) {
        return previousEval;
    }

    // Set the job to execute
    execEngine.setExecutionMode(mode == LearningMode::TRAINING);
    execEngine.setupJob(job);

    // Init results
    double result = 0.0;

    // Init utility
    double utility = 0.0;

    // Number of evaluations
    uint64_t nbEvaluation = (mode == LearningMode::TRAINING)
                                ? this->params.nbIterationsPerPolicyEvaluation
                                : this->params.nbIterationsPerPolicyValidation;

    // Init global selection metric
    std::shared_ptr<Selector::SelectionMetrics> globalSelectionMetrics =
        selector.createSelectionMetrics();
    globalSelectionMetrics->initMetrics(agent, le);

    // Evaluate nbIteration times
    for (auto iterationNumber = 0; iterationNumber < nbEvaluation;
         iterationNumber++) {
        // Compute a Hash
        Data::Hash<uint64_t> hasher;
        uint64_t hash = hasher(generationNumber) ^ hasher(iterationNumber);

        // Init selectionMetrics for this episode.
        std::shared_ptr<Selector::SelectionMetrics> selectionMetrics =
            selector.createSelectionMetrics();
        selectionMetrics->initMetrics(agent, le);

        // Reset the learning Environment
        le.reset(hash, mode, iterationNumber, generationNumber);

        uint64_t nbActions = 0;
        while (!le.isTerminal() &&
               nbActions < this->params.maxNbActionsPerEval) {
            // Get the actions
            std::vector<double> actionsID =
                execEngine.execute();

            // Do it
            le.doActions(actionsID);
            // Count actions
            nbActions++;

            // Extract the metrics.
            selectionMetrics->extractMetricsStep(agent, actionsID, le);
        }

        // Extract the metrics.
        selectionMetrics->extractMetricsEpisode(agent, nbActions, le);

        // Add the extracted metrics to the total.
        globalSelectionMetrics->weightedSum(selectionMetrics, iterationNumber, 1);
    }

    // Create the EvaluationResult
    auto evaluationResult = std::shared_ptr<EvaluationResult>(
        new EvaluationResult(globalSelectionMetrics, nbEvaluation));

    // Combine it with previous one if any
    if (previousEval != nullptr) {
        *evaluationResult += *previousEval;
    }
    return evaluationResult;
}

std::multimap<std::shared_ptr<Learn::EvaluationResult>, std::reference_wrapper<const Algorithm::Agent>>
Learn::LearningAgent::evaluateAllAgents(uint64_t generationNumber,
                                       Learn::LearningMode mode)
{
    std::multimap<std::shared_ptr<EvaluationResult>, std::reference_wrapper<const Algorithm::Agent>>
        results;


    for(Algorithm::Algorithm& algorithm: this->algorithms){
        // set current executed algorithm
        this->setCurrentAlgorithm(&algorithm);

        // Evaluate the algorithm agents and insert the results
        auto algoResults = this->evaluateCurrentAlgorithmAgents(generationNumber, mode);
        results.insert(algoResults.begin(), algoResults.end());
    }

    return results;
}


std::multimap<std::shared_ptr<Learn::EvaluationResult>, std::reference_wrapper<const Algorithm::Agent>>
Learn::LearningAgent::evaluateCurrentAlgorithmAgents(uint64_t generationNumber,
                                       Learn::LearningMode mode)
{
    if(this->currentExecutedAlgorithm == nullptr){
        throw std::runtime_error("LearningAgent::evaluateOneAlgorithmAgents: currentExecutedAlgorithm is not set.");
    }
    if(!this->containsAlgorithm(*this->currentExecutedAlgorithm)){
        throw std::runtime_error("LearningAgent::evaluateOneAlgorithmAgents: The learning agent does not contain the given algorithm.");
    }

    std::multimap<std::shared_ptr<EvaluationResult>, std::reference_wrapper<const Algorithm::Agent>>
        results;

    std::unique_ptr<Algorithm::ExecutionEngine> execEngine =
        this->currentExecutedAlgorithm->getManager().createExecutionEngine();

    auto jobs = this->makeJobs(mode);
    for(auto job: jobs) {
        std::shared_ptr<EvaluationResult> result = this->evaluateJob(
            *execEngine, *job, generationNumber, mode, this->learningEnvironment);
        results.emplace(result, (*job).getAgent());
    }


    // Update the algorithm after evaluation with the jobs processed
    this->currentExecutedAlgorithm->updateAfterEvaluation(jobs, mode);

    return results;
}

std::shared_ptr<Learn::EvaluationResult> Learn::LearningAgent::evaluateOneAgent(
    uint64_t generationNumber, Learn::LearningMode mode,
    const Algorithm::Agent& agent)
{
    const Algorithm::Algorithm& algorithm = this->findCorrespondingAlgorithm(agent);

    // Create the execution engine of the agent.
    std::unique_ptr<Algorithm::ExecutionEngine> execEngine =
        algorithm.getManagerCst().createExecutionEngine();

    // Create and evaluate the job
    auto job = algorithm.createJob(agent, mode, this->rng);
    std::shared_ptr<EvaluationResult> avgScore = this->evaluateJob(
        *execEngine, *job, generationNumber, mode, this->learningEnvironment);

    // Return the result
    return avgScore;
}

const Algorithm::Algorithm& Learn::LearningAgent::getBestAlgorithm()
{
    return *this->currentBestAlgorithm;
}

void Learn::LearningAgent::launchAlgorithmsSelection(
            std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                          std::reference_wrapper<const Algorithm::Agent>>& results,
            RNG::RNG& rng)
{
    bool speciesAlgorithmTest = true;
    if(speciesAlgorithmTest) {
        std::multimap<std::shared_ptr<Learn::EvaluationResult>,
            std::reference_wrapper<const Algorithm::Agent>>
            resultsCopy(results);

        results.clear();


        std::map<double, std::reference_wrapper<Algorithm::Algorithm>> scoreAlgorithm;
        for(Algorithm::Algorithm& algorithm: algorithms){
            std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                        std::reference_wrapper<const Algorithm::Agent>>
                resultsAlgo;
            
            for(auto it = resultsCopy.begin(); it != resultsCopy.end(); ){
                if(algorithm.containsAgent(it->second)){
                    resultsAlgo.insert(*it);
                    it = resultsCopy.erase(it);
                } else {
                    ++it;
                }
            }

            scoreAlgorithm.insert({algorithm.getSelector().getScoreAlgorithm(), algorithm});
            // Do the selection for this algorithm
            algorithm.getSelector().doSelection(*this->graph, resultsAlgo, rng);
            // Update the evaluation records
            algorithm.getSelector().updateEvaluationRecords(resultsAlgo);
            algorithm.getSelector().computeScoreAlgorithm(resultsAlgo);

            results.insert(resultsAlgo.begin(), resultsAlgo.end());
        }

        int position = 1;
        for (auto it = scoreAlgorithm.rbegin(); it != scoreAlgorithm.rend(); ++it, ++position) {
            double prop = dynamic_cast<Algorithm::SpeciesAlgorithm&>(it->second.get()).calculateProportion(position, scoreAlgorithm.size());
            it->second.get().getManager().setExpectedNbAgents(prop * this->params.mutation.tpg.nbRoots);
        }

        this->currentBestAlgorithm = &scoreAlgorithm.rbegin()->second.get();


        double probaAdd = params.mutation.tpg.pEdgeAddition;
        double probaDel = params.mutation.tpg.pEdgeDeletion;
        if(probaAdd > rng.getDouble(0, 1) && this->algorithms.size() < 5) {
            std::cout<<"CREATE ALGOOOOO";
            // Dupplicate algorithm ->
            Algorithm::SpeciesAlgorithm& algoToDupplicate = dynamic_cast<Algorithm::SpeciesAlgorithm&>(scoreAlgorithm.rbegin()->second.get());

            this->createdSpeciesAlgorithms.push_back(std::move(algoToDupplicate.initNewSpecies(this->rng)));
            Algorithm::Algorithm& newAlgorithm = **this->createdSpeciesAlgorithms.rbegin();
            newAlgorithm.getManager().setExpectedNbAgents(0.5 * this->params.mutation.tpg.nbRoots);
            //newAlgorithm.initPopulation(this->rng);
            this->algorithms.push_back(newAlgorithm);
        }

        else if(probaDel > rng.getDouble(0, 1) && this->algorithms.size() > 3) {
            std::cout<<"DESTROY ALGOOOOO";
            auto& algoRef = scoreAlgorithm.begin()->second.get();
            auto it = std::find(this->algorithms.begin(), this->algorithms.end(), algoRef);
            this->algorithms.erase(it);

            auto it2 = std::find_if(
                this->createdSpeciesAlgorithms.begin(),
                this->createdSpeciesAlgorithms.end(),
                [&algoRef](const std::unique_ptr<Algorithm::Algorithm>& ptr) {
                    return ptr.get() == &algoRef; // Compare raw pointers
                }
            );

            if (it2 != this->createdSpeciesAlgorithms.end()) {
                this->createdSpeciesAlgorithms.erase(it2);
            }
        }

        



    } else if(this->algorithms.size() == 1){
        // Do the selection for this algorithm
        this->algorithms.front().get().getSelector().doSelection(*this->graph, results, rng);

        // Update the evaluation records
        this->algorithms.front().get().getSelector().updateEvaluationRecords(results);
    } else {

        std::multimap<std::shared_ptr<Learn::EvaluationResult>,
            std::reference_wrapper<const Algorithm::Agent>>
            resultsCopy(results);

        results.clear();

        for(Algorithm::Algorithm& algorithm: algorithms){
            std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                        std::reference_wrapper<const Algorithm::Agent>>
                resultsAlgo;
            
            for(auto it = resultsCopy.begin(); it != resultsCopy.end(); ){
                if(algorithm.containsAgent(it->second)){
                    resultsAlgo.insert(*it);
                    it = resultsCopy.erase(it);
                } else {
                    ++it;
                }
            }

            // Do the selection for this algorithm
            algorithm.getSelector().doSelection(*this->graph, resultsAlgo, rng);
            // Update the evaluation records
            algorithm.getSelector().updateEvaluationRecords(resultsAlgo);

            results.insert(resultsAlgo.begin(), resultsAlgo.end());
        }
    }
}

void Learn::LearningAgent::trainOneGeneration(uint64_t generationNumber,
                                              bool doPopulate)
{
    for (auto logger : loggers) {
        logger.get().logNewGeneration(generationNumber);
    }


    // Evaluate
    auto results =
        this->evaluateAllAgents(generationNumber, LearningMode::TRAINING);
    for (auto logger : loggers) {
        logger.get().logAfterEvaluate(results);
    }

    // Remove worst performing roots
    this->launchAlgorithmsSelection(results, rng);

    for (auto logger : loggers) {
        logger.get().logAfterDecimate();
    }

    // Does a validation or not according to the parameter doValidation
    if (params.doValidation) {
        std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                      std::reference_wrapper<const Algorithm::Agent>>
            validationResults;

        if (generationNumber % params.stepValidation == 0 ||
            generationNumber == params.nbGenerations - 1) {
            validationResults = evaluateAllAgents(
                generationNumber, Learn::LearningMode::VALIDATION);
        }
        for (auto logger : loggers) {
            logger.get().logAfterValidate(validationResults);
        }
    }

    if (doPopulate) {
        // Populate Sequentially
        for(Algorithm::Algorithm& algorithm: algorithms){
            algorithm.populate(this->rng, this->maxNbThreads);
        }
    }

    for (auto logger : loggers) {
        logger.get().logAfterPopulateTPG();
    }

    for (auto logger : loggers) {
        logger.get().logEndOfTraining();
    }
}

uint64_t Learn::LearningAgent::train(volatile bool& altTraining,
                                     bool printProgressBar)
{
    const int barLength = 50;
    uint64_t generationNumber = 0;

    while (!altTraining && generationNumber < this->params.nbGenerations) {
        // Train one generation
        trainOneGeneration(generationNumber,
                           generationNumber != this->params.nbGenerations - 1);
        generationNumber++;

        // Print progressBar (homemade, probably not ideal)
        if (printProgressBar) {
            printf("\rTraining ["); // back
            // filling ratio
            double ratio =
                (double)generationNumber / (double)this->params.nbGenerations;
            int filledPart = (int)((double)ratio * (double)barLength);
            // filled part
            for (int i = 0; i < filledPart; i++) {
                printf("%c", (char)219);
            }

            // empty part
            for (int i = filledPart; i < barLength; i++) {
                printf(" ");
            }

            printf("] %4.2f%%", ratio * 100.00);
        }
    }

    if (printProgressBar) {
        if (!altTraining) {
            printf("\nTraining completed\n");
        }
        else {
            printf("\nTraining alted at generation %" PRIu64 ".\n",
                   generationNumber);
        }
    }
    return generationNumber;
}

std::vector<std::shared_ptr<Algorithm::Job>> Learn::LearningAgent::makeJobs(
    Learn::LearningMode mode)
{
    if(this->currentExecutedAlgorithm == nullptr){
        throw std::runtime_error("LearningAgent::makeJobs: Current executed algorithm is nullptr.");
    }

    std::vector<std::shared_ptr<Algorithm::Job>> jobs;
    size_t idx = 0;
    for(auto agent: this->currentExecutedAlgorithm->getAgents()){
        auto job = this->currentExecutedAlgorithm->createJob(agent, mode, rng, idx);
        jobs.push_back(job);
        idx++;
    }
    return jobs;
}

Algorithm::Algorithm& Learn::LearningAgent::findCorrespondingAlgorithm(const Algorithm::Agent& agent){
    for(Algorithm::Algorithm& algorithm: this->algorithms){
        if(algorithm.containsAgent(agent)){
            return algorithm;
        }
    }

    throw std::runtime_error("Agent not found in any algorithm");
}

bool Learn::LearningAgent::containsAlgorithm(Algorithm::Algorithm& algorithm){
    for(Algorithm::Algorithm& algo: this->algorithms){
        if(algo == algorithm){
            return true;
        }
    }
    return false;
}