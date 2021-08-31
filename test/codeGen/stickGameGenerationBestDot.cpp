#ifdef CODE_GENERATION

#include <cfloat>
#include <file/parametersParser.h>
#include <gtest/gtest.h>
#include <iostream>

#include "../learn/stickGameAdversarial.h"
#include "code_gen/programGenerationEngine.h"
#include "code_gen/tpgGenerationEngine.h"
#include "environment.h"
#include "file/tpgGraphDotImporter.h"
#include "instructions/lambdaInstruction.h"
#include "instructions/set.h"
#include "tpg/tpgExecutionEngine.h"
#include "tpg/tpgGraph.h"
#include "tpg/tpgVertex.h"

class StickGameGenerationBestDotTest : public ::testing::Test
{
  protected:
    Instructions::Set set;
    Environment* e;
    StickGameAdversarial* le;
    std::vector<std::reference_wrapper<const Data::DataHandler>> data;
    TPG::TPGGraph* tpg;
    TPG::TPGExecutionEngine* tee;
    CodeGen::TPGGenerationEngine* tpgGen;
    File::TPGGraphDotImporter* dot = nullptr;
    std::string cmdCompile;
    std::string cmdExec;
    std::string dataIn;
    TPG::TPGVertex const* rootVertex;

    virtual void SetUp() override
    {

        auto minus = [](int a, int b) -> double {
            return (double)a - (double)b;
        };
        auto cast = [](int a) -> double { return (double)a; };
        auto add = [](double a, double b) -> double { return a + b; };
        auto max = [](double a, double b) -> double { return std::max(a, b); };
        auto nulltest = [](double a) -> double {
            return (a == 0.0) ? 10.0 : 0.0;
        };
        auto modulo = [](double a, double b) -> double {
            if (b != 0.0) {
                return fmod(a, b);
            }
            else {
                return DBL_MIN;
            }
        };

        set.add(*(new Instructions::LambdaInstruction<double, double>(
            modulo, "$0 = (($2) != 0.0) ? fmod($1, $2) : DBL_MIN ;")));
        set.add(*(new Instructions::LambdaInstruction<int, int>(
            minus, "$0 = (double)($1) - (double)($2);")));
        set.add(*(new Instructions::LambdaInstruction<double, double>(
            add, "$0 = $1 + $2;")));
        set.add(*(new Instructions::LambdaInstruction<int>(
            cast, "$0 = (double)($1);")));
        set.add(*(new Instructions::LambdaInstruction<double, double>(
            max, "$0 = (($1) < ($2)) ? ($2) : ($1); ")));
        set.add(*(new Instructions::LambdaInstruction<double>(
            nulltest, "$0 = ($1 == 0.0) ? 10.0 : 0.0;")));

        le = new StickGameAdversarial();
        data = {le->getDataSources().at(0), le->getDataSources().at(1)};
        e = new Environment(set, le->getDataSources(), 8);
        tpg = new TPG::TPGGraph(*e);
        tee = new TPG::TPGExecutionEngine(*e);
        dot = new File::TPGGraphDotImporter(
            TESTS_DAT_PATH "StickGame_out_best.dot", *e, *tpg);
        dot->importGraph();
        rootVertex = tpg->getRootVertices().back();

        cmdCompile = TESTS_DAT_PATH "codeGen/";
#ifdef _MSC_VER
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
        cmdCompile += TESTS_DAT_PATH " StickGameBest_TPG";
        cmdExec += "StickGameBest_TPG";
    }

    virtual void TearDown() override
    {

        delete e;
        delete tpg;
        delete le;
        delete tee;

        delete (&set.getInstruction(0));
        delete (&set.getInstruction(1));
        delete (&set.getInstruction(2));
        delete (&set.getInstruction(3));
        delete (&set.getInstruction(4));
        delete (&set.getInstruction(5));

        delete dot;
    }
};

TEST_F(StickGameGenerationBestDotTest, BestTPG)
{
    int inferenceCodeGen, inferenceGegelati, status;
    tpgGen =
        new CodeGen::TPGGenerationEngine("StickGameBest_TPG", *tpg, "./src/");
    ASSERT_NO_THROW(tpgGen->generateTPGGraph())
        << "Fail to generate the C file to test StickGame";
    // call destructor to close generated files
    delete tpgGen;

    ASSERT_EQ(system(cmdCompile.c_str()), 0)
        << "Fail to compile generated files to test stick game";

    while (!le->isTerminal()) {
        dataIn = "";
        for (int i = 0; i < 3; ++i) {
            dataIn += std::to_string((*(le->getDataSources()
                                            .at(0)
                                            .get()
                                            .getDataAt(typeid(int), i)
                                            .getSharedPointer<int>()
                                            .get()))) +
                      " ";
        }
        dataIn += std::to_string((*(le->getDataSources()
                                        .at(1)
                                        .get()
                                        .getDataAt(typeid(int), 0)
                                        .getSharedPointer<int>()
                                        .get())));

#ifdef _MSC_VER
        std::string cmd{cmdExec + ".exe " + dataIn};
        inferenceCodeGen = system(cmd.c_str());
#elif __GNUC__
        std::string cmd{cmdExec + " " + dataIn};
        status = system(cmd.c_str());
        inferenceCodeGen = WEXITSTATUS(status);
#endif
        inferenceGegelati =
            (int)(((const TPG::TPGAction*)tee->executeFromRoot(*rootVertex)
                       .back())
                      ->getActionID());
        ASSERT_EQ(inferenceCodeGen, inferenceGegelati)
            << "Error inference of Stick Game has changed";
        le->doAction(inferenceGegelati);
    }
}
#endif // CODE_GENERATION