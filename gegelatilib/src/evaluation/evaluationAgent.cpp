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

void Evaluation::EvaluationAgent::addRequestedMetric(const EvaluationMetric& metric)
{
    // Don't add twice the same metric :)
    for(const auto& currentMetric: this->requestedMetrics){
        if(currentMetric->typeId() == metric.typeId()) {
            return;
        }
    }
    this->requestedMetrics.push_back(std::move(metric.cloneEmptyUniquePtr(0)));
}

std::vector<std::unique_ptr<Evaluation::EvaluationMetric>> Evaluation::EvaluationAgent::createEvaluationMetrics(size_t seed) const
{
    std::vector<std::unique_ptr<EvaluationMetric>> metrics;
    for(const auto& metric: this->requestedMetrics) {
        metrics.push_back(metric->cloneEmptyUniquePtr(seed));
    }
    return std::move(metrics);
}



const std::vector<Dimensions::Requirement>& Evaluation::EvaluationAgent::getInputDimensions() const
{
    return this->problem.getInputDimensions();
}

const Dimensions::Requirement& Evaluation::EvaluationAgent::getOutputDimension() const
{
    return this->problem.getOutputDimension();
}

std::string Evaluation::EvaluationAgent::summary() const
{
    std::ostringstream result;            
    result << "Inputs (" << this->problem.getInputDimensions().size() << "):\n";
    for (const auto& input : this->problem.getInputDimensions()) {
        result << "  * " << input.summary() << "\n";
    }
    result << "\nOutput: " << this->problem.getOutputDimension() << "\n";
    return result.str();
}


void Evaluation::EvaluationAgent::evaluateIndividuals(
    const std::set<std::shared_ptr<const Individual>, SharedLess<Individual>>& individuals, 
    uint64_t generationNumber,
    LearningMode mode) const
{
    // Evaluate the individuals and insert the results
    for(const std::shared_ptr<const Individual>& indiv: individuals){
        this->evaluateIndividual(*indiv, this->problem, generationNumber, mode);        
    }
}

