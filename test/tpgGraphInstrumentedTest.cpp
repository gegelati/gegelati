
#include <gtest/gtest.h>

#include "instructions/addPrimitiveType.h"
#include "instructions/lambdaInstruction.h"

#include "tpg/instrumented/tpgGraphInstrumented.h"
#include "tpg/instrumented/tpgTeamInstrumented.h"

class TPGInstrumentedTest : public ::testing::Test
{
  protected:
    const size_t size1{24};
    const size_t size2{32};
    std::vector<std::reference_wrapper<const Data::DataHandler>> vect;
    Instructions::Set set;
    Environment* e = NULL;
    std::shared_ptr<Program::Program> progPointer;

    virtual void SetUp()
    {
        vect.push_back(
            *(new Data::PrimitiveTypeArray<double>((unsigned int)size1)));
        vect.push_back(
            *(new Data::PrimitiveTypeArray<float>((unsigned int)size2)));

        set.add(*(new Instructions::AddPrimitiveType<float>()));
        auto minus = [](double a, double b) -> double { return a - b; };
        set.add(*(new Instructions::LambdaInstruction<double, double>(minus)));

        e = new Environment(set, vect, 8, 5);
        progPointer =
            std::shared_ptr<Program::Program>(new Program::Program(*e));
    }

    virtual void TearDown()
    {
        delete e;
        delete (&(vect.at(0).get()));
        delete (&(vect.at(1).get()));
        delete (&set.getInstruction(0));
        delete (&set.getInstruction(1));
    }
};

TEST_F(TPGInstrumentedTest, TPGTeamInstrumentedAndTPGActionInstrumentedConstructorsDestructors)
{
    TPG::TPGVertex* team;
    TPG::TPGVertex* action;

    
    ASSERT_NO_THROW(team = new TPG::TPGTeamInstrumented());
    ASSERT_NO_THROW(action = new TPG::TPGAction(0));

    ASSERT_NO_THROW(delete team);
    ASSERT_NO_THROW(delete action);
}

TEST_F(TPGInstrumentedTest, TPGGraphConstructorDestructor)
{
    TPG::TPGGraphInstrumented* tpgi;

    ASSERT_NO_THROW(tpgi = new TPG::TPGGraphInstrumented(*e))
        << "Error while calling a TPGGraph constructor.";

    ASSERT_NE(tpgi, nullptr)
        << "TPGGraph construction succeded but returned a null pointer.";

    ASSERT_NO_THROW(delete tpgi) << "Destruction of a TPGGraph failed.";
}

TEST_F(TPGInstrumentedTest, TPGMoveOperator)
{
    TPG::TPGGraphInstrumented source(*e);
    TPG::TPGGraphInstrumented* destination =
        new TPG::TPGGraphInstrumented(*e); // creates an empty tpg graph

    const TPG::TPGTeam& vertex0 = source.addNewTeam();
    const TPG::TPGAction& vertex1 = source.addNewAction(4);
    const TPG::TPGTeam& vertex2 = source.addNewTeam();
    const TPG::TPGEdge& edge = source.addNewEdge(vertex0, vertex1, progPointer);
    const TPG::TPGEdge& edge2 =
        source.addNewEdge(vertex2, vertex1, progPointer);
    const TPG::TPGEdge& edge3 =
        source.addNewEdge(vertex0, vertex2, progPointer);

    /*
     *	 T2
     *	  ^	\
     *    |	  A4
     *	 T0	/
     */

    ASSERT_NO_THROW(*destination = std::move(source))
        << "The move operator is never supposed to fail";
    ASSERT_EQ(destination->getNbVertices(), 3)
        << "All verticies were not moved";
    ASSERT_EQ(source.getNbVertices(), 0)
        << "Some verticies are still present in the source graph";
    ASSERT_EQ(destination->getEdges().size(), 3) << "All edges were not moved";
    ASSERT_EQ(source.getEdges().size(), 0)
        << "Some edges are still present in the source graph";
    delete (destination);
}