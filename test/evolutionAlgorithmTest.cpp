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
#include "learn/stickGameWithOpponent.h"
#include "evaluation/reinforcementProblem.h"
#include "evaluation/evaluator.h"
#include "metrics/rewardsMetric.h"

#include "representations/LGP.h"
#include "representations/TPG.h"

#include "mutation/pointMutator.h"
#include "reproduction/replicator.h"
#include "selection/randomSelector.h"
#include "selection/truncationSelector.h"
#include "fitnessAssignment/defaultAssigner.h"

#include "util/counterReset.h"
// Set all file in comment

class EvolutionAlgorithmTest : public ::testing::Test
{
  protected:
    Instructions::Set set;
    StickGameWithOpponent le;


    virtual void SetUp()
    {
        CounterReset::counterReset();
        auto add = [](double a, double b) -> double { return a + b; };
        auto minus = [](int a, double b) -> double { return a - b; };
        auto times = [](double a, int b) -> double { return a * b; };
        auto div = [](int a, double b) -> double { return a / b; };
        
        set.add(*(new Instructions::LambdaInstruction<double, double, double>(add)));
        set.add(*(new Instructions::LambdaInstruction<int, double, double>(minus)));
        set.add(*(new Instructions::LambdaInstruction<double, int, double>(times)));
        set.add(*(new Instructions::LambdaInstruction<int, double, double>(div)));
    
    }

    virtual void TearDown()
    {
        delete (&set.getInstruction(0));
        delete (&set.getInstruction(1));
        delete (&set.getInstruction(2));
        delete (&set.getInstruction(3));
    }
};


TEST_F(EvolutionAlgorithmTest, customEvolutionLGP) {
    RNG::RNG rng;
    rng.setSeed(4);

    // Create representation
    Representations::LGP lgpRep(le.getInputDimensions(), 1, set, 8, 10);

    // Create Mutator and breeder
    Reproduction::Replicator breeder;
    Mutation::PointMutator mutator(0.5, 0.1, 0.1);

    // Create selectors
    Selection::RandomSelector parentSelection(true);
    Selection::TruncationSelector survivingSelection;
    FitnessAssignment::DefaultAssigner assigner;
    
    // Create evaluationAgent
    Evaluation::ReinforcementProblem problem(le, 3);
    Evaluation::Evaluator evaluator;
    std::shared_ptr<Metrics::MetricMapTemplate> metrics 
        = std::make_shared<Metrics::MetricMapTemplate>(std::make_unique<Metrics::RewardsMetric>());


    size_t sizePopulation = 100;
    size_t nbOffspring = 100;

    // Initialize population
    std::set<std::shared_ptr<Individual>, SharedLess<Individual>> individuals = Mutation::initIndividuals(mutator, lgpRep, sizePopulation, rng);
    std::set<std::shared_ptr<const Individual>, SharedLess<Individual>> population(individuals.begin(), individuals.end());
    individuals.clear();

    // Initial evaluation
    std::map<std::shared_ptr<const Individual>, std::unique_ptr<Metrics::MetricMap>, SharedLess<Individual>> evaluationResults = 
            evaluator.evaluateIndividuals(population, problem, metrics, 3, 0, Evaluation::Mode::TRAINING);
    // Fitness assignment 
    std::vector<std::pair<double, std::shared_ptr<const Individual>>> individualFitness = assigner.assignFitness(evaluationResults, Metrics::RewardsMetric::staticHash());

    size_t nbGen = 20;
    for (size_t idxGen = 0; idxGen < nbGen; idxGen++) {


        // Parent selection 
        std::vector<std::shared_ptr<const Individual>> parents = parentSelection.select(individualFitness, nbOffspring, rng);

        // Reproduce the parents
        std::set<std::shared_ptr<Individual>, SharedLess<Individual>> offspring = breeder.reproduce(parents, nbOffspring, rng);

        // Mutate the offspring
        Mutation::mutateIndividuals(mutator, offspring, rng);

        // Evaluate the population
        std::set<std::shared_ptr<const Individual>, SharedLess<Individual>> evaluatedIndividuals(population);
        evaluatedIndividuals.insert(offspring.begin(), offspring.end());
        evaluationResults = evaluator.evaluateIndividuals(evaluatedIndividuals, problem, metrics, 3, idxGen, Evaluation::Mode::TRAINING);

        // Do replacement
        individualFitness = assigner.assignFitness(evaluationResults, Metrics::RewardsMetric::staticHash());
        std::vector<std::shared_ptr<const Individual>> survivors = survivingSelection.select(individualFitness, sizePopulation, rng);
        population.clear();
        population.insert(survivors.begin(), survivors.end());


        // Print best individual
        std::cout<<"ID: "<<individualFitness.begin()->second->getIndividualID() 
        <<" and score: " << individualFitness.begin()->first <<std::endl;
    }
}


TEST_F(EvolutionAlgorithmTest, customEvolutionTPGPlusLGP) {
    RNG::RNG rng;
    rng.setSeed(4);

    // Create representations
    Representations::LGP lgpRep(le.getInputDimensions(), 1, set, 8, 10);
    Representations::TPG tpgRep(le.getInputDimensions(), 3, 2, 10);

    // Create Mutator and breeder
    Reproduction::Replicator breeder;
    Mutation::PointMutator mutator(0.5, 0.1, 0.1);

    // Create selectors
    Selection::RandomSelector parentSelection(true);
    Selection::TruncationSelector survivingSelection;
    FitnessAssignment::DefaultAssigner assigner;
    
    // Create evaluationAgent
    Evaluation::ReinforcementProblem problem(le, 3);
    Evaluation::Evaluator evaluator;
    std::shared_ptr<Metrics::MetricMapTemplate> metrics 
        = std::make_shared<Metrics::MetricMapTemplate>(std::make_unique<Metrics::RewardsMetric>());
    



    size_t sizePopulation = 100;
    size_t nbOffspring = 100;

    // Initialize LGP population
    std::set<std::shared_ptr<Individual>, SharedLess<Individual>> individualsLGP = Mutation::initIndividuals(mutator, lgpRep, sizePopulation, rng);
    std::set<std::shared_ptr<const Individual>, SharedLess<Individual>> populationLGP(individualsLGP.begin(), individualsLGP.end());
    individualsLGP.clear();
    std::map<std::shared_ptr<const Individual>, std::unique_ptr<Metrics::MetricMap>, SharedLess<Individual>> evaluationResultsLGP = 
            evaluator.evaluateIndividuals(populationLGP, problem, metrics, 3, 0, Evaluation::Mode::TRAINING);
    // Fitness assignment 
    std::vector<std::pair<double, std::shared_ptr<const Individual>>> individualFitnessLGP = assigner.assignFitness(evaluationResultsLGP, Metrics::RewardsMetric::staticHash());


    // Initialize TPG population

    std::vector<std::shared_ptr<const Individual>> members = parentSelection.select(individualFitnessLGP, nbOffspring, rng);
    tpgRep.setAvailableMembers(members);

    std::set<std::shared_ptr<Individual>, SharedLess<Individual>> individualsTPG = Mutation::initIndividuals(mutator, tpgRep, sizePopulation, rng);
    std::set<std::shared_ptr<const Individual>, SharedLess<Individual>> populationTPG(individualsTPG.begin(), individualsTPG.end());
    individualsTPG.clear();
    std::map<std::shared_ptr<const Individual>, std::unique_ptr<Metrics::MetricMap>, SharedLess<Individual>> evaluationResultsTPG = 
            evaluator.evaluateIndividuals(populationTPG, problem, metrics, 3, 0, Evaluation::Mode::TRAINING);
    // Fitness assignment 
    std::vector<std::pair<double, std::shared_ptr<const Individual>>> individualFitnessTPG = assigner.assignFitness(evaluationResultsTPG, Metrics::RewardsMetric::staticHash());

    size_t nbGen = 20;
    for (size_t idxGen = 0; idxGen < nbGen; idxGen++) {

        // LGP Evolution : variation
        std::vector<std::shared_ptr<const Individual>> parentsLGP = parentSelection.select(individualFitnessLGP, nbOffspring, rng);
        std::set<std::shared_ptr<Individual>, SharedLess<Individual>> offspringLGP = breeder.reproduce(parentsLGP, nbOffspring, rng);
        Mutation::mutateIndividuals(mutator, offspringLGP, rng);
        // LGP Evolution : evaluation
        std::set<std::shared_ptr<const Individual>, SharedLess<Individual>> evaluatedIndividualsLGP(populationLGP);
        evaluatedIndividualsLGP.insert(offspringLGP.begin(), offspringLGP.end());
        evaluationResultsLGP = evaluator.evaluateIndividuals(evaluatedIndividualsLGP, problem, metrics, 3, idxGen, Evaluation::Mode::TRAINING);
        // LGP Evolution : replacement
        individualFitnessLGP = assigner.assignFitness(evaluationResultsLGP, Metrics::RewardsMetric::staticHash());
        std::vector<std::shared_ptr<const Individual>> survivorsLGP = survivingSelection.select(individualFitnessLGP, sizePopulation, rng);
        populationLGP.clear();
        populationLGP.insert(survivorsLGP.begin(), survivorsLGP.end());


        // TPG Evolution : variation
        std::vector<std::shared_ptr<const Individual>> parentsTPG = parentSelection.select(individualFitnessTPG, nbOffspring, rng);
        std::set<std::shared_ptr<Individual>, SharedLess<Individual>> offspringTPG = breeder.reproduce(parentsTPG, nbOffspring, rng);

        
        std::vector<std::shared_ptr<const Individual>> members = parentSelection.select(individualFitnessLGP, nbOffspring, rng);
        std::vector<std::shared_ptr<const Individual>> tangleds = parentSelection.select(individualFitnessTPG, nbOffspring, rng);
        tpgRep.setAvailableMembers(members);
        tpgRep.setAvailableTangledIndiv(tangleds);

        Mutation::mutateIndividuals(mutator, offspringTPG, rng);
        // TPG Evolution : evaluation
        std::set<std::shared_ptr<const Individual>, SharedLess<Individual>> evaluatedIndividualsTPG(populationTPG);
        evaluatedIndividualsTPG.insert(offspringTPG.begin(), offspringTPG.end());
        evaluationResultsTPG = evaluator.evaluateIndividuals(evaluatedIndividualsTPG, problem, metrics, 3, idxGen, Evaluation::Mode::TRAINING);
        // TPG Evolution : replacement
        individualFitnessTPG = assigner.assignFitness(evaluationResultsTPG, Metrics::RewardsMetric::staticHash());
        std::vector<std::shared_ptr<const Individual>> survivorsTPG = survivingSelection.select(individualFitnessTPG, sizePopulation, rng);
        populationTPG.clear();
        populationTPG.insert(survivorsTPG.begin(), survivorsTPG.end());


        // Print best individual
        std::cout<<"ID: "<<individualFitnessTPG.begin()->second->getIndividualID() 
        <<" and score: " << individualFitnessTPG.begin()->first <<std::endl;
    }

    ASSERT_EQ(5681813666992117604U, rng.uniformSample<uint64_t>(0, UINT64_MAX)) << "bouh bouh bouh th determinism";
}