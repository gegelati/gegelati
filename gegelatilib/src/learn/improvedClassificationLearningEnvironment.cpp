#include "learn/improvedClassificationLearningEnvironment.h"

#include <algorithm>
#include <numeric>

void Learn::ImprovedClassificationLearningEnvironment::doAction(uint64_t actionID)
{
    // Base method
    LearningEnvironment::doAction(actionID);

    // Classification table update
    this->classificationTable.at(this->currentClass).at(actionID)++;
}

const std::vector<std::vector<uint64_t>>& Learn::
    ImprovedClassificationLearningEnvironment::getClassificationTable() const
{
    return this->classificationTable;
}

double Learn::ImprovedClassificationLearningEnvironment::getScore_DEFAULT() const
{
    // Compute the average f1 score over all classes
    // (chosen instead of the global f1 score as it gives an equal weight to
    // the f1 score of each class, no matter its ratio within the observed
    // population)
    double averageF1Score = 0.0;

    // for each class
    for (uint64_t classIdx = 0; classIdx < classificationTable.size();
         classIdx++) {
        uint64_t truePositive = classificationTable.at(classIdx).at(classIdx);
        uint64_t falseNegative =
            std::accumulate(classificationTable.at(classIdx).begin(),
                            classificationTable.at(classIdx).end(),
                            (uint64_t)0) -
            truePositive;
        uint64_t falsePositive = 0;
        std::for_each(classificationTable.begin(), classificationTable.end(),
                      [&classIdx, &falsePositive](
                          const std::vector<uint64_t>& classifForClass) {
                          falsePositive += classifForClass.at(classIdx);
                      });
        falsePositive -= truePositive;

        double recall =
            (double)truePositive / (double)(truePositive + falseNegative);
        double precision =
            (double)truePositive / (double)(truePositive + falsePositive);
        // If true positive is 0, set score to 0.
        double fScore = (truePositive != 0)
                            ? 2 * (precision * recall) / (precision + recall)
                            : 0.0;
        averageF1Score += fScore;
    }

    averageF1Score /= (double)this->classificationTable.size();

    return averageF1Score;
}

double Learn::ImprovedClassificationLearningEnvironment::getScore_BRSS() const
{
    double score;
    uint64_t good=0, total=0;
    auto classTable = this->classificationTable;

    for(uint64_t i=0 ; i<classTable.size() ; i++)
        for(uint64_t j=0 ; j<classTable.at(i).size() ; j++)
        {
            if(i == j)
                good += classTable.at(i).at(j);
            total += classTable.at(i).at(j);
        }

    score = (double)good / (double)total;

    return score;
}

double Learn::ImprovedClassificationLearningEnvironment::getScore_FS() const
{

}

double Learn::ImprovedClassificationLearningEnvironment::getScore() const
{
    switch(this->currentAlgo)
    {
    case(Learn::LearningAlgorithm::BRSS):
        //return this->getScore_BRSS();
        return this->getScore_DEFAULT();
    case(Learn::LearningAlgorithm::FS):
        return this->getScore_FS();
    default:
        return this->getScore_DEFAULT();
    }
}

void Learn::ImprovedClassificationLearningEnvironment::reset(size_t seed, LearningMode mode)
{
    // reset scores to 0 in classification table
    for (std::vector<uint64_t>& perClass : this->classificationTable) {
        for (uint64_t& score : perClass) {
            score = 0;
        }
    }

    //reset the RNG
    this->rng.setSeed(seed);
}

void Learn::ImprovedClassificationLearningEnvironment::setAlgorithm(Learn::LearningAlgorithm algo)
{
    this->currentAlgo = algo;
}

void Learn::ImprovedClassificationLearningEnvironment::setDataset(Learn::DS *newDataset)
{
    this->dataset->first = newDataset->first;
    this->dataset->second = newDataset->second;
    this->datasubset->first = newDataset->first;
    this->datasubset->second = newDataset->second;
}


void Learn::ImprovedClassificationLearningEnvironment::refreshDatasubset_BRSS()
{
    auto datasubsetSize = (uint64_t)floor(this->datasubsetSizeRatio * (float)this->dataset->first.size());
    auto actualSize = this->datasubset->first.size();

    // -------------------- Resize in case -------------------------------

    if(actualSize > datasubsetSize)
    {
        for(uint64_t i=datasubsetSize ; i<actualSize ; i++)
        {
            this->datasubset->first.pop_back();
            this->datasubset->second.pop_back();
        }
    }
    else if(actualSize < datasubsetSize)
    {
        for(uint64_t i=actualSize ; i<datasubsetSize ; i++)
        {
            this->datasubset->first.push_back(*new std::vector<double>());
            this->datasubset->second.push_back(0);
        }
    }

    // --------------------------- Refresh -------------------------------

    uint64_t nbSamplesToRefresh = (uint64_t)floor(this->datasubsetRefreshRatio * (float)this->datasubset->first.size());

    for(int sample=0 ; sample < nbSamplesToRefresh ; sample++)
    {
        uint64_t wanted_class = this->rng.getUnsignedInt64(0, this->nbActions-1);
        uint64_t dataset_idx = 0;
        uint64_t datasubset_idx = this->rng.getUnsignedInt64(0, this->datasubset->first.size()-1);

        while((uint64_t)this->dataset->second.at(dataset_idx) != wanted_class)
            dataset_idx = this->rng.getUnsignedInt64(0, this->dataset->first.size()-1);


        this->datasubset->first.at(datasubset_idx) = this->dataset->first.at(dataset_idx);
        this->datasubset->second.at(datasubset_idx) = this->dataset->second.at(dataset_idx);
    }
}

void Learn::ImprovedClassificationLearningEnvironment::refreshDatasubset()
{
    switch(this->currentAlgo)
    {
    case(LearningAlgorithm::BRSS):
        this->refreshDatasubset_BRSS();
        break;
    case(LearningAlgorithm::FS):
        this->refreshDatasubset_BRSS();
        break;
    default:
        this->datasubset = this->dataset;
        break;
    }
}

void Learn::ImprovedClassificationLearningEnvironment::setDatasubsetSizeRatio(float ratio)
{
    if(ratio > 0 && ratio <1)
        this->datasubsetSizeRatio = ratio;
    else
        printf("\nRatio need to be between 0 and 1, nothing have been done\n");
}

void Learn::ImprovedClassificationLearningEnvironment::setDatasubsetRefreshRatio(float ratio)
{
    if(ratio > 0 && ratio <1)
        this->datasubsetRefreshRatio = ratio;
    else
        printf("\nRatio need to be between 0 and 1, nothing have been done\n");
}

void Learn::ImprovedClassificationLearningEnvironment::changeCurrentSample(LearningMode mode)
{
    if(mode != LearningMode::TESTING)
        this->currentSampleIndex = this->rng.getUnsignedInt64(0, this->datasubset->first.size()-1);
    else
        this->currentSampleIndex = (this->currentSampleIndex + 1) % this->datasubset->first.size();

    this->currentSample.setPointer(&this->datasubset->first.at(this->currentSampleIndex));
    this->currentClass = (uint64_t)this->datasubset->second.at(this->currentSampleIndex);
}
