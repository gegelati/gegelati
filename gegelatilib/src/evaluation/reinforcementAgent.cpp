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
    const Individual& individual, 
    uint64_t generationNumber,
    LearningMode mode) const
{

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

        // create Evaluation metric and initialize them.
        std::vector<std::unique_ptr<EvaluationMetric>> metrics = std::move(this->createEvaluationMetrics(hash));
        for(const std::unique_ptr<EvaluationMetric>& metric: metrics) { 
            metric->initExtraction(individual, learningEnvironment); }

        uint64_t nbActions = 0;
        while (!reinforcementEnvironment.isTerminal() &&
               nbActions < this->params->maxNbActionsPerEval) {


            // Extract the metrics before execution
            for(const std::unique_ptr<EvaluationMetric>& metric: metrics) { 
                metric->extractBeforeExecution(individual, learningEnvironment); }

            // Get the actions
            Data::DataValue action = std::move(individual.execute(learningEnvironment.getDataSources()));

            
            // Extract the metrics after execution
            for(const std::unique_ptr<EvaluationMetric>& metric: metrics) { 
                metric->extractAfterExecution(individual, action, learningEnvironment); }

            // Do it
            reinforcementEnvironment.doAction(action);

            // Extract the metrics after environment step
            for(const std::unique_ptr<EvaluationMetric>& metric: metrics) { 
                metric->extractAfterStep(individual, learningEnvironment); }

            // Count actions
            nbActions++;
        }

        // Extract metric at the end of the run, and add the metric to the individual.
        for(auto it = metrics.begin(); it != metrics.end(); it++) { 
            (*it)->extractMetricRun(individual, nbActions, learningEnvironment); 
            individual.addEvaluationMetric(std::move(*it));
        }
    }
}


