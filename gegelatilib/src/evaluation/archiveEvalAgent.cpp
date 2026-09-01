#include "evaluation/archiveEvalAgent.h"
#if 0

void Evaluation::ArchiveEvalAgent::evaluateIndividual(
    const Evolution::Individual& individual, 
    const Evolution::Representation& representation,
    uint64_t generationNumber,
    LearningMode mode) const
{
    if(!representation.isValid(individual)){
        throw std::runtime_error("Evaluation::ArchiveEvalAgent::evaluateIndividual: Individual not valid for the representation");
    }


    Evaluation::ArchiveEnvironment& archive = dynamic_cast<Evaluation::ArchiveEnvironment&>(this->learningEnvironment);
    // Create a list with all individual IDs.
    std::set<size_t> currentIndenticalIndivID = archive.getCurrentIDs();

    double tau = 1e-4;

    // For each pair input/outputs in the archive, compare the current individual output to the outputs of every individuals recorded.
    for (size_t idx = 0; idx < archive.getCurrentSize(); idx++) {
        const std::vector<std::reference_wrapper<const Data::DataHandler>>& input = archive.getInput(idx);
        // Get the outputs
        double outputs =
            representation.executeIndividual(individual, input).at(0);

        
        // Execute only if the id is still in the set of possible identical IDs.
        for (auto it=currentIndenticalIndivID.begin(); it!=currentIndenticalIndivID.end();) {
            if(std::abs(outputs - archive.getOutput(idx, *it)) > tau) {
                it = currentIndenticalIndivID.erase(it);
            } else {
                it++;
            }
        }

        // No need to continue
        if(currentIndenticalIndivID.empty()) {
            break;
        }
    }


    // Novelty score is 1 if the identical list is empty, else 0
    double noveltyScore = double(currentIndenticalIndivID.empty());
    individual.addEvaluationRun(
        std::move(std::make_unique<EvaluationRun>(
           std::move(std::make_unique<ScoreMetric>(noveltyScore))))
        , 0
    );
}





void Evaluation::ArchiveEvalAgent::evaluateIndividuals(
    const std::set<std::reference_wrapper<const Evolution::Individual>>& individuals, 
    const Evolution::Representation& representation,
    uint64_t generationNumber,
    LearningMode mode) const
{
    Evaluation::ArchiveEnvironment& archive = dynamic_cast<Evaluation::ArchiveEnvironment&>(this->learningEnvironment);
    archive.updateArchiveInputs();
    archive.updateArchiveOutputs(representation);

    // Evaluate the individuals
    for(const Evolution::Individual& indiv: individuals){
        this->evaluateIndividual(
            indiv, representation, generationNumber, mode
        );
    }
}

#endif