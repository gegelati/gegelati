#include "node/nodeTemplate.h"
            
            
void Node::NodeTemplate::addValueTemplate(const NodeValueTemplate& nodeValueTemplate)
{
    this->nodeValueTemplates.push_back(nodeValueTemplate);
}

const std::vector<Node::NodeValueTemplate>& Node::NodeTemplate::getValueTemplates() const
{
    return this->nodeValueTemplates;
}

size_t Node::NodeTemplate::size() const 
{
    return this->nodeValueTemplates.size();
}