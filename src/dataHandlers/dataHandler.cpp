#include "dataHandlers/dataHandler.h"

bool DataHandlers::DataHandler::canProvide(const std::type_info& type) const
{
	
	size_t res = this->handledTypes.count(type);
	return res == 1;
}
