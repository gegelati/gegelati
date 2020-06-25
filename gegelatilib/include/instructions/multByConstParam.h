/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2019 - 2020) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2019 - 2020)
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

#ifndef INSTRUCTION_MULT_BY_CONST_PARAM
#define INSTRUCTION_MULT_BY_CONST_PARAM

#include <memory>

#include "data/untypedSharedPtr.h"
#include "instruction.h"

namespace Instructions {

	/**
	* \brief Instruction for multiplying a unique argument of type T by a constant parameter of type U.
	*/
	template <class T, class U> class MultByConstParam : public Instruction {
		static_assert(std::is_fundamental<T>::value&& std::is_fundamental<U>::value, "Template class MultByConstParam<T,U> can only be used for primitive types.");
		static_assert(std::is_same<int16_t, U>() || std::is_same<float, U>(), "Param type can either be int16_t or float.");
	public:
		/**
		*  \brief Constructor for the MultByConstParam class.
		*/
		MultByConstParam();

		virtual double execute(
			const std::vector<std::reference_wrapper<const Parameter>>& params,
			const std::vector<Data::UntypedSharedPtr>& args) const override;
	};

	template <class T, class U> MultByConstParam<T, U>::MultByConstParam() {
		this->operandTypes.push_back(typeid(T));
		this->nbParameters = 1;
	}

	template<class T, class U> double MultByConstParam<T, U>::execute(
		const std::vector<std::reference_wrapper<const Parameter>>& params,
		const std::vector<Data::UntypedSharedPtr>& args) const
	{
#ifndef NDEBUG
		if (Instruction::execute(params, args) != 1.0) {
			return 0.0;
		}
#endif // !NDEBUG

		const U pValue = (const U&)params.at(0).get();
		return *(args.at(0).getSharedPointer<const T>()) * (double)pValue;
	};
}

#endif
