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

void Learn::LearningAgent::setNbGen(size_t gen)
{
    this->params->nbGenerations = gen;
}

const Representation::Representation& Learn::LearningAgent::getBestRepresentation()
{
    return *this->currentBestRepresentation;
}

void Learn::LearningAgent::setCurrentRepresentation(Representation::Representation* representation)
{
    if(representation == nullptr){
        throw std::runtime_error("LearningAgent::setCurrentRepresentation: given representation is a null pointer.");
    }
    if(this->representations.find(representation->getRepresentationID()) == this->representations.end()){
        throw std::runtime_error("LearningAgent::setCurrentRepresentation: given representation is not part of the learning agent representations.");
    }

    this->currentExecutedRepresentation = representation;
}

void Learn::LearningAgent::addRepresentation(Representation::Representation& representation)
{
    this->representations.insert({representation.getRepresentationID(), representation});
}


Representation::Representation& Learn::LearningAgent::getRepresentation(const Representation::Representation& representation)
{
    auto iterator = this->representations.find(representation.getRepresentationID());
    if(iterator == this->representations.end() || (*iterator).second.get().getRepresentationID() != representation.getRepresentationID()){
        throw std::invalid_argument("LearningAgent::getRepresentation: the given representation is not managed by this learning agent.");
    }
    return iterator->second;
}

EvoGraph::Graph& Learn::LearningAgent::getGraph()
{
    return *this->graph;
}

std::vector<std::reference_wrapper<const Representation::Representation>> Learn::LearningAgent::cGetRepresentations() const
{
    std::vector<std::reference_wrapper<const Representation::Representation>> result;
    for(const auto& pair : this->representations) {
        result.push_back(pair.second);
    }
    return result;
}

std::vector<std::reference_wrapper<Representation::Representation>> Learn::LearningAgent::getRepresentations()
{
    std::vector<std::reference_wrapper<Representation::Representation>> result;
    for(const auto& pair : this->representations) {
        result.push_back(pair.second);
    }
    return result;
}

Representation::Representation& Learn::LearningAgent::getRepresentationAt(size_t idx)
{
    return this->representations.at(idx);
}

RNG::RNG& Learn::LearningAgent::getRNG()
{
    return this->rng;
}

void Learn::LearningAgent::init(uint64_t seed, bool doGeneratePopulation)
{
    // Initialize Randomness
    this->rng.setSeed(seed);

    if(this->representations.empty()){
        throw std::runtime_error("LearningAgent::init: No representation to init.");
    }

    for(const auto& pair: representations){
        pair.second.get().initRepresentation(this->rng, *this->learningEnvironment.getActions(), this->learningEnvironment.getDataSources(), this->graph);
        if(doGeneratePopulation) {
            pair.second.get().initPopulation(this->rng);
        }
    }
}

void Learn::LearningAgent::addLogger(Log::LALogger& logger)
{
    logger.doValidation = this->params->doValidation;
    logger.useUtility = this->learningEnvironment.isUsingUtility();
    // logs for example the headers of the columns the logger will print
    loggers.push_back(std::reference_wrapper<Log::LALogger>(logger));
}

size_t Learn::LearningAgent::getNbEvaluationIndiv(std::shared_ptr<Learn::EvaluationResult> previousEval, Learn::LearningMode mode) const
{
    if (mode != LearningMode::TRAINING) {
        return this->params->nbIterationsPerPolicyValidation;
    }

    // In training mode, we need to check if the individual has already been evaluated and how many times.
    size_t nbEvaluationTraining = this->params->nbIterationsPerPolicyEvaluation;
    if(previousEval != nullptr) {
        nbEvaluationTraining = std::clamp(
            this->params->maxNbEvaluationPerPolicy - previousEval->getNbEvaluation(),
            static_cast<size_t>(0),
            nbEvaluationTraining
        );
    }
    return nbEvaluationTraining;    
}


std::shared_ptr<Learn::EvaluationResult> Learn::LearningAgent::evaluateJob(
    Representation::ExecutionEngine& execEngine, const Representation::Job& job, uint64_t generationNumber,
    Learn::LearningMode mode, LearningEnvironment& le) const
{
    if(this->currentExecutedRepresentation == nullptr){
        throw std::runtime_error("LearningAgent::evaluateJob: currentExecutedRepresentation is not set.");
    }

    // Get the current individual and the current representation
    const Representation::Individual& individual = job.getIndividual();
    const Selector::Selector& selector = this->currentExecutedRepresentation->getSelector();



    // Skip the individual evaluation process if enough evaluations were already
    // performed. In the evaluation mode only.
    std::shared_ptr<Learn::EvaluationResult> previousEval = selector.getResultsOf(job.getIndividual());
    size_t nbEvaluationToDo = this->getNbEvaluationIndiv(previousEval, mode);


    // Set the job to execute
    execEngine.setExecutionMode(mode == LearningMode::TRAINING);
    execEngine.setupJob(job);


    // Init global selection metric
    std::shared_ptr<Selector::SelectionMetrics> globalSelectionMetrics =
        selector.createSelectionMetrics();
    globalSelectionMetrics->initMetrics(individual, le);


    // Evaluate nbIteration times
    for (auto iterationNumber = 0; iterationNumber < nbEvaluationToDo;
         iterationNumber++) {
        // Compute a Hash
        Data::Hash<uint64_t> hasher;
        uint64_t hash;
        if(mode == Learn::LearningMode::TRAINING) {
            hash = hasher(generationNumber) ^ hasher(iterationNumber) ^ hasher(static_cast<int>(mode));
        } else {
            hash = hasher(iterationNumber) ^ hasher(static_cast<int>(mode));
        }

        // Init selectionMetrics for this episode.
        std::shared_ptr<Selector::SelectionMetrics> selectionMetrics =
            selector.createSelectionMetrics();
        selectionMetrics->initMetrics(individual, le);

        // Reset the learning Environment
        le.reset(hash, mode, iterationNumber, generationNumber);

        uint64_t nbActions = 0;
        while (!le.isTerminal() &&
               nbActions < this->params->maxNbActionsPerEval) {
            // Get the actions
            std::vector<double> actionsID =
                execEngine.execute();
            // Do it
            le.doActions(actionsID);
            // Count actions
            nbActions++;

            // Extract the metrics.
            selectionMetrics->extractMetricsStep(individual, actionsID, le);
        }

        // Extract the metrics.
        selectionMetrics->extractMetricsEpisode(individual, nbActions, le);

        // Add the extracted metrics to the total.
        globalSelectionMetrics->weightedSum(selectionMetrics, iterationNumber, 1);
    }

    // Create the EvaluationResult
    auto evaluationResult = std::shared_ptr<EvaluationResult>(
        new EvaluationResult(globalSelectionMetrics, nbEvaluationToDo));

    // Combine it with previous one if any
    if (previousEval != nullptr) {
        *evaluationResult += *previousEval;
    }
    return evaluationResult;
}

std::multimap<std::shared_ptr<Learn::EvaluationResult>, std::reference_wrapper<const Representation::Individual>>
Learn::LearningAgent::evaluateAllIndividuals(uint64_t generationNumber,
                                       Learn::LearningMode mode)
{
    std::multimap<std::shared_ptr<EvaluationResult>, std::reference_wrapper<const Representation::Individual>>
        results;


    for(const auto& pair: this->representations){
        // set current executed representation
        this->setCurrentRepresentation(&pair.second.get());

        // Evaluate the representation individuals and insert the results
        auto algoResults = this->evaluateCurrentRepresentationIndividuals(generationNumber, mode);
        results.insert(algoResults.begin(), algoResults.end());
    }

    return results;
}


std::multimap<std::shared_ptr<Learn::EvaluationResult>, std::reference_wrapper<const Representation::Individual>>
Learn::LearningAgent::evaluateCurrentRepresentationIndividuals(uint64_t generationNumber,
                                       Learn::LearningMode mode)
{
    if(this->currentExecutedRepresentation == nullptr){
        throw std::runtime_error("LearningAgent::evaluateOneRepresentationIndividuals: currentExecutedRepresentation is not set.");
    }
    if(!this->containsRepresentation(*this->currentExecutedRepresentation)){
        throw std::runtime_error("LearningAgent::evaluateOneRepresentationIndividuals: The learning agent does not contain the given representation.");
    }

    std::multimap<std::shared_ptr<EvaluationResult>, std::reference_wrapper<const Representation::Individual>>
        results;

    std::unique_ptr<Representation::ExecutionEngine> execEngine =
        this->currentExecutedRepresentation->getPopulation().createExecutionEngine();

    auto jobs = this->makeJobs(mode);
    for(auto job: jobs) {
        if (job.get()->getIdx() == 28) {
            int a = 0;
        }
        std::shared_ptr<EvaluationResult> result = this->evaluateJob(
            *execEngine, *job, generationNumber, mode, this->learningEnvironment);
        results.emplace(result, (*job).getIndividual());
    }


    // Update the representation after evaluation with the jobs processed
    this->currentExecutedRepresentation->updateAfterEvaluation(jobs, mode);

    return results;
}

std::shared_ptr<Learn::EvaluationResult> Learn::LearningAgent::evaluateOneIndividual(
    uint64_t generationNumber, Learn::LearningMode mode,
    const Representation::Individual& individual)
{
    const Representation::Representation& representation = this->getRepresentationAt(individual.getRepresentationID());

    // Create the execution engine of the individual.
    std::unique_ptr<Representation::ExecutionEngine> execEngine =
        representation.getPopulationCst().createExecutionEngine();

    // Create and evaluate the job
    auto job = representation.createJob(individual, mode, this->rng);
    std::shared_ptr<EvaluationResult> avgScore = this->evaluateJob(
        *execEngine, *job, generationNumber, mode, this->learningEnvironment);

    // Return the result
    return avgScore;
}

void Learn::LearningAgent::launchRepresentationsSelection(
            std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                          std::reference_wrapper<const Representation::Individual>>& results,
            RNG::RNG& rng)
{

    // set current best representation
    this->currentBestRepresentation = &this->getRepresentationAt(results.rbegin()->second.get().getRepresentationID());
    

    if(this->representations.size() == 1){
        // Do the selection for this representation
        this->representations.begin()->second.get().getSelector().doSelection(*this->graph, results, rng);

        // Update the evaluation records
        this->representations.begin()->second.get().getSelector().updateEvaluationRecords(results);

    } else {
        std::multimap<std::shared_ptr<Learn::EvaluationResult>,
            std::reference_wrapper<const Representation::Individual>>
            resultsCopy(results);

        results.clear();

        for(const auto& pair: representations){
            std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                        std::reference_wrapper<const Representation::Individual>>
                resultsAlgo;
            
            for(auto it = resultsCopy.begin(); it != resultsCopy.end(); ){
                if(pair.second.get().containsIndividual(it->second)){
                    resultsAlgo.insert(*it);
                    it = resultsCopy.erase(it);
                } else {
                    ++it;
                }
            }

            // Do the selection for this representation
            pair.second.get().getSelector().doSelection(*this->graph, resultsAlgo, rng);
            // Update the evaluation records
            pair.second.get().getSelector().updateEvaluationRecords(resultsAlgo);

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
        this->evaluateAllIndividuals(generationNumber, LearningMode::TRAINING);
    for (auto logger : loggers) {
        logger.get().logAfterEvaluate(results);
    }

    // Remove worst performing roots
    this->launchRepresentationsSelection(results, rng);

    for (auto logger : loggers) {
        logger.get().logAfterDecimate();
    }

    // Does a validation or not according to the parameter doValidation
    if (params->doValidation) {
        std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                      std::reference_wrapper<const Representation::Individual>>
            validationResults;

        if (generationNumber % params->stepValidation == 0 ||
            generationNumber == params->nbGenerations - 1) {
            validationResults = evaluateAllIndividuals(
                generationNumber, Learn::LearningMode::VALIDATION);
        }
        for (auto logger : loggers) {
            logger.get().logAfterValidate(validationResults);
        }
    }

    if (doPopulate) {
        // Populate Sequentially
        for(const auto& pair: representations){
            pair.second.get().populate(this->rng, this->maxNbThreads);
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

    while (!altTraining && generationNumber < this->params->nbGenerations) {
        // Train one generation
        trainOneGeneration(generationNumber,
                           generationNumber != this->params->nbGenerations - 1);
        generationNumber++;

        // Print progressBar (homemade, probably not ideal)
        if (printProgressBar) {
            printf("\rTraining ["); // back
            // filling ratio
            double ratio =
                (double)generationNumber / (double)this->params->nbGenerations;
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

std::vector<std::shared_ptr<Representation::Job>> Learn::LearningAgent::makeJobs(
    Learn::LearningMode mode)
{
    if(this->currentExecutedRepresentation == nullptr){
        throw std::runtime_error("LearningAgent::makeJobs: Current executed representation is nullptr.");
    }

    std::vector<std::shared_ptr<Representation::Job>> jobs;
    size_t idx = 0;
    for(auto individual: this->currentExecutedRepresentation->getIndividuals()){
        auto job = this->currentExecutedRepresentation->createJob(individual, mode, rng, idx);
        jobs.push_back(job);
        idx++;
    }
    return jobs;
}

bool Learn::LearningAgent::containsRepresentation(Representation::Representation& representation){
    return this->representations.find(representation.getRepresentationID()) != this->representations.end();
}