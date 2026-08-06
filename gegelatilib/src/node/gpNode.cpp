#include "node/gpNode.h"

// Declaration of static GPNnode ID Counter in local here because it creates
// error in the .h file for MSVC compiler See:
// https://discourse.cmake.org/t/exporting-a-static-data-member-of-a-class-for-dll-using-msvc/5892
static size_t GP_NODE_COUNTER_ID = 0;

size_t Node::GPNode::incrementeCounter()
{
    return GP_NODE_COUNTER_ID++;
}

size_t Node::GPNode::getGPNodeIDCounter()
{
    return GP_NODE_COUNTER_ID;
}

void Node::GPNode::resetGPNodeIDCounter()
{
    GP_NODE_COUNTER_ID = 0;
}


size_t Node::GPNode::getGPNodeID() const
{
    return this->gpNodeID;
}

void Node::GPNode::setGPNodeID(size_t newID)
{
    this->gpNodeID = newID;

    // Update the ID counter if needed
    if (newID >= GP_NODE_COUNTER_ID) {
        GP_NODE_COUNTER_ID = newID + 1;
    }
}

bool Node::operator<(const Node::GPNode& a, const Node::GPNode& b)
{
    return a.getGPNodeID() < b.getGPNodeID();
}

bool Node::operator==(const Node::GPNode& a, const Node::GPNode& b)
{
    return a.getGPNodeID() == b.getGPNodeID();
}
bool Node::operator!=(const Node::GPNode& a, const Node::GPNode& b)
{
    return a.getGPNodeID() != b.getGPNodeID();
}

void Node::GPNode::setValue(size_t index, size_t value)
{
    if(index >= this->size){
        throw std::runtime_error("Node::GPNode::setValue: index out of range.");
    }
    if(value >= this->max_ranges[index]){
        throw std::runtime_error("Node::GPNode::setValue: value out of range.");
    }
    this->values[index] = value;
}


size_t Node::GPNode::getValue(size_t index) const
{
    if(index >= this->size){
        throw std::runtime_error("Node::GPNode::getValue: index out of range.");
    }
    return this->values[index];
}

size_t Node::GPNode::getMaxRange(size_t index) const
{
    if(index >= this->size){
        throw std::runtime_error("Node::GPNode::getMaxRange: index out of range.");
    }
    return this->max_ranges[index];
}


size_t Node::GPNode::getSize() const
{
    return this->size;
}


const std::vector<size_t>& Node::GPNode::getValues() const
{
    return this->values;
}


const std::vector<size_t>& Node::GPNode::getMaxRanges() const
{
    return this->max_ranges;
}