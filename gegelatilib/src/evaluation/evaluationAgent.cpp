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



#include "evaluation/evaluationAgent.h"

RNG::RNG& Learn::EvaluationAgent::getRNG()
{
    return this->rng;
}

size_t Learn::EvaluationAgent::getNbEvaluationIndiv(std::shared_ptr<Learn::EvaluationResult> previousEval, Learn::LearningMode mode) const
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


std::shared_ptr<Learn::EvaluationResult> Learn::EvaluationAgent::evaluateIndividual(
    const Evolution::Individual& individual, 
    const Evolution::Representation& representation,
    const Selector::Selector& selector,
    Learn::LearningEnvironment& le,
    uint64_t generationNumber,
    Learn::LearningMode mode) const
{
    if(!representation.isValid(individual)){
        throw std::runtime_error("Learn::EvaluationAgent::evaluateIndividual: Individual not valid for the representation");
    }

    Representation::Individual falseIndividual(0);

    // Skip the individual evaluation process if enough evaluations were already
    // performed. In the evaluation mode only.
    std::shared_ptr<Learn::EvaluationResult> previousEval =nullptr;// = selector.getResultsOf(individual);
    size_t nbEvaluationToDo = this->getNbEvaluationIndiv(previousEval, mode);

    // Init global selection metric
    std::shared_ptr<Selector::SelectionMetrics> globalSelectionMetrics =
        selector.createSelectionMetrics();
    globalSelectionMetrics->initMetrics(falseIndividual, le);


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
        selectionMetrics->initMetrics(falseIndividual, le);

        // Reset the learning Environment
        le.reset(hash, mode, iterationNumber, generationNumber);

        uint64_t nbActions = 0;
        while (!le.isTerminal() &&
               nbActions < this->params->maxNbActionsPerEval) {
            // Get the actions
            std::vector<double> actionsID =
                representation.executeIndividual(individual, le.getDataSources());
            // Do it
            le.doActions(actionsID);
            // Count actions
            nbActions++;

            // Extract the metrics.
            selectionMetrics->extractMetricsStep(falseIndividual, actionsID, le);
        }

        // Extract the metrics.
        selectionMetrics->extractMetricsEpisode(falseIndividual, nbActions, le);

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



std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                        std::reference_wrapper<const Evolution::Individual>>
Learn::EvaluationAgent::evaluateIndividuals(
    const std::vector<std::reference_wrapper<const Evolution::Individual>>& individuals, 
    const Evolution::Representation& representation,
    const Selector::Selector& selector,
    uint64_t generationNumber,
    Learn::LearningMode mode) const
{
    std::multimap<std::shared_ptr<EvaluationResult>, std::reference_wrapper<const Evolution::Individual>>
        results;


    for(const Evolution::Individual& indiv: individuals){
        // Evaluate the individuals and insert the results
        const auto& result = this->evaluateIndividual(
            indiv, representation, selector, this->learningEnvironment, generationNumber, mode
        );
        
        results.insert({result, indiv});
    }

    return results;
}