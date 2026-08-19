/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2019 - 2025) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2019 - 2022)
 * Nicolas Sourbier <nsourbie@insa-rennes.fr> (2020)
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

#include <algorithm>
#include <fstream>
#include <gtest/gtest.h>
#include <numeric>

#include "instructions/set.h"
#include "instructions/lambdaInstruction.h"
#include "evolution/evolutionAlgorithm.h"
#include "learn/stickGameWithOpponentDupDouble.h"

#include "representations/lgpRepresentation.h"
#include "representations/tpgRepresentation.h"

#include "util/counterReset.h"
// Set all file in comment

class EvolutionAlgorithmTest : public ::testing::Test
{
  protected:
    Instructions::Set set;
    Data::PrimitiveTypeArray<double>* inputSource;
    Evolution::Representation* representation;

    std::unique_ptr<Learn::LearningParameters> evalParams;

    StickGameWithOpponentD le;


    virtual void SetUp()
    {
        CounterReset::counterReset();
        auto add = [](double a, double b) -> double { return a + b; };
        auto minus = [](double a, double b) -> double { return a - b; };
        auto times = [](double a, double b) -> double { return a * b; };
        auto div = [](double a, double b) -> double { return a / b; };
        
        set.add(*(new Instructions::LambdaInstruction<double, double>(add)));
        set.add(*(new Instructions::LambdaInstruction<double, double>(minus)));
        set.add(*(new Instructions::LambdaInstruction<double, double>(times)));
        set.add(*(new Instructions::LambdaInstruction<double, double>(div)));

        inputSource = new Data::PrimitiveTypeArray<double>(4);
    
        representation = new Representations::LGPRepresentation(set, 8, 10);

        evalParams = std::make_unique<Learn::LearningParameters>();
    }

    virtual void TearDown()
    {
        delete (&set.getInstruction(0));
        delete (&set.getInstruction(1));
        delete (&set.getInstruction(2));
        delete (&set.getInstruction(3));
        delete inputSource;
        delete representation;
    }
};


TEST_F(EvolutionAlgorithmTest, Constructor)
{
    Evolution::EvolutionAlgorithm* ea;

    ASSERT_NO_THROW(ea = new Evolution::EvolutionAlgorithm(*representation, le, std::move(evalParams), 12, 10)) << "Constructor of EA failed.";

    ASSERT_EQ(ea->getRepresentation().getMaxNbNodes(), representation->getMaxNbNodes()) << "Constructor should have copied the representation";

    ASSERT_NO_THROW(ea->getMutation()) << "For Coverage :D";
    ASSERT_NO_THROW(ea->getEvaluation()) << "For Coverage :D";
    ASSERT_NO_THROW(ea->getSelector()) << "For Coverage :D";
    ASSERT_NO_THROW(ea->getRNG()) << "For Coverage :D";

    ASSERT_NO_THROW(delete ea) << "Destructor of EA failed.";
}

TEST_F(EvolutionAlgorithmTest, initializePopulation)
{
    Evolution::EvolutionAlgorithm ea(*representation, le, std::move(evalParams), 12, 10);

    ASSERT_NO_THROW(ea.initializePopulation()) << "Initialization of population failed.";

    ASSERT_EQ(ea.getPopulation().size(), 100) << "Population size is wrong after initialization.";

    for(const Evolution::Individual& indiv: ea.getPopulation().getIndividuals()) {
        ASSERT_TRUE(ea.getRepresentation().isValid(indiv)) << "An individual is not valid after population initialization";
    }
}

TEST_F(EvolutionAlgorithmTest, selectParents) 
{
    Evolution::EvolutionAlgorithm ea(*representation, le, std::move(evalParams), 12, 10);
    ea.initializePopulation();

    std::vector<std::reference_wrapper<const Evolution::Individual>> parents;
    ASSERT_NO_THROW(parents = ea.selectParents(100)) << "Selecting parents failed";
    for(const Evolution::Individual& parent: parents) {
        ASSERT_TRUE(ea.getPopulation().containsIndividual(parent)) << "Parent should be contained in the population";
    }

    std::vector<std::reference_wrapper<const Evolution::Individual>> parents2;
    ASSERT_NO_THROW(parents2 = ea.selectParents(100)) << "Selecting parents failed";
    bool sameParents = true;
    for(size_t idx = 0; idx < 100 && sameParents; idx++) {
        sameParents = parents.at(idx) == parents2.at(idx);
    }
    ASSERT_FALSE(sameParents) << "It is very unlikely that the parents were sampled in the same order";
}

TEST_F(EvolutionAlgorithmTest, reproduceParents) 
{
    
    Evolution::EvolutionAlgorithm ea(*representation, le, std::move(evalParams), 12, 10);
    ea.initializePopulation();
    std::vector<std::reference_wrapper<const Evolution::Individual>> parents = ea.selectParents(100);

    ASSERT_EQ(ea.getPopulation().size(), 100) << "Population Size should be 100 before reproduction";

    std::set<std::unique_ptr<Evolution::Individual>, UniqueLess<Evolution::Individual>> offspring;
    ASSERT_NO_THROW(offspring = ea.reproduceParents(parents)) << "Reproducing failed.";

    ASSERT_EQ(ea.getPopulation().size(), 100) << "Population Size should still be 100 after reproduction";
    ASSERT_EQ(offspring.size(), 100) << "Offspring Size should be 100 after reproduction";

    size_t idx = 0;
    for(auto it = offspring.begin(); it != offspring.end(); it++) {
        ASSERT_TRUE((*it)->getGenotype() == parents.at(idx).get().getGenotype()) << "Offspring should have the same genotype has their parents";
        idx++;
    }
}

TEST_F(EvolutionAlgorithmTest, mutateOffspring) 
{
    Evolution::EvolutionAlgorithm ea(*representation, le, std::move(evalParams), 12, 10);
    ea.initializePopulation();
    std::vector<std::reference_wrapper<const Evolution::Individual>> parents = ea.selectParents(100);
    std::set<std::unique_ptr<Evolution::Individual>, UniqueLess<Evolution::Individual>> offspring = ea.reproduceParents(parents);


    size_t idx = 0;
    for(auto it = offspring.begin(); it != offspring.end(); it++) {
        ASSERT_TRUE((*it)->getGenotype() == parents.at(idx).get().getGenotype()) << "Offspring should have the same genotype has their parents";
        idx++;
    }

    ASSERT_NO_THROW(ea.mutateOffspring(offspring)) << "Mutating the offspring vector failed";
    idx = 0;
    for(auto it = offspring.begin(); it != offspring.end(); it++) {
        ASSERT_TRUE((*it)->getGenotype() != parents.at(idx).get().getGenotype()) << "Offspring should have a different genotype has their parents after mutation with high probability of mutation";
        idx++;
    }
}



TEST_F(EvolutionAlgorithmTest, evaluatePopulation) 
{
    Evolution::EvolutionAlgorithm ea(*representation, le, std::move(evalParams), 12, 10);
    ea.initializePopulation();

    std::set<std::unique_ptr<Evolution::Individual>, UniqueLess<Evolution::Individual>> offspring = ea.reproduceParents(ea.selectParents(100));
    ea.mutateOffspring(offspring);

    std::multimap<std::shared_ptr<Learn::EvaluationResult>, std::reference_wrapper<const Evolution::Individual>> results1;
    ASSERT_NO_THROW(results1 = ea.evaluatePopulation(offspring, 0, Learn::LearningMode::TRAINING)) << "Evaluation failed";

    ASSERT_EQ(results1.size(), ea.getPopulation().size() + offspring.size()) << "Results should have the size of the current population + offspring size";
    
    std::multimap<std::shared_ptr<Learn::EvaluationResult>, std::reference_wrapper<const Evolution::Individual>> results2;
    ASSERT_NO_THROW(results2 = ea.evaluatePopulation(offspring, 0, Learn::LearningMode::TRAINING)) << "Evaluation of entire population failed";

    auto it1 = results1.begin();
    auto it2 = results2.begin();
    while(it1 != results1.end() && it2 != results2.end()) {
        ASSERT_EQ(it1->first->getSelectionMetrics()->getScore(), it2->first->getSelectionMetrics()->getScore()) << "EvaluationResults scores should be equal";
        ASSERT_EQ(it1->second, it2->second) << "Individuals should be equal";

        it1++; it2++;
    }
}

TEST_F(EvolutionAlgorithmTest, survivorSelection) 
{
    Evolution::EvolutionAlgorithm ea(*representation, le, std::move(evalParams), 12, 10);
    ea.initializePopulation();

    std::set<std::unique_ptr<Evolution::Individual>, UniqueLess<Evolution::Individual>> offspring = ea.reproduceParents(ea.selectParents(100));
    ea.mutateOffspring(offspring);

    std::multimap<std::shared_ptr<Learn::EvaluationResult>, std::reference_wrapper<const Evolution::Individual>> results = 
        ea.evaluatePopulation(offspring, 0, Learn::LearningMode::TRAINING);
    std::multimap<std::shared_ptr<Learn::EvaluationResult>, std::reference_wrapper<const Evolution::Individual>> resultsCopy(results);

    ASSERT_EQ(results.size(), 200) << "Results size should be 200 before replacement";

    std::map<std::reference_wrapper<const Evolution::Individual>, bool> survivors;
    ASSERT_NO_THROW(survivors = ea.selectSurvivors(results)) << "Fail to replace population";

    ASSERT_EQ(results.size(), 100) << "Results size should be 100 after replacement";
    ASSERT_EQ(survivors.size(), 200) << "survivors size should be 200 after replacement";
    size_t nbSurvived = 0;
    size_t nbDeleted = 0;
    for(const auto& pair: survivors) {
        if(pair.second) {
            nbSurvived++;
        } else {
            nbDeleted++;
        }
    }
    ASSERT_EQ(nbSurvived, 100) << "100 individuals should have survived";
    ASSERT_EQ(nbDeleted, 100) << "100 individuals should have been set as deleted";

    auto rit = results.rbegin();
    auto ritCopy = resultsCopy.rbegin();
    while (rit != results.rend() && ritCopy != resultsCopy.rend()) {
        ASSERT_EQ(rit->first->getSelectionMetrics()->getScore(), ritCopy->first->getSelectionMetrics()->getScore()) << "EvaluationResults scores should be equal";
        ASSERT_EQ(rit->second, ritCopy->second) << "Individuals should be equal";

        rit++; ritCopy++;
    }
}

TEST_F(EvolutionAlgorithmTest, replacePopulation) 
{
    Evolution::EvolutionAlgorithm ea(*representation, le, std::move(evalParams), 12, 10);
    ea.initializePopulation();

    std::set<std::unique_ptr<Evolution::Individual>, UniqueLess<Evolution::Individual>> offspring = ea.reproduceParents(ea.selectParents(100));
    ea.mutateOffspring(offspring);

    std::multimap<std::shared_ptr<Learn::EvaluationResult>, std::reference_wrapper<const Evolution::Individual>> results = 
        ea.evaluatePopulation(offspring, 0, Learn::LearningMode::TRAINING);
    std::map<std::reference_wrapper<const Evolution::Individual>, bool> survivors = ea.selectSurvivors(results);

    size_t minIndexOffspring = offspring.begin()->get()->getIndividualID();
    
    ASSERT_EQ(offspring.size(), 100) << "offspring size should be 100 before replacement";

    ASSERT_NO_THROW(ea.replacePopulation(offspring, survivors)) << "Fail to replace population";

    ASSERT_EQ(ea.getPopulation().size(), 100) << "Population size should be 100 after replacement";
    ASSERT_EQ(offspring.size(), 0) << "Offspring size should be 0 after replacement";

    size_t nbOffspringSurviving = 0;
    std::vector<std::reference_wrapper<const Evolution::Individual>> indivs(ea.getPopulation().getIndividuals());
    for(size_t idx = 0; idx < ea.getPopulation().size(); idx++){
        if(indivs.at(idx).get().getIndividualID() >= minIndexOffspring) {
            nbOffspringSurviving = ea.getPopulation().size() - idx;
            break;
        }
    }
    ASSERT_GE(nbOffspringSurviving, 1) << "At least one offspring should have survive";


    
    offspring = ea.reproduceParents(ea.selectParents(100));
    ea.mutateOffspring(offspring);

    results = ea.evaluatePopulation(offspring, 0, Learn::LearningMode::TRAINING);
    survivors = ea.selectSurvivors(results);

    ASSERT_EQ(ea.getPopulation().sizeProtected(), 0) << "No individual should be protected";

    // First individual will be eliminated.
    results = ea.evaluatePopulation(offspring, 0, Learn::LearningMode::TRAINING);

    // Protect this individual before suppression
    std::vector<std::weak_ptr<const Evolution::Individual>> indivPtrs(ea.getPopulation().getIndividualPtrs());
    std::shared_ptr<const Evolution::Individual> indivSharedPtr =nullptr;
    for(const std::weak_ptr<const Evolution::Individual>& indivPtr: indivPtrs){
        for(const auto& pair: survivors) {
            const Evolution::Individual& survivor = pair.first;
            if(!pair.second && indivPtr.lock().get() == &survivor) {
                indivSharedPtr = indivPtr.lock();
                break;
            }
        }
        if(indivSharedPtr != nullptr) {
            break;
        }
    }
    ASSERT_EQ(ea.getPopulation().sizeProtected(), 1) << "One individual should now be protected";
    ASSERT_THROW(ea.replacePopulation(offspring, survivors), std::runtime_error) << "Should throw by failing to delete the protected individual";
}

TEST_F(EvolutionAlgorithmTest, doGenerations) {
    
    Evolution::EvolutionAlgorithm ea(*representation, le, std::move(evalParams), 12, 10);
    ea.initializePopulation();

    size_t nbGen = 20;
    double formerBest = -1;
    for (size_t idxGen = 0; idxGen < nbGen; idxGen++) {
        std::set<std::unique_ptr<Evolution::Individual>, UniqueLess<Evolution::Individual>> offspring = ea.reproduceParents(ea.selectParents(100));
        ea.mutateOffspring(offspring);
        auto results = ea.evaluatePopulation(offspring, 0, Learn::LearningMode::TRAINING);
        std::map<std::reference_wrapper<const Evolution::Individual>, bool> survivors = ea.selectSurvivors(results);
        ea.replacePopulation(offspring, survivors);

        double best = results.rbegin()->first->getSelectionMetrics()->getScore();
        ASSERT_GE(best, formerBest) << "Performances should not decrease with fixed generation seed";
        formerBest = best;
    }
}

TEST_F(EvolutionAlgorithmTest, evolveTPGandLGP) {
    Evolution::EvolutionAlgorithm eaLgp(*representation, le, std::move(evalParams), 12, 10);
    eaLgp.initializePopulation();

    Representations::TPGRepresentation tpgRep = Representations::TPGRepresentation(eaLgp.getRepresentation(), eaLgp.getPopulation(), 5, 10);
    Evolution::EvolutionAlgorithm eaTpg(tpgRep, le);
    ASSERT_NO_THROW(eaTpg.initializePopulation()) << "Initializing population failed.";

    size_t nbGen = 20;
    double formerBest = -1;
    for (size_t idxGen = 0; idxGen < nbGen; idxGen++) {

        {
            std::set<std::unique_ptr<Evolution::Individual>, UniqueLess<Evolution::Individual>> offspring = eaLgp.reproduceParents(eaLgp.selectParents(100));
            eaLgp.mutateOffspring(offspring);
            auto results = eaLgp.evaluatePopulation(offspring, 0, Learn::LearningMode::TRAINING);
            std::map<std::reference_wrapper<const Evolution::Individual>, bool> survivors = eaLgp.selectSurvivors(results);
            eaLgp.replacePopulation(offspring, survivors);
        }



        {
            std::set<std::unique_ptr<Evolution::Individual>, UniqueLess<Evolution::Individual>> offspring = eaTpg.reproduceParents(eaTpg.selectParents(100));
            eaTpg.mutateOffspring(offspring);
            auto results = eaTpg.evaluatePopulation(offspring, 0, Learn::LearningMode::TRAINING);
            std::map<std::reference_wrapper<const Evolution::Individual>, bool> survivors = eaTpg.selectSurvivors(results);
            eaTpg.replacePopulation(offspring, survivors);
    
            double best = results.rbegin()->first->getSelectionMetrics()->getScore();
            ASSERT_GE(best, formerBest) << "Performances should not decrease with fixed generation seed";
            formerBest = best;
        }
    }

    ASSERT_EQ(eaTpg.getRNG().getUnsignedInt64(0, UINT64_MAX), 13482886404445128142U) << "RNG not determinist";
    ASSERT_EQ(Evolution::Individual::getIndividualIDCounter(), 4200) << "Individual ID counter not determinist";
    ASSERT_EQ(eaTpg.getPopulation().size(), 383) << "Size of TPG population not determinist";
}