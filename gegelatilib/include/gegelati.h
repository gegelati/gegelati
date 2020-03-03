/**
* \file gegelati.h
* \brief Helper file gathering all headers from the GEGELATI lib to ease their
* inclusion in apps.
*/
#ifndef GEGELATI_H
#define GEGELATI_H

#include <dataHandlers/dataHandler.h>  
#include <dataHandlers/primitiveTypeArray.h>
#include <data/hash.h>

#include <file/tpgGraphDotExporter.h>

#include <file/tpgGraphDotImporter.h>

#include <instructions/addPrimitiveType.h>  
#include <instructions/instruction.h>
#include <instructions/lambdaInstruction.h>
#include <instructions/multByConstParam.h>
#include <instructions/set.h>

#include <learn/evaluationResult.h>
#include <learn/learningAgent.h>
#include <learn/parallelLearningAgent.h>
#include <learn/learningEnvironment.h>
#include <learn/learningParameters.h>

#include <learn/classificationEvaluationResult.h>
#include <learn/classificationLearningEnvironment.h>
#include <learn/classificationLearningAgent.h>

#include <mutator/lineMutator.h>
#include <mutator/mutationParameters.h>
#include <mutator/programMutator.h>
#include <mutator/rng.h>
#include <mutator/tpgMutator.h>

#include <program/line.h>  
#include <program/program.h>  
#include <program/programExecutionEngine.h>

#include <tpg/tpgAction.h>
#include <tpg/tpgEdge.h>
#include <tpg/tpgExecutionEngine.h>
#include <tpg/tpgGraph.h>
#include <tpg/tpgTeam.h>
#include <tpg/tpgVertex.h>

#endif