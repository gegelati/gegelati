#include "graphBased/gpNode.h"

// Declaration of static GPNnode ID Counter in local here because it creates
// error in the .h file for MSVC compiler See:
// https://discourse.cmake.org/t/exporting-a-static-data-member-of-a-class-for-dll-using-msvc/5892
static size_t GP_NODE_COUNTER_ID = 0;

size_t GraphBased::GPNode::incrementeCounter()
{
    return GP_NODE_COUNTER_ID++;
}

size_t GraphBased::GPNode::getGPNodeIDCounter()
{
    return GP_NODE_COUNTER_ID;
}

void GraphBased::GPNode::resetGPNodeIDCounter()
{
    GP_NODE_COUNTER_ID = 0;
}


size_t GraphBased::GPNode::getGPNodeID() const
{
    return this->gpNodeID;
}

void GraphBased::GPNode::setGPNodeID(size_t newID)
{
    this->gpNodeID = newID;

    // Update the ID counter if needed
    if (newID >= GP_NODE_COUNTER_ID) {
        GP_NODE_COUNTER_ID = newID + 1;
    }
}


std::unique_ptr<GraphBased::GPNode> GraphBased::GPNode::cloneUniquePtr() const
{
    return std::make_unique<GPNode>(this->values, this->isIntron);
}

bool GraphBased::operator<(const GraphBased::GPNode& a, const GraphBased::GPNode& b)
{
    return a.getGPNodeID() < b.getGPNodeID();
}

bool GraphBased::operator==(const GraphBased::GPNode& a, const GraphBased::GPNode& b)
{
    return a.getGPNodeID() == b.getGPNodeID();
}
bool GraphBased::operator!=(const GraphBased::GPNode& a, const GraphBased::GPNode& b)
{
    return a.getGPNodeID() != b.getGPNodeID();
}

void GraphBased::GPNode::addValue(const Data::DataValue& value)
{
    this->values.push_back(value.clone());
}

void GraphBased::GPNode::setValue(size_t index, const Data::DataValue& value)
{
    if(index >= this->getSize()){
        throw std::runtime_error("GraphBased::GPNode::setValue: index out of range.");
    }
    this->values[index] = value.clone();
}


const Data::DataValue& GraphBased::GPNode::getValue(size_t index) const
{
    if(index >= this->getSize()){
        throw std::runtime_error("GraphBased::GPNode::getValue: index out of range.");
    }
    return this->values[index];
}

void GraphBased::GPNode::setIsIntron(bool isIntron)
{
    this->isIntron = isIntron;
}

bool GraphBased::GPNode::getIsIntron() const
{
    return this->isIntron;
}

size_t GraphBased::GPNode::getSize() const
{
    return this->values.size();
}


const std::vector<Data::DataValue>& GraphBased::GPNode::getValues() const
{
    return this->values;
}

bool GraphBased::GPNode::hasSameValues(const GPNode& other) const
{
    if(this->getSize() != other.getSize()) {
        return false;
    }

    for(size_t idxValue = 0; idxValue < this->getSize(); idxValue++) {
        if(this->getValue(idxValue) != other.getValue(idxValue)) {
            return false;
        }
    }
    return true;
}