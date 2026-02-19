/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2019 - 2025) :
 *
 * Elinor Montmasson <elinor.montmasson@gmail.com> (2022)
 * Karol Desnos <kdesnos@insa-rennes.fr> (2019 - 2022)
 * Mickaël Dardaillon <mdardail@insa-rennes.fr> (2022)
 * Nicolas Sourbier <nsourbie@insa-rennes.fr> (2019 - 2020)
 * Pierre-Yves Le Rolland-Raumer <plerolla@insa-rennes.fr> (2020)
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

/**
 * \file gegelati.h
 * \brief Helper file gathering all headers from the GEGELATI lib to ease their
 * inclusion in apps.
 */
#ifndef GEGELATI_H
#define GEGELATI_H

#include <util/counterReset.h>
#include <util/genericComparator.h>
#include <util/timestamp.h>

#include <algorithm/agent.h>
#include <algorithm/algorithm.h>
#include <algorithm/job.h>
#include <algorithm/mutator.h>
#include <algorithm/agentManager.h>
#include <algorithm/executionEngine.h>

#include <algorithm/atpg/atpgAlgorithm.h>
#include <algorithm/atpg/atpgManager.h>
#include <algorithm/atpg/atpgMutator.h>
#include <algorithm/atpg/atpgExecutionEngine.h>

#include <algorithm/lgp/environment.h>
#include <algorithm/lgp/lgpAlgorithm.h>
#include <algorithm/lgp/lgpAgent.h>
#include <algorithm/lgp/lgpManager.h>
#include <algorithm/lgp/lgpMutator.h>
#include <algorithm/lgp/lgpLine.h>
#include <algorithm/lgp/lgpLineMutator.h>
#include <algorithm/lgp/lgpExecutionEngine.h>

#include <algorithm/maple/mapleAgent.h>
#include <algorithm/maple/mapleAlgorithm.h>
#include <algorithm/maple/mapleExecutionEngine.h>
#include <algorithm/maple/mapleManager.h>
#include <algorithm/maple/mapleMutator.h>

#include <algorithm/tpg/archive.h>
#include <algorithm/tpg/tpgAlgorithm.h>
#include <algorithm/tpg/tpgAgent.h>
#include <algorithm/tpg/tpgManager.h>
#include <algorithm/tpg/tpgMutator.h>
#include <algorithm/tpg/tpgExecutionEngine.h>


#include <data/array2DWrapper.h>
#include <data/arrayWrapper.h>
#include <data/constant.h>
#include <data/constantHandler.h>
#include <data/dataHandler.h>
#include <data/hash.h>
#include <data/pointerWrapper.h>
#include <data/primitiveTypeArray.h>
#include <data/primitiveTypeArray2D.h>
#include <data/untypedSharedPtr.h>


#include <evoGraph/abstractEngine.h>
#include <evoGraph/action.h>
#include <evoGraph/edge.h>
#include <evoGraph/element.h>
#include <evoGraph/factory.h>
#include <evoGraph/graph.h>
#include <evoGraph/team.h>
#include <evoGraph/vertex.h>

#include <evoGraph/instrumented/actionInstrumented.h>
#include <evoGraph/instrumented/edgeInstrumented.h>
#include <evoGraph/instrumented/factoryInstrumented.h>
#include <evoGraph/instrumented/teamInstrumented.h>
#include <evoGraph/instrumented/vertexInstrumented.h>

#include <file/parametersParser.h>
#include <file/graphDotExporter.h>
#include <file/graphDotImporter.h>

#include <instructions/addPrimitiveType.h>
#include <instructions/instruction.h>
#include <instructions/lambdaInstruction.h>
#include <instructions/multByConstant.h>
#include <instructions/set.h>

#include <learn/evaluationResult.h>
#include <learn/learningAgent.h>
#include <learn/learningEnvironment.h>
#include <learn/learningParameters.h>
#include <learn/parallelLearningAgent.h>

// #include <learn/adversarialEvaluationResult.h>
// #include <learn/adversarialJob.h>
// #include <learn/adversarialLearningAgent.h>
// #include <learn/adversarialLearningEnvironment.h>

#include <learn/classificationLearningEnvironment.h>

#include <log/cycleDetectionLALogger.h>
#include <log/laBasicLogger.h>
#include <log/laLogger.h>
#include <log/laPolicyStatsLogger.h>
#include <log/logger.h>
#include <log/mapElitesArchiveLogger.h>

#include <mutator/mutationParameters.h>
#include <mutator/rng.h>

#include <selector/classificationSelectionMetrics.h>
#include <selector/classificationSelector.h>
#include <selector/mapElites/cvtMapElitesArchive.h>
#include <selector/mapElites/mapElitesArchive.h>
#include <selector/mapElites/mapElitesDefaultDescriptors.h>
#include <selector/mapElites/mapElitesDescriptor.h>
#include <selector/mapElites/mapElitesSelectionMetrics.h>
#include <selector/mapElites/mapElitesSelector.h>
#include <selector/selectionContext.h>
#include <selector/selectionMetrics.h>
#include <selector/selector.h>
#include <selector/selectorFactory.h>
#include <selector/tournamentSelector.h>
#include <selector/truncationSelector.h>

#include <util/activationFunctions.h>
#include <util/counterReset.h>
#include <util/genericComparator.h>
#include <util/timestamp.h>

#ifdef CODE_GENERATION
#include <codeGen/programGenerationEngine.h>
#include <codeGen/tpgGenerationEngine.h>
#include <codeGen/tpgGenerationEngineFactory.h>
#include <codeGen/tpgStackGenerationEngine.h>
#include <codeGen/tpgSwitchGenerationEngine.h>
#endif

#include <outputInfo.h>

#endif
