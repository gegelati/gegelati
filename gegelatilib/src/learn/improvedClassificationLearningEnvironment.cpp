#include "learn/improvedClassificationLearningEnvironment.h"

Learn::ImprovedClassificationLearningEnvironment::ImprovedClassificationLearningEnvironment(uint64_t nbClass) : LearningEnvironment(nbClass), _currentClass(0), _classificationTable(nbClass, std::vector<double>(nbClass, 0)), _currentMode(Learn::LearningMode::TRAINING)
{
    this->_dataset = new std::pair< std::vector< std::vector<double> > , std::vector<double> >();
    this->_currentSampleIndex = 0;
}

const std::vector<std::vector<double>>& Learn::ImprovedClassificationLearningEnvironment::getClassificationTable() const
{
    return this->_classificationTable;
}

void Learn::ImprovedClassificationLearningEnvironment::doAction(uint64_t actionID)
{
    LearningEnvironment::doAction(actionID);
    this->_classificationTable.at(this->_currentClass).at(actionID)++;
    this->changeCurrentSample();
}

void Learn::ImprovedClassificationLearningEnvironment::reset(size_t seed, LearningMode mode)
{
    for(auto & i : this->_classificationTable)
        for(double & j : i)
            j = 0;

    this->_currentMode = mode;
    this->_rng.setSeed(seed);
    this->_currentSampleIndex = -1;

    this->changeCurrentSample();
}

double Learn::ImprovedClassificationLearningEnvironment::getScore() const
{
    double score = 0;

    for (uint64_t classIdx = 0; classIdx < this->nbActions; classIdx++)
    {
        uint64_t truePositive = this->_classificationTable.at(classIdx).at(classIdx);

        uint64_t falseNegative = 0;
        for(int i=0 ; i<this->nbActions ; i++)
            falseNegative += this->_classificationTable.at(classIdx).at(i);
        falseNegative -= truePositive;

        uint64_t falsePositive = 0;
        for(int i=0 ; i<this->nbActions ; i++)
            falsePositive += this->_classificationTable.at(i).at(classIdx);
        falsePositive -= truePositive;

        double recall = (double)truePositive / (double)(truePositive + falseNegative);
        double precision = (double)truePositive / (double)(truePositive + falsePositive);
        // If true positive is 0, set score to 0.
        score += (truePositive != 0) ? 2 * (precision * recall) / (precision + recall) : 0.0;
    }

    score /= (double)this->nbActions;

    return score;
}

void Learn::ImprovedClassificationLearningEnvironment::changeCurrentSample()
{
        if(this->_currentMode != Learn::LearningMode::TESTING)
            this->_currentSampleIndex = this->_rng.getUnsignedInt64(0, this->_datasubset->first.size()-1);
        else
            this->_currentSampleIndex = (this->_currentSampleIndex + 1) % this->_datasubset->first.size();
}

void Learn::ImprovedClassificationLearningEnvironment::setDatasubset(DS * datasubset)
{
    this->_datasubset = datasubset;
}

std::vector<std::reference_wrapper<const Data::DataHandler>> Learn::ImprovedClassificationLearningEnvironment::getDataSources()
{
    return { this->_currentSample };
}
