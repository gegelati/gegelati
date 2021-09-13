/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2019 - 2020) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2019 - 2020)
 * Nicolas Sourbier <nsourbie@insa-rennes.fr> (2019 - 2020)
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

#include <fstream>

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

#include "file/tpgGraphDotExporter.h"

#include "goldenReferenceComparison.h"

class ExporterTest : public ::testing::Test
{
  public:
    ExporterTest() : tpg(NULL){};

  protected:
    const size_t size1{24};
    std::vector<std::reference_wrapper<const Data::DataHandler>> vect;
    Instructions::Set set;
    Environment* e = NULL;
    std::vector<std::shared_ptr<Program::Program>> progPointers;

    TPG::TPGGraph* tpg;
    std::vector<const TPG::TPGEdge*> edges;

    virtual void SetUp()
    {
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

        size_t constant_size = 5;

        e = new Environment(set, vect, 8, 5);
        tpg = new TPG::TPGGraph(*e);

        // Create 10 programs
        for (int i = 0; i < 8; i++) {
            std::shared_ptr<Program::Program> p =
                std::make_shared<Program::Program>(*e);
            for (int j = 0; j < constant_size; j++) {
                p.get()->getConstantHandler().setDataAt(typeid(Data::Constant),
                                                        j, {j - 2});
            }
            progPointers.push_back(p);
        }

        // add instructions to at least one program.
        for (int i = 0; i < 3; i++) {
            Program::Line& l = progPointers.at(0).get()->addNewLine();
            l.setInstructionIndex(0);
            l.setDestinationIndex(1);
            l.setOperand(0, 0, 1);
        }

        // Create a TPG
        // (T= Team, A= Action)
        //
        //        .------.
        //        v      |
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

        // Check the characteristics
        ASSERT_EQ(tpg->getNbVertices(), 9);
        ASSERT_EQ(tpg->getEdges().size(), 9);
        ASSERT_EQ(tpg->getRootVertices().size(), 3);
    }

    virtual void TearDown()
    {
        delete tpg;
        delete e;
        delete (&(vect.at(0).get()));
        delete (&set.getInstruction(0));
        delete (&set.getInstruction(1));
    }
};

TEST_F(ExporterTest, Constructor)
{
    File::TPGGraphDotExporter* dotExporter;
    ASSERT_NO_THROW(dotExporter =
                        new File::TPGGraphDotExporter("exported_tpg.dot", *tpg))
        << "The TPGGraphDotExporter could not be constructed with a valid file "
           "path.";

    ASSERT_NO_THROW(delete dotExporter;)
        << "TPGGraphDotExporter could not be deleted.";

    ASSERT_THROW(dotExporter =
                     new File::TPGGraphDotExporter("XXX://INVALID_PATH", *tpg),
                 std::runtime_error)
        << "The TPGGraphDotExplorer construction should fail with an invalid "
           "path.";
}

TEST_F(ExporterTest, print)
{
    File::TPGGraphDotExporter dotExporter("exported_tpg.dot", *tpg);

    ASSERT_NO_THROW(dotExporter.print())
        << "File export was executed without error.";
}

TEST_F(ExporterTest, FileContentVerification)
{
    // This Test checks the content of the exported file against a golden
    // reference.
    File::TPGGraphDotExporter dotExporter("exported_tpg.dot", *tpg);

    dotExporter.print();

    // Compare the two files
    ASSERT_TRUE(compare_files("exported_tpg.dot",
                              TESTS_DAT_PATH "exported_tpg_ref.dot"))
        << "Differences between reference file and exported "
           "file were detected.";
}
