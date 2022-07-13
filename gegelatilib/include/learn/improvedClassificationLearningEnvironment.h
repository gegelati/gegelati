#ifndef GEGELATI_IMPROVEDCLASSIFICATIONLEARNINGENVIRONMENT_H
#define GEGELATI_IMPROVEDCLASSIFICATIONLEARNINGENVIRONMENT_H

#include "gegelati.h"
#include "learningEnvironment.h"

namespace Learn
{
    using DS = std::pair<std::vector<std::vector<double>>,std::vector<double>>;

    class ImprovedClassificationLearningEnvironment : public LearningEnvironment
    {
      protected:
        std::vector<std::vector<double>> _classificationTable;
        uint64_t _currentClass, _currentSampleIndex;
        DS * _dataset, * _datasubset;
        Learn::LearningMode _currentMode;
        Mutator::RNG _rng;
        Data::Array2DWrapper<double> _currentSample;

        void changeCurrentSample();

      public:
        explicit ImprovedClassificationLearningEnvironment(uint64_t nbClass);

        [[nodiscard]] const std::vector<std::vector<double>>& getClassificationTable() const;
        void doAction(uint64_t actionID) override = 0;
        [[nodiscard]] double getScore() const override;
        void reset(size_t seed = 0, LearningMode mode = LearningMode::TRAINING) override = 0;

        // Getters and setters
        void setDatasubset(DS * datasubset);
        std::vector<std::reference_wrapper<const Data::DataHandler>> getDataSources() override;
    };
}

#endif // GEGELATI_IMPROVEDCLASSIFICATIONLEARNINGENVIRONMENT_H
