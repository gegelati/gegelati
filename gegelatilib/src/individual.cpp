
#include "individual.h"

// Declaration of static individual ID Counter in local here because it creates
// error in the .h file for MSVC compiler See:
// https://discourse.cmake.org/t/exporting-a-static-data-member-of-a-class-for-dll-using-msvc/5892
static size_t INDIVIDUAL_COUNTER_ID = 0;

size_t Individual::incrementeCounter()
{
    return INDIVIDUAL_COUNTER_ID++;
}

size_t Individual::getIndividualIDCounter()
{
    return INDIVIDUAL_COUNTER_ID;
}

void Individual::resetIndividualIDCounter()
{
    INDIVIDUAL_COUNTER_ID = 0;
}

size_t Individual::getIndividualID() const
{
    return this->individualID;
}

void Individual::setIndividualID(size_t newID)
{
    this->individualID = newID;

    // Update the ID counter if needed
    if (newID >= INDIVIDUAL_COUNTER_ID) {
        INDIVIDUAL_COUNTER_ID = newID + 1;
    }
}        

size_t Individual::getSize() const
{
    return this->genotype->getFullSize();
}

const Representations::Representation& Individual::getRepresentation() const
{
    return this->representation;
}

const GraphBased::Genotype& Individual::getGenotype() const
{
    return *this->genotype;
}

void Individual::setGenotype(std::unique_ptr<GraphBased::Genotype> genotype)
{
    this->genotype = std::move(genotype);
    this->updateValidity();
}

void Individual::updateValidity()
{
    this->valid = this->representation.isValid(*this->genotype);
}
       
bool Individual::isValid() const
{
    return this->valid;
}

std::unique_ptr<Individual> Individual::cloneUniquePtr() const
{
    return std::make_unique<Individual>(this->representation, this->genotype->cloneUniquePtr());
}

std::shared_ptr<Individual> Individual::cloneSharedPtr() const
{
    return std::make_shared<Individual>(this->representation, this->genotype->cloneUniquePtr());
}

void Individual::addEvaluationMetric(std::unique_ptr<Evaluation::EvaluationMetric> metric) const
{
    this->result->addEvaluationMetric(std::move(metric));
}

const Evaluation::EvaluationResult& Individual::getEvaluationResult() const
{
    return *this->result;
}


Data::DataValue Individual::execute(const std::vector<Data::DataView>& inputSources) const
{
    // Individual need to be valid to be executed.
    if(!valid){
        throw std::runtime_error("Individual::execute: Individual's genotype not valid for the representation");
    }
    return this->representation.execute(*this->genotype, inputSources);
}

bool operator<(const Individual& a, const Individual& b)
{
    return a.getIndividualID() < b.getIndividualID();
}

bool operator==(const Individual& a, const Individual& b)
{
    return a.getIndividualID() == b.getIndividualID();
}
bool operator!=(const Individual& a, const Individual& b)
{
    return !(a==b);
}
bool operator>(const Individual& a, const Individual& b)
{
    return b < a;
}
bool operator<=(const Individual& a, const Individual& b)
{
    return (a < b) || (a == b);
}
bool operator>=(const Individual& a, const Individual& b)
{
    return (a > b) || (a == b);
}