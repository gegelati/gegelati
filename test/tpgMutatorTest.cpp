/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2019 - 2025) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2019 - 2022)
 * Nicolas Sourbier <nsourbie@insa-rennes.fr> (2020)
 * Quentin Vacher <qvacher@insa-rennes.fr> (2024 - 2025)
 * Thomas Bourgoin <tbourgoi@insa-rennes.fr> (2021)
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

#include <gtest/gtest.h>

#include <algorithm>
#include <set>
#include <vector>

#include "algorithm/lgp/lgpMutator.h"
#include "algorithm/lgp/lgpLineMutator.h"
#include "algorithm/mutator.h"
#include "algorithm/tpg/tpgMutator.h"
#include "algorithm/tpg/tpgExecutionEngine.h"
#include "data/dataHandler.h"
#include "data/primitiveTypeArray.h"
#include "algorithm/lgp/environment.h"
#include "instructions/addPrimitiveType.h"
#include "instructions/instruction.h"
#include "instructions/lambdaInstruction.h"
#include "instructions/multByConstant.h"
#include "mutator/rng.h"
#include "selector/truncationSelector.h"
#include "selector/tournamentSelector.h"
#include "evoGraph/graph.h"
#include "util/counterReset.h"

#include "parameters.h"

class TpgMutatorTest : public ::testing::Test
{
  protected:
    const size_t size1{24};
    const size_t size2{32};
    const double value0{2.3};
    const float value1{4.2f};
    std::vector<std::reference_wrapper<const Data::DataHandler>> vect;
    Instructions::Set set;
    std::shared_ptr<const Algorithm::LGP::LGPEnvironment> e;
    Parameters params;
    std::shared_ptr<EvoGraph::Graph> graph;
    std::shared_ptr<Algorithm::TPG::TPGArchive> archive;
    std::shared_ptr<Algorithm::LGP::LGPManager> lgpManager;
    std::shared_ptr<Algorithm::LGP::LGPMutator> lgpMutator;
    const Algorithm::Agent* lgpAgent;

    std::shared_ptr<Algorithm::TPG::TPGManager> tpgManager;
    std::shared_ptr<Algorithm::TPG::TPGMutator> tpgMutator;
    Output::OutputHandler* actions;
    Output::OutputHandler* lgpOutput;

    std::shared_ptr<Selector::TruncationSelector> selector;

    TpgMutatorTest() : e{nullptr} {};

    virtual void SetUp()
    {

        actions = new Output::OutputHandler(5);
        lgpOutput = new Output::OutputHandler(Output::Output());

        CounterReset::counterReset();
        vect.push_back(
            *(new Data::PrimitiveTypeArray<int>((unsigned int)size1)));
        vect.push_back(
            *(new Data::PrimitiveTypeArray<double>((unsigned int)size2)));

        ((Data::PrimitiveTypeArray<double>&)vect.at(1).get())
            .setDataAt(typeid(double), 25, value0);

        std::function<double(double, double)> minus =
            [](double a, double b) -> double { return a - b; };
        std::function<double(double, double)> add =
            [](double a, double b) -> double { return a + b; };

        set.add(*(new Instructions::MultByConstant<double>()));
        set.add(*(new Instructions::AddPrimitiveType<double>()));
        set.add(*(new Instructions::LambdaInstruction<double, double>(minus)));
        set.add(*(new Instructions::LambdaInstruction<double, double>(add)));



        // the environment and the LGPs have 5 Constant parameters
        params.algorithm.lgp.nbRegisters = 8;
        params.algorithm.lgp.nbProgramConstant = 5;
        e = std::make_shared<Algorithm::LGP::LGPEnvironment>(set, params.algorithm.lgp.nbRegisters, params.algorithm.lgp.nbProgramConstant, vect);

        archive = std::make_shared<Algorithm::TPG::TPGArchive>(params.algorithm.tpg.archiveSize,
                                            params.algorithm.tpg.archivingProbability);

        graph = std::make_shared<EvoGraph::Graph>();

        
        lgpManager = std::make_shared<Algorithm::LGP::LGPManager>(*e, *lgpOutput, (uint64_t)0);
        lgpAgent = &lgpManager->createAgent(*graph);

        tpgManager = std::make_shared<Algorithm::TPG::TPGManager>(*actions, (uint64_t)1);
        tpgManager->addSubManager(*lgpManager);
        tpgManager->setProgramAlgorithmID((uint64_t)0);

        selector = std::make_shared<Selector::TruncationSelector>(std::make_unique<Selector::SelectionParameters>(params.selection));
        selector->setManager(*tpgManager);
        selector->setNbAgents(params.algorithm.nbAgents);

        lgpMutator = std::make_shared<Algorithm::LGP::LGPMutator>(*selector, (uint64_t)0);

        tpgMutator = std::make_shared<Algorithm::TPG::TPGMutator>(*selector, (uint64_t)1, *archive);
        tpgMutator->addSubMutator(*lgpMutator);
        tpgMutator->setProgramAlgorithmID((uint64_t)0);

    }

    virtual void TearDown()
    {
        delete (&(vect.at(0).get()));
        delete (&(vect.at(1).get()));
        delete (&set.getInstruction(0));
        delete (&set.getInstruction(1));
        delete (&set.getInstruction(2));
        delete (&set.getInstruction(3));
    }
};

TEST_F(TpgMutatorTest, RNG)
{
    RNG::RNG rng;
    rng.setSeed(0);

    // With this seed, the current pseudo-random number generator returns 24
    // on its first use
    ASSERT_EQ(rng.getUnsignedInt64(0, 100), 24)
        << "Returned pseudo-random value changed with a known seed.";

    ASSERT_EQ(rng.getDouble(0, 1.0), 0.99214520962982877)
        << "Returned pseudo-random value changed with a known seed.";
}



TEST_F(TpgMutatorTest, TPGMutatorInitRandomTPG)
{
    RNG::RNG rng;
    rng.setSeed(0);


    params.algorithm.tpg.maxInitOutgoingEdges = 4;
    params.algorithm.lgp.maxProgramSize = 96;
    params.algorithm.lgp.pConstantMutation = 0.5;
    params.algorithm.lgp.minConstValue = 0;
    params.algorithm.lgp.maxConstValue = 1;

    ASSERT_NO_THROW(
        tpgMutator->initRandomPopulation(*graph, *tpgManager, params.algorithm, rng))
        << "TPG Initialization failed.";
    auto vertexSet = graph->getVertices();
    // Check number or vertex, roots, actions, teams, edges
    ASSERT_EQ(vertexSet.size(), this->actions->front().getNbValues() + params.algorithm.nbAgents)
        << "Number of vertices after initialization is incorrect.";
    ASSERT_EQ(graph->getRootVertices().size(), params.algorithm.nbAgents)
        << "Number of root vertices after initialization is incorrect.";
    ASSERT_EQ(std::count_if(vertexSet.begin(), vertexSet.end(),
                            [](const EvoGraph::Vertex& vert) {
                                return dynamic_cast<const EvoGraph::Action*>(
                                           &vert) != nullptr;
                            }),
              this->actions->front().getNbValues())
        << "Number of action vertex in the graph is incorrect.";
    ASSERT_EQ(std::count_if(vertexSet.begin(), vertexSet.end(),
                            [](const EvoGraph::Vertex& vert) {
                                return dynamic_cast<const EvoGraph::Team*>(
                                           &vert) != nullptr;
                            }),
              params.algorithm.nbAgents)
        << "Number of team vertex in the graph is incorrect.";
    ASSERT_GE(graph->getEdges().size(), 2 * params.algorithm.nbAgents)
        << "Insufficient number of edges in the initialized TPG.";
    ASSERT_LE(graph->getEdges().size(),
              params.algorithm.nbAgents * params.algorithm.tpg.maxInitOutgoingEdges)
        << "Too many edges in the initialized TPG.";

    // Check number of Programs.
    std::set<std::reference_wrapper<const Algorithm::Agent>> programs;
    for (const auto& edge : graph->getEdges()) {
        const Algorithm::Agent& program = edge.get().getProgram();
            programs.insert(program);
    }
    ASSERT_EQ(programs.size(), params.algorithm.nbAgents * 2)
        << "Number of distinct program in the TPG is incorrect.";
    // Check that no team has the same program twice
    for (auto team :graph->getRootVertices()) {
    std::set<std::reference_wrapper<const Algorithm::Agent>> teamPrograms;
        std::for_each(team.get().getOutgoingEdges().begin(),
                      team.get().getOutgoingEdges().end(),
                      [&teamPrograms](const EvoGraph::Edge& edge) {
                          teamPrograms.insert(edge.getProgram());
                      });
        ASSERT_EQ(teamPrograms.size(), team.get().getOutgoingEdges().size())
            << "A team is connected to the same program twice.";
    }

    // Cover bad parameterization error
    params.algorithm.tpg.maxInitOutgoingEdges = 6;
    ASSERT_THROW(
        tpgMutator->initRandomPopulation(*graph, *tpgManager, params.algorithm, rng),
        std::runtime_error)
        << "TPG Initialization should fail with bad parameters.";
    params.algorithm.tpg.maxInitOutgoingEdges = 0;
    actions = new Output::OutputHandler(1);;
    ASSERT_THROW(
        tpgMutator->initRandomPopulation(*graph, *tpgManager, params.algorithm, rng),
        std::runtime_error)
        << "TPG Initialization should fail with bad parameters.";
}

/*
TEST_F(TpgMutatorTest, TPGMutatorInitRandomTPGContinuous)
{

    RNG::RNG rng;
    Mutator::MutationParameters& mutParams = params.mutation;
    mutParams.mutation.tpg.maxInitOutgoingEdges = 4;
    mutParams.mutation.prog.maxProgramSize = 96;
    mutParams.mutation.prog.pConstantMutation = 0.5;
    mutParams.mutation.tpg.ratioTeamsOverActions = 0.5;
    mutParams.mutation.prog.minConstValue = 0;
    mutParams.mutation.prog.maxConstValue = 1;
    params.algorithm.lgp.nbRegisters = 8;

    // Error on number of registers
    rng.setSeed(0);
    params.algorithm.tpg.useActionProgram = false;
    uint64_t nbActions = 8;
    Environment ce0(set, params, vect, nbActions);
    EvoGraph::Graph tpg0(ce0);
    ASSERT_THROW(
        TpgMutator->initRandomPopulation(tpg0, mutParams, rng, nbActions),
        std::runtime_error)
        << "TPG Initialization should fail with bad parameters.";

    // No error
    rng.setSeed(0);
    nbActions = 7;
    params.algorithm.tpg.useActionProgram = true;
    Environment ce1(set, params, vect, nbActions);
    EvoGraph::Graph tpg1(ce1);
    ASSERT_NO_THROW(
        TpgMutator->initRandomPopulation(tpg1, mutParams, rng, nbActions))
        << "TPG Initialization should not fail.";

    // Error on number of registers
    rng.setSeed(0);
    nbActions = 9;
    Environment ce2(set, params, vect, nbActions);
    EvoGraph::Graph tpg2(ce2);
    ASSERT_THROW(
        TpgMutator->initRandomPopulation(tpg2, mutParams, rng, nbActions),
        std::runtime_error)
        << "TPG Initialization should fail with bad parameters.";

    rng.setSeed(0);
    nbActions = 8;
    params.algorithm.tpg.teamAccessAllActions = true;
    Environment ce3(set, params, vect, nbActions);
    EvoGraph::Graph tpg3(ce3);
    ASSERT_NO_THROW(
        TpgMutator->initRandomPopulation(tpg3, mutParams, rng, nbActions))
        << "TPG Initialization should fail with bad parameters.";

    auto vertexSet = tpg3.getVertices();
    // Check number or vertex, roots, actions, teams, edges
    ASSERT_EQ(vertexSet.size(),
              mutParams.mutation.tpg.nbRoots * 3 * mutParams.mutation.tpg.ratioTeamsOverActions +
                  mutParams.mutation.tpg.nbRoots *
                      (1 - mutParams.mutation.tpg.ratioTeamsOverActions))
        << "Number of vertices after initialization is incorrect.";
    ASSERT_EQ(tpg3.getRootVertices().size(), mutParams.mutation.tpg.nbRoots)
        << "Number of root vertices after initialization is incorrect.";
    ASSERT_EQ(
        std::count_if(vertexSet.begin(), vertexSet.end(),
                      [](const EvoGraph::Vertex* vert) {
                          return dynamic_cast<const EvoGraph::Action*>(vert) !=
                                 nullptr;
                      }),
        mutParams.mutation.tpg.nbRoots * 2 * mutParams.mutation.tpg.ratioTeamsOverActions +
            mutParams.mutation.tpg.nbRoots * (1 - mutParams.mutation.tpg.ratioTeamsOverActions))
        << "Number of action vertex in the graph is incorrect.";
    ASSERT_EQ(std::count_if(vertexSet.begin(), vertexSet.end(),
                            [](const EvoGraph::Vertex* vert) {
                                return dynamic_cast<const EvoGraph::Team*>(
                                           vert) != nullptr;
                            }),
              mutParams.mutation.tpg.nbRoots * mutParams.mutation.tpg.ratioTeamsOverActions)
        << "Number of team vertex in the graph is incorrect.";
    ASSERT_GE(tpg3.getEdges().size(), mutParams.mutation.tpg.nbRoots * 2)
        << "Insufficient number of edges in the initialized TPG.";
    ASSERT_LE(tpg3.getEdges().size(),
              mutParams.mutation.tpg.nbRoots * (mutParams.mutation.tpg.maxInitOutgoingEdges + 1))
        << "Too many edges in the initialized TPG.";

    // Check number of Programs.
    std::set<Program::Program*> programs;
    std::for_each(tpg3.getEdges().begin(), tpg3.getEdges().end(),
                  [&programs](const std::unique_ptr<EvoGraph::Edge>& edge) {
                      programs.insert(&edge->getProgram());
                  });
    // 2 contexts programs and 2 actions program per roots for team roots, one
    // program for action roots
    ASSERT_EQ(programs.size(),
              4 * mutParams.mutation.tpg.nbRoots * mutParams.mutation.tpg.ratioTeamsOverActions +
                  mutParams.mutation.tpg.nbRoots *
                      (1 - mutParams.mutation.tpg.ratioTeamsOverActions))
        << "Number of distinct program in the TPG is incorrect.";

    // Test coverage: useMultiActionProgram
    mutParams.mutation.tpg.useMultiActionProgram = true;
    mutParams.mutation.tpg.useActionProgram = true;
    mutParams.mutation.tpg.nbRoots = 4;
    mutParams.mutation.tpg.ratioTeamsOverActions = 0.5;
    nbActions = 8;
    Environment ce4(set, params, vect, nbActions);
    EvoGraph::Graph tpg4(ce4);

    // Test coverage: teamAccessAllActions = true
    mutParams.mutation.tpg.useMultiActionProgram = false;
    mutParams.mutation.tpg.teamAccessAllActions = true;
    ASSERT_NO_THROW(
        TpgMutator->initRandomPopulation(tpg4, mutParams, rng, nbActions))
        << "Should not throw with teamAccessAllActions enabled.";

    params.algorithm.lgp.nbRegisters = 8;
    mutParams.mutation.tpg.useActionProgram = true;
    mutParams.mutation.tpg.teamAccessAllActions = false;
    mutParams.mutation.tpg.ratioTeamsOverActions = 1.0;
    Environment ce5(set, params, vect, 4);
    EvoGraph::Graph tpg5(ce5);
    ASSERT_THROW(TpgMutator->initRandomPopulation(tpg5, mutParams, rng, 4),
                 std::runtime_error)
        << "Should throw when nbContinuousActions == 0.";
}

TEST_F(TpgMutatorTest, TPGMutatorInitRandomTPGMAPLE)
{

    RNG::RNG rng;
    Mutator::MutationParameters& mutParams = params.mutation;
    mutParams.mutation.tpg.maxInitOutgoingEdges = 4;
    mutParams.mutation.prog.maxProgramSize = 96;
    mutParams.mutation.prog.pConstantMutation = 0.5;
    mutParams.mutation.prog.minConstValue = 0;
    mutParams.mutation.prog.maxConstValue = 1;
    params.algorithm.lgp.nbRegisters = 8;

    // Error on number of registers
    rng.setSeed(0);
    params.algorithm.tpg.useActionProgram = true;
    params.algorithm.tpg.useMultiActionProgram = true;
    params.algorithm.tpg.nbActionEdgeInit = 10;
    params.algorithm.tpg.ratioTeamsOverActions = 0.0;
    uint64_t nbActions = 5;
    Environment ce2(set, params, vect, nbActions);
    EvoGraph::Graph tpg2(ce2);
    ASSERT_THROW(
        TpgMutator->initRandomPopulation(tpg2, mutParams, rng, nbActions),
        std::runtime_error)
        << "TPG Initialization should fail with bad parameters.";

    // Error on number of registers
    rng.setSeed(0);
    nbActions = 8;
    params.algorithm.tpg.nbActionEdgeInit = 3;
    Environment ce3(set, params, vect, nbActions);
    EvoGraph::Graph tpg3(ce3);
    ASSERT_NO_THROW(
        TpgMutator->initRandomPopulation(tpg3, mutParams, rng, nbActions))
        << "TPG Initialization should fail with bad parameters.";

    auto vertexSet = tpg3.getVertices();
    // Check number or vertex, roots, actions, teams, edges
    ASSERT_EQ(vertexSet.size(), mutParams.mutation.tpg.nbRoots)
        << "Number of vertices after initialization is incorrect.";
    ASSERT_EQ(tpg3.getRootVertices().size(), mutParams.mutation.tpg.nbRoots)
        << "Number of root vertices after initialization is incorrect.";
    ASSERT_EQ(std::count_if(vertexSet.begin(), vertexSet.end(),
                            [](const EvoGraph::Vertex* vert) {
                                return dynamic_cast<const EvoGraph::Action*>(
                                           vert) != nullptr;
                            }),
              mutParams.mutation.tpg.nbRoots)
        << "Number of action vertex in the graph is incorrect.";
    ASSERT_EQ(std::count_if(vertexSet.begin(), vertexSet.end(),
                            [](const EvoGraph::Vertex* vert) {
                                return dynamic_cast<const EvoGraph::Team*>(
                                           vert) != nullptr;
                            }),
              0)
        << "Number of team vertex in the graph is incorrect.";
    ASSERT_EQ(tpg3.getEdges().size(),
              mutParams.mutation.tpg.nbRoots * params.algorithm.tpg.nbActionEdgeInit)
        << "Number of edges in the initialized TPG is incorrect.";

    // Check number of Programs.
    std::set<Program::Program*> programs;
    std::for_each(tpg3.getEdges().begin(), tpg3.getEdges().end(),
                  [&programs](const std::unique_ptr<EvoGraph::Edge>& edge) {
                      programs.insert(&edge->getProgram());
                  });
    // 2 contexts programs and 2 actions program per roots
    ASSERT_EQ(programs.size(),
              mutParams.mutation.tpg.nbRoots * params.algorithm.tpg.nbActionEdgeInit)
        << "Number of distinct program in the TPG is incorrect.";
}*/

TEST_F(TpgMutatorTest, TPGMutatorRemoveRandomEdge)
{
    const EvoGraph::Team& vertex0 = graph->addNewTeam();
    const EvoGraph::Action& vertex1 = graph->addNewAction(0);
    const EvoGraph::Team& vertex2 = graph->addNewTeam();
    const EvoGraph::Action& vertex3 = graph->addNewAction(1);
    const EvoGraph::Edge& edge0 = graph->addNewEdge(vertex0, vertex1, *lgpAgent);
    const EvoGraph::Edge& edge1 = graph->addNewEdge(vertex0, vertex2, *lgpAgent);
    const EvoGraph::Edge& edge2 = graph->addNewEdge(vertex0, vertex3, *lgpAgent);

    for(auto vertex: graph->getVertices()){
        tpgManager->createAgent(vertex);
    }

    RNG::RNG rng;
    rng.setSeed(0);
    ASSERT_NO_THROW(tpgMutator->removeRandomEdge(*graph, vertex0, rng))
        << "Removing a random edge failed unexpectedly.";
    // Check properties of the tpg
    ASSERT_EQ(graph->getEdges().size(), 2) << "No edge was removed from the TPG.";
    // With known seed edge 0 was removed
    auto edges = graph->getEdges();
    ASSERT_EQ(std::count_if(edges.begin(), edges.end(),
                      [&edge1](const EvoGraph::Edge& other) {
                          return edge1 == other;
                      }),
        1)
        << "With a known seed, edge1 should not be removed from the TPG.";
    ASSERT_EQ(std::count_if(edges.begin(), edges.end(),
                      [&edge2](const EvoGraph::Edge& other) {
                          return edge2 == other;
                      }),
        1)
        << "With a known seed, edge2 should not be removed from the TPG.";
}

TEST_F(TpgMutatorTest, TPGMutatorAddRandomEdge)
{
    const EvoGraph::Team& vertex0 = graph->addNewTeam();
    const EvoGraph::Action& vertex1 = graph->addNewAction(0);
    const EvoGraph::Team& vertex2 = graph->addNewTeam();
    const EvoGraph::Action& vertex3 = graph->addNewAction(1);
    const EvoGraph::Action& vertex4 = graph->addNewAction(2);


    graph->addNewEdge(vertex0, vertex1, *lgpAgent);
    graph->addNewEdge(vertex0, vertex2, *lgpAgent);
    graph->addNewEdge(vertex0, vertex3, *lgpAgent);
    graph->addNewEdge(vertex2, vertex4, *lgpAgent);

    for(auto vertex: graph->getVertices()){
        tpgManager->createAgent(vertex);
    }
    

    RNG::RNG rng;
    rng.setSeed(0);
    tpgMutator->updateSpecificContext(*graph, *tpgManager, params.algorithm, rng);
    // Run the add
    ASSERT_NO_THROW(
        tpgMutator->addRandomEdge(*graph, vertex2, rng))
        << "Adding an edge to the TPG should succeed.";

    // Check properties of the tpg
    ASSERT_EQ(graph->getEdges().size(), 5) << "No edge was added from the TPG.";
    ASSERT_EQ(vertex2.getOutgoingEdges().size(), 2)
        << "The random edge was not added to the right team.";

    // Edge was added with vertex1 (with known seed)
    ASSERT_EQ(vertex1.getIncomingEdges().size(), 2)
        << "The random edge was not added with the right (pseudo)random "
           "destination.";
}

TEST_F(TpgMutatorTest, TPGMutatorMutateEdgeDestination)
{
    const EvoGraph::Action& vertex1 = graph->addNewAction(0);
    const EvoGraph::Action& vertex2 = graph->addNewAction(1);
    const EvoGraph::Team& vertex3 = graph->addNewTeam();
    const EvoGraph::Team& vertex4 = graph->addNewTeam();


    for(auto vertex: graph->getRootTeams()){
        tpgManager->createAgent(vertex);
    }

    params.algorithm.tpg.pEdgeDestinationIsAction = 0.5;

    RNG::RNG rng;
    rng.setSeed(2);
    tpgMutator->updateSpecificContext(*graph, *tpgManager, params.algorithm, rng);

    const EvoGraph::Team& vertex0 = graph->addNewTeam();
    const EvoGraph::Edge& edge0 = graph->addNewEdge(vertex0, vertex1, *lgpAgent);
    const EvoGraph::Edge& edge1 = graph->addNewEdge(vertex0, vertex3, *lgpAgent);

    ASSERT_NO_THROW(tpgMutator->mutateEdgeDestination(
        *graph, edge1, params.algorithm, rng));
    // Check properties of the tpg
    ASSERT_EQ(graph->getEdges().size(), 2)
        << "Number of edge should remain unchanged after destination change.";
    ASSERT_EQ(vertex0.getOutgoingEdges().size(), 2)
        << "The edge source should not be altered.";
    ASSERT_EQ(vertex3.getIncomingEdges().size(), 0)
        << "The edge Destination should be vertex4 (with known seed).";
    ASSERT_EQ(vertex4.getIncomingEdges().size(), 1)
        << "The edge Destination should be vertex4 (with known seed).";
}

TEST_F(TpgMutatorTest, TPGMutatorMutateOutgoingEdge)
{
    RNG::RNG rng;
    rng.setSeed(0);

    // Init a TPG
    lgpMutator->initRandomSpecificAgent(*lgpAgent, *graph, *lgpManager, params.algorithm, rng);
    const EvoGraph::Team& vertex0 = graph->addNewTeam();
    const EvoGraph::Action& vertex1 = graph->addNewAction(0);
    const EvoGraph::Edge& edge0 = graph->addNewEdge(vertex0, vertex1, *lgpAgent);


    for(auto vertex: graph->getVertices()){
        tpgManager->createAgent(vertex);
    }
    
    params.algorithm.lgp.maxProgramSize = 96;
    params.algorithm.lgp.pConstantMutation = 0.5;
    params.algorithm.lgp.minConstValue = 0;
    params.algorithm.lgp.maxConstValue = 1;

    // Init its program and fill the archive
    auto execEngine = tpgManager->createExecutionEngine({}, true);
    execEngine->setExecutedAgent(tpgManager->getAgents().at(0));
    execEngine->execute();

    // Mutate (params selected for code coverage)
    params.algorithm.lgp.pAdd = 0.5;
    params.algorithm.lgp.pDelete = 0.5;
    params.algorithm.lgp.pMutate = 1.0;
    params.algorithm.lgp.pSwap = 1.0;
    params.algorithm.tpg.pEdgeDestinationChange = 1.0;

    std::vector<std::reference_wrapper<const Algorithm::Agent>> newPrograms;

    tpgMutator->updateSpecificContext(*graph, *tpgManager, params.algorithm, rng);

    ASSERT_NO_THROW(tpgMutator->mutateOutgoingEdge(
        *graph, edge0, *tpgManager, newPrograms, params.algorithm, rng));
}

/*
TEST_F(TpgMutatorTest, TPGMutatorRemoveRandomActionEdge)
{
    auto graph = std::make_shared<EvoGraph::Graph>(*e);
    const EvoGraph::Action& action = graph->addNewAction(0);
    auto prog1 = std::make_shared<Program::Program>(*e, true);
    auto prog2 = std::make_shared<Program::Program>(*e, true);
   graph->addNewActionEdge(action, prog1, 0);
   graph->addNewActionEdge(action, prog2, 1);

    RNG::RNG rng;
    rng.setSeed(0);

    size_t before = action.getOutgoingEdges().size();
    ASSERT_NO_THROW(
        tpgMutator->removeRandomActionEdge(*graph, action, rng));
    ASSERT_EQ(action.getOutgoingEdges().size(), before - 1);
}

TEST_F(TpgMutatorTest, TPGMutatorAddRandomActionEdge)
{
    // Prepare a Action and a list of candidate edges
    auto graph = std::make_shared<EvoGraph::Graph>(*e);
    const EvoGraph::Action& action = graph->addNewAction(0);
    auto prog1 = std::make_shared<Program::Program>(*e, true);
    auto prog2 = std::make_shared<Program::Program>(*e, true);
    const EvoGraph::Action& action2 = graph->addNewAction(1);
   graph->addNewActionEdge(action, prog1, 0);
   graph->addNewActionEdge(action2, prog2, 1);

    Selector::SelectionContext context;
    for (auto& edge :graph->getEdges()) {
        context.preExistingEdges.push_back(edge.get());
    }

    RNG::RNG rng;
    rng.setSeed(0);

    // Can add an edge without throwing (even if nothing changes if no edge is
    // pickable)
    ASSERT_NO_THROW(
        tpgMutator->addRandomActionEdge(*graph, action, &context, rng));

    context.preExistingEdges.clear();
    // Can add an edge without throwing (even if nothing changes if no edge is
    // pickable)
    ASSERT_NO_THROW(
        tpgMutator->addRandomActionEdge(*graph, action, &context, rng));
}

TEST_F(TpgMutatorTest, TPGMutatorSwapActionEdges)
{
    // Prepare a Action with at least 3 outgoing edges
    auto graph = std::make_shared<EvoGraph::Graph>(*e);
    const EvoGraph::Action& action = graph->addNewAction(0);
    auto prog1 = std::make_shared<Program::Program>(*e, true);
    auto prog2 = std::make_shared<Program::Program>(*e, true);
    auto prog3 = std::make_shared<Program::Program>(*e, true);
   graph->addNewActionEdge(action, prog1, 0);
   graph->addNewActionEdge(action, prog2, 1);
   graph->addNewActionEdge(action, prog3, 2);

    RNG::RNG rng;
    // Set a known seed to ensure deterministic behavior and have equality
    // during random choice of swapping Should be change if determinism is
    // changed
    rng.setSeed(4);

    // We check that the swapActionEdges function works correctly
    auto before = std::vector<uint64_t>();
    for (auto edge : action.getOutgoingEdges()) {
        before.push_back(
            dynamic_cast<EvoGraph::ActionEdge*>(edge)->getActionClass());
    }
    ASSERT_NO_THROW(tpgMutator->swapActionEdges(*graph, action, rng));
    auto after = std::vector<uint64_t>();
    for (auto edge : action.getOutgoingEdges()) {
        after.push_back(
            dynamic_cast<EvoGraph::ActionEdge*>(edge)->getActionClass());
    }
    // There must be at least one change
    ASSERT_NE(before, after);
}

TEST_F(TpgMutatorTest, TPGMutatorMutateAction_MultiAction)
{
    // Teste la mutation d'une action avec useMultiActionProgram
    RNG::RNG rng;
    rng.setSeed(0);

    params.algorithm.tpg.useActionProgram = true;
    params.algorithm.tpg.useMultiActionProgram = true;
    params.algorithm.tpg.pActionEdgeDeletion = 0.7;
    params.algorithm.tpg.pActionEdgeAddition = 0.7;
    params.algorithm.tpg.pSwapActionProgram = 0.7;
    params.algorithm.tpg.pMutateActionProgram = 0.7;

    uint64_t nbActions = 5;
    Environment ce(set, params, vect, nbActions);

    EvoGraph::Graph tpg(ce);
    const EvoGraph::Action* action = graph->addNewAction(0);
    const EvoGraph::Action* actionUnused = graph->addNewAction(1);
    auto prog1 = std::make_shared<Program::Program>(ce, true);
    auto prog2 = std::make_shared<Program::Program>(ce, true);
    auto prog3 = std::make_shared<Program::Program>(ce, true);
    auto prog4 = std::make_shared<Program::Program>(ce, true);
   graph->addNewActionEdge(*action, prog1, 0);
   graph->addNewActionEdge(*action, prog2, 1);
   graph->addNewActionEdge(*actionUnused, prog3, 2);
   graph->addNewActionEdge(*actionUnused, prog4, 1);

    std::list<std::shared_ptr<Program::Program>> newPrograms;

    Selector::SelectionContext context;
    std::for_each(*graph->getEdges().begin(),graph->getEdges().end(),
                  [&context](const std::unique_ptr<EvoGraph::Edge>& edge) {
                      context.preExistingEdges.push_back(edge.get());
                  });

    // Do it several times to cover all branches
    for (size_t i = 0; i < 5; i++) {
        const EvoGraph::Action* copiedAction =
            dynamic_cast<const EvoGraph::Action*>(*graph->cloneVertex(*action));

        // Check that all branches are covered (deletion, addition, swap,
        // mutation)
        ASSERT_NO_THROW(tpgMutator->mutateAction(
            tpg, *copiedAction, &context, newPrograms, params.mutation, rng));
    }
}

TEST_F(TpgMutatorTest, TPGMutatorMutateAction)
{

    RNG::RNG rng;
    rng.setSeed(0);

    params.algorithm.tpg.useActionProgram = true;
    uint64_t nbActions = 8;
    Environment ce(set, params, vect, nbActions);

    std::shared_ptr<Program::Program> progPointer1 =
        std::shared_ptr<Program::Program>(new Program::Program(ce, true));

    // Init a TPG
    EvoGraph::Graph tpg(ce);
    const EvoGraph::Team& vertex0 = graph->addNewTeam();
    const EvoGraph::Action& vertex1 = graph->addNewAction(0);
    const EvoGraph::Edge& edge0 = graph->addNewEdge(*vertex0, *vertex1, *lgpAgent);
    const EvoGraph::Edge& edge1 = graph->addNewActionEdge(vertex1, progPointer1, 0);

    // Init its program and fill the archive
    Mutator::MutationParameters params;
    Algorithm::TPG::TPGArchive archive;
    EvoGraph::OldExecutionEngine tee(ce, &archive);
    params.algorithm.lgp.maxProgramSize = 96;
    params.algorithm.lgp.pConstantMutation = 0.5;
    params.algorithm.lgp.minConstValue = 0;
    params.algorithm.lgp.maxConstValue = 1;
    Mutator::ProgramMutator::initRandomProgram(*progPointer1, params, rng);
    tee.executeFromRoot(vertex0);

    Selector::SelectionContext context;
    context.preExistingEdges.push_back(&edge0);

    std::list<std::shared_ptr<Program::Program>> newPrograms;
    ASSERT_NO_THROW(tpgMutator->mutateAction(
        tpg, vertex1, &context, newPrograms, params, rng));

    // Check that progPointer use count was decreased since the mutated program
    // is a copy of the original
    ASSERT_EQ(progPointer1.use_count(), 1)
        << "Shared pointer should no longer be used inside the TPG after "
           "mutation.";
}

TEST_F(TpgMutatorTest, TPGMutatorMutateActionEdge_MultiAction)
{
    // Teste la mutation d'une action edge avec useMultiActionProgram
    RNG::RNG rng;
    rng.setSeed(1);

    params.algorithm.tpg.useActionProgram = true;
    params.algorithm.tpg.useMultiActionProgram = true;
    params.algorithm.tpg.pChangeActionClass =
        1.0; // Pour forcer le changement d'actionClass
    uint64_t nbActions = 3;
    Environment ce(set, params, vect, nbActions);

    EvoGraph::Graph tpg(ce);
    const EvoGraph::Action& action = graph->addNewAction(0);
    auto prog = std::make_shared<Program::Program>(ce, true);
   graph->addNewActionEdge(action, prog, 0);
   graph->addNewActionEdge(action, std::make_shared<Program::Program>(ce, true),
                         1);

    std::list<std::shared_ptr<Program::Program>> newPrograms;
    auto edge =
        dynamic_cast<EvoGraph::ActionEdge*>(*action.getOutgoingEdges().begin());

    // On vérifie que le changement d'actionClass est bien tenté
    ASSERT_NO_THROW(tpgMutator->mutateActionEdge(
        tpg, action, edge, newPrograms, params.mutation, rng));
}

TEST_F(TpgMutatorTest, TPGMutatorOutgoingEdgeMutateAction)
{

    RNG::RNG rng;
    rng.setSeed(0);

    params.algorithm.tpg.useActionProgram = true;
    uint64_t nbActions = 8;
    Environment ce(set, params, vect, nbActions);

    std::shared_ptr<Program::Program> progPointer1 =
        std::shared_ptr<Program::Program>(new Program::Program(ce, true));

    // Init a TPG
    EvoGraph::Graph tpg(ce);
    const EvoGraph::Team& vertex0 = graph->addNewTeam();
    const EvoGraph::Action& vertex1 = graph->addNewAction(0);
    const EvoGraph::Edge& edge0 = graph->addNewEdge(*vertex0, *vertex1, *lgpAgent);
    const EvoGraph::Edge& edge1 = graph->addNewActionEdge(vertex1, progPointer1, 0);

    // Init its program and fill the archive
    Mutator::MutationParameters params;
    Algorithm::TPG::TPGArchive archive;
    EvoGraph::OldExecutionEngine tee(ce, &archive);
    params.algorithm.lgp.maxProgramSize = 96;
    params.algorithm.lgp.pConstantMutation = 0.5;
    params.algorithm.lgp.minConstValue = 0;
    params.algorithm.lgp.maxConstValue = 1;
    params.algorithm.tpg.probaContextOverActionProgram = 0;
    params.algorithm.tpg.useActionProgram = true;
    Mutator::ProgramMutator::initRandomProgram(*progPointer1, params, rng);
    tee.executeFromRoot(vertex0);

    Selector::SelectionContext context;
    context.preExistingTeams.push_back(&vertex0);
    context.preExistingActions.push_back(&vertex1);
    context.preExistingEdges.push_back(&edge0);

    std::list<std::shared_ptr<Program::Program>> newPrograms;
    ASSERT_NO_THROW(tpgMutator->mutateOutgoingEdge(
        tpg, &edge0, &context, newPrograms, params, rng));

    ASSERT_EQ(*graph->getNbVertices(), 3) << "Action should have been duplicated.";

   graph->removeVertex(vertex1);

    // Check that progPointer use count was decreased since the mutated program
    // is a copy of the original
    ASSERT_EQ(progPointer1.use_count(), 1)
        << "Shared pointer should no longer be used inside the TPG after "
           "mutation.";
}*/

TEST_F(TpgMutatorTest, TPGMutatorMutateTeam)
{
    RNG::RNG rng;
    rng.setSeed(0);

    // Create a TPG
    const EvoGraph::Team& vertex0 = graph->addNewTeam();
    const EvoGraph::Action& vertex1 = graph->addNewAction(0);
    const EvoGraph::Action& vertex2 = graph->addNewAction(1);
    const EvoGraph::Edge& edge0 = graph->addNewEdge(vertex0, vertex1, *lgpAgent);
    const EvoGraph::Edge& edge1 = graph->addNewEdge(vertex0, vertex2, *lgpAgent);
    const EvoGraph::Action& vertex3 = graph->addNewAction(2);
    const EvoGraph::Team& vertex4 = graph->addNewTeam();
    const EvoGraph::Edge& edge2 = graph->addNewEdge(vertex4, vertex3, *lgpAgent);
    const EvoGraph::Edge& edge3 = graph->addNewEdge(vertex0, vertex3, *lgpAgent);

    for(auto vertex: graph->getVertices()){
        tpgManager->createAgent(vertex);
    }
    
    params.algorithm.lgp.maxProgramSize = 96;
    params.algorithm.tpg.pEdgeDeletion = 0.7;
    params.algorithm.tpg.pEdgeAddition = 0.7;
    params.algorithm.tpg.pProgramMutation = 0.2;
    params.algorithm.tpg.pEdgeDestinationChange = 0.1;
    params.algorithm.tpg.pEdgeDestinationIsAction = 0.5;
    params.algorithm.lgp.pAdd = 0.5;
    params.algorithm.lgp.pDelete = 0.5;
    params.algorithm.lgp.pMutate = 1.0;
    params.algorithm.lgp.pSwap = 1.0;
    params.algorithm.lgp.pConstantMutation = 0.5;
    params.algorithm.lgp.minConstValue = 0;
    params.algorithm.lgp.maxConstValue = 1;

    // Init its program and fill the archive
    auto execEngine = tpgManager->createExecutionEngine({}, true);
    execEngine->setExecutedAgent(tpgManager->getAgents().at(0));
    execEngine->execute();


    std::vector<std::reference_wrapper<const Algorithm::Agent>> newPrograms;

    tpgMutator->updateSpecificContext(*graph, *tpgManager, params.algorithm, rng);

    auto& newAgent = tpgManager->copyAgent(tpgManager->getAgents().at(0), *graph);

    // Test the function in normal conditions
    // (only edge2 can be part of "preExistingEdges" since all other edges are
    // outgoing from vertex0, which would mean they are not pre-existing in
    // the mutation process.)
    ASSERT_NO_THROW(tpgMutator->mutateAgent(newAgent, *graph, *tpgManager,
                                            newPrograms, params.algorithm, rng))
        << "Mutate team should not fail in these conditions.";

    // No other check really needed since individual mutation functions are
    // already covered in other unit tests.
}

TEST_F(TpgMutatorTest, TPGMutatorMutateProgramBehaviorAgainstArchive)
{
    RNG::RNG rng;
    rng.setSeed(0);

    // Init a TPG
    const EvoGraph::Team& vertex0 = graph->addNewTeam();
    const EvoGraph::Action& vertex1 = graph->addNewAction(0);
    const EvoGraph::Edge& edge0 = graph->addNewEdge(vertex0, vertex1, *lgpAgent);

    for(auto vertex: graph->getVertices()){
        tpgManager->createAgent(vertex);
    }
    
    // Init its program and fill the archive
    params.algorithm.lgp.maxProgramSize = 96;
    params.algorithm.lgp.pConstantMutation = 0.5;
    params.algorithm.lgp.minConstValue = 0;
    params.algorithm.lgp.maxConstValue = 1;

    // Init its program and fill the archive
    auto execEngine = tpgManager->createExecutionEngine({}, true);
    execEngine->setExecutedAgent(tpgManager->getAgents().at(0));
    execEngine->execute();


    // Mutate (params selected for code coverage)
    params.algorithm.lgp.pAdd = 0.5;
    params.algorithm.lgp.pDelete = 0.5;
    params.algorithm.lgp.pMutate = 1.0;
    params.algorithm.lgp.pSwap = 1.0;
    params.algorithm.tpg.pEdgeDestinationChange = 1.0;

    std::vector<std::reference_wrapper<const Algorithm::Agent>> newPrograms;

    tpgMutator->updateSpecificContext(*graph, *tpgManager, params.algorithm, rng);

    tpgMutator->mutateOutgoingEdge(*graph, edge0, *tpgManager, newPrograms,
                                            params.algorithm, rng);
    
    tpgMutator->setArchive(*archive);
    ASSERT_NO_THROW(tpgMutator->mutateProgramAgentAgainstArchive(
        newPrograms.front(), *graph, *lgpManager, params.algorithm, rng))
        << "Mutating a Program behavior failed unexpectedly.";

    // Check the unicity against the Algorithm::TPG::TPGArchive
    // Verify new program uniqueness
    
    execEngine = lgpManager->createExecutionEngine({}, true);
    execEngine->setExecutedAgent(newPrograms.front());
    double result = execEngine->execute().at(0);
    std::map<size_t, double> hashesAndResults = {
        {archive->getCombinedHash(e->getDataSources()), result}};
    ASSERT_TRUE(archive->areProgramResultsUnique(hashesAndResults))
        << "Mutated program associated to the edge should return a unique bid "
           "on the environment.";
}

TEST_F(TpgMutatorTest, TPGMutatorMutateNewProgramBehaviorsSequential)
{
    RNG::RNG rng;
    rng.setSeed(0);


    uint64_t nbActions = 4;
    params.algorithm.tpg.maxInitOutgoingEdges = 3;
    params.algorithm.lgp.maxProgramSize = 96;
    params.algorithm.nbAgents = 7;
    // Proba as in Kelly's paper
    params.algorithm.tpg.pEdgeDeletion = 0.7;
    params.algorithm.tpg.pEdgeAddition = 0.7;
    params.algorithm.tpg.pProgramMutation = 0.2;
    params.algorithm.tpg.pEdgeDestinationChange = 0.1;
    params.algorithm.tpg.pEdgeDestinationIsAction = 0.5;
    params.algorithm.lgp.pAdd = 0.5;
    params.algorithm.lgp.pDelete = 0.5;
    params.algorithm.lgp.pMutate = 1.0;
    params.algorithm.lgp.pSwap = 1.0;
    params.algorithm.lgp.pConstantMutation = 0.5;
    params.algorithm.lgp.minConstValue = 0;
    params.algorithm.lgp.maxConstValue = 10;

    tpgMutator->initRandomPopulation(*graph, *tpgManager, params.algorithm, rng);
    // Init its program and fill the archive
    auto execEngine = tpgManager->createExecutionEngine({}, true);
    for (auto& agent :tpgManager->getAgents()) {
        execEngine->setExecutedAgent(agent);
        execEngine->execute();
    }

    // Create a list of Programs to mutate
    std::vector<std::reference_wrapper<const Algorithm::Agent>> newAgents;
    for (auto& edge :graph->getEdges()) {
        newAgents.emplace_back(lgpManager->copyAgent(edge.get().getProgram(), *graph));
    }

    // Mutate them sequentially
    ASSERT_NO_THROW(tpgMutator->mutateSubAgents(
        newAgents, *graph, *tpgManager, params.algorithm, rng, 0))
        << "Program behavior mutation failed (sequentially).";
}

TEST_F(TpgMutatorTest, TPGMutatorMutateNewProgramBehaviorsParallel)
{
    RNG::RNG rng;
    rng.setSeed(0);


    uint64_t nbActions = 4;
    params.algorithm.tpg.maxInitOutgoingEdges = 3;
    params.algorithm.lgp.maxProgramSize = 96;
    params.algorithm.nbAgents = 7;
    // Proba as in Kelly's paper
    params.algorithm.tpg.pEdgeDeletion = 0.7;
    params.algorithm.tpg.pEdgeAddition = 0.7;
    params.algorithm.tpg.pProgramMutation = 0.2;
    params.algorithm.tpg.pEdgeDestinationChange = 0.1;
    params.algorithm.tpg.pEdgeDestinationIsAction = 0.5;
    params.algorithm.lgp.pAdd = 0.5;
    params.algorithm.lgp.pDelete = 0.5;
    params.algorithm.lgp.pMutate = 1.0;
    params.algorithm.lgp.pSwap = 1.0;
    params.algorithm.lgp.pConstantMutation = 0.5;
    params.algorithm.lgp.minConstValue = 0;
    params.algorithm.lgp.maxConstValue = 10;

    tpgMutator->initRandomPopulation(*graph, *tpgManager, params.algorithm, rng);
    // Init its program and fill the archive
    auto execEngine = tpgManager->createExecutionEngine({}, true);
    for (auto& agent :tpgManager->getAgents()) {
        execEngine->setExecutedAgent(agent);
        execEngine->execute();
    }

    // Create a list of Programs to mutate
    std::vector<std::reference_wrapper<const Algorithm::Agent>> newAgents;
    for (auto& edge :graph->getEdges()) {
        newAgents.emplace_back(lgpManager->copyAgent(edge.get().getProgram(), *graph));
    }

    // Mutate them sequentially
    ASSERT_NO_THROW(tpgMutator->mutateSubAgents(
        newAgents, *graph, *tpgManager, params.algorithm, rng, 4))
        << "Program behavior mutation failed (Parallelism).";
}

TEST_F(TpgMutatorTest, TPGMutatorMutateNewProgramBehaviorsDeterminism)
{
    RNG::RNG rng;


    uint64_t nbActions = 4;
    params.algorithm.tpg.maxInitOutgoingEdges = 3;
    params.algorithm.lgp.maxProgramSize = 96;
    params.algorithm.nbAgents = 7;
    // Proba as in Kelly's paper
    params.algorithm.tpg.pEdgeDeletion = 0.7;
    params.algorithm.tpg.pEdgeAddition = 0.7;
    params.algorithm.tpg.pProgramMutation = 0.2;
    params.algorithm.tpg.pEdgeDestinationChange = 0.1;
    params.algorithm.tpg.pEdgeDestinationIsAction = 0.5;
    params.algorithm.lgp.pAdd = 0.5;
    params.algorithm.lgp.pDelete = 0.5;
    params.algorithm.lgp.pMutate = 1.0;
    params.algorithm.lgp.pSwap = 1.0;
    params.algorithm.lgp.pConstantMutation = 0.5;
    params.algorithm.lgp.minConstValue = 0;
    params.algorithm.lgp.maxConstValue = 10;



    tpgMutator->initRandomPopulation(*graph, *tpgManager, params.algorithm, rng);
    // Init its program and fill the archive
    auto execEngine = tpgManager->createExecutionEngine({}, true);
    for (auto& agent :tpgManager->getAgents()) {
        execEngine->setExecutedAgent(agent);
        execEngine->execute();
    }


    // Create a list of Programs to mutate
    std::vector<std::reference_wrapper<const Algorithm::Agent>> newAgentsSequential;
    std::vector<std::reference_wrapper<const Algorithm::Agent>> newAgentsParallel;
    for (auto& edge :graph->getEdges()) {
        newAgentsSequential.emplace_back(lgpManager->copyAgent(edge.get().getProgram(), *graph));
        newAgentsParallel.emplace_back(lgpManager->copyAgent(edge.get().getProgram(), *graph));
    }
    rng.setSeed(0);
    tpgMutator->mutateSubAgents(
        newAgentsSequential, *graph, *tpgManager, params.algorithm, rng, 0);

    rng.setSeed(0);
    tpgMutator->mutateSubAgents(
        newAgentsParallel, *graph, *tpgManager, params.algorithm, rng, 4);

    // Check determinism
    // Using nb lines of programs
    for (auto i = 0; i < newAgentsSequential.size(); i++) {
        ASSERT_EQ(dynamic_cast<const Algorithm::LGP::LGPAgent&>(newAgentsSequential.at(i).get()).getNbLines(),
                  dynamic_cast<const Algorithm::LGP::LGPAgent&>(newAgentsParallel.at(i).get()).getNbLines())
            << "Different number of line in mutatedPrograms.";
    }

}

TEST_F(TpgMutatorTest, TPGMutatorPopulate)
{
    RNG::RNG rng;
    rng.setSeed(0);

    std::shared_ptr<EvoGraph::Graph> graph = std::make_shared<EvoGraph::Graph>();

    uint64_t nbActions = 4;
    params.algorithm.tpg.maxInitOutgoingEdges = 3;
    params.algorithm.lgp.maxProgramSize = 96;
    params.algorithm.nbAgents = 7;
    // Proba as in Kelly's paper
    params.algorithm.tpg.pEdgeDeletion = 0.7;
    params.algorithm.tpg.pEdgeAddition = 0.7;
    params.algorithm.tpg.pProgramMutation = 0.2;
    params.algorithm.tpg.pEdgeDestinationChange = 0.1;
    params.algorithm.tpg.pEdgeDestinationIsAction = 0.5;
    params.algorithm.lgp.pAdd = 0.5;
    params.algorithm.lgp.pDelete = 0.5;
    params.algorithm.lgp.pMutate = 1.0;
    params.algorithm.lgp.pSwap = 1.0;
    params.algorithm.lgp.pConstantMutation = 0.5;
    params.algorithm.lgp.minConstValue = 0;
    params.algorithm.lgp.maxConstValue = 10;

    selector->setNbAgents(params.algorithm.nbAgents);
    tpgMutator->initRandomPopulation(*graph, *tpgManager, params.algorithm, rng);
    // Init its program and fill the archive
    auto execEngine = tpgManager->createExecutionEngine({}, true);
    for (auto& agent :tpgManager->getAgents()) {
        execEngine->setExecutedAgent(agent);
        execEngine->execute();
    }

    // Check the correct execution
    ASSERT_NO_THROW(tpgMutator->mutatePopulation(
        *graph, *tpgManager, params.algorithm, rng, 0))
        << "Populating a TPG failed.";
    // Check the number of roots
    ASSERT_EQ(graph->getRootVertices().size(), params.algorithm.nbAgents);
    ASSERT_EQ(tpgManager->getAgents().size(), params.algorithm.nbAgents);

    // Increase coverage with a TPG that has no root team
    std::shared_ptr<EvoGraph::Graph> graph2 = std::make_shared<EvoGraph::Graph>();
    auto tpgManager2 = std::make_shared<Algorithm::TPG::TPGManager>(nbActions, 1);
    tpgManager2->addSubManager(*lgpManager);
    ASSERT_NO_THROW(tpgMutator->mutatePopulation(
        *graph2, *tpgManager2, params.algorithm, rng, 0))
        << "Populating an empty TPG failed.";
}

/*
TEST_F(TpgMutatorTest, TPGMutatorPopulateActionRoots)
{
    RNG::RNG rng;
    rng.setSeed(0);

    uint64_t nbActions = 5;
    Environment ce(set, params, vect, nbActions);

    std::shared_ptr<EvoGraph::Graph> tpg = std::make_shared<EvoGraph::Graph>(ce);

    params.algorithm.tpg.maxInitOutgoingEdges = 3;
    params.algorithm.lgp.maxProgramSize = 96;
    params.algorithm.nbAgents = 10;
    params.algorithm.tpg.useActionProgram = true;
    params.algorithm.tpg.useMultiActionProgram = true;
    params.algorithm.tpg.ratioTeamsOverActions = 0.5;
    // Proba as in Kelly's paper
    params.algorithm.tpg.pEdgeDeletion = 0.7;
    params.algorithm.tpg.pEdgeAddition = 0.7;
    params.algorithm.tpg.pProgramMutation = 0.2;
    params.algorithm.tpg.pEdgeDestinationChange = 0.1;
    params.algorithm.tpg.pEdgeDestinationIsAction = 0.5;
    params.algorithm.lgp.pAdd = 0.5;
    params.algorithm.lgp.pDelete = 0.5;
    params.algorithm.lgp.pMutate = 1.0;
    params.algorithm.lgp.pSwap = 1.0;
    params.algorithm.lgp.pConstantMutation = 0.5;
    params.algorithm.lgp.minConstValue = 0;
    params.algorithm.lgp.maxConstValue = 10;
    Algorithm::TPG::TPGArchive archive;
    Selector::Selector selector(*graph, params);

    TpgMutator->initRandomPopulation(*tpg, params.mutation, rng, nbActions);
    // fill the archive before populating to test uniqueness of new prog
    EvoGraph::OldExecutionEngine tee(*e, &archive);
    for (auto rootVertex : tpg->getRootVertices()) {
        tee.executeFromRoot(*rootVertex);
    }

    // Check the correct execution
    ASSERT_NO_THROW(tpgMutator->populateTPG(
        *tpg, selector, archive, params.mutation, rng, nbActions, 0))
        << "Populating a TPG failed.";
    // Check the number of roots
    ASSERT_EQ(tpg->getRootVertices().size(), params.algorithm.nbAgents);

    size_t nbActionsRoots = 0;
    size_t nbTeamsRoots = 0;
    for (const auto& root : tpg->getRootVertices()) {
        if (dynamic_cast<const EvoGraph::Action*>(root)) {
            nbActionsRoots++;
        }
        else if (dynamic_cast<const EvoGraph::Team*>(root)) {
            nbTeamsRoots++;
        }
    }
    // Check the ratio of teams over actions
    ASSERT_EQ(nbTeamsRoots, params.algorithm.nbAgents *
                                params.algorithm.tpg.ratioTeamsOverActions)
        << "The number of team roots is not as expected.";
    ASSERT_EQ(nbActionsRoots, params.algorithm.nbAgents - nbTeamsRoots)
        << "The number of action roots is not as expected.";
}

TEST_F(TpgMutatorTest, TPGMutatorPopulateTPGWithTournamentSelection)
{
    RNG::RNG rng;
    rng.setSeed(0);

    uint64_t nbActions = 5;
    params.selection._selectionMode = "tournament";
    params.selection.tournament.sizeTournament = 2;
    params.selection.tournament.ratioSavedRoots = 0.1;
    Environment ce(set, params, vect, nbActions);

    std::shared_ptr<EvoGraph::Graph> tpg = std::make_shared<EvoGraph::Graph>(ce);

    params.algorithm.tpg.maxInitOutgoingEdges = 3;
    params.algorithm.lgp.maxProgramSize = 96;
    params.algorithm.nbAgents = 10;
    params.algorithm.tpg.useActionProgram = true;
    params.algorithm.tpg.useMultiActionProgram = true;
    params.algorithm.tpg.ratioTeamsOverActions = 0.5;
    // Proba as in Kelly's paper
    params.algorithm.tpg.pEdgeDeletion = 0.7;
    params.algorithm.tpg.pEdgeAddition = 0.7;
    params.algorithm.tpg.pProgramMutation = 0.2;
    params.algorithm.tpg.pEdgeDestinationChange = 0.1;
    params.algorithm.tpg.pEdgeDestinationIsAction = 0.5;
    params.algorithm.lgp.pAdd = 0.5;
    params.algorithm.lgp.pDelete = 0.5;
    params.algorithm.lgp.pMutate = 1.0;
    params.algorithm.lgp.pSwap = 1.0;
    params.algorithm.lgp.pConstantMutation = 0.5;
    params.algorithm.lgp.minConstValue = 0;
    params.algorithm.lgp.maxConstValue = 10;
    Algorithm::TPG::TPGArchive archive;
    Selector::TournamentSelector selector(*graph, params);

    TpgMutator->initRandomPopulation(*tpg, params.mutation, rng, nbActions);
    // fill the archive before populating to test uniqueness of new prog
    EvoGraph::OldExecutionEngine tee(*e, &archive);

    // Do fake results to fill the verticesToDelete set.
    std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                  const EvoGraph::Vertex*>
        fakeResults;
    for (auto rootVertex : tpg->getRootVertices()) {
        std::shared_ptr<Learn::EvaluationResult> er =
            std::make_shared<Learn::EvaluationResult>(
                std::make_shared<Selector::SelectionMetrics>(
                    rng.getDouble(0, 1)),
                1);
        fakeResults.insert(std::make_pair(er, rootVertex));
    }
    selector.doSelection(fakeResults, rng);

    // Check the correct execution
    ASSERT_NO_THROW(tpgMutator->populateTPG(
        *tpg, selector, archive, params.mutation, rng, nbActions, 0))
        << "Populating a TPG failed.";
    // Check the number of roots
    ASSERT_EQ(tpg->getRootVertices().size(), params.algorithm.nbAgents);

    ASSERT_EQ(selector.getVerticesToDelete().size(), 0)
        << "After populateTPG with tournament selection, the set of vertices "
           "to delete should be empty.";
}

TEST_F(TpgMutatorTest, TPGMutatorCrossEdgesSwapPrograms)
{
    Environment ce(set, params, vect, 2);
    EvoGraph::Graph graph(ce);
    RNG::RNG rng;
    rng.setSeed(0);

    Mutator::MutationParameters params;
    params.algorithm.lgp.maxProgramSize = 64;
    params.algorithm.lgp.initMaxProgramSize = 8;
    params.algorithm.lgp.initMinProgramSize = 4;
    params.algorithm.lgp.maxConstValue = 10;
    params.algorithm.lgp.minConstValue = 0;

    // Create 2 action vertices with known ids (0 and 1)
    const EvoGraph::Action* a0 = &(*graph->addNewAction(0));
    const EvoGraph::Action* a1 = &(*graph->addNewAction(1));

    // Create two distinct programs and attach them as action edges for actionID
    // = 0
    auto p0 = std::make_shared<Program::Program>(*graph->getEnvironment(), true);
    auto p1 = std::make_shared<Program::Program>(*graph->getEnvironment(), true);

    // Make sure programs have at least one line (so getNbLines() > 0)
    Mutator::ProgramMutator::initRandomProgram(*p0, params, rng);
    Mutator::ProgramMutator::initRandomProgram(*p1, params, rng);

    graph->addNewActionEdge(*a0, p0, 0); // a0 has an edge for actionID 0 -> p0
    graph->addNewActionEdge(*a1, p1, 0); // a1 has an edge for actionID 0 -> p1

    // Sanity checks before swap
    auto edgeA0_before = a0->getEdgeOfAction(0);
    auto edgeA1_before = a1->getEdgeOfAction(0);
    ASSERT_NE(edgeA0_before, nullptr);
    ASSERT_NE(edgeA1_before, nullptr);

    auto progA0_before = edgeA0_before->getProgramSharedPointer();
    auto progA1_before = edgeA1_before->getProgramSharedPointer();
    ASSERT_EQ(progA0_before.get(), p0.get());
    ASSERT_EQ(progA1_before.get(), p1.get());

    // Perform crossEdges which should effectively swap the action-edge programs
    std::vector<const EvoGraph::Action*> childs = {a0, a1};
    ASSERT_NO_THROW(
        tpgMutator->crossEdges(*graph, childs, 0, params, rng));

    // After crossEdges: a0's edge should now hold p1 and a1's edge should hold
    // p0
    auto edgeA0_after = a0->getEdgeOfAction(0);
    auto edgeA1_after = a1->getEdgeOfAction(0);
    ASSERT_NE(edgeA0_after, nullptr);
    ASSERT_NE(edgeA1_after, nullptr);

    auto progA0_after = edgeA0_after->getProgramSharedPointer();
    auto progA1_after = edgeA1_after->getProgramSharedPointer();

    ASSERT_EQ(progA0_after.get(), p1.get())
        << "a0 should now point to p1 after crossEdges";
    ASSERT_EQ(progA1_after.get(), p0.get())
        << "a1 should now point to p0 after crossEdges";
}

TEST_F(TpgMutatorTest, TPGMutatorCrossProgram)
{
    Environment ce(set, params, vect, 2);
    EvoGraph::Graph graph(ce);
    RNG::RNG rng;
    rng.setSeed(1);

    Mutator::MutationParameters params;
    params.algorithm.lgp.maxProgramSize = 10;
    params.algorithm.lgp.initMaxProgramSize = 10;
    params.algorithm.lgp.initMinProgramSize = 6;
    params.algorithm.lgp.maxConstValue = 10;
    params.algorithm.lgp.minConstValue = 0;

    // Create two parent actions
    const EvoGraph::Action* parent0 = &(*graph->addNewAction(0));
    const EvoGraph::Action* parent1 = &(*graph->addNewAction(1));

    // Create two parent programs (ensure they have multiple lines)
    auto parentProg0 =
        std::make_shared<Program::Program>(*graph->getEnvironment(), true);
    auto parentProg1 =
        std::make_shared<Program::Program>(*graph->getEnvironment(), true);
    Mutator::ProgramMutator::initRandomProgram(*parentProg0, params, rng);
    Mutator::ProgramMutator::initRandomProgram(*parentProg1, params, rng);

    // Attach to both parents an action edge for actionID = 0
    graph->addNewActionEdge(*parent0, parentProg0, 0);
    graph->addNewActionEdge(*parent1, parentProg1, 0);

    // Keep copies of parent sizes and pointers
    uint64_t sizeParent0 = parentProg0->getNbLines();
    uint64_t sizeParent1 = parentProg1->getNbLines();
    ASSERT_GE(sizeParent0, 2u);
    ASSERT_GE(sizeParent1, 2u);

    // Prepare children actions: create two new actions which will receive the
    // children
    const EvoGraph::Action* child0 = &(*graph->addNewAction(2));
    const EvoGraph::Action* child1 = &(*graph->addNewAction(3));

    // Add dummy edges to children for the same actionID so crossProgram can set
    // them. Use copies of parent programs initially to ensure edge exists.
    graph->addNewActionEdge(*child0, parentProg0, 0);
    graph->addNewActionEdge(*child1, parentProg1, 0);

    std::vector<const EvoGraph::Action*> childs = {child0, child1};

    // Call crossProgram: it should create new program objects for each child
    ASSERT_NO_THROW(
        tpgMutator->crossProgram(*graph, childs, 0, params, rng));

    // After crossProgram, the childrens' program pointers must NOT be the exact
    // same as parents (new programs)
    auto c0edge = child0->getEdgeOfAction(0);
    auto c1edge = child1->getEdgeOfAction(0);
    ASSERT_NE(c0edge, nullptr);
    ASSERT_NE(c1edge, nullptr);

    auto c0prog = c0edge->getProgramSharedPointer();
    auto c1prog = c1edge->getProgramSharedPointer();

    ASSERT_NE(c0prog.get(), parentProg0.get())
        << "child 0 must have a new program (not parent0 pointer)";
    ASSERT_NE(c1prog.get(), parentProg1.get())
        << "child 1 must have a new program (not parent1 pointer)";

    // Programs must be non-empty and respect maxProgramSize
    ASSERT_GT(c0prog->getNbLines(), 0u);
    ASSERT_GT(c1prog->getNbLines(), 0u);
    ASSERT_LE(c0prog->getNbLines(), params.algorithm.lgp.maxProgramSize);
    ASSERT_LE(c1prog->getNbLines(), params.algorithm.lgp.maxProgramSize);
}

TEST_F(TpgMutatorTest, TPGMutatorCrossAction)
{
    Environment ce(set, params, vect, 2);
    EvoGraph::Graph graph(ce);
    RNG::RNG rng;
    rng.setSeed(2);

    Mutator::MutationParameters params;
    // Force cross operations to happen
    params.algorithm.tpg.pCrossAgents = 0.9; // allow repeated attempts
    params.algorithm.tpg.pCrossPrograms =
        0.9; // prefer program-level crossover when possible
    params.algorithm.lgp.maxProgramSize = 64;
    params.algorithm.lgp.initMaxProgramSize = 8;
    params.algorithm.lgp.initMinProgramSize = 4;
    params.algorithm.lgp.maxConstValue = 10;
    params.algorithm.lgp.minConstValue = 0;

    // create two parents that assess the same actions (we'll ensure both assess
    // action 0 and 1)
    const EvoGraph::Action* p0 = &(*graph->addNewAction(0));
    const EvoGraph::Action* p1 = &(*graph->addNewAction(1));

    // Make programs for actions 0 and 1 for both parents and attach them
    auto pp00 =
        std::make_shared<Program::Program>(*graph->getEnvironment(), true);
    auto pp01 =
        std::make_shared<Program::Program>(*graph->getEnvironment(), true);
    auto pp10 =
        std::make_shared<Program::Program>(*graph->getEnvironment(), true);
    auto pp11 =
        std::make_shared<Program::Program>(*graph->getEnvironment(), true);

    Mutator::ProgramMutator::initRandomProgram(*pp00, params, rng);
    Mutator::ProgramMutator::initRandomProgram(*pp01, params, rng);
    Mutator::ProgramMutator::initRandomProgram(*pp10, params, rng);
    Mutator::ProgramMutator::initRandomProgram(*pp11, params, rng);

    // attach each parent two action edges (actionID 0 and 1)
    graph->addNewActionEdge(*p0, pp00, 0);
    graph->addNewActionEdge(*p0, pp01, 1);
    graph->addNewActionEdge(*p1, pp10, 0);
    graph->addNewActionEdge(*p1, pp11, 1);

    // Ensure both parents assess actions 0 and 1, update assessed actions
    graph->updateAssessedActions(p0);
    graph->updateAssessedActions(p1);

    // Save some original program pointers to compare after crossAction
    auto before_p0_a0 = p0->getEdgeOfAction(0)->getProgramSharedPointer().get();
    auto before_p0_a1 = p0->getEdgeOfAction(1)->getProgramSharedPointer().get();
    auto before_p1_a0 = p1->getEdgeOfAction(0)->getProgramSharedPointer().get();
    auto before_p1_a1 = p1->getEdgeOfAction(1)->getProgramSharedPointer().get();

    std::vector<const EvoGraph::Action*> childs = {p0, p1};

    // Run crossAction: with pCrossAgents=1.0 and pCrossPrograms=1.0 we
    // expect at least one crossover.
    ASSERT_NO_THROW(
        tpgMutator->crossAction(*graph, childs, params, rng));

    // After crossover, at least one of the action-program pointers should have
    // changed
    bool changed = false;
    if (p0->getEdgeOfAction(0)->getProgramSharedPointer().get() != before_p0_a0)
        changed = true;
    if (p0->getEdgeOfAction(1)->getProgramSharedPointer().get() != before_p0_a1)
        changed = true;
    if (p1->getEdgeOfAction(0)->getProgramSharedPointer().get() != before_p1_a0)
        changed = true;
    if (p1->getEdgeOfAction(1)->getProgramSharedPointer().get() != before_p1_a1)
        changed = true;

    ASSERT_TRUE(changed) << "crossAction should change at least one child's "
                            "action program/pointer";
}*/
