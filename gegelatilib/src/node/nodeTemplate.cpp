#include "node/nodeTemplate.h"
            
            
void Node::NodeTemplate::addValueTemplate(std::shared_ptr<const NodeValueTemplate> nodeValueTemplate)
{
    if(nodeValueTemplate->size() == 0) {
        throw std::runtime_error("Node:NodeTemplate:addValueTemplate: Cannot add empty nodeValueTemplate.");
    }
    this->nodeValueTemplates.push_back(nodeValueTemplate);
}

const std::vector<std::shared_ptr<const Node::NodeValueTemplate>>& Node::NodeTemplate::getValueTemplates() const
{
    return this->nodeValueTemplates;
}
std::shared_ptr<const Node::NodeValueTemplate> Node::NodeTemplate::getValueTemplateAt(size_t idxValue) const
{
    if(idxValue >= this->nodeValueTemplates.size()) {
        throw std::runtime_error("Node:NodeTemplate:getValueTemplateAt: index out of bounds.");
    }
    return this->nodeValueTemplates.at(idxValue);
}

size_t Node::NodeTemplate::size() const 
{
    return this->nodeValueTemplates.size();
}