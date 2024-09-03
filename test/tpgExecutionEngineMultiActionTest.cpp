/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2019 - 2022) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2019 - 2022)
 * Nicolas Sourbier <nsourbie@insa-rennes.fr> (2020)
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

#include "data/dataHandler.h"
#include "data/primitiveTypeArray.h"
#include "instructions/addPrimitiveType.h"
#include "instructions/multByConstant.h"
#include "program/program.h"
#include "tpg/tpgAction.h"
#include "tpg/tpgEdge.h"
#include "tpg/tpgGraph.h"
#include "tpg/tpgTeam.h"
#include "tpg/tpgVertex.h"

#include "tpg/tpgExecutionEngine.h"

#ifndef PARAM_FLOAT_PRECISION
#define PARAM_FLOAT_PRECISION (float)(int16_t(1) / (float)(-INT16_MIN))
#endif

class TPGExecutionEngineTestMultiAction : public ::testing::Test
{
  protected:
    const size_t size1{24};
    const size_t size2{32};
    std::vector<std::reference_wrapper<const Data::DataHandler>> vect;
    Instructions::Set set;
    Environment* e = NULL;
    std::vector<std::shared_ptr<Program::Program>> progPointers;

    TPG::TPGGraph* tpg;
    std::vector<const TPG::TPGEdge*> edges;
    Archive a;

    /**
     * Populate the program instructions so that it returns the given value.
     *
     * \param[in] value a double value between 0 and 10.
     */
    void makeProgramReturn(Program::Program& prog, double value)
    {
        auto& line = prog.addNewLine();
        // do an multby constant with DHandler 0
        line.setInstructionIndex(1);
        line.setOperand(0, 2, 0);    // Dhandler 0 location 0
        line.setOperand(1, 1, 0);    // CHandler at location 0
        line.setDestinationIndex(0); // 0th register dest
        prog.getConstantHandler().setDataAt(typeid(Data::Constant), 0,
                                            {static_cast<int32_t>(value)});
    }

    virtual void SetUp()
    {
        // Setup environment
        vect.push_back(
            *(new Data::PrimitiveTypeArray<double>((unsigned int)size1)));
        vect.push_back(
            *(new Data::PrimitiveTypeArray<int>((unsigned int)size2)));

        // Put a 1 in the dataHandler to make it easy to have non-zero return in
        // Programs.
        ((Data::PrimitiveTypeArray<double>&)vect.at(0).get())
            .setDataAt(typeid(double), 0, 1.0);

        set.add(*(new Instructions::AddPrimitiveType<double>()));
        set.add(*(new Instructions::MultByConstant<double>()));
        e = new Environment(set, vect, 8, 1);
        tpg = new TPG::TPGGraph(*e);

        // Create 9 programs
        for (int i = 0; i < 10; i++) {
            progPointers.push_back(
                std::shared_ptr<Program::Program>(new Program::Program(*e)));
        }

        // Create a TPG
        // (T= Team, Ai-j= Action of class i and ID j)
        //
        // T0------>T1------>T2       T3
        // |       /| \      |        | \ 
        // v      / v  \     v        v  \ 
        // A0-0<-'  A0-1 `->A1-0     A1-1 `->A1-0
        //
        // With four action and four teams
        for (int i = 0; i < 4; i++) {
            tpg->addNewTeam();
        }
        for (int i = 0; i < 4; i++) {
            // Each action is linked to a team (and vice-versa)
            tpg->addNewAction(i % 2, i / 2);
            edges.push_back(&tpg->addNewEdge(*tpg->getVertices().at(i),
                                             *tpg->getVertices().back(),
                                             progPointers.at(i)));
        }

        // Add new Edges between teams
        edges.push_back(&tpg->addNewEdge(*tpg->getVertices().at(0),
                                         *tpg->getVertices().at(1),
                                         progPointers.at(4)));
        edges.push_back(&tpg->addNewEdge(*tpg->getVertices().at(1),
                                         *tpg->getVertices().at(2),
                                         progPointers.at(5)));

        // Add new outgoing edge to one team
        edges.push_back(&tpg->addNewEdge(*tpg->getVertices().at(1),
                                         *tpg->getVertices().at(4),
                                         progPointers.at(6)));
        edges.push_back(&tpg->addNewEdge(*tpg->getVertices().at(1),
                                         *tpg->getVertices().at(6),
                                         progPointers.at(7)));

        // Add new outgoing edge to another team
        edges.push_back(&tpg->addNewEdge(*tpg->getVertices().at(3),
                                         *tpg->getVertices().at(6),
                                         progPointers.at(8)));

        // Put a weight on edges
        makeProgramReturn(*progPointers.at(0), 5); // T0->A0-0
        makeProgramReturn(*progPointers.at(1), 6); // T1->A0-1
        makeProgramReturn(*progPointers.at(2), 3); // T2->A1-0
        makeProgramReturn(*progPointers.at(3), 3); // T3->A1-1
        makeProgramReturn(*progPointers.at(4), 8); // T0->T1
        makeProgramReturn(*progPointers.at(5), 9); // T1->T2
        makeProgramReturn(*progPointers.at(6), 5); // T1->A0-0
        makeProgramReturn(*progPointers.at(7), 3); // T1->A1-0
        makeProgramReturn(*progPointers.at(8), 3); // T3->A1-0

        // Check the characteristics
        ASSERT_EQ(tpg->getNbVertices(), 8);
        ASSERT_EQ(tpg->getEdges().size(), 9);
        ASSERT_EQ(tpg->getRootVertices().size(), 2);
    }

    virtual void TearDown()
    {
        delete tpg;
        delete e;
        delete (&(vect.at(0).get()));
        delete (&(vect.at(1).get()));
        delete (&set.getInstruction(0));
        delete (&set.getInstruction(1));
    }
};

TEST_F(TPGExecutionEngineTestMultiAction, EvaluateTeam)
{
    TPG::TPGExecutionEngine tpee(*e);

    std::vector<int64_t> initActions(2, -1);
    std::vector<const TPG::TPGVertex*> visitedVertices;

    std::vector<const TPG::TPGEdge*> result;
    ASSERT_NO_THROW(result =
                        tpee.executeTeam(tpg->getVertices().at(1),
                                         visitedVertices, &initActions, 2);)
        << "Evaluation of a valid TPGTeam with no exclusion failed.";
    // Expected result is edge between T1 -> T2 (with 9.0) and edge between T1
    // -> A0-0 (with 6.0)
    ASSERT_EQ(result[0], edges.at(5))
        << "Edge selected during team evaluation is incorrect.";
    ASSERT_EQ(result[1], edges.at(1))
        << "Edge selected during team evaluation is incorrect.";
}

TEST_F(TPGExecutionEngineTestMultiAction, EvaluateFromRootZero)
{
    TPG::TPGExecutionEngine tpee(*e);

    std::vector<uint64_t> initActions(2, 2);
    uint64_t nbEdgesActivable = 2;

    std::pair<std::vector<const TPG::TPGVertex*>, std::vector<uint64_t>> result;

    ASSERT_NO_THROW(result =
                        tpee.executeFromRoot(*tpg->getRootVertices().at(0),
                                             initActions, nbEdgesActivable))
        << "Execution of a TPGGraph from a valid root failed.";

    std::vector<const TPG::TPGVertex*> visitedVertexResult = result.first;
    std::vector<uint64_t> actionResult = result.second;

    // Check the traversed path of T0
    ASSERT_EQ(visitedVertexResult.size(), 6)
        << "Size of the traversed path during the execution of the TPGGraph is "
           "not as expected.";
    ASSERT_EQ(visitedVertexResult.at(0), tpg->getVertices().at(0))
        << "0th element (i.e. the root) of the traversed path during execution "
           "is incorrect.";
    ASSERT_EQ(visitedVertexResult.at(1), tpg->getVertices().at(1))
        << "1st element of the traversed path during execution is incorrect.";
    ASSERT_EQ(visitedVertexResult.at(2), tpg->getVertices().at(2))
        << "2nd element of the traversed path during execution is incorrect.";
    ASSERT_EQ(visitedVertexResult.at(3), tpg->getVertices().at(6))
        << "3rd element of the traversed path during execution is incorrect.";
    ASSERT_EQ(visitedVertexResult.at(4), tpg->getVertices().at(5))
        << "4th element of the traversed path during execution is incorrect.";
    ASSERT_EQ(visitedVertexResult.at(5), tpg->getVertices().at(4))
        << "5th element of the traversed path during execution is incorrect.";

    ASSERT_EQ(actionResult.size(), initActions.size())
        << "Action results should have the same size has the initActions "
           "vector.";
    ASSERT_EQ(actionResult[0], 1) << "Action of class 0 choosen is incorrect.";
    ASSERT_EQ(actionResult[1], 0) << "Action of class 1 choosen is incorrect.";
}

TEST_F(TPGExecutionEngineTestMultiAction, EvaluateFromRootOne)
{

    TPG::TPGExecutionEngine tpee(*e);

    std::vector<uint64_t> initActions(2, 2);
    uint64_t nbEdgesActivable = 2;

    std::pair<std::vector<const TPG::TPGVertex*>, std::vector<uint64_t>> result;

    ASSERT_NO_THROW(result =
                        tpee.executeFromRoot(*tpg->getRootVertices().at(1),
                                             initActions, nbEdgesActivable))
        << "Execution of a TPGGraph from a valid root failed.";

    std::vector<const TPG::TPGVertex*> visitedVertexResult = result.first;
    std::vector<uint64_t> actionResult = result.second;

    // Check the traversed path of T3
    ASSERT_EQ(visitedVertexResult.size(), 3)
        << "Size of the traversed path during the execution of the TPGGraph is "
           "not as expected.";
    ASSERT_EQ(visitedVertexResult.at(0), tpg->getVertices().at(3))
        << "0th element (i.e. the root) of the traversed path during execution "
           "is incorrect.";
    ASSERT_EQ(visitedVertexResult.at(1), tpg->getVertices().at(6))
        << "2nd element of the traversed path during execution is incorrect. "
           "Bids are equal but this edges was added after, so it has the "
           "priority.";
    ASSERT_EQ(visitedVertexResult.at(2), tpg->getVertices().at(7))
        << "3rd element of the traversed path during execution is incorrect.";

    ASSERT_EQ(actionResult.size(), initActions.size())
        << "Action results should have the same size has the initActions "
           "vector.";
    ASSERT_EQ(actionResult[0], 2)
        << "Action of class 0 choosen is incorrect. It should take the value "
           "in the initActions vector";
    ASSERT_EQ(actionResult[1], 0) << "Action of class 1 choosen is incorrect.";
}

TEST_F(TPGExecutionEngineTestMultiAction, EarlyExit)
{

    progPointers.push_back(
        std::shared_ptr<Program::Program>(new Program::Program(*e)));

    // Add new outgoing edge between T3 -> T0
    edges.push_back(&tpg->addNewEdge(*tpg->getVertices().at(3),
                                     *tpg->getVertices().at(0),
                                     progPointers.at(9)));

    makeProgramReturn(*progPointers.at(0), 10); // T0->A0-0
    makeProgramReturn(*progPointers.at(8), 0);  // T3->A1-0
    makeProgramReturn(*progPointers.at(9), 2);  // T3->T1

    TPG::TPGExecutionEngine tpee(*e);

    std::vector<uint64_t> initActions(2, 2);
    uint64_t nbEdgesActivable = 2;

    std::pair<std::vector<const TPG::TPGVertex*>, std::vector<uint64_t>> result;

    ASSERT_NO_THROW(result =
                        tpee.executeFromRoot(*tpg->getRootVertices().at(0),
                                             initActions, nbEdgesActivable))
        << "Execution of a TPGGraph from a valid root failed.";

    std::vector<const TPG::TPGVertex*> visitedVertexResult = result.first;
    std::vector<uint64_t> actionResult = result.second;

    // Check the traversed path of T3
    ASSERT_EQ(visitedVertexResult.size(), 4)
        << "Size of the traversed path during the execution of the TPGGraph is "
           "not as expected. If higher, probably because the early exit did "
           "not worked";
    ASSERT_EQ(visitedVertexResult.at(0), tpg->getVertices().at(3))
        << "0th element (i.e. the root) of the traversed path during execution "
           "is incorrect.";
    ASSERT_EQ(visitedVertexResult.at(1), tpg->getVertices().at(7))
        << "1st element of the traversed path during execution is incorrect.";
    ASSERT_EQ(visitedVertexResult.at(2), tpg->getVertices().at(0))
        << "2nd element of the traversed path during execution is incorrect.";
    ASSERT_EQ(visitedVertexResult.at(3), tpg->getVertices().at(4))
        << "3rd element of the traversed path during execution is incorrect.";

    ASSERT_EQ(actionResult.size(), initActions.size())
        << "Action results should have the same size has the initActions "
           "vector.";
    ASSERT_EQ(actionResult[0], 0)
        << "Action of class 0 choosen is incorrect. It should take the value "
           "in the initActions vector";
    ASSERT_EQ(actionResult[1], 1) << "Action of class 1 choosen is incorrect.";
}
