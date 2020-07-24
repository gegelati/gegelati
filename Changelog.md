# GEGELATI Changelog

## Release version 0.4.0
_2020.07.24_

### New features
* Fix code for building GEGELATI with clang (v7+) and added a dedicated job with travis-ci.
* Add osx job in travis-ci.
* Add adversarial training support. AdversarialLearningAgent can be declared and used with an AdversarialLearningEnvironment to train random groups of several roots together.

### Changes
* Learning Agents now handle jobs instead of simple roots, allowing new concepts like adversarial learning (a job containing several roots that will train together).
* Adding `LearningAgent::forgetPreviousResults()` method to support changing LearningEnvironment where score of previous generation need to be forgotten from time to time. This is usefull when the LearningEnvironment evolves every _N_ generations to teach new skills gradually.
* Add support for 2D arrays with.
    * New Data::PrimitiveTypeArray2D data handler for providing 2D data sources.
    * Support in UntypedSharedPtr for fetching 2D operands for instructions. Due to C++ constraints, 2D arrays must be packaged into 1D dynamically allocated arrays.
    * Support for 2D primitive C-style array in LambdaInstructions.
* New LAPolicyStatsLogger that logs the PolicyStats of the LearningAgent::bestRoot each time it is updated.

### Changes
* Remove redundant typecheck in `PrimitiveTypeArray::setDataAt()` in `NDEBUG` mode. A performance gain of ~25% was observed on the MNIST application (without a thorough profiling method).
* tpgGraphDotImporter now has a line MAX_READ_SIZE limit of 4096 (instead of 1024), allowing to read greater tpg dot files.
* Add a caching mechanism in `Data::PrimitiveTypeArray[2D]` to avoid reexecuting the `getAddressSpace()` regular expression more than once for each data type. (This had a huge overhead on execution time.)
* Logger
    * Refactor the LALogger class (and daughter classes) to keep a reference to the associated LearningAgent at construction time.
    * Update formatting of LABasicLogger to produce a more compact log.
    * Add mutation time "T_mutat" to LABasicLogger log.
    * Rename Logger files to start with a lower-case character.

### Bug fix
* Adapt code for building GEGELATI with clang standard library: libc++. (see Issue #49 for mode details)
* Fix implicit template type for std::vector in ClassificationLearningAgentTest.
* Fix call to LALogger when validation is activated. LALoggers were called on training results even when validation was activated.
* Fix missing virtual destructor in the Logger class.


## Release version 0.3.0
_2020.07.02_

### New features
* Parameters (learningParameters and mutationParameters) can now be set with a JSON file using parametersParser.
* A generic Logger class was added to ease the creation of log files within gegelati.
* A specific logging mechanism is available for LearningAgent, using a child class of the LALogger class. A logger, LABasicLogger with basic learning information is provided.
* In Release configuration, the `NDEBUG` macro is used to deactivate redundant operand type checks during Program execution. A performance gain of 8% was observed on the MNIST application.
* Automate code formatting with clang-format-10.
    * A new script, `fix_code_format.sh`, can be used to check and format all C++ files of the `gegelatilib` and `test` folders. Use the `--doCommit` option to commit the reformatting that was done.
    * Update of Travis configuration to fail when files are not properly formatted when building a Pull Request. Other builds (i.e. non-PR) allow the failure of this job.
* An automated validation is now possible when the parameter `doValidation` is set to true. In this case, the basicLALogger hides evaluation statistics, and displays validation ones instead.
* Nicer README.md with illustrated links to application repositories, and useless but fancy emojis.

### Changes
* maxNbThreads is now a variable of learningAgent allowing to have a more generic trainOneGeneration method

### Bug fix


## Release version 0.2.1
_2020.06.12_

### New features

### Changes
* LearningAgent now receives the number of threads and registers from the LearningParameter.

### Bug fix
* Fix non-determinism issue in ClassificationLearningAgent.
* Fix intron detection when importing a dot file with TPGGraphDotImporter.


## Release version 0.2.0
_2020.06.03_

### New features
* The Instructions::LambdaInstruction template class now accepts any number and type of arguments instead of only two. To use the feature, the LambdaInstruction should be instanciated with the list of data types of its arguments as a template parameter. For example `Instructions::LambdaInstruction<int, double, const float[2]>` is a lamda instructions requiring 3 arguments for its execution, whose types are `int`, `double`, and `const float[2]`, respectively.
* Learning agents (`Learn::LearningAgent` and `Learn::ParallelLearningAgent`) now keep a record of the root `TPGVertex` that has led to the best `EvaluationResult` throughout the training process. This root and the associated evaluation result can be retrieved using the new `getBestRoot()` method.
* New TPG::PolicyStats class for analyzing the topology and the Program of a TPG::TPGGraph starting from a given root TPG::TPGVertex.
* In Learn::EvaluationResult:
    * A new attribute `nbEvaluation` was added to count the number of times the associated policy was evaluated to produce this result. 
    * A new addition assignment operator (`operator+=`) can be used to combine two existing EvaluationResult.
* Learn::LearningAgent now store EvaluationResult for each non-decimated root TPGVertex. Each time a root is reevaluated, its EvaluationResult is updated by combining it with the new result. The number of evaluation of each policy (i.e. each root TPGVertex) can now be bounded using the Learn::LearningParameters::maxNbEvaluationPerPolicy parameter. Passed this number, previous EvaluationResult for this root, stored by the LearningAgent, will directly be returned instead of reevaluating the root.

### Changes
* New features cause a change in training determinism. Following this release, training result for a known LearningEnvironment with a known seed may differ.
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
