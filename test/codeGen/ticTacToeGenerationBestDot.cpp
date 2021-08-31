#ifdef CODE_GENERATION
#include <cstddef>
#include <file/parametersParser.h>
#include <gtest/gtest.h>

#ifdef _MSC_VER
// C++17 not available in gcc7 or clang7
#include <filesystem>
#endif

#include "code_gen/tpgGenerationEngine.h"
#include "environment.h"
#include "file/tpgGraphDotImporter.h"
#include "instructions/lambdaInstruction.h"
#include "instructions/set.h"
#include "tpg/tpgGraph.h"
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
    std::string cmdCompile;
    std::string cmdExec{"\"./bin/"};

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
            minus, "$0 = (double)($1) - (double)($2);")));
        set.add(*(new Instructions::LambdaInstruction<double, double>(
            add, "$0 = $1 + $2;")));
        set.add(*(new Instructions::LambdaInstruction<double, double>(
            max, "$0 = (($1) < ($2)) ? ($2) : ($1); ")));
        set.add(*(new Instructions::LambdaInstruction<double, double>(
            modulo, "$0 = (($2) != 0.0) ? fmod($1, $2) : DBL_MIN ;")));
        set.add(*(new Instructions::LambdaInstruction<double>(
            nulltest, "$0 = ($1) == -1.0 ? 10.0 : 0.0;")));
        set.add(*(new Instructions::LambdaInstruction<double>(
            circletest, "$0 = ($1) == 0.0 ? 10.0 : 0.0;")));
        set.add(*(new Instructions::LambdaInstruction<double>(
            crosstest, "$0 = ($1) == 1.0 ? 10.0 : 0.0;")));
        set.add(*(new Instructions::LambdaInstruction<double>(
            test15, "$0 = ($1) >= 15.0 ? 10.0 : 0.0;")));
        set.add(*(new Instructions::LambdaInstruction<double, double>(
            cond, "$0 = ($1) < ($2) ? -1*($1) : ($1);")));

        data.push_back(currentState);

        e = new Environment(set, data, 8);
        tpg = new TPG::TPGGraph(*e);

        cmdCompile = TESTS_DAT_PATH "codeGen/";
#ifdef _MSC_VER  
        // Set working directory to BIN_DIR_PATH where the "src" directory was
        // created.
        std::filesystem::current_path(BIN_DIR_PATH);
        cmdCompile += "compile.bat ";
        cmdExec = BIN_DIR_PATH "/bin/debug/";
#elif __GNUC__
        cmdCompile += "compile.sh ";
        cmdExec = "./bin/";
#endif
#ifdef DEBUG
        cmdCompile += "1 ";
#else
        cmdCompile += "0 ";
#endif // DEBUG
        cmdCompile += TESTS_DAT_PATH " TicTacToeBest_TPG";
        cmdExec += "TicTacToeBest_TPG 7 -1 -1 -1 -1 -1 -1 -1 -1 -1";
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

    tpgGen =
        new CodeGen::TPGGenerationEngine("TicTacToeBest_TPG", *tpg, "./src/");
    ASSERT_NO_THROW(tpgGen->generateTPGGraph())
        << "Fail to generate the C file to test TicTacToe";
    // call destructor to close generated files
    delete tpgGen;

    ASSERT_EQ(system(cmdCompile.c_str()), 0)
        << "Fail to compile generated files to test TicTacToe";

    ASSERT_EQ(system(cmdExec.c_str()), 0)
        << "Error inference of TicTacToe has changed";
}
#endif // CODE_GENERATION
