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
#include "evaluation/reinforcementAgent.h"
#include "evaluation/archiveEvalAgent.h"

#include "representations/lgpRepresentation.h"
#include "representations/tpgRepresentation.h"

#include "util/counterReset.h"
// Set all file in comment

class EvolutionAlgorithmTest : public ::testing::Test
{
  protected:
    Instructions::Set set;
    Evolution::Representation* representation;


    StickGameWithOpponentD le;
    Evaluation::EvaluationAgent* evalAgent;


    virtual void SetUp()
    {
        CounterReset::counterReset();
        auto add = [](double a, double b) -> double { return a + b; };
        auto minus = [](double a, double b) -> double { return a - b; };
        auto times = [](double a, double b) -> double { return a * b; };
        auto div = [](double a, double b) -> double { return a / b; };
        
        set.add(*(new Instructions::LambdaInstruction<double, double, double>(add)));
        set.add(*(new Instructions::LambdaInstruction<double, double, double>(minus)));
        set.add(*(new Instructions::LambdaInstruction<double, double, double>(times)));
        set.add(*(new Instructions::LambdaInstruction<double, double, double>(div)));
    
        representation = new Representations::LGPRepresentation(set, 8, 10);

        evalAgent = new Evaluation::ReinforcementAgent(le);
    }

    virtual void TearDown()
    {
        delete (&set.getInstruction(0));
        delete (&set.getInstruction(1));
        delete (&set.getInstruction(2));
        delete (&set.getInstruction(3));
        delete representation;
        delete evalAgent;
    }
};


TEST_F(EvolutionAlgorithmTest, Constructor)
{
    Evolution::EvolutionAlgorithm* ea;

    ASSERT_NO_THROW(ea = new Evolution::EvolutionAlgorithm(*representation, *evalAgent, 12)) << "Constructor of EA failed.";

    ASSERT_EQ(ea->getRepresentation().getMaxNbNodes(), representation->getMaxNbNodes()) << "Constructor should have copied the representation";

    ASSERT_NO_THROW(ea->getMutation()) << "For Coverage :D";
    ASSERT_NO_THROW(ea->getEvaluation()) << "For Coverage :D";
    ASSERT_NO_THROW(ea->getSelector()) << "For Coverage :D";
    ASSERT_NO_THROW(ea->getRNG()) << "For Coverage :D";

    ASSERT_NO_THROW(delete ea) << "Destructor of EA failed.";
}

TEST_F(EvolutionAlgorithmTest, initializePopulation)
{
    Evolution::EvolutionAlgorithm ea(*representation, *evalAgent, 12);

    ASSERT_NO_THROW(ea.initializePopulation()) << "Initialization of population failed.";

    ASSERT_EQ(ea.getPopulation().size(), 100) << "Population size is wrong after initialization.";

    for(const Evolution::Individual& indiv: ea.getPopulation().getIndividuals()) {
        ASSERT_TRUE(ea.getRepresentation().isValid(indiv)) << "An individual is not valid after population initialization";
    }
}

TEST_F(EvolutionAlgorithmTest, selectParents) 
{
    Evolution::EvolutionAlgorithm ea(*representation, *evalAgent, 12);
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
    
    Evolution::EvolutionAlgorithm ea(*representation, *evalAgent, 12);
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
    Evolution::EvolutionAlgorithm ea(*representation, *evalAgent, 12);
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
    Evolution::EvolutionAlgorithm ea(*representation, *evalAgent, 12);
    ea.initializePopulation();

    std::set<std::unique_ptr<Evolution::Individual>, UniqueLess<Evolution::Individual>> offspring = ea.reproduceParents(ea.selectParents(100));
    ea.mutateOffspring(offspring);

    ASSERT_NO_THROW(ea.evaluatePopulation(offspring, 0, Evaluation::LearningMode::TRAINING)) << "Evaluation failed";


    Evolution::EvolutionAlgorithm ea2(*representation, *evalAgent, 12);
    ea2.initializePopulation();

    std::set<std::unique_ptr<Evolution::Individual>, UniqueLess<Evolution::Individual>> offspring2 = ea2.reproduceParents(ea2.selectParents(100));
    ea2.mutateOffspring(offspring2);    
    std::map<std::reference_wrapper<const Evolution::Individual>, std::shared_ptr<Evaluation::EvaluationResult>> results2;
    ASSERT_NO_THROW(ea2.evaluatePopulation(offspring2, 0, Evaluation::LearningMode::TRAINING)) << "Evaluation of entire population failed";

    auto it1 = offspring.begin();
    auto it2 = offspring2.begin();
    while(it1 != offspring.end() && it2 != offspring2.end()) {
        ASSERT_TRUE(dynamic_cast<const Evaluation::ScoreMetric*>(&it1->get()->getEvaluationResult().getEvaluationRuns().begin()->second->getMetricAt(0)) != nullptr) << "Metric should be scoreMetric";
        ASSERT_TRUE(dynamic_cast<const Evaluation::ScoreMetric*>(&it2->get()->getEvaluationResult().getEvaluationRuns().begin()->second->getMetricAt(0)) != nullptr) << "Metric should be scoreMetric";
        const Evaluation::ScoreMetric* metric1 = dynamic_cast<const Evaluation::ScoreMetric*>(&it1->get()->getEvaluationResult().getEvaluationRuns().begin()->second->getMetricAt(0));
        const Evaluation::ScoreMetric* metric2 = dynamic_cast<const Evaluation::ScoreMetric*>(&it2->get()->getEvaluationResult().getEvaluationRuns().begin()->second->getMetricAt(0));
        ASSERT_EQ(metric1->getScore(), metric2->getScore()) << "EvaluationResults scores should be equal";

        it1++; it2++;
    }
}

TEST_F(EvolutionAlgorithmTest, survivorSelection) 
{
    Evolution::EvolutionAlgorithm ea(*representation, *evalAgent, 12);
    ea.initializePopulation();

    std::set<std::unique_ptr<Evolution::Individual>, UniqueLess<Evolution::Individual>> offspring = ea.reproduceParents(ea.selectParents(100));
    ea.mutateOffspring(offspring);

    ea.evaluatePopulation(offspring, 0, Evaluation::LearningMode::TRAINING);

    size_t minIndexOffspring = offspring.begin()->get()->getIndividualID();
    ASSERT_EQ(offspring.size(), 100) << "offspring size should be 100 before replacement";

    ASSERT_NO_THROW(ea.selectSurvivors(offspring)) << "Fail to replace population";

    ASSERT_EQ(ea.getPopulation().size(), 100) << "Population size should be 100 after replacement";
    ASSERT_EQ(offspring.size(), 0) << "Offspring size should be 0 after replacement";

    size_t nbOffspringSurviving = 0;
    std::set<std::reference_wrapper<const Evolution::Individual>> indivs(ea.getPopulation().getIndividuals());
    for(auto it = indivs.begin(); it != indivs.end(); it++){
        if(it->get().getIndividualID() >= minIndexOffspring) {
            nbOffspringSurviving++;
        }
    }
    ASSERT_GE(nbOffspringSurviving, 1) << "At least one offspring should have survive";
}

TEST_F(EvolutionAlgorithmTest, doGenerations) {
    
    Evolution::EvolutionAlgorithm ea(*representation, *evalAgent, 12);
    ea.initializePopulation();

    size_t nbGen = 20;
    double formerBest = -1;
    for (size_t idxGen = 0; idxGen < nbGen; idxGen++) {
        std::set<std::unique_ptr<Evolution::Individual>, UniqueLess<Evolution::Individual>> offspring = ea.reproduceParents(ea.selectParents(100));
        ea.mutateOffspring(offspring);
        ea.evaluatePopulation(offspring, 0, Evaluation::LearningMode::TRAINING);
        ea.selectSurvivors(offspring);
    }
    
    ASSERT_EQ(Evolution::Individual::getIndividualIDCounter(), 2100) << "Individual ID counter not determinist";
    ASSERT_EQ(ea.getPopulation().size(), 100) << "Size of population not determinist";
    ASSERT_EQ(ea.getRNG().getUnsignedInt64(0, UINT64_MAX), 5977790652527000119U) << "RNG not determinist";
}

/*
TEST_F(EvolutionAlgorithmTest, testArchive) {
    
    Evolution::EvolutionAlgorithm ea(*representation, *evalAgent, 12);
    ea.initializePopulation();
    evalAgent->addRequestedMetric(Evaluation::ArchiveMetric(1.0));

    size_t nbGen = 20;
    double formerBest = -1;

    std::set<std::unique_ptr<Evolution::Individual>, UniqueLess<Evolution::Individual>> offspring = ea.reproduceParents(ea.selectParents(100));
    ea.mutateOffspring(offspring);
    ea.evaluatePopulation(offspring, 0, Evaluation::LearningMode::TRAINING);
    ea.selectSurvivors(offspring);

    // This is some dataSource sampled from the environment
    ASSERT_EQ(ea.getPopulation().getIndividuals().begin()->get().getEvaluationResult().getEvaluationRuns().begin()->second->getSize(), 2) << "meh";
}*/


TEST_F(EvolutionAlgorithmTest, evolveTPGandLGP) {
    Evolution::EvolutionAlgorithm eaLgp(*representation, *evalAgent, 12);
    eaLgp.initializePopulation();

    Representations::TPGRepresentation tpgRep(eaLgp.getRepresentation(), eaLgp.getPopulation(), 5, 10);
    Evolution::EvolutionAlgorithm eaTpg(tpgRep, *evalAgent);
    ASSERT_NO_THROW(eaTpg.initializePopulation()) << "Initializing population failed.";

    size_t nbGen = 20;
    for (size_t idxGen = 0; idxGen < nbGen; idxGen++) {

        {
            std::set<std::unique_ptr<Evolution::Individual>, UniqueLess<Evolution::Individual>> offspring = eaLgp.reproduceParents(eaLgp.selectParents(100));
            eaLgp.mutateOffspring(offspring);
            eaLgp.evaluatePopulation(offspring, 0, Evaluation::LearningMode::TRAINING);
            eaLgp.selectSurvivors(offspring);
        }



        {
            std::set<std::unique_ptr<Evolution::Individual>, UniqueLess<Evolution::Individual>> offspring = eaTpg.reproduceParents(eaTpg.selectParents(100));
            eaTpg.mutateOffspring(offspring);
            eaTpg.evaluatePopulation(offspring, 0, Evaluation::LearningMode::TRAINING);
            eaTpg.selectSurvivors(offspring);
        }
    }

    ASSERT_EQ(Evolution::Individual::getIndividualIDCounter(), 4200) << "Individual ID counter not determinist";
    ASSERT_EQ(eaTpg.getPopulation().size(), 308) << "Size of TPG population not determinist";
    ASSERT_EQ(eaTpg.getRNG().getUnsignedInt64(0, UINT64_MAX), 16486736260054747606U) << "RNG not determinist";
}


/*
TEST_F(EvolutionAlgorithmTest, testArchiveTPG) {
    

    Evaluation::ReinforcementAgent evalAgent(le);
    Representations::LGPRepresentation representation(set, 8, 10);

    Evaluation::ArchiveEnvironment archiveEnv(le.getDataSources(), 10);
    Evaluation::ArchiveEvalAgent archiveEval(archiveEnv, std::make_unique<Learn::LearningParameters>(), 2);

    Evolution::EvolutionAlgorithm eaLgp(representation, archiveEval, 12);
    eaLgp.initializePopulation();

    Representations::TPGRepresentation tpgRep(eaLgp.getRepresentation(), eaLgp.getPopulation(), 5, 10);
    Evolution::EvolutionAlgorithm eaTpg(tpgRep, evalAgent, 0);
    
    eaTpg.getEvaluation().addRequestedMetric(Evaluation::ArchiveMetric(0.5));
    archiveEnv.setArchiveInputPopulation(eaTpg.getPopulation());
    archiveEnv.setArchiveOutputPopulation(eaLgp.getPopulation());

    ASSERT_NO_THROW(eaTpg.initializePopulation()) << "Initializing population failed.";
    eaTpg.evaluatePopulation({}, 0, Evaluation::LearningMode::TRAINING);

    size_t nbGen = 20;
    for (size_t idxGen = 0; idxGen < nbGen; idxGen++) {

        size_t nbGood = 0;
        while(nbGood < 100) {
            std::set<std::unique_ptr<Evolution::Individual>, UniqueLess<Evolution::Individual>> offspring = eaLgp.reproduceParents(eaLgp.selectParents(100));
            eaLgp.mutateOffspring(offspring);
            eaLgp.evaluatePopulation(offspring, 0, Evaluation::LearningMode::TRAINING);

            
            std::set<std::reference_wrapper<const Evolution::Individual>> evaluatedIndividuals = eaLgp.getPopulation().getNotProtectedIndividuals();
            for (const std::unique_ptr<Evolution::Individual>& os: offspring) {
                evaluatedIndividuals.insert(*os);
            }
            std::vector<std::pair<double, std::reference_wrapper<const Evolution::Individual>>> fitnesses = eaLgp.getSelector().getRankedScores(evaluatedIndividuals);
            nbGood = std::count_if(fitnesses.begin(), fitnesses.end(),
                                [](const std::pair<double, std::reference_wrapper<const Evolution::Individual>>& pair) {
                                    return pair.first == 1.0;
                                });

            eaLgp.selectSurvivors(offspring);
        }



        {
            std::set<std::unique_ptr<Evolution::Individual>, UniqueLess<Evolution::Individual>> offspring = eaTpg.reproduceParents(eaTpg.selectParents(100));
            eaTpg.mutateOffspring(offspring);
            eaTpg.evaluatePopulation(offspring, 0, Evaluation::LearningMode::TRAINING);
            eaTpg.selectSurvivors(offspring);
        }

    }
    //ASSERT_EQ(Evolution::Individual::getIndividualIDCounter(), 6500) << "Individual ID counter not determinist";
    ASSERT_EQ(eaTpg.getPopulation().size(), 309) << "Size of TPG population not determinist";
    ASSERT_EQ(eaTpg.getRNG().getUnsignedInt64(0, UINT64_MAX), 16814013097088067763U) << "RNG not determinist";
}
*/