#include "learn/improvedClassificationLearningEnvironment.h"

Learn::ImprovedClassificationLearningEnvironment::ImprovedClassificationLearningEnvironment(uint64_t nbClass) : LearningEnvironment(nbClass), _currentClass(0), _classificationTable(nbClass, std::vector<uint64_t>(nbClass, 0)), _currentMode(Learn::LearningMode::TRAINING)
{
    this->_dataset = new std::pair< std::vector< std::vector<double> > , std::vector<double> >();
    this->_currentSampleIndex = 0;
    this->setRefreshRatio(0.3);
}

const std::vector<std::vector<uint64_t>>& Learn::ImprovedClassificationLearningEnvironment::getClassificationTable() const
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
        for(uint64_t & j : i)
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

bool Learn::ImprovedClassificationLearningEnvironment::isTerminal() const
{
    return false;
}

void Learn::ImprovedClassificationLearningEnvironment::setDataset(Learn::DS *dataset)
{
    this->_dataset = dataset;
    this->_datasubset = dataset;
    this->changeCurrentSample();
}

void Learn::ImprovedClassificationLearningEnvironment::refreshDatasubset(Learn::LearningAlgorithm algo, size_t seed)
{
    if(algo == BRSS || algo == FS)
        refreshDatasubset_BRSS(seed);
    else if(algo == BANDIT || algo == LEXICASE)
        refreshDatasubset_BANDIT(seed);
}

void Learn::ImprovedClassificationLearningEnvironment::refreshDatasubset_BRSS(size_t seed)
{
    srand(seed);
    for(int i=0 ; i<this->_datasubsetRefreshRatio * (double)this->_datasubset->first.size() ; i++)
    {
        auto idx_ds = rand() % this->_dataset->first.size(), idx_dss = rand() % this->_datasubset->first.size();
        for(int k=0 ; k<this->_dataset->first.size() ; k++)
        {
            if(k < this->_datasubset->first.size())
                this->_datasubset->first.at(idx_dss).at(k) = this->_dataset->first.at(idx_ds).at(k);
            else
                this->_datasubset->first.at(idx_dss).push_back(this->_dataset->first.at(idx_ds).at(k));
        }

        if(this->_datasubset->first.size() > this->_dataset->first.size())
            for(int k=0 ; k<(this->_datasubset->first.size() - this->_dataset->first.size()) ; k++)
                this->_datasubset->first.pop_back();

        this->_datasubset->second.at(idx_dss) = this->_dataset->second.at(idx_ds);
    }
}

void Learn::ImprovedClassificationLearningEnvironment::refreshDatasubset_BANDIT(size_t seed)
{

}

void Learn::ImprovedClassificationLearningEnvironment::setRefreshRatio(float ratio)
{
    if(ratio < 1 && ratio > 0)
        this->_datasubsetRefreshRatio = ratio;
    else
        printf("Datasubset Refreshing Ratio Must Be between 0 and 1. Nothing is done.\n");
}
