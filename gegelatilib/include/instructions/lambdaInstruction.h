/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2019 - 2020) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2019 - 2020)
 * Nicolas Sourbier <nsourbie@insa-rennes.fr> (2020)
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

#ifndef LAMBDA_INSTRUCTION_H
#define LAMBDA_INSTRUCTION_H

#include <functional>
#include <typeinfo>

#include "data/untypedSharedPtr.h"
#include "instructions/instruction.h"

namespace Instructions {

	/**
	* \brief Template instruction for simplifying the creation of an
	* Instruction from a c++ lambda function.
	*
	* Template parameter T can be any type.
	*
	* Currently, LambdaInstructions all require two operands, with the same
	* type determined by T.
	*
	* The second template parameter is using the SFINAE to specialize the class
	* implementation differently, depending on the template parameter.
	*/
	template <class T, typename Enable = void> class LambdaInstruction : public Instruction {};

	/**
	* \brief Specialization of the LambdaInstruction template class for
	* primitive types.
	*/
	template <class T>
	class LambdaInstruction <T, typename std::enable_if<std::is_fundamental<T>::value>::type> : public Instruction
	{
	protected:

		/**
		* \brief Function executed for this Instruction.
		*/
		const std::function<double(T, T)> func;

	public:
		/**
		* \brief delete the default constructor.
		*/
		LambdaInstruction() = delete;

		/**
		* \brief Constructor for the LambdaInstruction.
		*
		* \param[in] function the c++ std::function that will be executed for
		* this Instruction.
		*/
		LambdaInstruction(std::function<double(T, T)> function) : func{ function } {
			this->operandTypes.push_back(typeid(T));
			this->operandTypes.push_back(typeid(T));
		};

		double execute(
			const std::vector<std::reference_wrapper<const Parameter>>& params,
			const std::vector<Data::UntypedSharedPtr>& args) const override {

			if (Instruction::execute(params, args) != 1.0) {
				return 0.0;
			}

			const T& arg1 = *(args.at(0).getSharedPointer<const T>());
			const T& arg2 = *(args.at(1).getSharedPointer<const T>());
			double result = this->func(arg1, arg2);
			return result;
		}
	};

	/**
	* \brief Specialization of the LambdaInstruction template class for
	* for arrays of primitive types.
	*/
	template <class T>
	class LambdaInstruction < T, typename std::enable_if <std::is_array<T>::value>::type> : public Instruction
	{
	    using U = std::remove_all_extents_t<T>;
	protected:
		/**
		* \brief Function executed for this Instruction.
		*/
		const std::function<double(const U*, const U*)> func;

	public:
		/**
		* \brief delete the default constructor.
		*/
		LambdaInstruction() = delete;

		/**
		* \brief Constructor for the LambdaInstruction.
		*
		* \param[in] function the c++ std::function that will be executed for
		* this Instruction.
		*/
		LambdaInstruction(std::function<double(const U*, const U*)> function) : func{ function } {
			this->operandTypes.push_back(typeid(T));
			this->operandTypes.push_back(typeid(T));
		};

		/// Inherited from Instruction
		virtual bool checkOperandTypes(const std::vector<Data::UntypedSharedPtr>& arguments) const override {
			if (arguments.size() != this->operandTypes.size()) {
				return false;
			}

			for (int i = 0; i < arguments.size(); i++) {
				// Argument Type
				const std::type_info& argType = arguments.at(i).getType();
				// Expected type.
				// When requesting a c-style array "T[n]", the expected type is "const T[0]"
				const std::type_info& ownType = typeid(const std::remove_all_extents_t<T>[]);
				if (argType != ownType) {
					return false;
				}
			}
			return true;
		};

		double execute(
			const std::vector<std::reference_wrapper<const Parameter>>& params,
			const std::vector<Data::UntypedSharedPtr>& args) const override {

			// Check if operands and types have the right type.
			if (Instruction::execute(params, args) != 1.0) {
				return 0.0;
			}

			std::shared_ptr<const U[]> arg1 = (args.at(0)).getSharedPointer<const U[]>();
			std::shared_ptr<const U[]> arg2 = (args.at(1)).getSharedPointer<const U[]>();
			double result = this->func(arg1.get(),  arg2.get());
			return result;
		};
	};
};

#endif
