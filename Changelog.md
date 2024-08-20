# GEGELATI Changelog

## Release version x.y.z
_2024.01.10_

### New features

### Changes
* Update to improve the diversity of the TPGs.
  * Add a new parameter to select the number of roots to initialise a TPG.
  * Add a new parameter to create new program during the mutation, during the training .

* Add two parameters to the reset method of the learning environment. These parameters are used for environments that use specific initialization.
  * Parameter `iterationNumber`: an integer indicating the current iteration number when the `nbIterationsPerPolicyEvaluation` parameter is greater than 1, default value = 0.
  * Parameter `generationNumber`: an integer indicating the current generation number, default value = 0.

* CI now uses build caches for Linux and Macos runner.
  * Build caches are saved from the `develop` branch and used on feature branches.
  * `main` and `develop` branches still do full compilation.
  * Build caches are also used for Sonar analysis.

* Updated CI actions versions
  * nick-invision/retry@v2 -> nick-fields/retry@v3
  * actions/upload-artifact@v3 -> actions/upload-artifact@v4
  * actions/download-artifact@v3 -> actions/download-artifact@v4
  * JamesIves/github-pages-deploy-action@4.1.5 -> JamesIves/github-pages-deploy-action@4 (will used latest v4, currently 4.6.1)
  * softprops/action-gh-release@v1 -> softprops/action-gh-release@v2
  * actions/setup-java@v3 -> actions/setup-java@v4
  * actions/cache@v3 -> actions/cache@v4

### Bug fix
* Fixed a bug in mutationEdgeDestination.
  * When changing the destination of an edge, if the new destination was an action, an index i between 0 and nbAction was sampled, but the new destination was the team of index i instead of the action of index i.

## Release version 1.3.1 - Donanatella flavor with extra sprinkles
_2023.12.14_

### Changes
* Add possibility to build gegelati without the documentation, using the `-DSKIP_DOXYGEN_BUILD=ON` option during CMake configuration.

### Bug fix
* Build MinGW Release zip with Codegen module


## Release version 1.3.0 - Donanatella flavor
_2023.12.13_

### New features
* Add a `Log::CycleDetectionLALogger` for detecting directed cycles in TPG graphs. Using this logger, the detection is automatically at each generation right after populating the TPG with new vertices. In case a cycle is detected, a message is printed in `std::cerr`.
* Make GEGELATI compatible with MinGW on windows. Adds a dedicated job in Github action. Package a pre-built library `gegelati-mingw-<version>.zip` for future releases.

### Changes
* Remove support for cycles in trained TPG graphs. _This change alters the deterministic outputs of trainings._ As explained in [Issue #106](https://github.com/gegelati/gegelati/issues/106), the genetic mutation of the graph, as described by Stephen Kelly in his PhD manuscript will never introduce any cycle in the trained TPG. As a consequence, parts of the code have been simplified:
  * Mutations: It is no longer mandatory to have an action referenced in each team of the TPG.
  * TPG Execution (in gegelati and in generated code): When executing outgoing edges of a team, providing a mechanism to exclude already visited edges is not needed.
* Add the attribute "bestScoreLastGen" to the learning agent that records the best score achieved in each generation. This score can be obtained using the "getBestScoreLastGen" method.
* Update the generated CMake support for `find_package(GEGELATI)`. Make the `GEGELATI_INCLUDE_DIRS` and `GEGELATI_LIBRARIES` visible in the CMakeGui and in the whole CMake scope in case subdirectory are used.
* Rename Visual Studio pre-built library `gegelati-msvc-<version>.zip` to differentiate from the new MinGW `gegelati-mingw-<version>.zip` version.

### Bug fix
* Fix build issue with MSVC19. `#include <ctime>` needed in timestamp.cpp.
* Fix silly installation timeout for CI on Windows.


## Release version 1.2.0 - Cantutuccini flavor
_2022.08.31_

### New features
* Add `Data::PointerWrapper` class to simplify interracing of GEGELATI with primitive variables (non-array) data from a learning environment.
* Add `TPG::ExecutionsStats` class to analyze and export execution statistics gathered using an instrumented TPGGraph. Statistics include averages on numbers of evaluated teams, programs, program lines and instructions, execution traces and various distributions based on execution traces. The class also provides a method to export these statistics to a JSon file, which can be used by other programs and scripts.
* Add a `File::TPGDotExporter::printSubgraph()` method to print only a subgraph from a TPG, starting from a specified `TPGVertex`. This method can notably be used to export the champion TPG throughout the training process, without having to remove other roots from the TPG.
* Add a new `Learn::LearningAgent::evaluateOneRoot()` method to ease the evaluation of individual policies in a trained TPG.
* Add a new `Learn::LearningAgent::getEnvironment()` method for convenience.

### Changes
* CodeGen TPGs (Switch and Stack modes) now match the execution behaviour of `TPGExecutionEngine`, especially regarding cycle handling. Now, a team can't be visited more than once per inference, and edges leading to an already visited team are not evaluated (their program are not executed).
* Change in the `Learn::LearningAgent::makeJob()` that now takes a `TPG::TPGVertex*` as an input instead of the index of a root. As a result, the method can now be used to evaluate a TPG from any vertex, including non-roots ones.

### Bug fix
* Fix penalty score given to edges in cycle handling in `TPGSwitchGenerationEngine`, was `DBL_MIN` but should be `-DBL_MAX`.


## Release version 1.1.0 - Babacio flavor
_2022.04.20_

### New features
* Add instrumented specializations of the `TPGVertex` and `TPGEdge` classes that contain internal counters to keep track of the number of time these graph elements are visited or traversed. To exploit these counters, one must create a `TPGGraph` using a `TPGFactoryInstrumented`, use a new execution engine: `TPGExecutionEngineInstrumented` that will update the counters. The `TPGFactoryInstrumented` also contains utility methods to reset all counter in a `TPGGraph` with instrumented elements, or remove from a `TPGGraph` all the vertices and edges that were never visited or traversed.
* A new code generation backend is proposed. Instead of using the stack-based traversal of the TPG graph, this approach is based on a Switch system, achieving slightly faster inference. The `TPGSwitchGenerationEngine` has been made the default code generator, thus replacing the `TPGStackGenerationEngine`.
* Generated C code and exported Dot files are now time-stamped when generated.

### Changes
* Refactor the `LearningAgent` class to access and store the trained `TPGGraph` as a shared pointer. This is needed to ensure that the trained `TPGGraph` can survive to the destruction of its parent `LearningAgent`.
* Refactor the `TPGGraph` class to:
    * Use a `TPGFactory` to create the TPGAction, TPGTeam, TPGEdge of a TPGGraph.
    * Store `TPGEdge` with `unique_ptr` to support polymorphic operations on them.
* In code generation:
    * The `TPGGenerationEngine` becomes a pure virtual class, whose former code is transferred to the `TPGStackGenerationEngine` class.
    * A new factory system, has been developed to switch between the two generation engine: `TPGGenerationEngineFactory`.

### Bug fix
* Change google test repo config. Branch name is now `main` instead of `develop`.
* Fix determinism issue with ARM 64-bit architecture in `Data::Hash`. Determinism still not provided on non 64bit architectures (X86, ARM, or others).
* Fix issue with programs returning `NaN` during a TPG execution. Comparison with a `NaN` always returns false, which made `NaN` a winning bid for the first edge of a team, and a losing bid for any other edge. During TPG execution, in gegelati and generated code, any `NaN` bid is now replaced with `-inf`. **This change will change the deterministic results obtained with previous version of the library.**


## Release version 1.0.0 - Amamaretto version/flavor
_2021.10.11_

### New features
* A code generation feature is added to GEGELATI. The code generation feature enables generating a stand-alone C code implementing the inference behavior of a trained Tangled Program Graph. To enable this feature, PrintableInstruction have been added to GEGELATI. Each PrintableInstruction possess its own print template which defines, with a regex-like syntax, how this instruction should be printed in the generated C code. The code generation feature can be disabled when building the library, by appending the `-DCODE_GEN=OFF` directive to the cmake command.

### Changes
* Change the typedef Learn::LearningMode into an enum class to remove compilation warnings in MSVC. Because of this change, members of the enum must now be used with the class name: `TRAINING` > `LearningMode::TRAINING`.
* Add conditional builds without the code-generation for the continuous integration.
* Fasten the continuous integration on windows build by building the `Release` target only for builds of the `develop` and `master` branches.
* Migrate CI to Github Actions instead of Travis.

### Bug fix
* Fix memory leak when demangling type name in GNU environment.
* Fix dangling pointers warning from MSVC.
* Fix unitialized attributes in various classes.


## Release version 0.6.0
_2021.06.02_

### New features
* Add `ArrayWrapper` and `ArrayWrapper2D` classes inheriting from `DataHandler`, in the `Data` namespace. These two classes act as a wrapper between a pointer to `std::vector` of data, and the `DataHandler` methods. Contrary to `PrimitiveTypeArray` and `PrimitiveTypeArray2D`, there is no need to copy all the data into the `ArrayWrapper`s.
* `Learn::LearningParameters` can now be exported into a commented JSon file using the `File::ParameterParser::writeParametersToJson()` function.
* Add an assignment operator `operator=` to `PrimitiveTypeArray`and `PrimitiveTypeArray2D`.
* Add methods to clear introns from Program and TPGGraph.

### Changes
* Update `LABasicLogger` to log rewards both on training and validation sets.
* Update `PrimitiveTypeArray` and `PrimitiveTypeArray2D` to be child classes of `ArrayWrapper` and `ArrayWrapper2D`, respectively.

### Bug fix
* Fix travis config for OSX. Build old doxygen from source because `brew install doxygen` no longer works.

## Release version 0.5.1
_2020.10.18_

### New features

### Changes

### Bug fix
* Fix bugs with Constants:
    * Negative values for Constant mutations were not supported by RNG.
    * Program behavior unicity was not scaling adresses of Constant properly.

## Release version 0.5.0
_2020.10.16_

### New features
* Optionnaly, it is now possible to force a Program to have a new behavior after being mutated.
    * New methods were added for testing equality of `Program::Line` and `Program::Program`. Program equality is based on an analysis of non-intron lines of the Program.
    * Mutation of the Program behavior is enforced by comparing its state before and after being mutated. This comparison can be activated complementary to the legacy archiving mechanism from Kelly's PhD.
    * The `tpg.forceProgramBehaviorChangeOnMutation` boolean was added to the MutationParameters.

* Optionnaly, it is now possible to use constants during the training.
   * A fixed number of constants can be defined in the parameters. They can be used by instructions designed to use the Data::Constant types.
   * Mutations of the program affect the values of the constants. A constant is mutated with a probability and bounds defined as MutationParameters.

### Changes
* The Parameter stored within Instructions have been removed entirely. Similar behavior is now supported by newly introduced Data::Constant that belong to Program instead of Instruction.
* During eval, the main thread of ParallelLearningAgent now use the main LearningEnvironment instead of systematically cloning it. It enables sequential mode with non-copyable environment for the AdversarialLearningAgent.
* AdversarialLearningAgent now proceeds evaluations by making each root play against/with champions of the previous generation.
* New Unit Test cover the use of LambdaInstruction with non-primitive data types.

### Bug fix

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
