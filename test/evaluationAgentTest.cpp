/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2019 - 2025) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2019 - 2022)
 * Nicolas Sourbier <nsourbie@insa-rennes.fr> (2020)
 * Pierre-Yves Le Rolland-Raumer <plerolla@insa-rennes.fr> (2020)
 * Quentin Vacher <qvacher@insa-rennes.fr> (2023 - 2025)
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

#include <algorithm>
#include <fstream>
#include <gtest/gtest.h>
#include <numeric>

#include "evaluation/evaluationAgent.h"
#include "instructions/set.h"
#include "instructions/lambdaInstruction.h"
#include "representations/lgpRepresentation.h"

#include "learn/stickGameWithOpponentDupDouble.h"
#include "selector/truncationSelector.h"

// Set all file in comment

class EvaluationAgentTest : public ::testing::Test
{
  protected:
    Instructions::Set set;
    Evolution::Representation* representation;

    std::unique_ptr<Learn::LearningParameters> params;

    Selector::Selector* selector;

    StickGameWithOpponentD le;

    virtual void SetUp()
    {
        auto add = [](double a, double b) -> double { return a + b; };
        auto minus = [](double a, double b) -> double { return a - b; };
        auto times = [](double a, double b) -> double { return a * b; };
        auto div = [](double a, double b) -> double { return a / b; };
        
        set.add(*(new Instructions::LambdaInstruction<double, double>(add)));
        set.add(*(new Instructions::LambdaInstruction<double, double>(minus)));
        set.add(*(new Instructions::LambdaInstruction<double, double>(times)));
        set.add(*(new Instructions::LambdaInstruction<double, double>(div)));
    
        representation = new Representations::LGPRepresentation(set, 8, 5, 10);
        

        selector = new Selector::TruncationSelector();

        params = std::make_unique<Learn::LearningParameters>();
    }

    virtual void TearDown()
    {
        delete (&set.getInstruction(0));
        delete (&set.getInstruction(1));
        delete (&set.getInstruction(2));
        delete (&set.getInstruction(3));
        delete representation;
        delete selector;
    }
};


TEST_F(EvaluationAgentTest, Constructor)
{
    Evaluation::EvaluationAgent* evalAgent;
    Evaluation::EvaluationAgent* evalAgent2;
    size_t seed = 0;

    ASSERT_NO_THROW(evalAgent = new Evaluation::EvaluationAgent(le, std::move(params), seed)) << "Constructor of evalAgent failed.";

    ASSERT_NO_THROW(evalAgent2 = new Evaluation::EvaluationAgent(le)) << "Constructor of evalAgent failed.";

    ASSERT_NO_THROW(delete evalAgent) << "Destructor of evalAgent failed.";
    ASSERT_NO_THROW(delete evalAgent2) << "Destructor of evalAgent failed.";
}

TEST_F(EvaluationAgentTest, evaluateIndividual)
{
    Evaluation::EvaluationAgent evalAgent(le, std::move(params));
    Learn::LearningMode mode = Learn::LearningMode::TRAINING;

    Evolution::Individual indiv;
    Evolution::Genotype& genotype = indiv.getMutableGenotype();
    Node::NodeGroup& group = genotype.addNodeGroup();
    
    representation->setInputDimensions(le.getDataSources());

    ASSERT_THROW(evalAgent.evaluateIndividual(indiv, *representation, le, 0, mode), std::runtime_error) << "Evaluation of empty individual should have fail";

    // Fill individual
    group.addNode(std::make_unique<Node::GPNode>(std::vector<size_t>{1, 2, 1, 5, 1, 2}));// R[1] = S[1] * S[2] = 3.0
    group.addNode(std::make_unique<Node::GPNode>(std::vector<size_t>{2, 0, 0, 3, 1, 0}));// R[2] = R[3] + S[0] = 1.0
    group.addNode(std::make_unique<Node::GPNode>(std::vector<size_t>{2, 3, 0, 2, 0, 2}));// R[2] = R[2] / R[2] = 1.0 / 1.0 = 1.0
    group.addNode(std::make_unique<Node::GPNode>(std::vector<size_t>{0, 1, 1, 2, 1, 1}));// R[0] = S[2] - S[1] = 0.5
    group.addNode(std::make_unique<Node::GPNode>(std::vector<size_t>{0, 1, 0, 0, 0, 2}));// R[0] = R[0] - R[2] = 0.5 - 1 = -0.5

    std::shared_ptr<Evaluation::EvaluationResult> result;
    ASSERT_NO_THROW(result = evalAgent.evaluateIndividual(indiv, *representation, le, 0, mode)) << "Evaluation should not have fail";
}
