/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2021 - 2023) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2021 - 2023)
 * Mickaël Dardaillon <mdardail@insa-rennes.fr> (2022)
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

#ifdef CODE_GENERATION

#include <cfloat>
#include <file/parametersParser.h>
#include <gtest/gtest.h>
#include <iostream>

#if defined(_MSC_VER) || defined(__MINGW32__)
// C++17 not available in gcc7 or clang7
#include <filesystem>
#endif

#include "../learn/stickGameAdversarial.h"
#include "codeGen/programGenerationEngine.h"
#include "codeGen/tpgGenerationEngineFactory.h"
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
    Environment* e = nullptr;
    StickGameAdversarial* le = nullptr;
    std::vector<std::reference_wrapper<const Data::DataHandler>> data;
    TPG::TPGGraph* tpg = nullptr;
    TPG::TPGExecutionEngine* tee = nullptr;
    std::unique_ptr<CodeGen::TPGGenerationEngine> tpgGen = nullptr;
    File::TPGGraphDotImporter* dot = nullptr;
    std::string cmdCompile;
    std::string cmdExec;
    std::string dataIn;

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

        cmdCompile = TESTS_DAT_PATH "codeGen/";
#if defined(_MSC_VER) || defined(__MINGW32__)
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
    int inferenceCodeGen, inferenceGegelati;
    CodeGen::TPGGenerationEngineFactory factory;
    tpgGen = factory.create("StickGameBest_TPG", *tpg, "./src/");
    ASSERT_NO_THROW(tpgGen->generateTPGGraph())
        << "Fail to generate the C file to test StickGame";
    // call destructor to close generated files
    tpgGen.reset();

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

#if defined(_MSC_VER) || defined(__MINGW32__)
        std::string cmd{cmdExec + ".exe " + dataIn};
        inferenceCodeGen = system(cmd.c_str());
#elif __GNUC__
        std::string cmd{cmdExec + " " + dataIn};
        int status = system(cmd.c_str());
        inferenceCodeGen = WEXITSTATUS(status);
#endif
        inferenceGegelati =
            (int)(tee->executeFromRoot(*tpg->getRootVertices().back(), {0}, 1).second[0]);
        ASSERT_EQ(inferenceCodeGen, inferenceGegelati)
            << "Error inference of Stick Game has changed";
        le->doActions(std::vector<size_t>(1, inferenceGegelati));
    }
}
#endif // CODE_GENERATION
