# GEGELATI Changelog

## Release version x.y.z
_aaaa.mm.dd_

### New features
* The Instructions::LambdaInstruction template class now accepts any number and type of arguments instead of only two. To use the feature, the LambdaInstruction should be instanciated with the list of data types of its arguments as a template parameter. For example `Instructions::LambdaInstruction<int, double, const float[2]>` is a lamda instructions requiring 3 arguments for its execution, whose types are `int`, `double`, and `const float[2]`, respectively.
* Learning agents (`Learn::LearningAgent` and `Learn::ParallelLearningAgent`) now keep a record of the root `TPGVertex` that has led to the best `EvaluationResult` throughout the training process. This root and the associated evaluation result can be retrieved using the new `getBestRoot()` method.
* New TPG::PolicyStats class for analyzing the topology and the Program of a TPG::TPGGraph starting from a given root TPG::TPGVertex.

### Changes
* Instructions:LambdaInstruction evolution listed in New Features lead to incompatibility with previous version. To update your code, simply double the template parameter of your previously existing LambdaInstruction. For example `LambdaInstruction<double>` becomes `LambdaInstruction<double, double>`.
* The behavior of the `LearningAgent::keepBestPolicy()` is now based on the new `bestRoot` attribute, and not on a new evaluation of the remaining roots.
* Method Program::ProgramExecutionEngine::scaleLocation() moved to Data::DataHandler::scaleLocation().

### Bug fix


## Release version 0.1.1
_2020.04.28_

### New features
* Configure SonarCloud for automated code analysis reports. [Custom quality gate for gegelati](https://sonarcloud.io/organizations/gegelati/quality_gates/show/23677) is a more restrictive version of the default sonar way.
* New script to update license in all files.

### Changes

### Bug fix
* Minor changes in code to pass SonarCloud quality gate.
* Fix LearningAgent::decimateWorstRoot method crash when too many root TPGAction were present in the decimated graph.


## Release version 0.1.0
_2020.04.07_

### New features
* Possibility to import a TPGGraph and its programs with the File::TPGGraphDotImporter class.
* New Data::Hash class providing a portable hash mechanism in replacement of std::hash.
* Use of Data::UntypedSharedPtr instead of std::reference_wrapper for fetching operands in DataHandler. This enables fetching "composite" operands, that is operands built on request from native data type in the data handler, and destroyed after use. Data::SupportedType and Data::PrimitiveType no longer needed after this change.
* Adding support for C-style 1D arrays of primitive types in LambdaInstruction.

### Changes
* Reorganization
  * Renaming the Exporter namespace into File.
  * Renaming DataHandlers namespace into Data.
* Switch from transfer.sh to file.io for supporting deployment.
* Update Data::DataHandler, Program::Program, Mutator::LineMutator to take composite operands into account.

### Bug fix
* Training and mutation process were not portable on multiple OSes and compilers because of the diverse implementations of std::hash.


## Release version 0.0.0
_2020.01.14_

### New features
* Implementation of TPG execution & evolution as described in [Stephen Kelly PhD thesis](http://stephenkelly.ca/research_files/Kelly-Stephen-PhD-CSCI-June-2018.pdf).

* Instructions
  * Customized instructions: Instructions used to build programs of the TPG can be customized for each learning process,
    * through specialization of the `Instructions::Instruction` class,
    * through c++ lambda function with the `Instructions::LambdaInstruction` template.
  * Constant arguments: In addition to data sources provided by the learning environment, instructions can take constant as arguments. These constant arguments are subject to mutations during the evolutionary process of the TPG. Class example: `Instructions::MultByConstParam`.

* Learn
  * Parallel TPG execution and evolution: Using the `Learn::ParallelLearningAgent`, a multithreaded execution is implemented for the evaluation of policies starting from several root vertices of the TPG, and for the evolution process of the TPG. To benefit from this parallelism, the `Learn::LearningEnvironment` given to the learning agent must be copyable (see `Learn::LearningEnvironment::isCopyable()` documentation).
  * Determinist learning: Learning process, that is TPG execution, archive management, and evolution process, is fully deterministic and portable based on a given seed and pseudo-random number generators. Determinism is also preserved in the parallel learning process.
  * Classification-oriented learning process: For learning environment representing a classification problem and specializing the `Learn::ClassificationLearningEnvironment` class, a dedicated (and hopefully more efficient) `Learn::ClassificationLearningAgent` is provided. Usage example: MNIST application on [GEGELATI-apps](https://github.com/gegelati/gegelati-apps).

* Exporter
  * DOT exporter: TPG graph resulting from a learning process can be exported for visualization in the dot format.

* Continuous Integration
  * Unit Tests: Unit tests implemented with the GoogleTest framework ensure full coverage of the library code.
  * Automated CI: Travis configuration for building the library and running unit tests under Windows MSVC19 and Linux GCC7, for all branches.
  * Neutral builds: All commits on the develop branch are deployed on the [Neutral builds page](https://gegelati.github.io/neutral-builds).
  * Applications CI: Applications on [GEGELATI-apps](https://github.com/gegelati/gegelati-apps) are built and run automatically for each new [Neutral build](https://gegelati.github.io/neutral-builds).

### Changes

### Bug fix
