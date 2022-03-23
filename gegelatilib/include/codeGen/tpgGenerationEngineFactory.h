/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2019 - 2022) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2019 - 2022)
 * Thomas Bourgoin <tbourgoi@insa-rennes.fr> (2021)
 * Mickael Dardaillon <mdardail@insa-rennes.fr> (2022)
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

#ifndef TPG_GENERATION_ENGINE_FACTORY_H
#define TPG_GENERATION_ENGINE_FACTORY_H
#include "codeGen/tpgGenerationEngine.h"

namespace CodeGen {
    /**
     * \brief Factory class to create code generators.
     */
    class TPGGenerationEngineFactory
    {
      public:
        /**
         * @brief Enumeration of possible codegen modes.
         *
         */
        enum generationEngineMode
        {
            stackMode,
            switchMode
        };

        /**
         * @brief Factory constructor with default mode
         *
         */
        TPGGenerationEngineFactory();

        /**
         * @brief Factory constructor with parameterized mode
         *
         * @param mode method chosen for code generation
         */
        TPGGenerationEngineFactory(enum generationEngineMode mode);

        /**
         * @brief Factory method to create a codegen with the configured mode.
         *
         * @return a unique_ptr<TPGGenerationEngine>.
         */
        std::unique_ptr<TPGGenerationEngine> create(const std::string& filename,
                                    const TPG::TPGGraph& tpg,
                                    const std::string& path = "./");

      private:
        enum generationEngineMode mode;
    };
} // namespace CodeGen

#endif // TPG_GENERATION_ENGINE_FACTORY_H

#endif // CODE_GENERATION
