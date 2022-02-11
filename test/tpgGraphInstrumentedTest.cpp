
#include <gtest/gtest.h>

#include "data/dataHandler.h"
#include "environment.h"
#include "instructions/set.h"
#include "program/program.h"
#include "instructions/addPrimitiveType.h"
#include "instructions/lambdaInstruction.h"

#include "tpg/instrumented/tpgActionInstrumented.h"
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

TEST_F(TPGInstrumentedTest,
       TPGTeamInstrumentedAndTPGActionInstrumentedConstructorsDestructors)
{
    TPG::TPGVertex* team;
    TPG::TPGVertex* action;

    ASSERT_NO_THROW(team = new TPG::TPGTeamInstrumented());
    ASSERT_NO_THROW(action = new TPG::TPGActionInstrumented(0));

    ASSERT_NO_THROW(delete team);
    ASSERT_NO_THROW(delete action);
}
