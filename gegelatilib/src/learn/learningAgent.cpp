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
#include "mutator/tpgMutator.h"
#include "selector/timingSelectionMetrics.h"
#include "tpg/tpgExecutionEngine.h"

#include "learn/learningAgent.h"

std::shared_ptr<TPG::TPGGraph> Learn::LearningAgent::getTPGGraph()
{
    return this->tpg;
}

std::shared_ptr<Selector::Selector> Learn::LearningAgent::getSelector()
{
    return this->selector;
}

const Archive& Learn::LearningAgent::getArchive() const
{
    return this->archive;
}

const Environment& Learn::LearningAgent::getEnvironment() const
{
    return this->env;
}

Mutator::RNG& Learn::LearningAgent::getRNG()
{
    return this->rng;
}

void Learn::LearningAgent::init(uint64_t seed)
{
    // Initialize Randomness
    this->rng.setSeed(seed);

    // Initialize the tpg
    Mutator::TPGMutator::initRandomTPG(
        *this->tpg, params.mutation, this->rng,
        this->learningEnvironment.getNbActions());

    // Populate Sequentially
    Mutator::TPGMutator::populateTPG(
        *this->tpg, *this->selector, this->archive, this->params.mutation,
        this->rng, this->learningEnvironment.getNbActions(), maxNbThreads);

    // Clear the archive
    this->archive.clear();

    // Clear the best agent in the selector
    this->selector->forgetPreviousResults();
}

void Learn::LearningAgent::addLogger(Log::LALogger& logger)
{
    logger.doValidation = this->params.doValidation;
    logger.useUtility = this->learningEnvironment.isUsingUtility();
    logger.detailedTiming = this->params.detailedTiming;
    // logs for example the headers of the columns the logger will print
    loggers.push_back(std::reference_wrapper<Log::LALogger>(logger));
}

bool Learn::LearningAgent::isRootEvalSkipped(
    const TPG::TPGVertex& root,
    std::shared_ptr<Learn::EvaluationResult>& previousResult) const
{
    // Has the root already been evaluated more times than
    // params.maxNbEvaluationPerPolicy
    const auto& iter = this->selector->getResultsPerRoot().find(&root);
    if (iter != this->selector->getResultsPerRoot().end()) {
        // The root has already been evaluated
        previousResult = iter->second;
        return iter->second->getNbEvaluation() >=
               this->params.maxNbEvaluationPerPolicy;
    }
    else {
        previousResult = nullptr;
        return false;
    }
}

std::shared_ptr<Learn::EvaluationResult> Learn::LearningAgent::evaluateJob(
    TPG::TPGExecutionEngine& tee, const Job& job, uint64_t generationNumber,
    Learn::LearningMode mode, LearningEnvironment& le) const
{
    // Only consider the first root of jobs
    const TPG::TPGVertex* root = job.getRoot();

    // Skip the root evaluation process if enough evaluations were already
    // performed. In the evaluation mode only.
    std::shared_ptr<Learn::EvaluationResult> previousEval;
    if (mode == LearningMode::TRAINING &&
        this->isRootEvalSkipped(*root, previousEval)) {
        return previousEval;
    }

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
        this->selector->createSelectionMetrics();
    if (params.detailedTiming) {
        globalSelectionMetrics =
            std::make_shared<Selector::TimingSelectionMetrics>(
                globalSelectionMetrics);
    }
    globalSelectionMetrics->initMetrics(root, le);

    // Evaluate nbIteration times
    for (auto iterationNumber = 0; iterationNumber < nbEvaluation;
         iterationNumber++) {
        // Compute a Hash
        Data::Hash<uint64_t> hasher;
        uint64_t hash = hasher(generationNumber) ^ hasher(iterationNumber);

        // Init selectionMetrics for this episode.
        std::shared_ptr<Selector::SelectionMetrics> selectionMetrics =
            this->selector->createSelectionMetrics();
        if (params.detailedTiming) {
            selectionMetrics =
                std::make_shared<Selector::TimingSelectionMetrics>(
                    selectionMetrics);
        }
        selectionMetrics->initMetrics(root, le);

        // Reset the learning Environment
        le.reset(hash, mode, iterationNumber, generationNumber);

        if (params.detailedTiming) {
            uint64_t nbActions = 0;
            double agentTime = 0;
            double leTime = 0;
            while (!le.isTerminal() &&
                   nbActions < this->params.maxNbActionsPerEval) {
                // Get the actions
                auto startTime = std::chrono::system_clock::now();
                std::vector<double> actionsID =
                    tee.executeFromRoot(*root, le.getInitActions()).second;
                auto endTime = std::chrono::system_clock::now();
                agentTime +=
                    ((std::chrono::duration<double>)(endTime - startTime))
                        .count();
                // Do it
                startTime = endTime;
                le.doActions(actionsID);
                endTime = std::chrono::system_clock::now();
                leTime += ((std::chrono::duration<double>)(endTime - startTime))
                              .count();
                // Count actions
                nbActions++;

                // Extract the metrics.
                selectionMetrics->extractMetricsStep(root, actionsID, le);
            }

            // Extract the metrics.
            auto timedSectionMetrics =
                std::static_pointer_cast<Selector::TimingSelectionMetrics>(
                    selectionMetrics);
            timedSectionMetrics->extractMetricsEpisodeWithTiming(
                root, nbActions, le, agentTime, leTime);
        }
        else {
            uint64_t nbActions = 0;
            while (!le.isTerminal() &&
                   nbActions < this->params.maxNbActionsPerEval) {
                // Get the actions
                std::vector<double> actionsID =
                    tee.executeFromRoot(*root, le.getInitActions()).second;
                // Do it
                le.doActions(actionsID);
                // Count actions
                nbActions++;

                // Extract the metrics.
                selectionMetrics->extractMetricsStep(root, actionsID, le);
            }

            // Extract the metrics.
            selectionMetrics->extractMetricsEpisode(root, nbActions, le);
        }
        // Add the extracted metrics to the total.
        globalSelectionMetrics->weightedSum(selectionMetrics, iterationNumber,
                                            1);
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

std::multimap<std::shared_ptr<Learn::EvaluationResult>, const TPG::TPGVertex*>
Learn::LearningAgent::evaluateAllRoots(uint64_t generationNumber,
                                       Learn::LearningMode mode)
{
    std::multimap<std::shared_ptr<EvaluationResult>, const TPG::TPGVertex*>
        result;

    // Create the TPGExecutionEngine for this evaluation.
    // The engine uses the Archive only in training mode.
    std::unique_ptr<TPG::TPGExecutionEngine> tee =
        this->tpg->getFactory().createTPGExecutionEngine(
            this->env,
            (mode == LearningMode::TRAINING) ? &this->archive : NULL);

    auto roots = tpg->getRootVertices();
    for (int i = 0; i < roots.size(); i++) {
        auto job = makeJob(roots.at(i), mode);
        this->archive.setRandomSeed(job->getArchiveSeed());
        std::shared_ptr<EvaluationResult> avgScore = this->evaluateJob(
            *tee, *job, generationNumber, mode, this->learningEnvironment);
        result.emplace(avgScore, (*job).getRoot());
    }

    return result;
}

std::shared_ptr<Learn::EvaluationResult> Learn::LearningAgent::evaluateOneRoot(
    uint64_t generationNumber, Learn::LearningMode mode,
    const TPG::TPGVertex* root)
{
    // Retrieve the index of the root TPGVertex
    const std::vector<const TPG::TPGVertex*> vertices = tpg->getVertices();
    std::vector<const TPG::TPGVertex*>::const_iterator iterator =
        std::find(vertices.begin(), vertices.end(), root);
    if (iterator == vertices.end()) {
        throw std::runtime_error("The vertex to evaluate does not exist in the "
                                 "TPGGraph of the LearningAgent.");
    }

    // Create the TPGExecutionEngine for this evaluation.
    // The engine uses the Archive only in training mode.
    std::unique_ptr<TPG::TPGExecutionEngine> tee =
        this->tpg->getFactory().createTPGExecutionEngine(
            this->env,
            (mode == LearningMode::TRAINING) ? &this->archive : NULL);

    // Create and evaluate the job
    auto job = makeJob(*iterator, mode);
    this->archive.setRandomSeed(job->getArchiveSeed());
    std::shared_ptr<EvaluationResult> avgScore = this->evaluateJob(
        *tee, *job, generationNumber, mode, this->learningEnvironment);

    // Return the result
    return avgScore;
}

void Learn::LearningAgent::trainOneGeneration(uint64_t generationNumber,
                                              bool doPopulate)
{
    for (auto logger : loggers) {
        logger.get().logNewGeneration(generationNumber);
    }

    // Evaluate
    auto results =
        this->evaluateAllRoots(generationNumber, LearningMode::TRAINING);
    for (auto logger : loggers) {
        logger.get().logAfterEvaluate(results);
    }

    // Remove worst performing roots
    this->selector->launchSelection(results, rng);
    // Update the evaluation records
    this->selector->updateEvaluationRecords(results);

    for (auto logger : loggers) {
        logger.get().logAfterDecimate();
    }

    // Does a validation or not according to the parameter doValidation
    if (params.doValidation) {
        std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                      const TPG::TPGVertex*>
            validationResults;

        if (generationNumber % params.stepValidation == 0 ||
            generationNumber == params.nbGenerations - 1) {
            validationResults = evaluateAllRoots(
                generationNumber, Learn::LearningMode::VALIDATION);
        }
        for (auto logger : loggers) {
            logger.get().logAfterValidate(validationResults);
        }
    }

    if (doPopulate) {
        // Populate Sequentially
        Mutator::TPGMutator::populateTPG(
            *this->tpg, *this->selector, this->archive, this->params.mutation,
            this->rng, this->learningEnvironment.getNbActions(), maxNbThreads);
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

std::shared_ptr<Learn::Job> Learn::LearningAgent::makeJob(
    const TPG::TPGVertex* vertex, Learn::LearningMode mode, int idx,
    TPG::TPGGraph* tpgGraph)
{
    // sets the tpg to the Learning Agent's one if no one was specified
    tpgGraph = tpgGraph == nullptr ? tpg.get() : tpgGraph;

    // Before each root evaluation, set a new seed for the archive in
    // TRAINING Mode Else, archiving should be deactivate anyway
    uint64_t archiveSeed = 0;
    if (mode == LearningMode::TRAINING) {
        archiveSeed = this->rng.getUnsignedInt64(0, UINT64_MAX);
    }

    if (tpgGraph->getNbRootVertices() > 0) {
        return std::make_shared<Learn::Job>(
            Learn::Job(vertex, archiveSeed, idx));
    }
    return nullptr;
}

std::queue<std::shared_ptr<Learn::Job>> Learn::LearningAgent::makeJobs(
    Learn::LearningMode mode, TPG::TPGGraph* tpgGraph)
{
    // sets the tpg to the Learning Agent's one if no one was specified
    tpgGraph = tpgGraph == nullptr ? tpg.get() : tpgGraph;

    std::queue<std::shared_ptr<Learn::Job>> jobs;
    auto roots = tpgGraph->getRootVertices();
    for (int i = 0; i < roots.size(); i++) {
        auto job = makeJob(roots.at(i), mode, i);
        jobs.push(job);
    }
    return jobs;
}
