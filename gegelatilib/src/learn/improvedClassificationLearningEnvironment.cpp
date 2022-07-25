#include "learn/improvedClassificationLearningEnvironment.h"

Learn::ImprovedClassificationLearningEnvironment::ImprovedClassificationLearningEnvironment(uint64_t nbClass, uint64_t sampleSize) : LearningEnvironment(nbClass), _classificationTable(nbClass, std::vector<uint64_t>(nbClass, 0)), _sampleSize(sampleSize), _currentMode(Learn::LearningMode::TRAINING),
      _currentSample(sampleSize, sampleSize)
{
    this->_dataset = new Learn::DS();
    this->_currentSampleIndex = -1;
    this->_datasubsetRefreshRatio = 0.3;
    this->_datasubsetSizeRatio = 0.4;
    this->_currentClass = -1;
    this->_datasubset = new Learn::DS();
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

double Learn::ImprovedClassificationLearningEnvironment::getScore_DEFAULT() const
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

double Learn::ImprovedClassificationLearningEnvironment::getScore_BRSS() const
{
    int good = 0, total = 0;

    for(int i=0 ; i<this->_classificationTable.size() ; i++)
        for(int j=0 ; j<this->_classificationTable.at(i).size() ; j++)
        {
            total += this->_classificationTable.at(i).at(j);
            if(i == j)
                good += this->_classificationTable.at(i).at(j);
        }

    return (double)good / (double)total;
}

double Learn::ImprovedClassificationLearningEnvironment::getScore() const
{
    double score = 0;

    //    if(algo == Learn::LearningAlgorithm::BRSS)
    //        score = getScore_BRSS();
    //    else if(algo == Learn::LearningAlgorithm::FS || algo == Learn::LearningAlgorithm::BANDIT)
    //        score = 0;
    //    else if(algo == Learn::LearningAlgorithm::LEXICASE)
    //        score = 0;
    //    else
    //        score = getScore_DEFAULT();

    score = getScore_BRSS();

    return score;
}

void Learn::ImprovedClassificationLearningEnvironment::changeCurrentSample()
{
    if(this->_currentMode != Learn::LearningMode::TESTING)
        this->_currentSampleIndex = this->_rng.getUnsignedInt64(0, this->_datasubset->first.size()-1);
    else
        this->_currentSampleIndex = (this->_currentSampleIndex + 1) % this->_datasubset->first.size();

    if(this->_datasubset->first.size() > 0)
        this->_currentClass = (uint64_t)this->_datasubset->second.at(this->_currentSampleIndex);
}

void Learn::ImprovedClassificationLearningEnvironment::setDatasubset(DS * datasubset)
{
    this->_datasubset->first = std::vector< std::vector<double> >(datasubset->first);
    this->_datasubset->second = std::vector<double>(datasubset->second);
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
    this->_dataset->first = std::vector< std::vector<double> >(dataset->first);
    this->_dataset->second =  std::vector<double>(dataset->second);
    this->setDatasubset(dataset);

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
    int samplesToRefresh = (int) (this->_datasubsetRefreshRatio * (double)this->_dataset->first.size());
    int subsetSize = (int) (this->_datasubsetSizeRatio * (double)this->_dataset->first.size());

    if(this->_datasubset->first.size() > subsetSize)
    {
        uint64_t temp_size = this->_datasubset->first.size();
        for(int i=0 ; i<temp_size-subsetSize ; i++)
        {
            this->_datasubset->first.pop_back();
            this->_datasubset->second.pop_back();
        }
    }
    else if(this->_datasubset->first.size() < subsetSize)
    {
        uint64_t temp_size = this->_datasubset->first.size();
        for(int i=temp_size ; i<subsetSize ; i++)
        {
            this->_datasubset->first.push_back(*new std::vector<double>());
            this->_datasubset->second.push_back(0);
        }
    }

    for(int i=0 ; i<samplesToRefresh ; i++)
    {
        int idx_ds = rand() % this->_dataset->first.size(), idx_dss = (rand() % this->_datasubset->first.size());

        this->_datasubset->first.at(idx_dss) = this->_dataset->first.at(idx_ds);
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
