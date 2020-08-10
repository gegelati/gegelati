/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2019 - 2020) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2019 - 2020)
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
#include "instructions/lambdaInstruction.h"
#include "program/program.h"
#include "tpg/tpgAction.h"
#include "tpg/tpgEdge.h"
#include "tpg/tpgGraph.h"
#include "tpg/tpgTeam.h"
#include "tpg/tpgVertex.h"

#include "tpg/tpgExecutionEngine.h"

//TODO REMOVE THAT. this is temporary.
#ifndef PARAM_FLOAT_PRECISION
#define PARAM_FLOAT_PRECISION 0.6
#endif
class TPGExecutionEngineTest : public ::testing::Test
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
     * \param[in] value a double valut between -1.0 and 1.0.
     */
    void makeProgramReturn(Program::Program& prog, int value)
    {
        auto& line = prog.addNewLine();
        // do an multby constant with DHandler 0
        line.setInstructionIndex(1);
        line.setOperand(0, 2, 0);    // Dhandler 0 location 0
        line.setOperand(1, 2, value);    // Dhandler 0 location value
        line.setDestinationIndex(0); // 0th register des
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
        ((Data::PrimitiveTypeArray<double>&)vect.at(0).get())
            .setDataAt(typeid(double), 1, 0.5);
        ((Data::PrimitiveTypeArray<double>&)vect.at(0).get())
            .setDataAt(typeid(double), 2, 0.5);
        ((Data::PrimitiveTypeArray<double>&)vect.at(0).get())
            .setDataAt(typeid(double), 3, 0.3);
        ((Data::PrimitiveTypeArray<double>&)vect.at(0).get())
            .setDataAt(typeid(double), 4, 0.0);
        ((Data::PrimitiveTypeArray<double>&)vect.at(0).get())
            .setDataAt(typeid(double), 5, 0.8);
        ((Data::PrimitiveTypeArray<double>&)vect.at(0).get())
            .setDataAt(typeid(double), 6, 0.9);
        ((Data::PrimitiveTypeArray<double>&)vect.at(0).get())
            .setDataAt(typeid(double), 7, 0.7);
        ((Data::PrimitiveTypeArray<double>&)vect.at(0).get())
            .setDataAt(typeid(double), 8, 0.6);
        ((Data::PrimitiveTypeArray<double>&)vect.at(0).get())
            .setDataAt(typeid(double), 9, 0.3);

        set.add(*(new Instructions::AddPrimitiveType<double>()));

        auto mult = [](double a, double b)->double {return a*b; };
        set.add(*(new Instructions::LambdaInstruction<double,double>(mult)));

        e = new Environment(set, vect, 8, 5);
        tpg = new TPG::TPGGraph(*e);

        // Create 10 programs
        for (int i = 0; i < 10; i++) {
            progPointers.push_back(
                std::shared_ptr<Program::Program>(new Program::Program(*e,5)));
        }

        // Create a TPG
        // (T= Team, A= Action)
        //
        //        .------.
        //        v      |
        // T0---->T1---->T2     T4
        // |     /| \    |      |
        // v    / v  \   v      v
        // A0<-'  A1  `->A2     A3
        //
        // With four action and four teams
        for (int i = 0; i < 4; i++) {
            tpg->addNewTeam();
        }
        for (int i = 0; i < 4; i++) {
            // Each action is linked to a team (and vice-versa)
            tpg->addNewAction(i);
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

        // Add a cyclic edge
        edges.push_back(&tpg->addNewEdge(*tpg->getVertices().at(2),
                                         *tpg->getVertices().at(1),
                                         progPointers.at(6)));

        // Add new outgoing edge to one team
        edges.push_back(&tpg->addNewEdge(*tpg->getVertices().at(1),
                                         *tpg->getVertices().at(4),
                                         progPointers.at(7)));
        edges.push_back(&tpg->addNewEdge(*tpg->getVertices().at(1),
                                         *tpg->getVertices().at(6),
                                         progPointers.at(8)));

        // Put a weight on edges
        makeProgramReturn(*progPointers.at(0), 1); // T0->A0
        makeProgramReturn(*progPointers.at(1), 2); // T1->A1
        makeProgramReturn(*progPointers.at(2), 3); // T2->A2
        makeProgramReturn(*progPointers.at(3), 4); // T3->A3
        makeProgramReturn(*progPointers.at(4), 5); // T0->T1
        makeProgramReturn(*progPointers.at(5), 6); // T1->T2
        makeProgramReturn(*progPointers.at(6), 7); // T2->T1
        makeProgramReturn(*progPointers.at(7), 8); // T1->A0
        makeProgramReturn(*progPointers.at(8), 9); // T1->A2

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

TEST_F(TPGExecutionEngineTest, ConstructorDestructor)
{
    TPG::TPGExecutionEngine* tpee;

    ASSERT_NO_THROW(tpee = new TPG::TPGExecutionEngine(*e))
        << "Construction of a TPGExecutionEngine failed.";

    ASSERT_NO_THROW(delete tpee) << "Deletion of a TPGExecutionEngine failed.";
}

TEST_F(TPGExecutionEngineTest, EvaluateEdge)
{
    TPG::TPGExecutionEngine tpee(*e);

    ASSERT_NEAR(tpee.evaluateEdge(*edges.at(0)), 0.5, PARAM_FLOAT_PRECISION)
        << "Evaluation of the program of an Edge failed.";
}

TEST_F(TPGExecutionEngineTest, ArchiveUsage)
{
    TPG::TPGExecutionEngine tpee(*e, &a);

    ASSERT_NEAR(tpee.evaluateEdge(*edges.at(0)), 0.5, PARAM_FLOAT_PRECISION)
        << "Evaluation of the program of an Edge failed when result is "
           "archived.";
    ASSERT_EQ(a.getNbRecordings(), 1)
        << "No recording was added to the archive.";
}

TEST_F(TPGExecutionEngineTest, EvaluateTeam)
{
    TPG::TPGExecutionEngine tpee(*e);

    const TPG::TPGEdge* result = NULL;
    ASSERT_NO_THROW(result = &tpee.evaluateTeam(
                        *(const TPG::TPGTeam*)(tpg->getVertices().at(1)), {});)
        << "Evaluation of a valid TPGTeam with no exclusion failed.";
    // Expected result is edge between T1 -> T2 (with 0.9)
    ASSERT_EQ(result, edges.at(5))
        << "Edge selected during team evaluation is incorrect.";

    // Exclude an edge
    ASSERT_NO_THROW(result = &tpee.evaluateTeam(
                        *(const TPG::TPGTeam*)(tpg->getVertices().at(1)),
                        {tpg->getVertices().at(2)});)
        << "Evaluation of a valid TPGTeam with one exclusion failed.";
    // Expected result is edge between T1 -> A0 (with 0.6)
    ASSERT_EQ(result, edges.at(7))
        << "Edge selected during team evaluation is incorrect.";

    // Exclude all edges
    ASSERT_THROW(result = &tpee.evaluateTeam(
                     *(const TPG::TPGTeam*)(tpg->getVertices().at(0)),
                     {tpg->getVertices().at(1), tpg->getVertices().at(4)}),
                 std::runtime_error)
        << "Evaluation of a TPGTeam with all edges excluded did not fail as "
           "expected.";
}

TEST_F(TPGExecutionEngineTest, EvaluateFromRoot)
{
    TPG::TPGExecutionEngine tpee(*e);

    std::vector<const TPG::TPGVertex*> result;

    ASSERT_NO_THROW(result =
                        tpee.executeFromRoot(*tpg->getRootVertices().at(0)))
        << "Execution of a TPGGraph from a valid root failed.";
    // Check the traversed path
    ASSERT_EQ(result.size(), 4)
        << "Size of the traversed path during the execution of the TPGGraph is "
           "not as expected.";
    ASSERT_EQ(result.at(0), tpg->getVertices().at(0))
        << "0th element (i.e. the root) of the traversed path during execution "
           "is incorrect.";
    ASSERT_EQ(result.at(1), tpg->getVertices().at(1))
        << "1st element of the traversed path during execution is incorrect.";
    ASSERT_EQ(result.at(2), tpg->getVertices().at(2))
        << "2nd element of the traversed path during execution is incorrect.";
    ASSERT_EQ(result.at(3), tpg->getVertices().at(6))
        << "2nd element of the traversed path during execution is incorrect.";
}
