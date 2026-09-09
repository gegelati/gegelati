#include <gtest/gtest.h>

#include <stdexcept>
#include <string>
#include <vector>

#include "dimensions/activationFunctions.h"
#include "dimensions/dimensionFlow.h"
#include "dimensions/requirement.h"
#include "dimensions/numericRange.h"
#include "data/dataValue.h"

// ============================================================================
// Constructor
// ============================================================================

TEST(DimensionFlowTest, ConstructsWithValidInputDimension)
{
    const auto input =
    Dimensions::Requirement::array1d<double>(4);


    EXPECT_NO_THROW(
        Dimensions::DimensionFlow flow({input}))
        << "DimensionFlow must accept a valid non-empty input dimension list.";
}

TEST(DimensionFlowTest, RejectsEmptyInputDimensions)
{
    EXPECT_THROW(
    Dimensions::DimensionFlow flow({}),
    std::runtime_error)
    << "DimensionFlow must reject construction with no input dimensions.";
}

TEST(DimensionFlowTest, RejectsInputDimensionWithoutElementType)
{
    const Dimensions::Requirement invalid(
    Data::DataType{});


    EXPECT_THROW(
        Dimensions::DimensionFlow flow({invalid}),
        std::runtime_error)
        << "DimensionFlow must reject an input dimension whose DataType has no element type.";
}

TEST(DimensionFlowTest, AcceptsMultipleInputDimensions)
{
    const auto input1 =
    Dimensions::Requirement::array1d<double>(4);


    const auto input2 =
        Dimensions::Requirement::scalar<int>();

    EXPECT_NO_THROW(
        Dimensions::DimensionFlow flow({input1, input2}))
        << "DimensionFlow must support multiple input dimensions.";
}


TEST(DimensionFlowTest, StartsWithoutOutput)
{
    const auto input =
    Dimensions::Requirement::array1d<double>(4);


    Dimensions::DimensionFlow flow({input});

    EXPECT_FALSE(
        flow.isValid())
        << "A DimensionFlow without any layers should not be considered a valid executable flow.";
}

TEST(DimensionFlowTest, GetOutputDimensionThrowsBeforeFirstLayer)
{
    const auto input =
    Dimensions::Requirement::array1d<double>(4);


    Dimensions::DimensionFlow flow({input});

    EXPECT_THROW(
        flow.getOutputDimension(),
        std::logic_error)
        << "getOutputDimension() must throw when the flow does not yet contain an output layer.";
}

TEST(DimensionFlowTest, IsCompatibleWithReturnsFalseBeforeFirstLayer)
{
    const auto input =
    Dimensions::Requirement::array1d<double>(4);

    const auto consumer =
        Dimensions::Requirement::scalar<double>();

    Dimensions::DimensionFlow flow({input});

    EXPECT_FALSE(
        flow.isCompatibleWith(consumer))
        << "A flow without an output layer cannot be compatible with a consumer requirement.";
}

TEST(DimensionFlowTest, ExposesOriginalInputDimensions)
{
    const auto input1 =
    Dimensions::Requirement::array1d<double>(4);


    const auto input2 =
        Dimensions::Requirement::scalar<int>();

    Dimensions::DimensionFlow flow({input1, input2});

    ASSERT_EQ(
        flow.getInputDimensions().size(),
        2u)
        << "DimensionFlow must preserve the number of input dimensions supplied to its constructor.";

    EXPECT_EQ(
        flow.getInputDimensions()[0],
        input1)
        << "The first input dimension returned by DimensionFlow must match the constructor input.";

    EXPECT_EQ(
        flow.getInputDimensions()[1],
        input2)
        << "The second input dimension returned by DimensionFlow must match the constructor input.";
}

TEST(DimensionFlowTest, FirstLayerConsumesFlowInputs)
{
    const auto input =
    Dimensions::Requirement::array1d<double>(4);


    const auto output =
        Dimensions::Requirement::scalar<double>();

    Dimensions::DimensionFlow flow({input});

    const bool compatible =
        flow.addLayer("first", {input}, output);

    EXPECT_TRUE(
        compatible)
        << "The first layer must be compatible when its required input matches the flow input.";

    EXPECT_TRUE(
        flow.isValid())
        << "A flow containing a compatible first layer must be valid.";

    EXPECT_EQ(
        flow.getOutputDimension(),
        output)
        << "The current output must be updated to the first layer's declared output.";
}

TEST(DimensionFlowTest, FirstLayerRejectsIncompatibleInput)
{
    const auto flowInput =
    Dimensions::Requirement::array1d<double>(4);


    const auto layerInput =
        Dimensions::Requirement::array1d<double>(5);

    const auto output =
        Dimensions::Requirement::scalar<double>();

    Dimensions::DimensionFlow flow({flowInput});

    const bool compatible =
        flow.addLayer("first", {layerInput}, output);

    EXPECT_FALSE(
        compatible)
        << "The first layer must be rejected when its required input is incompatible with the flow input.";

    EXPECT_FALSE(
        flow.isValid())
        << "A flow containing an incompatible first layer must be marked invalid.";
}

TEST(DimensionFlowTest, FirstLayerRejectsWrongInputType)
{
    const auto flowInput =
    Dimensions::Requirement::array1d<double>(4);


    const auto layerInput =
        Dimensions::Requirement::array1d<float>(4);

    const auto output =
        Dimensions::Requirement::scalar<double>();

    Dimensions::DimensionFlow flow({flowInput});

    const bool compatible =
        flow.addLayer("first", {layerInput}, output);

    EXPECT_FALSE(
        compatible)
        << "The first layer must reject an input requirement with the wrong element type.";

    EXPECT_FALSE(
        flow.isValid())
        << "An incompatible first layer must make the DimensionFlow invalid.";
}

TEST(DimensionFlowTest, FirstLayerRejectsWrongNumberOfInputs)
{
    const auto input =
    Dimensions::Requirement::array1d<double>(4);


    const auto secondInput =
        Dimensions::Requirement::scalar<int>();

    const auto output =
        Dimensions::Requirement::scalar<double>();

    Dimensions::DimensionFlow flow({input});

    const bool compatible =
        flow.addLayer("first", {input, secondInput}, output);

    EXPECT_FALSE(
        compatible)
        << "A layer requiring a different number of inputs than the flow provides must be incompatible.";

    EXPECT_FALSE(
        flow.isValid())
        << "A first layer with the wrong number of inputs must make the flow invalid.";
}

TEST(DimensionFlowTest, FirstLayerCanHaveMultipleInputs)
{
const auto input1 =
Dimensions::Requirement::array1d<double>(4);


const auto input2 =
    Dimensions::Requirement::scalar<int>();

const auto output =
    Dimensions::Requirement::scalar<double>();

Dimensions::DimensionFlow flow({input1, input2});

const bool compatible =
    flow.addLayer("first", {input1, input2}, output);

EXPECT_TRUE(
    compatible)
    << "A first layer with multiple compatible inputs must be accepted.";

EXPECT_TRUE(
    flow.isValid())
    << "A flow with a compatible multi-input first layer must be valid.";


}

// ============================================================================
// Subsequent layers
// ============================================================================

TEST(DimensionFlowTest, SecondLayerConsumesPreviousOutput)
{
const auto flowInput =
Dimensions::Requirement::array1d<double>(4);


const auto firstOutput =
    Dimensions::Requirement::scalar<double>();

const auto secondOutput =
    Dimensions::Requirement::scalar<int>();

Dimensions::DimensionFlow flow({flowInput});

ASSERT_TRUE(
    flow.addLayer("first", {flowInput}, firstOutput))
    << "The first layer must be compatible so that the second-layer test has a valid preceding output.";

const bool compatible =
    flow.addLayer("second", {firstOutput}, secondOutput);

EXPECT_TRUE(
    compatible)
    << "A subsequent layer must consume the output of the preceding layer.";

EXPECT_TRUE(
    flow.isValid())
    << "A flow with compatible sequential layers must remain valid.";

EXPECT_EQ(
    flow.getOutputDimension(),
    secondOutput)
    << "The current output must be updated to the second layer's output.";


}

TEST(DimensionFlowTest, SecondLayerDoesNotConsumeOriginalFlowInput)
{
const auto flowInput =
Dimensions::Requirement::array1d<double>(4);


const auto firstOutput =
    Dimensions::Requirement::scalar<double>();

const auto wrongSecondInput =
    Dimensions::Requirement::array1d<double>(4);

const auto secondOutput =
    Dimensions::Requirement::scalar<int>();

Dimensions::DimensionFlow flow({flowInput});

ASSERT_TRUE(
    flow.addLayer("first", {flowInput}, firstOutput))
    << "The first layer must be compatible so that the second layer can be tested.";

const bool compatible =
    flow.addLayer("second", {wrongSecondInput}, secondOutput);

EXPECT_FALSE(
    compatible)
    << "A subsequent layer must be checked against the previous layer's output rather than the original flow input.";

EXPECT_FALSE(
    flow.isValid())
    << "An incompatible second layer must invalidate the flow.";


}

TEST(DimensionFlowTest, IncompatibleSecondLayerInvalidatesFlow)
{
const auto input =
Dimensions::Requirement::array1d<double>(4);


const auto firstOutput =
    Dimensions::Requirement::scalar<double>();

const auto incompatibleInput =
    Dimensions::Requirement::scalar<int>();

const auto secondOutput =
    Dimensions::Requirement::scalar<double>();

Dimensions::DimensionFlow flow({input});

ASSERT_TRUE(
    flow.addLayer("first", {input}, firstOutput))
    << "The first layer must be compatible before testing an incompatible second layer.";

EXPECT_FALSE(
    flow.addLayer("second", {incompatibleInput}, secondOutput))
    << "The second layer must report incompatibility when its input does not accept the previous output.";

EXPECT_FALSE(
    flow.isValid())
    << "Once a layer is incompatible, the overall DimensionFlow must remain invalid.";


}

TEST(DimensionFlowTest, LaterCompatibleLayerDoesNotRestoreValidity)
{
const auto input =
Dimensions::Requirement::array1d<double>(4);


const auto firstOutput =
    Dimensions::Requirement::scalar<double>();

const auto badSecondInput =
    Dimensions::Requirement::scalar<int>();

const auto secondOutput =
    Dimensions::Requirement::scalar<double>();

const auto thirdInput =
    Dimensions::Requirement::scalar<double>();

const auto thirdOutput =
    Dimensions::Requirement::scalar<float>();

Dimensions::DimensionFlow flow({input});

ASSERT_TRUE(
    flow.addLayer("first", {input}, firstOutput))
    << "The first layer must be compatible before introducing an invalid layer.";

ASSERT_FALSE(
    flow.addLayer("second", {badSecondInput}, secondOutput))
    << "The second layer must be incompatible to establish the invalid flow state.";

EXPECT_TRUE(
    flow.addLayer("third", {thirdInput}, thirdOutput))
    << "A later layer may itself be compatible with the previous layer output.";

EXPECT_FALSE(
    flow.isValid())
    << "A later compatible layer must not erase an earlier incompatibility from the overall validity state.";


}

TEST(DimensionFlowTest, OutputIsUpdatedEvenWhenLayerIsIncompatible)
{
const auto input =
Dimensions::Requirement::array1d<double>(4);


const auto firstOutput =
    Dimensions::Requirement::scalar<double>();

const auto incompatibleInput =
    Dimensions::Requirement::scalar<int>();

const auto secondOutput =
    Dimensions::Requirement::scalar<float>();

Dimensions::DimensionFlow flow({input});

ASSERT_TRUE(
    flow.addLayer("first", {input}, firstOutput))
    << "The first layer must be compatible before testing output replacement.";

ASSERT_FALSE(
    flow.addLayer("second", {incompatibleInput}, secondOutput))
    << "The second layer must be reported as incompatible.";

EXPECT_EQ(
    flow.getOutputDimension(),
    secondOutput)
    << "DimensionFlow must update its current output even when the newly added layer is incompatible.";


}

// ============================================================================
// Requirement compatibility
// ============================================================================

TEST(DimensionFlowTest, CompatibleRangesAreAccepted)
{
const auto input =
Dimensions::Requirement::scalar<double>(
Dimensions::NumericRange<double>::between(-1.0, 1.0));


const auto layerInput =
    Dimensions::Requirement::scalar<double>(
        Dimensions::NumericRange<double>::between(-2.0, 2.0));

const auto output =
    Dimensions::Requirement::scalar<double>();

Dimensions::DimensionFlow flow({input});

EXPECT_TRUE(
    flow.addLayer("layer", {layerInput}, output))
    << "A producer range contained within the required consumer range must be considered compatible.";


}

TEST(DimensionFlowTest, NarrowerRequiredRangeIsRejected)
{
const auto input =
Dimensions::Requirement::scalar<double>(
Dimensions::NumericRange<double>::between(-1.0, 1.0));


const auto layerInput =
    Dimensions::Requirement::scalar<double>(
        Dimensions::NumericRange<double>::between(-0.5, 0.5));

const auto output =
    Dimensions::Requirement::scalar<double>();

Dimensions::DimensionFlow flow({input});

EXPECT_FALSE(
    flow.addLayer("layer", {layerInput}, output))
    << "A required range narrower than the provided range must be considered incompatible.";


}

TEST(DimensionFlowTest, UnconstrainedRequiredInputAcceptsConstrainedOutput)
{
const auto input =
Dimensions::Requirement::scalar<double>();


const auto layerInput =
    Dimensions::Requirement::scalar<double>();

const auto output =
    Dimensions::Requirement::scalar<double>();

Dimensions::DimensionFlow flow({input});

EXPECT_TRUE(
    flow.addLayer("layer", {layerInput}, output))
    << "An unconstrained compatible data type requirement must accept the corresponding provided dimension.";


}

// ============================================================================
// Final output compatibility
// ============================================================================

TEST(DimensionFlowTest, FinalOutputCanBeCompatibleWithConsumer)
{
const auto input =
Dimensions::Requirement::array1d<double>(4);


const auto output =
    Dimensions::Requirement::scalar<double>(
        Dimensions::NumericRange<double>::between(-1.0, 1.0));

const auto consumer =
    Dimensions::Requirement::scalar<double>(
        Dimensions::NumericRange<double>::between(-2.0, 2.0));

Dimensions::DimensionFlow flow({input});

ASSERT_TRUE(
    flow.addLayer("layer", {input}, output))
    << "The layer must be successfully added before checking final output compatibility.";

EXPECT_TRUE(
    flow.isCompatibleWith(consumer))
    << "A final output whose range is contained by the consumer range must be compatible.";


}

TEST(DimensionFlowTest, FinalOutputCanBeIncompatibleWithConsumer)
{
const auto input =
Dimensions::Requirement::array1d<double>(4);


const auto output =
    Dimensions::Requirement::scalar<double>(
        Dimensions::NumericRange<double>::between(-2.0, 2.0));

const auto consumer =
    Dimensions::Requirement::scalar<double>(
        Dimensions::NumericRange<double>::between(-1.0, 1.0));

Dimensions::DimensionFlow flow({input});

ASSERT_TRUE(
    flow.addLayer("layer", {input}, output))
    << "The layer must be successfully added before checking incompatible final output.";

EXPECT_FALSE(
    flow.isCompatibleWith(consumer))
    << "A final output broader than the consumer's required range must be incompatible.";


}

TEST(DimensionFlowTest, FinalOutputMustHaveCompatibleType)
{
const auto input =
Dimensions::Requirement::array1d<double>(4);


const auto output =
    Dimensions::Requirement::scalar<double>();

const auto consumer =
    Dimensions::Requirement::scalar<float>();

Dimensions::DimensionFlow flow({input});

ASSERT_TRUE(
    flow.addLayer("layer", {input}, output))
    << "The layer must be successfully added before checking output type compatibility.";

EXPECT_FALSE(
    flow.isCompatibleWith(consumer))
    << "A final output with a different element type must be incompatible with the consumer.";


}

// ============================================================================
// Multiple-input flows
// ============================================================================

TEST(DimensionFlowTest, MultiInputFirstLayerChecksInputsPositionally)
{
const auto first =
Dimensions::Requirement::array1d<double>(4);


const auto second =
    Dimensions::Requirement::scalar<int>();

const auto output =
    Dimensions::Requirement::scalar<double>();

Dimensions::DimensionFlow flow({first, second});

EXPECT_TRUE(
    flow.addLayer("layer", {first, second}, output))
    << "Multiple layer inputs matching the corresponding flow inputs must be accepted.";


}

TEST(DimensionFlowTest, MultiInputFirstLayerRejectsReorderedInputs)
{
const auto first =
Dimensions::Requirement::array1d<double>(4);


const auto second =
    Dimensions::Requirement::scalar<int>();

const auto output =
    Dimensions::Requirement::scalar<double>();

Dimensions::DimensionFlow flow({first, second});

EXPECT_FALSE(
    flow.addLayer("layer", {second, first}, output))
    << "Layer inputs must be checked positionally and must not be accepted when their order is incompatible.";


}

TEST(DimensionFlowTest, MultiInputFirstLayerRejectsOneIncorrectInput)
{
const auto first =
Dimensions::Requirement::array1d<double>(4);


const auto second =
    Dimensions::Requirement::scalar<int>();

const auto wrongSecond =
    Dimensions::Requirement::scalar<float>();

const auto output =
    Dimensions::Requirement::scalar<double>();

Dimensions::DimensionFlow flow({first, second});

EXPECT_FALSE(
    flow.addLayer("layer", {first, wrongSecond}, output))
    << "A multi-input layer must be rejected when any one of its required inputs is incompatible.";


}

TEST(DimensionFlowTest, MultiInputLayerRejectsDifferentInputCount)
{
const auto first =
Dimensions::Requirement::array1d<double>(4);


const auto second =
    Dimensions::Requirement::scalar<int>();

const auto output =
    Dimensions::Requirement::scalar<double>();

Dimensions::DimensionFlow flow({first, second});

EXPECT_FALSE(
    flow.addLayer("layer", {first}, output))
    << "A layer with fewer inputs than the flow provides must be incompatible.";


}

// ============================================================================
// Multiple layers and output propagation
// ============================================================================

TEST(DimensionFlowTest, ThreeCompatibleLayersRemainValid)
{
const auto input =
Dimensions::Requirement::array1d<double>(4);


const auto output1 =
    Dimensions::Requirement::scalar<double>();

const auto output2 =
    Dimensions::Requirement::scalar<float>();

const auto output3 =
    Dimensions::Requirement::scalar<int>();

Dimensions::DimensionFlow flow({input});

ASSERT_TRUE(
    flow.addLayer("first", {input}, output1))
    << "The first compatible layer must be accepted.";

ASSERT_TRUE(
    flow.addLayer("second", {output1}, output2))
    << "The second compatible layer must consume the first layer output.";

ASSERT_TRUE(
    flow.addLayer("third", {output2}, output3))
    << "The third compatible layer must consume the second layer output.";

EXPECT_TRUE(
    flow.isValid())
    << "A flow containing only compatible layers must remain valid.";

EXPECT_EQ(
    flow.getOutputDimension(),
    output3)
    << "The final output must be the output dimension of the last layer.";


}

TEST(DimensionFlowTest, LastLayerDeterminesFinalOutput)
{
const auto input =
Dimensions::Requirement::array1d<double>(4);


const auto firstOutput =
    Dimensions::Requirement::scalar<double>();

const auto finalOutput =
    Dimensions::Requirement::array1d<int>(2);

Dimensions::DimensionFlow flow({input});

ASSERT_TRUE(
    flow.addLayer("first", {input}, firstOutput))
    << "The first layer must be accepted.";

ASSERT_TRUE(
    flow.addLayer("second", {firstOutput}, finalOutput))
    << "The second layer must be accepted.";

EXPECT_EQ(
    flow.getOutputDimension(),
    finalOutput)
    << "The final output dimension must always correspond to the last added layer.";


}

// ============================================================================
// Static acceptsRequirements
// ============================================================================

TEST(DimensionFlowTest, AcceptsRequirementsReturnsTrueForMatchingRequirements)
{
const auto requirement =
Dimensions::Requirement::array1d<double>(4);


EXPECT_TRUE(
    Dimensions::DimensionFlow::acceptsRequirements(
        {requirement},
        {requirement}))
    << "acceptsRequirements must return true when provided and required requirements are compatible.";


}

TEST(DimensionFlowTest, AcceptsRequirementsReturnsFalseForDifferentNumberOfRequirements)
{
const auto first =
Dimensions::Requirement::array1d<double>(4);


const auto second =
    Dimensions::Requirement::scalar<int>();

EXPECT_FALSE(
    Dimensions::DimensionFlow::acceptsRequirements(
        {first},
        {first, second}))
    << "acceptsRequirements must reject vectors with different numbers of requirements.";


}

TEST(DimensionFlowTest, AcceptsRequirementsReturnsFalseForIncompatibleRequirement)
{
const auto provided =
Dimensions::Requirement::array1d<double>(4);


const auto required =
    Dimensions::Requirement::array1d<float>(4);

EXPECT_FALSE(
    Dimensions::DimensionFlow::acceptsRequirements(
        {provided},
        {required}))
    << "acceptsRequirements must return false when corresponding requirements are incompatible.";


}

TEST(DimensionFlowTest, AcceptsRequirementsChecksEveryRequirement)
{
const auto first =
Dimensions::Requirement::array1d<double>(4);


const auto second =
    Dimensions::Requirement::scalar<int>();

const auto wrongSecond =
    Dimensions::Requirement::scalar<float>();

EXPECT_FALSE(
    Dimensions::DimensionFlow::acceptsRequirements(
        {first, second},
        {first, wrongSecond}))
    << "acceptsRequirements must reject the complete set when any corresponding requirement is incompatible.";


}

TEST(DimensionFlowTest, AcceptsRequirementsupportsMultipleCompatibleRequirements)
{
const auto first =
Dimensions::Requirement::array1d<double>(4);


const auto second =
    Dimensions::Requirement::scalar<int>();

EXPECT_TRUE(
    Dimensions::DimensionFlow::acceptsRequirements(
        {first, second},
        {first, second}))
    << "acceptsRequirements must support multiple corresponding compatible requirements.";


}

// ============================================================================
// Summary
// ============================================================================

TEST(DimensionFlowTest, EmptyFlowSummary)
{
const auto input =
Dimensions::Requirement::array1d<double>(4);


Dimensions::DimensionFlow flow({input});

const std::string expected =
    "=== Dimension Flow Summary ===\n"
    "\n"
    "Inputs (1):\n"
    "  * " + input.summary() + "\n"
    "\n"
    "Pipeline (0 layers):\n"
    "Overall: VALID\n";

EXPECT_EQ(
    flow.summary(),
    expected)
    << "The summary of a newly constructed DimensionFlow must describe its input dimensions and empty pipeline.";


}

TEST(DimensionFlowTest, SummaryIncludesLayerName)
{
const auto input =
Dimensions::Requirement::array1d<double>(4);


const auto output =
    Dimensions::Requirement::scalar<double>();

Dimensions::DimensionFlow flow({input});

ASSERT_TRUE(
    flow.addLayer("Normalize", {input}, output))
    << "The layer must be compatible before testing its representation in the summary.";

const std::string summary = flow.summary();

EXPECT_NE(
    summary.find("Layer 1: Normalize"),
    std::string::npos)
    << "The summary must contain the name of each layer.";


}

TEST(DimensionFlowTest, SummaryIncludesInputAndOutputInformation)
{
const auto input =
Dimensions::Requirement::array1d<double>(4);


const auto output =
    Dimensions::Requirement::scalar<double>();

Dimensions::DimensionFlow flow({input});

ASSERT_TRUE(
    flow.addLayer("Normalize", {input}, output))
    << "The layer must be compatible before checking its summary information.";

const std::string summary = flow.summary();

EXPECT_NE(
    summary.find("Input (0):"),
    std::string::npos)
    << "The summary must identify each layer input.";

EXPECT_NE(
    summary.find("Output:"),
    std::string::npos)
    << "The summary must identify each layer output.";


}

TEST(DimensionFlowTest, SummaryReportsCompatibleLayer)
{
const auto input =
Dimensions::Requirement::array1d<double>(4);


const auto output =
    Dimensions::Requirement::scalar<double>();

Dimensions::DimensionFlow flow({input});

ASSERT_TRUE(
    flow.addLayer("Normalize", {input}, output))
    << "The layer must be compatible before checking its compatibility status in the summary.";

const std::string summary = flow.summary();

EXPECT_NE(
    summary.find("Compatible: YES"),
    std::string::npos)
    << "The summary must report YES for a compatible layer.";


}

TEST(DimensionFlowTest, SummaryReportsIncompatibleLayer)
{
const auto input =
Dimensions::Requirement::array1d<double>(4);


const auto incompatibleInput =
    Dimensions::Requirement::array1d<float>(4);

const auto output =
    Dimensions::Requirement::scalar<double>();

Dimensions::DimensionFlow flow({input});

ASSERT_FALSE(
    flow.addLayer("Invalid", {incompatibleInput}, output))
    << "The layer must be incompatible before checking the summary's invalid-layer representation.";

const std::string summary = flow.summary();

EXPECT_NE(
    summary.find("Compatible: NO"),
    std::string::npos)
    << "The summary must report NO for an incompatible layer.";

EXPECT_NE(
    summary.find("[INCOMPATIBLE INPUT]"),
    std::string::npos)
    << "The summary must explicitly identify an incompatible input.";


}

TEST(DimensionFlowTest, SummaryReportsOverallValidState)
{
const auto input =
Dimensions::Requirement::array1d<double>(4);


const auto output =
    Dimensions::Requirement::scalar<double>();

Dimensions::DimensionFlow flow({input});

ASSERT_TRUE(
    flow.addLayer("Valid", {input}, output))
    << "The layer must be compatible before checking the overall summary status.";

const std::string summary = flow.summary();

EXPECT_NE(
    summary.find("Overall: VALID"),
    std::string::npos)
    << "The summary must report the overall flow as VALID when all layers are compatible.";


}

TEST(DimensionFlowTest, SummaryReportsOverallInvalidState)
{
const auto input =
Dimensions::Requirement::array1d<double>(4);


const auto incompatibleInput =
    Dimensions::Requirement::array1d<float>(4);

const auto output =
    Dimensions::Requirement::scalar<double>();

Dimensions::DimensionFlow flow({input});

ASSERT_FALSE(
    flow.addLayer("Invalid", {incompatibleInput}, output))
    << "The layer must be incompatible before checking the overall invalid summary status.";

const std::string summary = flow.summary();

EXPECT_NE(
    summary.find("Overall: INVALID"),
    std::string::npos)
    << "The summary must report the overall flow as INVALID when a layer is incompatible.";


}

TEST(DimensionFlowTest, SummarySupportsPrefix)
{
const auto input =
Dimensions::Requirement::array1d<double>(4);


Dimensions::DimensionFlow flow({input});

const std::string summary =
    flow.summary(">> ");

EXPECT_EQ(
    summary.rfind(">> === Dimension Flow Summary ===", 0),
    0u)
    << "The summary must apply the supplied prefix to its header.";


}

TEST(DimensionFlowTest, SummaryContainsCorrectLayerCount)
{
const auto input =
Dimensions::Requirement::array1d<double>(4);


const auto output1 =
    Dimensions::Requirement::scalar<double>();

const auto output2 =
    Dimensions::Requirement::scalar<float>();

Dimensions::DimensionFlow flow({input});

ASSERT_TRUE(
    flow.addLayer("First", {input}, output1))
    << "The first layer must be compatible.";

ASSERT_TRUE(
    flow.addLayer("Second", {output1}, output2))
    << "The second layer must be compatible.";

const std::string summary = flow.summary();

EXPECT_NE(
    summary.find("Pipeline (2 layers):"),
    std::string::npos)
    << "The summary must report the exact number of layers in the pipeline.";


}

// ============================================================================
// Integration with ActivationFunctions
// ============================================================================

TEST(DimensionFlowTest, CanRepresentTanhAsPipelineLayer)
{
const auto input =
Dimensions::Requirement::array1d<double>(4);


Dimensions::ActivationFunctions::Tanh<double> tanh(input);

Dimensions::DimensionFlow flow({input});

const bool compatible =
    flow.addLayer(
        tanh.name(),
        tanh.inputDimensions(),
        tanh.outputDimension());

EXPECT_TRUE(
    compatible)
    << "A Tanh activation function with matching input dimensions must be representable as a compatible DimensionFlow layer.";

EXPECT_TRUE(
    flow.isValid())
    << "A DimensionFlow containing a compatible Tanh layer must be valid.";

EXPECT_EQ(
    flow.getOutputDimension(),
    tanh.outputDimension())
    << "The DimensionFlow output must match the Tanh activation function output dimension.";


}

TEST(DimensionFlowTest, CanChainTanhIntoArgMax)
{
    const auto input =
    Dimensions::Requirement::array1d<double>(4);


    Dimensions::ActivationFunctions::Tanh<double> tanh(input);
    Dimensions::ActivationFunctions::ArgMax<double> argmax(
        tanh.outputDimension());

    Dimensions::DimensionFlow flow({input});

    ASSERT_TRUE(
        flow.addLayer(
            tanh.name(),
            tanh.inputDimensions(),
            tanh.outputDimension()))
        << "The Tanh layer must be compatible with the DimensionFlow input.";

    EXPECT_TRUE(
        flow.addLayer(
            argmax.name(),
            argmax.inputDimensions(),
            argmax.outputDimension()))
        << "ArgMax must be accepted when its input requirement matches the output dimension of the preceding Tanh layer.";
}
