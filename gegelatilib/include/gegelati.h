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
 * modify and/or redistribute the software under the terms of the CeCILL-C
 * license as circulated by CEA, CNRS and INRIA at the following URL:
 * "http://www.cecill.info".
 *
 * As a counterpart to the rights granted by the license, users are provided
 * only with a limited warranty. The software's authors and licensors have
 * limited liability.
 *
 * The fact that you are presently reading this means that you have had
 * knowledge of the CeCILL-C license and that you accept its terms.
 */

/**
 * \file gegelati.h
 * \brief Helper file gathering all current public headers from the GEGELATI
 * library to ease their inclusion in applications.
 */

#ifndef GEGELATI_H
#define GEGELATI_H

// Data
#include <data/constant.h>
#include <data/dataConcept.h>
#include <data/dataType.h>
#include <data/dataValue.h>
#include <data/dataView.h>
#include <data/demangle.h>
#include <data/hash.h>

// Dimensions
#include <dimensions/activationFunctions.h>
#include <dimensions/constraint.h>
#include <dimensions/dataValueGenerator.h>
#include <dimensions/dimensionFlow.h>
#include <dimensions/individualArchiveValueGenerator.h>
#include <dimensions/individualValueGenerator.h>
#include <dimensions/numericRange.h>
#include <dimensions/requirement.h>

// Evaluation
#include <evaluation/dataset.h>
#include <evaluation/evaluate.h>
#include <evaluation/predictionProblem.h>
#include <evaluation/problem.h>
#include <evaluation/reinforcementEnvironment.h>
#include <evaluation/reinforcementProblem.h>

// Descriptors
#include <fitnessAssignment/noveltyDescriptors/averageVectorDescriptor.h>
#include <fitnessAssignment/noveltyDescriptors/descriptor.h>
#include <fitnessAssignment/noveltyDescriptors/scalarListDescriptor.h>

// Evolution
#include <evolution/basicEvolutionConfig.h>
#include <evolution/basicLogConfig.h>
#include <evolution/entry.h>
#include <evolution/evolutionData.h>
#include <evolution/originTPGEvolutionConfig.h>
#include <evolution/module.h>
#include <evolution/steps/assignFitnessStep.h>
#include <evolution/steps/evaluateStep.h>
#include <evolution/steps/exportStep.h>
#include <evolution/steps/initPopulationStep.h>
#include <evolution/steps/lambdaStep.h>
#include <evolution/steps/logColumnStep.h>
#include <evolution/steps/logStep.h>
#include <evolution/steps/mutateStep.h>
#include <evolution/steps/reproduceStep.h>
#include <evolution/steps/selectStep.h>
#include <evolution/steps/tpgSteps.h>

// File
#include <file/populationDotExporter.h>
#include <file/populationDotImporter.h>

// Fitness assignment
#include <fitnessAssignment/assigner.h>
#include <fitnessAssignment/classificationF1Assigner.h>
#include <fitnessAssignment/defaultAssigner.h>
#include <fitnessAssignment/noveltySearchAssigner.h>

// Graph-based representation
#include <graphBased/genotype.h>
#include <graphBased/genotypeConstraint.h>
#include <graphBased/genotypeGenerator.h>
#include <graphBased/gpNode.h>
#include <graphBased/nodeConstraint.h>
#include <graphBased/nodeGenerator.h>
#include <graphBased/nodeGroup.h>

// Instructions
#include <instructions/addPrimitiveType.h>
#include <instructions/instruction.h>
#include <instructions/lambdaInstruction.h>
#include <instructions/multByConstant.h>
#include <instructions/set.h>

// Metrics
#include <metrics/inputMetric.h>
#include <metrics/metric.h>
#include <metrics/metricMap.h>
#include <metrics/outputMetric.h>
#include <metrics/predictionMetric.h>
#include <metrics/rewardsMetric.h>
#include <metrics/scalarMetric.h>
#include <metrics/vectorMetric.h>

// Mutation
#include <mutation/mutator.h>
#include <mutation/nodeTransferMutation.h>
#include <mutation/pointMutator.h>

// Core
#include <individual.h>

// Representations
#include <representations/LGP.h>
#include <representations/representation.h>
#include <representations/TPG.h>

// Reproduction
#include <reproduction/replicator.h>
#include <reproduction/reproducer.h>

// Random number generation
#include <rng/deterministicRandom.h>
#include <rng/rng.h>

// Selection
#include <selection/randomSelector.h>
#include <selection/selector.h>
#include <selection/tournamentSelector.h>
#include <selection/truncationSelector.h>

// Utilities
#include <util/counterReset.h>
#include <util/genericComparator.h>
#include <util/individualUtils.h>
#include <util/stopwatch.h>
#include <util/timestamp.h>

#endif // GEGELATI_H