/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2019 - 2025) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2019 - 2022)
 * Nicolas Sourbier <nsourbie@insa-rennes.fr> (2019 - 2020)
 * Quentin Vacher <qvacher@insa-rennes.fr> (2025)
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

#include <fstream>
#include <gtest/gtest.h>
#include <iostream>

#include "data/dataHandler.h"
#include "data/primitiveTypeArray.h"
#include "instructions/addPrimitiveType.h"
#include "instructions/lambdaInstruction.h"
#include "learn/learningParameters.h"
#include "program/line.h"
#include "program/program.h"
#include "tpg/tpgAction.h"
#include "tpg/tpgEdge.h"
#include "tpg/tpgGraph.h"
#include "tpg/tpgTeam.h"
#include "tpg/tpgVertex.h"

#include "util/counterReset.h"

#include "file/tpgGraphDotExporter.h"
#include "file/tpgGraphDotImporter.h"

class ImporterTest : public ::testing::Test
{
  public:
    ImporterTest() : tpg(NULL){};

  protected:
    const size_t size1{24};
    std::vector<std::reference_wrapper<const Data::DataHandler>> vect;
    Instructions::Set set;
    Environment* e = NULL;
    Learn::LearningParameters params;
    std::vector<std::shared_ptr<Program::Program>> progPointers;
    File::TPGGraphDotExporter* dotExporter = NULL;

    TPG::TPGGraph* tpg;
    TPG::TPGGraph* tpg_copy;
    std::vector<const TPG::TPGEdge*> edges;

    std::fstream failpfile;
    std::fstream pfile;

    virtual void SetUp()
    {

        CounterReset::counterReset();
        // Setup environment
        vect.push_back(
            *(new Data::PrimitiveTypeArray<double>((unsigned int)size1)));

        // Put a 1 in the dataHandler to make it easy to have non-zero return in
        // Programs.
        ((Data::PrimitiveTypeArray<double>&)vect.at(0).get())
            .setDataAt(typeid(double), 0, 1.0);

        auto minus = [](double a, double b) -> double { return a - b; };

        set.add(*(new Instructions::AddPrimitiveType<double>()));
        set.add(*(new Instructions::LambdaInstruction<double, double>(minus)));

        params.nbRegisters = 8;
        params.nbProgramConstant = 5;
        e = new Environment(set, params, vect, 3);
        tpg = new TPG::TPGGraph(*e);
        tpg_copy = new TPG::TPGGraph(*e);

        // Create 12 programs
        for (int i = 0; i < 12; i++) {
            std::shared_ptr<Program::Program> p =
                std::make_shared<Program::Program>(*e, false);
            for (int j = 0; j < 5; j++) {
                p.get()->getConstantHandler().setDataAt(
                    typeid(Data::Constant), j, {(double)(j - 2) / 3});
            }
            progPointers.push_back(p);
        }

        // add instructions to at least one program. (here we add 3.)
        for (int i = 0; i < 3; i++) {
            Program::Line& l = progPointers.at(0).get()->addNewLine();
            l.setInstructionIndex(0);
            l.setDestinationIndex(1);
            l.setOperand(0, 0, 1);
        }

        // add constant instructions to at least one program.
        for (int i = 1; i < 2; i++) {
            Program::Line& l = progPointers.at(i).get()->addNewLine();
            l.setInstructionIndex(2);
            l.setDestinationIndex(1);
            l.setOperand(0, 0, 1);

            Program::Line& l2 = progPointers.at(i).get()->addNewLine();
            l2.setInstructionIndex(2);
            l2.setDestinationIndex(1);
            l2.setOperand(0, 0, 1);
        }

        // Create a TPG
        // (T= Team, A= Action)
        //
        //  .--.  .------.
        //  v   \ v      |
        // T0---->T1---->T2     T4
        // |     /| \    |      |
        // v    / v  \   v      v
        // A0<-'  A1  `->A2     A3   A4
        //
        // With four action and four teams
        // All Edges have a unique Program, except T1->A0 and T0->A0 which
        // share the same program: progPointers.at(0)
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

        // Add an additional Root Action
        tpg->addNewAction(4);

        // Add new Edges between teams
        edges.push_back(&tpg->addNewEdge(*tpg->getVertices().at(0),
                                         *tpg->getVertices().at(1),
                                         progPointers.at(4)));
        edges.push_back(&tpg->addNewEdge(*tpg->getVertices().at(1),
                                         *tpg->getVertices().at(2),
                                         progPointers.at(5)));
        edges.push_back(&tpg->addNewEdge(*tpg->getVertices().at(1),
                                         *tpg->getVertices().at(0),
                                         progPointers.at(8)));

        // Add a cyclic edge
        edges.push_back(&tpg->addNewEdge(*tpg->getVertices().at(2),
                                         *tpg->getVertices().at(1),
                                         progPointers.at(6)));

        // Add new outgoing edge to one team
        edges.push_back(&tpg->addNewEdge(*tpg->getVertices().at(1),
                                         *tpg->getVertices().at(4),
                                         progPointers.at(0)));
        edges.push_back(&tpg->addNewEdge(*tpg->getVertices().at(1),
                                         *tpg->getVertices().at(6),
                                         progPointers.at(7)));

        // Add new Action Edges to A0
        edges.push_back(&tpg->addNewActionEdge(*tpg->getVertices().at(4),
                                               progPointers.at(9), 2));
        edges.push_back(&tpg->addNewActionEdge(*tpg->getVertices().at(4),
                                               progPointers.at(10), 1));

        // Add new Action Edges to A2
        edges.push_back(&tpg->addNewActionEdge(*tpg->getVertices().at(6),
                                               progPointers.at(11), 0));
        if (dynamic_cast<const TPG::TPGAction*>(tpg->getVertices().at(4))) {
            // If the vertex is an action, we can order its edges
            tpg->orderActionEdges(
                dynamic_cast<const TPG::TPGAction*>(tpg->getVertices().at(4)));
        }

        // Check the characteristics
        ASSERT_EQ(tpg->getNbVertices(), 9);
        ASSERT_EQ(tpg->getEdges().size(), 13);
        ASSERT_EQ(tpg->getRootVertices().size(), 2);

        // Save the graph in a dot file.
        File::TPGGraphDotExporter dotexporter("exported_tpg.dot", *tpg);
        dotexporter.print();

        failpfile.open("fail_file.dot", std::fstream::out);
        // the header isrepresented by 3 lines
        failpfile << "a\na\na\n";
        for (int i = 0; i < 1025; i++)
            failpfile << 'a';
        failpfile.close();

        tpg->clear();

        // Create another TPG graph
        // (T= Team, A= Action)
        //
        //          T0 ---> A0
        //         /  \
		//        /    \
		//       |      |
        //  A1<--T1 <-- T2-->A2
        //       |
        //       v
        //      A3

        for (int i = 0; i < 3; i++) {
            tpg->addNewTeam();
        }
        tpg->addNewAction(0);
        edges.push_back(&tpg->addNewEdge(*tpg->getVertices().at(0),
                                         *tpg->getVertices().back(),
                                         progPointers.at(0)));
        tpg->addNewAction(1);
        edges.push_back(&tpg->addNewEdge(*tpg->getVertices().at(1),
                                         *tpg->getVertices().back(),
                                         progPointers.at(1)));
        tpg->addNewAction(2);
        edges.push_back(&tpg->addNewEdge(*tpg->getVertices().at(2),
                                         *tpg->getVertices().back(),
                                         progPointers.at(2)));
        tpg->addNewAction(3);
        edges.push_back(&tpg->addNewEdge(*tpg->getVertices().at(1),
                                         *tpg->getVertices().back(),
                                         progPointers.at(3)));

        // Add new Edges between teams
        edges.push_back(&tpg->addNewEdge(*tpg->getVertices().at(0),
                                         *tpg->getVertices().at(1),
                                         progPointers.at(4)));
        edges.push_back(&tpg->addNewEdge(*tpg->getVertices().at(0),
                                         *tpg->getVertices().at(2),
                                         progPointers.at(5)));
        edges.push_back(&tpg->addNewEdge(*tpg->getVertices().at(2),
                                         *tpg->getVertices().at(1),
                                         progPointers.at(8)));

        // Save the graph in a dot file.
        File::TPGGraphDotExporter exporter2("exported_tpg2.dot", *tpg);
        exporter2.print();
    }

    virtual void TearDown()
    {
        delete tpg;
        delete tpg_copy;
        delete e;
        delete (&(vect.at(0).get()));
        delete (&set.getInstruction(0));
        delete (&set.getInstruction(1));
    }
};

TEST_F(ImporterTest, Constructor)
{
    File::TPGGraphDotImporter* dotImporter;
    ASSERT_NO_THROW(dotImporter = new File::TPGGraphDotImporter(
                        "exported_tpg.dot", *e, *tpg_copy))
        << "The TPGGraphDotExporter could not be constructed with a valid file "
           "path.";

    ASSERT_NO_THROW(delete dotImporter;)
        << "TPGGraphDotExporter could not be deleted.";

    ASSERT_THROW(dotImporter = new File::TPGGraphDotImporter(
                     "XXX://INVALID_PATH", *e, *tpg_copy),
                 std::runtime_error)
        << "The TPGGraphDotExplorer construction should fail with an invalid "
           "path.";
}

TEST_F(ImporterTest, importGraph)
{
    File::TPGGraphDotImporter dotImporter("exported_tpg.dot", *e, *tpg_copy);

    // assert that we can import a tpg graph from a file
    ASSERT_NO_THROW(dotImporter.importGraph()) << "The Graph import failed.";

    // Check the imported graph characteristics
    ASSERT_EQ(tpg_copy->getNbVertices(), 9)
        << "the wrong number of vertices have been created.";
    ASSERT_EQ(tpg_copy->getEdges().size(), 13)
        << "the wrong number of edges have been created.";
    ASSERT_EQ(tpg_copy->getRootVertices().size(), 2)
        << "the wrong number of root teams have been created.";

    // Check action edges
    // Action A0
    const TPG::TPGVertex* action1 = tpg_copy->getVertices().at(4);
    ASSERT_NE(dynamic_cast<const TPG::TPGAction*>(action1), nullptr)
        << "The vertex at index 4 should be an action.";
    // If the vertex is an action, we can order its edges
    const TPG::TPGAction* action1_casted =
        dynamic_cast<const TPG::TPGAction*>(action1);
    ASSERT_EQ(action1_casted->getOutgoingEdges().size(), 2)
        << "The action A0 should have two action edges";

    auto it = action1_casted->getOutgoingEdges().begin();
    ASSERT_NE(dynamic_cast<const TPG::TPGActionEdge*>(*it), nullptr)
        << "The first outgoing edge of action A0 should be an action edge.";
    ASSERT_EQ(dynamic_cast<const TPG::TPGActionEdge*>(*it)->getActionClass(), 1)
        << "The action edge class should be 1.";

    ++it;
    ASSERT_NE(dynamic_cast<const TPG::TPGActionEdge*>(*it), nullptr)
        << "The second outgoing edge of action A0 should be an action edge.";
    ASSERT_EQ(dynamic_cast<const TPG::TPGActionEdge*>(*it)->getActionClass(), 2)
        << "The action edge class should be 2.";

    // Action A1
    const TPG::TPGVertex* action2 = tpg_copy->getVertices().at(5);
    ASSERT_NE(dynamic_cast<const TPG::TPGAction*>(action2), nullptr)
        << "The vertex at index 5 should be an action.";
    // If the vertex is an action, we can order its edges
    const TPG::TPGAction* action2_casted =
        dynamic_cast<const TPG::TPGAction*>(action2);
    ASSERT_EQ(action2_casted->getOutgoingEdges().size(), 0)
        << "The action A1 should not have any action edges";

    // Action A2
    const TPG::TPGVertex* action3 = tpg_copy->getVertices().at(6);
    ASSERT_NE(dynamic_cast<const TPG::TPGAction*>(action3), nullptr)
        << "The vertex at index 6 should be an action.";
    // If the vertex is an action, we can order its edges
    const TPG::TPGAction* action3_casted =
        dynamic_cast<const TPG::TPGAction*>(action3);
    ASSERT_EQ(action3_casted->getOutgoingEdges().size(), 1)
        << "The action A2 should have one action edge.";

    auto it2 = action3_casted->getOutgoingEdges().begin();
    ASSERT_NE(dynamic_cast<const TPG::TPGActionEdge*>(*it2), nullptr)
        << "The outgoing edge of action A1 should be an action edge.";

    // check that the imported program is the same as the one written in the
    // file.
    Program::Program& p = tpg_copy->getEdges().begin()->get()->getProgram();
    ASSERT_EQ(p.getNbLines(), 3)
        << "The number of lines of the copied program dismatch";
    // checking the first line
    ASSERT_EQ(p.getLine(0).getInstructionIndex(), 0)
        << "The Instruction Index changed";
    ASSERT_EQ(p.getLine(0).getDestinationIndex(), 1)
        << "The destination index of the first line changed";
    ASSERT_EQ(p.getLine(0).getOperand(0).first, 0)
        << "The first part of the operand changed";
    ASSERT_EQ(p.getLine(0).getOperand(0).second, 1)
        << "The second part of the operand changed";

    // checking the second line
    ASSERT_EQ(p.getLine(1).getInstructionIndex(), 0)
        << "The Instruction Index changed";
    ASSERT_EQ(p.getLine(1).getDestinationIndex(), 1)
        << "The destination index of the first line changed";
    ASSERT_EQ(p.getLine(1).getOperand(0).first, 0)
        << "The first part of the operand changed";
    ASSERT_EQ(p.getLine(1).getOperand(0).second, 1)
        << "The second part of the operand changed";

    // checking the third(and last) line
    ASSERT_EQ(p.getLine(2).getInstructionIndex(), 0)
        << "The Instruction Index changed";
    ASSERT_EQ(p.getLine(2).getDestinationIndex(), 1)
        << "The destination index of the first line changed";
    ASSERT_EQ(p.getLine(2).getOperand(0).first, 0)
        << "The first part of the operand changed";
    ASSERT_EQ(p.getLine(2).getOperand(0).second, 1)
        << "The second part of the operand changed";

    // checking the program's parameters
    ASSERT_EQ(static_cast<int32_t>(p.getConstantAt(0)), -2 / 3)
        << "The constant changed";
    ASSERT_EQ(static_cast<int32_t>(p.getConstantAt(1)), -1 / 3)
        << "The constant changed";
    ASSERT_EQ(static_cast<int32_t>(p.getConstantAt(2)), 0)
        << "The constant changed";
    ASSERT_EQ(static_cast<int32_t>(p.getConstantAt(3)), 1 / 3)
        << "The constant changed";
    ASSERT_EQ(static_cast<int32_t>(p.getConstantAt(4)), 2 / 3)
        << "The constant changed";
}

TEST_F(ImporterTest, importOldGraph)
{
    testing::internal::CaptureStderr();

    File::TPGGraphDotImporter dotImporter(
        TESTS_DAT_PATH "exported_tpg_old_ref.dot", *e, *tpg_copy);

    std::string output = testing::internal::GetCapturedStderr();

    // Check if "deprecating" appears in the output
    ASSERT_NE(output.find("Deprecating"), std::string::npos);
}

TEST_F(ImporterTest, importTrainedGraph)
{
    File::TPGGraphDotImporter dotImporter(
        TESTS_DAT_PATH "exported_trained_tpg_ref.dot", *e, *tpg_copy);

    // assert that we can import a tpg graph from a file
    ASSERT_NO_THROW(dotImporter.importGraph()) << "The Graph import failed.";

    // Check the imported graph characteristics
    ASSERT_EQ(tpg_copy->getNbVertices(), 5)
        << "the wrong number of vertices have been created.";
    ASSERT_EQ(tpg_copy->getEdges().size(), 5)
        << "the wrong number of edges have been created.";
    ASSERT_EQ(tpg_copy->getRootVertices().size(), 1)
        << "the wrong number of root teams have been created.";
}

TEST_F(ImporterTest, importTrainedGraphContinuous)
{
    params.nbProgramConstant = 10;
    e = new Environment(set, params, vect, 3);
    tpg_copy = new TPG::TPGGraph(*e);

    File::TPGGraphDotImporter dotImporter(
        TESTS_DAT_PATH "exported_trained_tpg_continuous_ref.dot", *e,
        *tpg_copy);

    // assert that we can import a tpg graph from a file
    ASSERT_NO_THROW(dotImporter.importGraph()) << "The Graph import failed.";

    // Check the imported graph characteristics
    ASSERT_EQ(tpg_copy->getNbVertices(), 5)
        << "the wrong number of vertices have been created.";
    ASSERT_EQ(tpg_copy->getEdges().size(), 27)
        << "the wrong number of edges have been created.";
    ASSERT_EQ(tpg_copy->getRootVertices().size(), 1)
        << "the wrong number of root teams have been created.";
}

TEST_F(ImporterTest, readLineFromFile)
{
    std::ofstream myfile;
    File::TPGGraphDotImporter* dotImporter;

    // Create a file where some lines are longer than the limit set in the
    // importer.
    myfile.open("wrongfile.dot");
    for (int i = 0; i < File::TPGGraphDotImporter::MAX_READ_SIZE + 1; i++)
        myfile << "aa";
    myfile.close();
    ASSERT_THROW(dotImporter = new File::TPGGraphDotImporter("wrongfile.dot",
                                                             *e, *tpg_copy),
                 std::ifstream::failure)
        << "Reading more than MAX_READ_SIZE(1024) should fail -- function "
           "ReadLineFromFile";
}

TEST_F(ImporterTest, setNewFilePath)
{
    File::TPGGraphDotImporter dotImporter("exported_tpg.dot", *e, *tpg_copy);

    // assert that we can import a tpg graph from a file
    ASSERT_NO_THROW(dotImporter.setNewFilePath("exported_tpg2.dot"))
        << "Changing the input file should be ok";

    // Check invalid filepath
    ASSERT_THROW(dotImporter.setNewFilePath("XXX://INVALID_PATH"),
                 std::runtime_error)
        << "Changing the input file with an invalid path should not work.";
}