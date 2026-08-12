
#include <fstream>
#include <gtest/gtest.h>

#include "node/nodeValueTemplate.h"
#include "node/nodeTemplate.h"
#include "node/genotypeTemplate.h"

// Require for comparison
#include "evolution/individual.h"

TEST(NodeValueTemplateTest, Constructor) 
{
    Node::NodeValueTemplate* nodeValueTemplate;
    Node::NodeValueTemplate* nodeValueTemplate1;
    Node::NodeValueTemplate* nodeValueTemplate2;

    auto nodeConfig(std::make_shared<Node::NodeValueConfiguration>(std::vector<Node::NodeValue>{size_t(0), size_t(1), size_t(2)}));
    std::vector<std::shared_ptr<const Node::NodeValueConfiguration>> vect{nodeConfig};

    ASSERT_NO_THROW(nodeValueTemplate = new Node::NodeValueTemplate()) << "Creation of template failed";
    ASSERT_NO_THROW(nodeValueTemplate1 = new Node::NodeValueTemplate(nodeConfig)) << "Creation of template failed";
    ASSERT_NO_THROW(nodeValueTemplate2 = new Node::NodeValueTemplate(vect)) << "Creation of template failed";
    
    ASSERT_NO_THROW(delete nodeValueTemplate) << "Destructor of template failed.";
    ASSERT_NO_THROW(delete nodeValueTemplate1) << "Destructor of template failed.";
    ASSERT_NO_THROW(delete nodeValueTemplate2) << "Destructor of template failed.";
}

TEST(NodeValueTemplateTest, addValue) {
    Node::NodeValueTemplate nodeValueTemplate;

    auto config0(std::make_shared<Node::NodeValueConfiguration>(std::make_pair(0.0, 2.0)));
    auto config1(std::make_shared<Node::NodeValueConfiguration>(std::vector<Node::NodeValue>{size_t(0), size_t(1), size_t(2)}));

    ASSERT_EQ(nodeValueTemplate.size(), 0) << "Size of nodeValueTemplate mismatch.";

    ASSERT_NO_THROW(nodeValueTemplate.addValueConfiguration(config0)) << "Adding configuration failed";
    ASSERT_EQ(nodeValueTemplate.size(), 1) << "Size of nodeValueTemplate mismatch.";

    ASSERT_NO_THROW(nodeValueTemplate.addValueConfiguration(config1)) << "Adding configuration failed";
    ASSERT_EQ(nodeValueTemplate.size(), 2) << "Size of nodeValueTemplate mismatch.";

    ASSERT_EQ(nodeValueTemplate.getconfigurationAt(0), config0) << "Configurations should be equal";
    ASSERT_EQ(nodeValueTemplate.getconfigurationAt(1), config1) << "Configurations should be equal";

    const auto& nodeConfigs = nodeValueTemplate.getconfigurations();
    ASSERT_EQ(nodeValueTemplate.getconfigurationAt(0), nodeConfigs.at(0)) << "Configurations should be equal";
    ASSERT_EQ(nodeValueTemplate.getconfigurationAt(1), nodeConfigs.at(1)) << "Configurations should be equal";

    ASSERT_THROW(nodeValueTemplate.getconfigurationAt(2), std::runtime_error) << "Should have failed with out of range.";


    // Check other constructors
    Node::NodeValueTemplate nodeValueTemplate1(config0);
    ASSERT_EQ(nodeValueTemplate1.size(), 1) << "Size of nodeValueTemplate mismatch.";
    ASSERT_EQ(nodeValueTemplate1.getconfigurationAt(0), config0) << "Configurations should be equal";

    
    Node::NodeValueTemplate nodeTemplate2({config0, config1});
    ASSERT_EQ(nodeTemplate2.size(), 2) << "Size of nodeTemplate mismatch.";
    ASSERT_EQ(nodeTemplate2.getconfigurationAt(1), config1) << "nodeValueTemplates should be equal";
}




TEST(NodeTemplateTest, Constructor) 
{
    Node::NodeTemplate* nodeTemplate;
    Node::NodeTemplate* nodeTemplate1;
    Node::NodeTemplate* nodeTemplate2;

    auto nodeValueTemplate(std::make_shared<Node::NodeValueTemplate>());
    std::vector<std::shared_ptr<const Node::NodeValueTemplate>> vect{nodeValueTemplate};

    ASSERT_NO_THROW(nodeTemplate = new Node::NodeTemplate()) << "Creation of template failed";
    ASSERT_NO_THROW(nodeTemplate1 = new Node::NodeTemplate(nodeValueTemplate)) << "Creation of template failed";
    ASSERT_NO_THROW(nodeTemplate2 = new Node::NodeTemplate(vect)) << "Creation of template failed";
    
    ASSERT_NO_THROW(delete nodeTemplate) << "Destructor of template failed.";
    ASSERT_NO_THROW(delete nodeTemplate1) << "Destructor of template failed.";
    ASSERT_NO_THROW(delete nodeTemplate2) << "Destructor of template failed.";
}

TEST(NodeTemplateTest, addValue) {
    Node::NodeTemplate nodeTemplate;

    auto config0(std::make_shared<Node::NodeValueConfiguration>(std::make_pair(0.0, 2.0)));
    auto config1(std::make_shared<Node::NodeValueConfiguration>(std::vector<Node::NodeValue>{size_t(0), size_t(1), size_t(2)}));
    auto config2(std::make_shared<Node::NodeValueConfiguration>(std::vector<Node::NodeValue>{0.5, 1.6, size_t(2)}));

    auto nodeValueTemplate0(std::make_shared<Node::NodeValueTemplate>(config0));
    std::vector<std::shared_ptr<const Node::NodeValueConfiguration>> vect = {config1, config2};
    auto nodeValueTemplate1(std::make_shared<Node::NodeValueTemplate>(vect));

    ASSERT_EQ(nodeTemplate.size(), 0) << "Size of nodeTemplate mismatch.";

    ASSERT_NO_THROW(nodeTemplate.addValueTemplate(nodeValueTemplate0)) << "Adding nodeValueTemplate failed";
    ASSERT_EQ(nodeTemplate.size(), 1) << "Size of nodeTemplate mismatch.";

    ASSERT_NO_THROW(nodeTemplate.addValueTemplate(nodeValueTemplate1)) << "Adding nodeValueTemplate failed";
    ASSERT_EQ(nodeTemplate.size(), 2) << "Size of nodeTemplate mismatch.";

    ASSERT_EQ(nodeTemplate.getValueTemplateAt(0), nodeValueTemplate0) << "nodeValueTemplates should be equal";
    ASSERT_EQ(nodeTemplate.getValueTemplateAt(1), nodeValueTemplate1) << "nodeValueTemplates should be equal";

    const auto& nodeValueTemplates = nodeTemplate.getValueTemplates();
    ASSERT_EQ(nodeTemplate.getValueTemplateAt(0), nodeValueTemplates.at(0)) << "nodeValueTemplates should be equal";
    ASSERT_EQ(nodeTemplate.getValueTemplateAt(1), nodeValueTemplates.at(1)) << "nodeValueTemplates should be equal";

    ASSERT_THROW(nodeTemplate.getValueTemplateAt(2), std::runtime_error) << "Should have failed with out of range.";

    auto emptyValueTemplate(std::make_shared<Node::NodeValueTemplate>());
    ASSERT_THROW(nodeTemplate.addValueTemplate(emptyValueTemplate), std::runtime_error) << "Should have failed with empty value.";


    // Check other constructors
    Node::NodeTemplate nodeTemplate1(nodeValueTemplate0);
    ASSERT_EQ(nodeTemplate1.size(), 1) << "Size of nodeTemplate mismatch.";
    ASSERT_EQ(nodeTemplate1.getValueTemplateAt(0), nodeValueTemplate0) << "nodeValueTemplates should be equal";

    
    Node::NodeTemplate nodeTemplate2({nodeValueTemplate0, nodeValueTemplate1});
    ASSERT_EQ(nodeTemplate2.size(), 2) << "Size of nodeTemplate mismatch.";
    ASSERT_EQ(nodeTemplate2.getValueTemplateAt(1), nodeValueTemplate1) << "nodeValueTemplates should be equal";

}



TEST(GenotypeTemplateTest, Constructor) 
{
    Node::GenotypeTemplate* genotypeTemplate;
    Node::GenotypeTemplate* genotypeTemplate1;
    Node::GenotypeTemplate* genotypeTemplate2;

    auto nodeTemplate(std::make_shared<Node::NodeTemplate>());
    std::vector<std::shared_ptr<const Node::NodeTemplate>> vect{nodeTemplate};

    std::pair<size_t, size_t> pair{1, 2};

    ASSERT_NO_THROW(genotypeTemplate = new Node::GenotypeTemplate()) << "Creation of template failed";
    ASSERT_NO_THROW(genotypeTemplate1 = new Node::GenotypeTemplate(nodeTemplate, pair)) << "Creation of template failed";
    ASSERT_NO_THROW(genotypeTemplate2 = new Node::GenotypeTemplate(vect, {pair})) << "Creation of template failed";
    
    ASSERT_NO_THROW(delete genotypeTemplate) << "Destructor of template failed.";
    ASSERT_NO_THROW(delete genotypeTemplate1) << "Destructor of template failed.";
    ASSERT_NO_THROW(delete genotypeTemplate2) << "Destructor of template failed.";

    ASSERT_THROW(Node::GenotypeTemplate(vect, {pair, pair}), std::runtime_error) << "Creation of template should have failed";
}

TEST(GenotypeTemplateTest, addValue) {
    Node::GenotypeTemplate genotypeTemplate;

    auto config0(std::make_shared<Node::NodeValueConfiguration>(std::make_pair(0.0, 2.0)));
    auto config1(std::make_shared<Node::NodeValueConfiguration>(std::vector<Node::NodeValue>{size_t(0), size_t(1), size_t(2)}));
    auto config2(std::make_shared<Node::NodeValueConfiguration>(std::vector<Node::NodeValue>{0.5, 1.6, size_t(2)}));

    auto nodeValueTemplate0(std::make_shared<Node::NodeValueTemplate>(config0));
    std::vector<std::shared_ptr<const Node::NodeValueConfiguration>> vect = {config1, config2};
    auto nodeValueTemplate1(std::make_shared<Node::NodeValueTemplate>(vect));
    
    auto nodeTemplate0(std::make_shared<Node::NodeTemplate>(
        std::vector<std::shared_ptr<const Node::NodeValueTemplate>>{nodeValueTemplate0, nodeValueTemplate0, nodeValueTemplate1}));
    auto nodeTemplate1(std::make_shared<Node::NodeTemplate>(
        std::vector<std::shared_ptr<const Node::NodeValueTemplate>>{nodeValueTemplate1}));

    ASSERT_EQ(genotypeTemplate.size(), 0) << "Size of genotypeTemplate mismatch.";

    ASSERT_NO_THROW(genotypeTemplate.addNodeTemplate(nodeTemplate0, 1, 5)) << "Adding nodeTemplate failed";
    ASSERT_EQ(genotypeTemplate.size(), 1) << "Size of genotypeTemplate mismatch.";

    ASSERT_NO_THROW(genotypeTemplate.addNodeTemplate(nodeTemplate1, 5)) << "Adding nodeTemplate failed";
    ASSERT_EQ(genotypeTemplate.size(), 2) << "Size of genotypeTemplate mismatch.";

    ASSERT_EQ(genotypeTemplate.getNodeTemplateAt(0), nodeTemplate0) << "nodeTemplates should be equal";
    ASSERT_EQ(genotypeTemplate.getNodeTemplateAt(1), nodeTemplate1) << "nodeTemplates should be equal";

    ASSERT_EQ(genotypeTemplate.getRangeAt(0), std::make_pair(size_t(1), size_t(5))) << "ranges should be equal";
    ASSERT_EQ(genotypeTemplate.getRangeAt(1), std::make_pair(size_t(5), size_t(5))) << "ranges should be equal";

    const auto& nodeTemplates = genotypeTemplate.getNodeTemplates();
    ASSERT_EQ(genotypeTemplate.getNodeTemplateAt(0), nodeTemplates.at(0)) << "nodeTemplates should be equal";
    ASSERT_EQ(genotypeTemplate.getNodeTemplateAt(1), nodeTemplates.at(1)) << "nodeTemplates should be equal";

    const auto& nodeRanges = genotypeTemplate.getRanges();
    ASSERT_EQ(genotypeTemplate.getRangeAt(0), nodeRanges.at(0)) << "ranges should be equal";
    ASSERT_EQ(genotypeTemplate.getRangeAt(1), nodeRanges.at(1)) << "ranges should be equal";

    ASSERT_THROW(genotypeTemplate.getNodeTemplateAt(2), std::runtime_error) << "Should have failed with out of range.";
    ASSERT_THROW(genotypeTemplate.getRangeAt(2), std::runtime_error) << "Should have failed with out of range.";

    auto emptyNodeTemplate(std::make_shared<Node::NodeTemplate>());
    ASSERT_THROW(genotypeTemplate.addNodeTemplate(emptyNodeTemplate), std::runtime_error) << "Should have failed with empty value.";


    // Check other constructors
    Node::GenotypeTemplate genotypeTemplate1(nodeTemplate0, std::make_pair(size_t(1), size_t(5)));
    ASSERT_EQ(genotypeTemplate1.size(), 1) << "Size of genotypeTemplate mismatch.";
    ASSERT_EQ(genotypeTemplate1.getNodeTemplateAt(0), nodeTemplate0) << "nodeTemplates should be equal";
    ASSERT_EQ(genotypeTemplate1.getRangeAt(0), std::make_pair(size_t(1), size_t(5))) << "ranges should be equal";


    Node::GenotypeTemplate genotypeTemplate2(
        {nodeTemplate0, nodeTemplate1}, 
        {std::make_pair(size_t(1), size_t(5)), std::make_pair(size_t(5), size_t(5))});
    ASSERT_EQ(genotypeTemplate2.size(), 2) << "Size of genotypeTemplate mismatch.";
    ASSERT_EQ(genotypeTemplate2.getNodeTemplateAt(1), nodeTemplate1) << "nodeTemplates should be equal";
    ASSERT_EQ(genotypeTemplate2.getRangeAt(1), std::make_pair(size_t(5), size_t(5))) << "ranges should be equal";
}