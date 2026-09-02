#include "evaluation/reinforcementAgent.h"




size_t Evaluation::ReinforcementAgent::getNbEvaluationIndiv(std::shared_ptr<Evaluation::EvaluationResult> previousEval, LearningMode mode) const
{
    /*if (mode != Learn::LearningMode::TRAINING) {
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
    return nbEvaluationTraining;    */
    return 0;
}


void Evaluation::ReinforcementAgent::evaluateIndividual(
    const Evolution::Individual& individual, 
    const Evolution::Representation& representation,
    uint64_t generationNumber,
    LearningMode mode) const
{
    if(!representation.isValid(individual)){
        throw std::runtime_error("Evaluation::ReinforcementAgent::evaluateIndividual: Individual not valid for the representation");
    }

    ReinforcementEnvironment& reinforcementEnvironment = dynamic_cast<ReinforcementEnvironment&>(this->learningEnvironment);

    // Skip the individual evaluation process if enough evaluations were already
    // performed. In the evaluation mode only.
    //std::shared_ptr<Evaluation::EvaluationResult> previousEval =nullptr;// = selector.getResultsOf(individual);
    size_t nbEvaluationToDo = params->nbIterationsPerPolicyValidation;//this->getNbEvaluationIndiv(previousEval, mode);

    // Evaluate nbIteration times
    for (auto iterationNumber = 0; iterationNumber < nbEvaluationToDo;
         iterationNumber++) {
        // Compute a Hash
        Data::Hash<uint64_t> hasher;
        uint64_t hash = hasher(iterationNumber) ^ hasher(static_cast<int>(mode)) ^ hasher(this->seed);
        if(mode == LearningMode::TRAINING) { 
            // In training, hash should take into consideration the generation number, else not (we don't want validation to change between generations).
            hash = hasher(generationNumber) ^ hash;
        }

        // Reset the learning Environment
        reinforcementEnvironment.reset(hash, mode, iterationNumber, generationNumber);

        // create Evaluation run for this episode with default metric for now.
        std::unique_ptr<EvaluationRun> evaluationRun = std::move(this->createEvaluationRun());

        // Init the metrics of the run
        for(const auto& metric: evaluationRun->getMetrics()) {
            metric->initMetrics(individual, learningEnvironment, hash);
        }

        uint64_t nbActions = 0;
        while (!reinforcementEnvironment.isTerminal() &&
               nbActions < this->params->maxNbActionsPerEval) {
            // Get the actions
            Data::DataValue action =
                std::move(representation.executeIndividual(individual, learningEnvironment.getDataSources()));

            // Do it
            reinforcementEnvironment.doAction(action);
            // Count actions
            nbActions++;

            // Extract the metrics of current stpe.
            for(const auto& metric: evaluationRun->getMetrics()) {
                metric->extractMetricsStep(individual, action.view(), learningEnvironment);
            }
        }

        // Extract the final metrics of the run.
        for(const auto& metric: evaluationRun->getMetrics()) {
            metric->extractMetricsRun(individual, nbActions, learningEnvironment);
        }

        // Add the evaluationRun to the evaluationResult.
        individual.addEvaluationRun(std::move(evaluationRun), hash);
    }
}


