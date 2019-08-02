#include <iostream>
#include <unordered_set>

#include "dataHandlers/dataHandler.h"
#include "dataHandlers/primitiveTypeArray.h"
#include "instructions/instruction.h"
#include "instructions/addPrimitiveType.h"
#include "supportedTypes.h"

int main() {
	std::cout << "Hello TPG World" << std::endl;

	Instructions::Instruction* i = new Instructions::AddPrimitiveType<double>();

	auto hash = [](const std::reference_wrapper<SupportedType>& n){ return typeid(n.get()).hash_code(); };
	auto equal = [](const  std::reference_wrapper<SupportedType>& n1, const  std::reference_wrapper<SupportedType>& n2) { return n1.get() == n2.get(); };
	std::unordered_set<std::reference_wrapper<SupportedType>, decltype(hash) , decltype(equal)> set(8 , hash, equal);
	SupportedType* t1 = new PrimitiveType<double>();
	set.insert(*t1);

	DataHandlers::DataHandler* dh = new DataHandlers::PrimitiveTypeArray<double>();
	//dh->getHandledTypes();

	return 0;
}