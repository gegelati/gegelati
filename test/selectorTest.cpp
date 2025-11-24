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

#include "instructions/addPrimitiveType.h"
#include "learn/fakeClassificationLearningEnvironment.h"
#include "learn/fakeMultiContinuousLearningEnvironment.h"
#include "learn/learningAgent.h"
#include "learn/learningEnvironment.h"
#include "learn/stickGameWithOpponent.h"

#include "selector/selectionContext.h"
#include "selector/selector.h"
#include "selector/selectorFactory.h"
#include "selector/tournamentSelector.h"
#include "selector/truncationSelector.h"
#include "util/counterReset.h"
class SelectorTest : public ::testing::Test
{
  protected:
    const size_t size1{24};
    const size_t size2{32};
    Instructions::Set set;
    StickGameWithOpponent le;
    FakeMultiContinuousLearningEnvironment cle;
    FakeClassificationLearningEnvironment classifLe;
    Learn::LearningParameters params;
    std::vector<std::reference_wrapper<const Data::DataHandler>> vect;
    std::shared_ptr<Program::Program> progPointer;

    Environment* e;

    virtual void SetUp()
    {

        CounterReset::counterReset();
        vect.push_back(
            *(new Data::PrimitiveTypeArray<int>((unsigned int)size1)));
        vect.push_back(
            *(new Data::PrimitiveTypeArray<double>((unsigned int)size2)));

        set.add(*(new Instructions::AddPrimitiveType<int>()));
        set.add(*(new Instructions::AddPrimitiveType<double>()));

        // Proba as in Kelly's paper
        params.mutation.tpg.maxInitOutgoingEdges = 3;
        params.mutation.prog.maxProgramSize = 96;
        params.mutation.tpg.nbRoots = 15;
        params.mutation.tpg.pEdgeDeletion = 0.7;
        params.mutation.tpg.pEdgeAddition = 0.7;
        params.mutation.tpg.pProgramMutation = 0.2;
        params.mutation.tpg.pEdgeDestinationChange = 0.1;
        params.mutation.tpg.pEdgeDestinationIsAction = 0.5;
        params.mutation.tpg.maxOutgoingEdges = 4;
        params.mutation.prog.pAdd = 0.5;
        params.mutation.prog.pDelete = 0.5;
        params.mutation.prog.pMutate = 1.0;
        params.mutation.prog.pSwap = 1.0;
        params.mutation.prog.pConstantMutation = 0.5;
        params.mutation.prog.minConstValue = 0;
        params.mutation.prog.maxConstValue = 1;
        params.nbProgramConstant = 5;

        e = new Environment(set, params, vect);
        progPointer =
            std::shared_ptr<Program::Program>(new Program::Program(*e, false));
    }

    virtual void TearDown()
    {
        delete (&(vect.at(0).get()));
        delete (&(vect.at(1).get()));
        delete (&set.getInstruction(0));
        delete (&set.getInstruction(1));
    }
};

TEST_F(SelectorTest, Constructor)
{
    std::shared_ptr<TPG::TPGGraph> graph = std::make_shared<TPG::TPGGraph>(*e);

    Selector::Selector* selector;

    ASSERT_NO_THROW(selector = new Selector::Selector(graph, params))
        << "Construction of the selector failed.";

    ASSERT_NO_THROW(delete selector) << "Destruction of the selector failed.";
}

TEST_F(SelectorTest, doAbstractSelection)
{
    std::shared_ptr<TPG::TPGGraph> graph = std::make_shared<TPG::TPGGraph>(*e);

    Selector::Selector selector(graph, params);

    std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                  const TPG::TPGVertex*>
        results;
    RNG::RNG rng;
    ASSERT_THROW(selector.doSelection(results, rng), std::runtime_error)
        << "Doing a selection with the Selector::Selector shoudl throw";
}

TEST_F(SelectorTest, KeepBestPolicy)
{
    params.archiveSize = 50;
    params.archivingProbability = 0.5;
    params.maxNbActionsPerEval = 11;
    params.nbIterationsPerPolicyEvaluation = 1;
    params.selection.truncation.ratioDeletedRoots = 0.2;
    params.nbGenerations = 5;

    Learn::LearningAgent la(le, set, params);
    la.init();
    bool alt = false;
    la.train(alt, true);

    std::shared_ptr<Selector::Selector> selector = la.getSelector();

    ASSERT_NO_THROW(selector->keepBestPolicy())
        << "Keeping the best policy after training should not fail.";
    ASSERT_EQ(selector->getGraph()->getNbRootVertices(), 1)
        << "A single root TPGVertex should remain in the TPGGraph when keeping "
           "the best policy only";
}

TEST_F(SelectorTest, UpdateEvaluationRecords)
{
    Learn::LearningAgent la(le, set, params);
    std::shared_ptr<Selector::Selector> selector = la.getSelector();

    // Check null at build
    ASSERT_EQ(selector->getBestRoot().first, nullptr)
        << "Best root should be a nullptr after building a new LearningAgent.";
    ASSERT_EQ(selector->getBestRoot().second, nullptr)
        << "Best root EvaluationResult should be a nullptr after building a "
           "new LearningAgent.";

    // Check null at init
    la.init();
    ASSERT_EQ(selector->getBestRoot().first, nullptr)
        << "Best root should be a nullptr after init of a LearningAgent.";
    ASSERT_EQ(selector->getBestRoot().second, nullptr)
        << "Best root EvaluationResult should be a nullptr after init of a "
           "LearningAgent.";

    // Update with a fake result for a root of the graph
    auto rootVertices = selector->getGraph()->getRootVertices();
    const TPG::TPGVertex* root = *rootVertices.begin();
    ASSERT_NO_THROW(selector->updateEvaluationRecords(
        {{std::make_shared<Learn::EvaluationResult>(
              std::make_shared<Selector::SelectionMetrics>(1.0), 10),
          root}}));
    ASSERT_EQ(selector->getBestRoot().first, root)
        << "Best root not updated properly.";
    ASSERT_EQ(selector->getBestRoot().second->getSelectionMetrics()->getScore(),
              1.0)
        << "Best root not updated properly.";

    // Update with a fake better result for another root of the graph
    const TPG::TPGVertex* root2 =
        *(selector->getGraph()->getRootVertices().begin() + 1);
    ASSERT_NO_THROW(selector->updateEvaluationRecords(
        {{std::make_shared<Learn::EvaluationResult>(
              std::make_shared<Selector::SelectionMetrics>(2.0), 10),
          root2}}));
    ASSERT_EQ(selector->getBestRoot().first, root2)
        << "Best root not updated properly.";
    ASSERT_EQ(selector->getBestRoot().second->getSelectionMetrics()->getScore(),
              2.0)
        << "Best root not updated properly.";

    // Update with a fake worse result for another root of the graph
    const TPG::TPGVertex* root3 =
        *(selector->getGraph()->getRootVertices().begin() + 2);
    ASSERT_NO_THROW(selector->updateEvaluationRecords(
        {{std::make_shared<Learn::EvaluationResult>(
              std::make_shared<Selector::SelectionMetrics>(1.5), 10),
          root3}}));
    ASSERT_EQ(selector->getBestRoot().first, root2)
        << "Best root not updated properly.";
    ASSERT_EQ(selector->getBestRoot().second->getSelectionMetrics()->getScore(),
              2.0)
        << "Best root not updated properly.";

    // Update with a root not from the graph
    TPG::TPGTeam fakeRoot;
    ASSERT_NO_THROW(selector->updateEvaluationRecords(
        {{std::make_shared<Learn::EvaluationResult>(
              std::make_shared<Selector::SelectionMetrics>(3.0), 10),
          &fakeRoot}}));
    ASSERT_EQ(selector->getBestRoot().first, &fakeRoot)
        << "Best root not updated properly.";
    ASSERT_EQ(selector->getBestRoot().second->getSelectionMetrics()->getScore(),
              3.0)
        << "Best root not updated properly.";

    // Update with a worse EvaluationResult (but still updated because previous
    // Root is not in the TPGGraph
    auto sharedPtr = std::make_shared<Learn::EvaluationResult>(
        std::make_shared<Selector::SelectionMetrics>(1.5), 10);
    ASSERT_NO_THROW(selector->updateEvaluationRecords({{sharedPtr, root3}}));
    ASSERT_EQ(selector->getBestRoot().first, root3)
        << "Best root not updated properly.";
    ASSERT_EQ(selector->getBestRoot().second->getSelectionMetrics()->getScore(),
              1.5)
        << "Best root not updated properly.";

    // Update with the EvaluationResult already registered in the resultsPerRoot
    // map (for code coverage)
    ASSERT_NO_THROW(selector->updateEvaluationRecords({{sharedPtr, root3}}));
}

TEST_F(SelectorTest, forgetPreviousResults)
{
    params.archiveSize = 50;
    params.archivingProbability = 0.5;
    params.maxNbActionsPerEval = 11;
    params.nbIterationsPerPolicyEvaluation = 10;
    params.mutation.tpg.maxInitOutgoingEdges = 2;
    params.selection.truncation.ratioDeletedRoots = 0.50;
    params.mutation.tpg.nbRoots = 10;
    params.nbRegisters = 4;

    Learn::LearningAgent la(le, set, params);
    la.init();
    std::shared_ptr<Selector::Selector> selector = la.getSelector();

    // Update with a fake result for a root of the graph
    auto rootVertices = selector->getGraph()->getRootVertices();
    const TPG::TPGVertex* root = *rootVertices.begin();
    ASSERT_NO_THROW(selector->updateEvaluationRecords(
        {{std::make_shared<Learn::EvaluationResult>(
              std::make_shared<Selector::SelectionMetrics>(1.0), 10),
          root}}));
    ASSERT_EQ(selector->getBestRoot().second->getSelectionMetrics()->getScore(),
              1.0)
        << "Best root not updated properly.";
    ASSERT_NO_THROW(*selector->getBestRoot().second += Learn::EvaluationResult(
                        std::make_shared<Selector::SelectionMetrics>(2.0), 10));
    ASSERT_EQ(selector->getBestRoot().second->getSelectionMetrics()->getScore(),
              1.5)
        << "Best root not updated properly.";

    // Looks for the eval record the Learning Agent should keep
    std::shared_ptr<Learn::EvaluationResult> previousEval;
    la.isRootEvalSkipped(*selector->getBestRoot().first, previousEval);

    ASSERT_NE(nullptr, previousEval)
        << "Learning agent should remember the last score of the root.";

    // Forgets the eval record
    ASSERT_NO_THROW(selector->forgetPreviousResults())
        << "forgetPreviousResults throws exception.";

    // Looks for the eval record the Learning Agent should keep
    la.isRootEvalSkipped(*selector->getBestRoot().first, previousEval);

    ASSERT_EQ(nullptr, previousEval)
        << "Learning agent should have forgotten the last score of the root";

    ASSERT_EQ(nullptr, selector->getBestRoot().first)
        << "Learning agent should have forgotten the best root";

    ASSERT_EQ(nullptr, selector->getBestRoot().second)
        << "Learning agent should have forgotten the last score of the root";

    ASSERT_NO_THROW(la.trainOneGeneration(0))
        << "trainOneGeneration doesn't work after a forgetPreviousResults";
}

TEST_F(SelectorTest, DoSelection)
{
    params.archiveSize = 50;
    params.archivingProbability = 0.5;
    params.maxNbActionsPerEval = 11;
    params.nbIterationsPerPolicyEvaluation = 3;
    params.mutation.tpg.maxInitOutgoingEdges = 2;
    params.selection.truncation.ratioDeletedRoots = 0.50;
    params.mutation.tpg.nbRoots =
        le.getNbActions() - 1; // Param used in decimation
    params.nbRegisters = 4;

    Learn::LearningAgent la(le, set, params);
    la.init();

    std::shared_ptr<Selector::Selector> selector = la.getSelector();

    // Remove the first team to make the first action a root
    TPG::TPGGraph& graph = *selector->getGraph();
    auto roots = graph.getRootVertices();
    graph.removeVertex(*roots.at(0));

    // Check that the action is now a root
    roots = graph.getRootVertices();
    auto* root = roots.at(0);
    ASSERT_EQ(typeid(*root), typeid(TPG::TPGAction))
        << "An action should have become a root of the TPGGraph.";

    // Create and fill results for each "root" artificially
    std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                  const TPG::TPGVertex*>
        results;
    double result = 0.0;
    for (const TPG::TPGVertex* root : roots) {
        results.emplace(
            new Learn::EvaluationResult(
                std::make_shared<Selector::SelectionMetrics>(result++), 5),
            root);
    }

    // Do the decimation
    ASSERT_NO_THROW(selector->doSelection(results, la.getRNG()))
        << "Decimating worst roots failed.";

    // Check the number of remaining roots.
    // Initial number of vertex - 2 removed vertices - deleted roots.
    ASSERT_EQ(graph.getNbVertices(),
              (le.getNbActions() * 2) - 2 -
                  params.selection.truncation.ratioDeletedRoots *
                      ((le.getNbActions() - 1)));
}

TEST_F(SelectorTest, DoSelectionActionsQuota)
{
    // We force the ratio to quickly reach quotas
    params.mutation.tpg.nbRoots = 20;
    params.selection.truncation.ratioDeletedRoots = 0.5;
    params.mutation.tpg.ratioTeamsOverActions = 0.6;
    params.mutation.tpg.useActionProgram =
        true; // To make action vertices removed too.
    Learn::LearningAgent cla(cle, set, params);
    cla.init();

    std::shared_ptr<Selector::Selector> selector = cla.getSelector();

    std::shared_ptr<TPG::TPGGraph> graph = selector->getGraph();

    // Set teams at a lower score than actions
    std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                  const TPG::TPGVertex*>
        results;
    double scoreActions = 10.0;
    double scoreTeams = 0.0;
    for (auto* root : graph->getRootVertices()) {
        if (dynamic_cast<const TPG::TPGAction*>(root) != nullptr) {
            results.emplace(std::make_shared<Learn::EvaluationResult>(
                                std::make_shared<Selector::SelectionMetrics>(
                                    scoreActions++),
                                1),
                            root);
        }
        else {
            results.emplace(
                std::make_shared<Learn::EvaluationResult>(
                    std::make_shared<Selector::SelectionMetrics>(scoreTeams++),
                    1),
                root);
        }
    }

    ASSERT_NO_THROW(selector->launchSelection(results, cla.getRNG()));

    size_t nbTeams = 0;
    size_t nbActions = 0;
    for (auto root : graph->getRootVertices()) {
        if (dynamic_cast<const TPG::TPGTeam*>(root) != nullptr) {
            nbTeams++;
        }
        else if (dynamic_cast<const TPG::TPGAction*>(root) != nullptr) {
            nbActions++;
        }
    }
    ASSERT_EQ(nbTeams, 6)
        << "After decimation, the number of teams should be 6.";
    ASSERT_EQ(nbActions, 4)
        << "After decimation, the number of actions should be 4.";
}

TEST_F(SelectorTest, DecimateWithTournamentSelection)
{
    params.selection._selectionMode = "tournament";
    params.mutation.tpg.nbRoots = 30;
    params.selection.tournament.ratioSavedRoots = 0.3;
    params.selection.tournament.sizeTournament = 4;
    Learn::LearningAgent tournamentLA(le, set, params);
    tournamentLA.init();

    std::shared_ptr<Selector::Selector> tournamentSelector =
        tournamentLA.getSelector();

    // Create a set of roots with different scores
    auto roots = tournamentSelector->getGraph()->getRootVertices();
    std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                  const TPG::TPGVertex*>
        results;
    double score = 0.0;
    for (auto* root : roots) {
        results.emplace(
            std::make_shared<Learn::EvaluationResult>(
                std::make_shared<Selector::SelectionMetrics>(score++), 1),
            root);
    }

    // Call doSelection, which should use tournament selection
    ASSERT_NO_THROW(
        tournamentSelector->doSelection(results, tournamentLA.getRNG()));

    // Check that the number of roots is reduced
    // The number of roots was 30. With a ratio of 0.7, only 9 should remain as
    // there were. 21 roots should have been played in the tournament. With a
    // size tournament of 4, we should do 6 selections, 5 with 4 roots and one
    // with 1 root. Thus, 6 roots should survived the tournamenent, leading to
    // 15 roots remaining.
    ASSERT_EQ(tournamentSelector->getGraph()->getNbRootVertices(), 15)
        << "After decimation with tournament selection, the number of roots "
           "should be reduced to 15.";

    ASSERT_TRUE(dynamic_cast<Selector::TournamentSelector*>(
                    tournamentSelector.get()) != nullptr)
        << "Selector for tournament selection should be a TournamentSelector";

    ASSERT_EQ(
        dynamic_cast<Selector::TournamentSelector*>(tournamentSelector.get())
            ->getVerticesToDelete()
            .size(),
        6)
        << "After decimation with tournament selection, the number of roots "
           "registered for deletion should be 6.";
}

TEST_F(SelectorTest, UpdateContext)
{
    params.mutation.tpg.useActionProgram = true;
    params.mutation.tpg.nbRoots = 2;
    params.mutation.tpg.ratioTeamsOverActions = 0.5;
    params.mutation.tpg.teamAccessAllActions = false;

    std::shared_ptr<TPG::TPGGraph> graph = std::make_shared<TPG::TPGGraph>(*e);

    // Create dummy teams and actions
    auto* team1 = &graph->addNewTeam();
    auto* team2 = &graph->addNewTeam();
    auto* action1 = &graph->addNewAction(0);
    auto* action2 = &graph->addNewAction(1);

    // Add one edge so that action1 is not a root (reachable by a team)
    graph->addNewEdge(*team1, *action1, progPointer);

    // Instantiate selector
    Selector::Selector selector(graph, params);

    // Call the function under test
    const Selector::SelectionContext& context = selector.updateContext();

    // TeamsClonable: should contain root teams
    ASSERT_FALSE(context.teamsClonable.empty())
        << "teamsClonable should not be empty because at least one team is a "
           "root.";
    ASSERT_TRUE(
        std::find(context.teamsClonable.begin(), context.teamsClonable.end(),
                  team1) != context.teamsClonable.end() ||
        std::find(context.teamsClonable.begin(), context.teamsClonable.end(),
                  team2) != context.teamsClonable.end())
        << "At least one of team1 or team2 should be present in teamsClonable "
           "since both were created as root teams.";

    // ActionsClonable: root actions only (action2 is root, action1 is not)
    ASSERT_EQ(context.actionsClonable.size(), 1)
        << "Only action2 should be clonable, since action1 has an incoming "
           "edge.";
    ASSERT_EQ(context.actionsClonable.front(), action2)
        << "The only root action expected in actionsClonable is action2.";

    // Pre-existing teams: both teams
    ASSERT_EQ(context.preExistingTeams.size(), 2)
        << "Both team1 and team2 should be listed as preExistingTeams.";
    ASSERT_NE(std::find(context.preExistingTeams.begin(),
                        context.preExistingTeams.end(), team1),
              context.preExistingTeams.end())
        << "team1 should appear in preExistingTeams.";
    ASSERT_NE(std::find(context.preExistingTeams.begin(),
                        context.preExistingTeams.end(), team2),
              context.preExistingTeams.end())
        << "team2 should appear in preExistingTeams.";

    // Pre-existing actions: action2 is root, action1 has an incoming edge so
    // excluded
    ASSERT_EQ(context.preExistingActions.size(), 1)
        << "Only action2 should be listed as a preExistingAction, since "
           "action1 has an incoming edge.";
    ASSERT_EQ(context.preExistingActions.front(), action2)
        << "The only action expected in preExistingActions is action2.";

    // Pre-existing edges: we only created one edge (team1 → action1)
    ASSERT_EQ(context.preExistingEdges.size(), 1)
        << "There should be exactly one preExistingEdge (team1 → action1).";
    ASSERT_EQ(context.preExistingEdges.front()->getDestination(), action1)
        << "The destination of the single preExistingEdge should be action1.";

    // NbTeamsToCreate & NbActionsToCreate
    // With nbRoots=2 and ratioTeamsOverActions=0.5:
    // ExpectedTeams = 1, nbRootTeams = 2 → nbTeamsToCreate = (1 - 2) = 0 (since
    // it's unsigned) ExpectedActions = 1, nbRootActions = 1 → nbActionsToCreate
    // = 0
    ASSERT_EQ(context.nbTeamsToCreate,
              (uint64_t)(1 - context.teamsClonable.size()))
        << "nbTeamsToCreate should reflect the deficit between expected root "
           "teams and actual root teams.";
    ASSERT_EQ(context.nbActionsToCreate, 0)
        << "nbActionsToCreate should be zero since the expected number of root "
           "actions equals the actual number.";
}

TEST_F(SelectorTest, UpdateContextTournament)
{
    params.mutation.tpg.useActionProgram = true;

    // Create dummy teams and actions, some marked as to be deleted
    std::shared_ptr<TPG::TPGGraph> graph = std::make_shared<TPG::TPGGraph>(*e);
    auto* team1 = &graph->addNewTeam();
    auto* team2 = &graph->addNewTeam();
    auto* action1 = &graph->addNewAction(0);
    auto* action2 = &graph->addNewAction(1);

    Selector::TournamentSelector selector(graph, params);

    // Mark team1 and action1 as to be deleted, team2 and action2 as not
    selector.addToVerticesToDelete(team1);
    selector.addToVerticesToDelete(action1);

    // Call the function under test
    const Selector::SelectionContext& context = selector.updateContext();

    // Only elements marked as to be deleted should remain in clonable,
    // and only elements NOT marked as to be deleted should remain in
    // preExisting
    ASSERT_EQ(context.teamsClonable.size(), 1)
        << "Only one root team should be clonable in this setup.";
    ASSERT_EQ(context.teamsClonable[0], team1)
        << "The only clonable team expected is team1.";

    ASSERT_EQ(context.actionsClonable.size(), 1)
        << "Only one root action should be clonable in this setup.";
    ASSERT_EQ(context.actionsClonable[0], action1)
        << "The only clonable action expected is action1.";

    ASSERT_EQ(context.preExistingTeams.size(), 1)
        << "Exactly one team should be registered as preExisting (team2).";
    ASSERT_EQ(context.preExistingTeams[0], team2)
        << "The only preExisting team expected is team2.";

    ASSERT_EQ(context.preExistingActions.size(), 1)
        << "Exactly one action should be registered as preExisting (action2).";
    ASSERT_EQ(context.preExistingActions[0], action2)
        << "The only preExisting action expected is action2.";
}

TEST_F(SelectorTest, updateAfterPopulate)
{
    params.mutation.tpg.useActionProgram = true;

    // Create dummy teams and actions, some marked as to be deleted
    std::shared_ptr<TPG::TPGGraph> graph = std::make_shared<TPG::TPGGraph>(*e);
    auto* team1 = &graph->addNewTeam();
    auto* team2 = &graph->addNewTeam();
    auto* action1 = &graph->addNewAction(0);
    auto* action2 = &graph->addNewAction(1);

    Selector::TournamentSelector selector(graph, params);

    // Mark team1 and action1 as to be deleted, team2 and action2 as not
    selector.addToVerticesToDelete(team1);
    selector.addToVerticesToDelete(action1);

    ASSERT_EQ(selector.getVerticesToDelete().size(), 2)
        << "The selector should have 2 vertices in the verticesToDelete set.";
    ASSERT_EQ(graph->getNbRootVertices(), 4)
        << "The graph should have 4 roots.";

    selector.updateAfterPopulate();

    ASSERT_EQ(selector.getVerticesToDelete().size(), 0)
        << "After deleting useless parents, the selector should have 0 "
           "vertices in the verticesToDelete set.";
    ASSERT_EQ(graph->getNbRootVertices(), 2)
        << "After deleting useless parents, the graph should have 2 roots.";
}

TEST_F(SelectorTest, FactorySelector)
{
    std::shared_ptr<TPG::TPGGraph> graph = std::make_shared<TPG::TPGGraph>(*e);
    std::shared_ptr<Selector::Selector> selector;

    ASSERT_NO_THROW(selector =
                        Selector::selectorFactory(graph, classifLe, params));
    ASSERT_TRUE(std::dynamic_pointer_cast<Selector::ClassificationSelector>(
                    selector) != nullptr);

    params.selection._selectionMode = "truncation";
    ASSERT_NO_THROW(selector = Selector::selectorFactory(graph, le, params));
    ASSERT_TRUE(std::dynamic_pointer_cast<Selector::TruncationSelector>(
                    selector) != nullptr);

    params.selection._selectionMode = "tournament";
    ASSERT_NO_THROW(selector = Selector::selectorFactory(graph, le, params));
    ASSERT_TRUE(std::dynamic_pointer_cast<Selector::TournamentSelector>(
                    selector) != nullptr);

    params.selection._selectionMode = "mapElites";
    ASSERT_NO_THROW(selector = Selector::selectorFactory(graph, le, params));
    ASSERT_TRUE(std::dynamic_pointer_cast<Selector::MapElitesSelector>(
                    selector) != nullptr);

    params.mutation.tpg.ratioTeamsOverActions = 0.5;
    ASSERT_THROW(selector = Selector::selectorFactory(graph, le, params),
                 std::runtime_error);

    params.selection._selectionMode = "fake";
    ASSERT_THROW(selector = Selector::selectorFactory(graph, le, params),
                 std::runtime_error);
}