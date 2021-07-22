#include <file/parametersParser.h>
#include <gtest/gtest.h>
#include <stddef.h>

#include "code_gen/TpgGenerationEngine.h"
#include "environment.h"
#include "file/tpgGraphDotImporter.h"
#include "instructions/lambdaInstruction.h"
#include "instructions/set.h"
#include "tpg/tpgAction.h"
#include "tpg/tpgGraph.h"
#include "tpg/tpgTeam.h"
#include "tpg/tpgVertex.h"

class TicTacToeGenerationBestDotTest : public ::testing::Test
{
  protected:
    const size_t s1{9};
    Instructions::Set set;
    Environment* e;
    std::vector<std::reference_wrapper<const Data::DataHandler>> data;
    Data::PrimitiveTypeArray<double> currentState{s1};
    TPG::TPGGraph* tpg;
    File::TPGGraphDotImporter* dot = nullptr;
    CodeGen::TPGGenerationEngine* tpgGen;
    std::string cmdCompile{"dir=" BIN_DIR_PATH " make -C " TESTS_DAT_PATH
                           "codeGen"};
    std::string cmdExec{BIN_DIR_PATH
                        "/bin/TicTacToeBest_TPG 7 -1 -1 -1 -1 -1 -1 -1 -1"};

    virtual void SetUp()
    {
        auto minus = [](double a, double b) -> double {
            return (double)a - (double)b;
        };
        auto add = [](double a, double b) -> double { return a + b; };
        auto max = [](double a, double b) -> double { return std::max(a, b); };
        auto modulo = [](double a, double b) -> double {
            return b != 0.0 ? fmod(a, b) : DBL_MIN;
        };
        auto nulltest = [](double a) -> double {
            return (a == -1.0) ? 10.0 : 0.0;
        };
        auto circletest = [](double a) -> double {
            return (a == 0.0) ? 10.0 : 0.0;
        };
        auto crosstest = [](double a) -> double {
            return (a == 1.0) ? 10.0 : 0.0;
        };
        auto test15 = [](double a) -> double {
            return (a >= 15.0) ? 10.0 : 0.0;
        };
        auto cond = [](double a, double b) -> double { return a < b ? -a : a; };

        set.add(*(new Instructions::LambdaInstruction<double, double>(
            "$0 = (double)($1) - (double)($2);", minus)));
        set.add(*(new Instructions::LambdaInstruction<double, double>(
            "$0 = $1 + $2;", add)));
        set.add(*(new Instructions::LambdaInstruction<double, double>(
            "$0 = (($1) < ($2)) ? ($2) : ($1); ", max)));
        set.add(*(new Instructions::LambdaInstruction<double, double>(
            "$0 = (($2) != 0.0) ? fmod($1, $2) : DBL_MIN ;", modulo)));
        set.add(*(new Instructions::LambdaInstruction<double>(
            "$0 = ($1) == -1.0 ? 10.0 : 0.0;", nulltest)));
        set.add(*(new Instructions::LambdaInstruction<double>(
            "$0 = ($1) == 0.0 ? 10.0 : 0.0;", circletest)));
        set.add(*(new Instructions::LambdaInstruction<double>(
            "$0 = ($1) == 1.0 ? 10.0 : 0.0;", crosstest)));
        set.add(*(new Instructions::LambdaInstruction<double>(
            "$0 = ($1) >= 15.0 ? 10.0 : 0.0;", test15)));
        set.add(*(new Instructions::LambdaInstruction<double, double>(
            "$0 = ($1) < ($2) ? -1*($1) : ($1);", cond)));

        data.push_back(currentState);

        e = new Environment(set, data, 8);
        tpg = new TPG::TPGGraph(*e);
    }

    virtual void TearDown()
    {
        delete e;
        delete tpg;
        delete (&set.getInstruction(0));
        delete (&set.getInstruction(1));
        delete (&set.getInstruction(2));
        delete (&set.getInstruction(3));
        delete (&set.getInstruction(4));
        delete (&set.getInstruction(5));
        delete (&set.getInstruction(6));
        delete (&set.getInstruction(7));
        delete (&set.getInstruction(8));

        delete dot;
    }
};

TEST_F(TicTacToeGenerationBestDotTest, BestTPG)
{
    dot = new File::TPGGraphDotImporter(TESTS_DAT_PATH "TicTacToe_out_best.dot",
                                        *e, *tpg);
    ASSERT_NO_THROW(dot->importGraph())
        << "Failed to Import the graph to test inference of TicTacToe";

    tpgGen = new CodeGen::TPGGenerationEngine("TicTacToeBest_TPG", *tpg,
                                              BIN_DIR_PATH "/src/");
    ASSERT_NO_THROW(tpgGen->generateTPGGraph())
        << "Fail to generate the C file to test TicTacToe";
    // call destructor to close generated files
    delete tpgGen;

    cmdCompile += " TicTacToeBest_TPG";

    ASSERT_EQ(system(cmdCompile.c_str()), 0)
        << "Fail to compile generated files to test TicTacToe";
    ASSERT_EQ(system(cmdExec.c_str()), 0)
        << "Error inference of TicTacToe has changed";
}
