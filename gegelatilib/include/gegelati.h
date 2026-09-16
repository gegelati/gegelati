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

#include <oldRepresentations/individual.h>
#include <oldRepresentations/representation.h>
#include <oldRepresentations/job.h>
#include <oldRepresentations/mutator.h>
#include <oldRepresentations/population.h>
#include <oldRepresentations/executionEngine.h>

#include <oldRepresentations/atpg/atpgRepresentation.h>
#include <oldRepresentations/atpg/atpgPopulation.h>
#include <oldRepresentations/atpg/atpgMutator.h>
#include <oldRepresentations/atpg/atpgExecutionEngine.h>

#include <oldRepresentations/cgp/cgpRepresentation.h>
#include <oldRepresentations/cgp/cgpLineMutator.h>
#include <oldRepresentations/cgp/cgpMutator.h>

#include <oldRepresentations/lgp/environment.h>
#include <oldRepresentations/lgp/lgpRepresentation.h>
#include <oldRepresentations/lgp/lgpIndividual.h>
#include <oldRepresentations/lgp/lgpPopulation.h>
#include <oldRepresentations/lgp/lgpMutator.h>
#include <oldRepresentations/lgp/lgpLine.h>
#include <oldRepresentations/lgp/lgpLineMutator.h>
#include <oldRepresentations/lgp/lgpCodeGenerationEngine.h>
#include <oldRepresentations/lgp/lgpExecutionEngine.h>

#include <oldRepresentations/maple/mapleIndividual.h>
#include <oldRepresentations/maple/mapleRepresentation.h>
#include <oldRepresentations/maple/mapleExecutionEngine.h>
#include <oldRepresentations/maple/maplePopulation.h>
#include <oldRepresentations/maple/mapleMutator.h>

#include <oldRepresentations/tgp/tgpRepresentation.h>
#include <oldRepresentations/tgp/tgpLineMutator.h>
#include <oldRepresentations/tgp/tgpMutator.h>

#include <oldRepresentations/tpg/archive.h>
#include <oldRepresentations/tpg/tpgRepresentation.h>
#include <oldRepresentations/tpg/tpgIndividual.h>
#include <oldRepresentations/tpg/tpgPopulation.h>
#include <oldRepresentations/tpg/tpgMutator.h>
#include <oldRepresentations/tpg/tpgExecutionEngine.h>


#include <oldData/array2DWrapper.h>
#include <oldData/arrayWrapper.h>
#include <data/constant.h>
#include <oldData/constantHandler.h>
#include <oldData/dataHandler.h>
#include <data/hash.h>
#include <oldData/pointerWrapper.h>
#include <oldData/primitiveTypeArray.h>
#include <oldData/primitiveTypeArray2D.h>
#include <oldData/untypedSharedPtr.h>


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

#include <graphBased/gpNode.h>

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

#include <dimensions/activationFunctions.h>
#include <util/counterReset.h>
#include <util/genericComparator.h>
#include <util/timestamp.h>

#include <codeGen/codeGenerationExporter.h>

#include <parameters.h>
#include <outputInfo.h>

#endif
