/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2019 - 2025) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2019 - 2020)
 * Quentin Vacher <qvacher@insa-rennes.fr> (2025)
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

#include "evaluation/evaluationResult.h"

void Evaluation::EvaluationResult::addEvaluationMetric(std::unique_ptr<EvaluationMetric> metric)
{
    std::type_index index = metric->typeId();
    if(!this->hasTypeIndex(index)) {
        this->evaluationMetrics[index] = std::vector<std::unique_ptr<EvaluationMetric>>();   
    }
    this->evaluationMetrics[index].push_back(std::move(metric));
}


size_t Evaluation::EvaluationResult::getSize() const
{
    return this->evaluationMetrics.size();
}

bool Evaluation::EvaluationResult::hasTypeIndex(const std::type_index& index) const
{
    return this->evaluationMetrics.find(index) != this->evaluationMetrics.end();
}

const std::vector<std::unique_ptr<Evaluation::EvaluationMetric>>& Evaluation::EvaluationResult::getEvaluationMetricsAt(const std::type_index& index) const
{
    if(!this->hasTypeIndex(index)) {
        throw std::runtime_error("Evaluation::EvaluationResult::getEvaluationMetricsAt: Results does not contains index " + std::string(index.name()));
    }
    return this->evaluationMetrics.at(index);
}

const std::map<std::type_index, std::vector<std::unique_ptr<Evaluation::EvaluationMetric>>>& Evaluation::EvaluationResult::getEvaluationMetrics() const
{
    return this->evaluationMetrics;
}

std::string Evaluation::EvaluationResult::toString(std::string prefix) const
{
    std::ostringstream oss;

    oss << prefix << "EvaluationResult{\n";

    for (const auto& [typeIndex, metrics] : this->evaluationMetrics) {
        oss << prefix << "\t" << DEMANGLE_TYPEID_NAME(typeIndex.name()) << " [\n";

        for (const auto& metric : metrics) {
            oss << metric->toString(prefix + "\t\t") << ",\n";
        }

        oss << prefix << "\t],\n";
    }

    oss << prefix << "}";

    return oss.str();
}