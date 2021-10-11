/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2021) :
 *
 * Thomas Bourgoin <tbourgoi@insa-rennes.fr> (2021)
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

#ifdef CODE_GENERATION
#include <gtest/gtest.h>

#include "data/dataHandlerPrinter.h"
#include "data/primitiveTypeArray.h"
#include "data/primitiveTypeArray2D.h"

class DataHandlerPrinterTest : public ::testing::Test
{
    class Fake3DDataHandler : public Data::Array2DWrapper<double>
    {
      public:
        /// Inherited from DataHandler
        virtual std::vector<size_t> getDimensionsSize() const override
        {
            std::vector<size_t> sizes = {1, 2, 3};
            return sizes;
        };
    };

  protected:
    Data::DataHandlerPrinter* printer;
    Data::PrimitiveTypeArray<double>* array1D;
    Data::PrimitiveTypeArray2D<double>* array2D;
    Fake3DDataHandler* fake3D;
    const std::type_info& scalar = typeid(double);
    const std::type_info& array = typeid(double[3]);
    const std::type_info& matrix = typeid(double[2][2]);
    const std::type_info& operandArray2D = typeid(double[2][3]);
    std::string nameVar = "in1";

    virtual void SetUp()
    {
        array1D = new Data::PrimitiveTypeArray<double>(8);
        array2D = new Data::PrimitiveTypeArray2D<double>(5, 5);
        printer = new Data::DataHandlerPrinter();
        fake3D = new Fake3DDataHandler;
    }

    virtual void Teardown()
    {
        delete printer;
        delete array1D;
        delete array2D;
    }
};

TEST_F(DataHandlerPrinterTest, constructorDestructor)
{
    Data::DataHandlerPrinter* dp;
    ASSERT_NO_THROW(dp = new Data::DataHandlerPrinter())
        << "Construction of a DataHandlerPrinter with a Primitive1DArray "
           "failed.";

    ASSERT_NO_THROW(delete dp) << "Destruction failed.";
}

TEST_F(DataHandlerPrinterTest, print1DArray)
{
    std::string print;
    size_t start = 2;
    size_t size = 2;
    std::string expected = "{" + nameVar + "[" + std::to_string(start) + "], " +
                           nameVar + "[" + std::to_string(start + 1) + "]}";
    ASSERT_NO_THROW(print = printer->print1DArray(start, size, nameVar))
        << "Failed to extract a 1D array of size 2 from a 1D array of size 8.";
    ASSERT_EQ(print, expected)
        << "Error the array generated does not have the right format.";

    ASSERT_NO_THROW(print = printer->print1DArray(start, size, nameVar))
        << "Failed to extract a 1D array of size 2 from a 2D array of size "
           "5*5.";
    ASSERT_EQ(print, expected)
        << "Error the array generated does not have the right format.";
}

TEST_F(DataHandlerPrinterTest, print2DArray)
{
    std::string print;
    size_t start = 3;
    std::string expected = "{{" + nameVar + "[" + std::to_string(start) +
                           "], " + nameVar + "[" + std::to_string(start + 1) +
                           "]}, {" + nameVar + "[" + std::to_string(start + 5) +
                           "], " + nameVar + "[" +
                           std::to_string(start + 5 + 1) + "]}}";
    ASSERT_NO_THROW(print =
                        printer->print2DArray(start, {5, 5}, {2, 2}, nameVar))
        << "Failed to extract a 2D array at address 3 of size 2*2 from a 2D "
           "array of size 5*5.";
    ASSERT_EQ(print, expected)
        << "Error the array generated does not have the right format.";

    start = 4;
    expected = "{{in1[5], in1[6]}, {in1[10], in1[11]}}";
    ASSERT_NO_THROW(print =
                        printer->print2DArray(start, {5, 5}, {2, 2}, nameVar))
        << "Failed to extract a 2D array at address 4 of size 2*2 from a 2D "
           "array of size 5*5.";
    ASSERT_EQ(print, expected)
        << "Error the array generated does not have the right format.";
}

TEST_F(DataHandlerPrinterTest, printDataAt)
{
    std::string print;
    size_t start = 3;

    std::string expected = "[3] = {" + nameVar + "[" + std::to_string(5) +
                           "], " + nameVar + "[" + std::to_string(6) + "], " +
                           nameVar + "[" + std::to_string(7) + "]};";
    ASSERT_NO_THROW(print =
                        printer->printDataAt(*array2D, array, start, nameVar))
        << "Failed to extract a 1D array of size 2 at address 4 from a 2D "
           "array of size 5*5.";
    ASSERT_EQ(print, expected)
        << "Error the array generated does not have the right format.";

    start = 4;
    expected = "[2][2] = {{in1[5], in1[6]}, {in1[10], in1[11]}};";
    ASSERT_NO_THROW(print =
                        printer->printDataAt(*array2D, matrix, start, nameVar))
        << "Failed to extract a 2D array of size 2*2 at address 4 from a 2D "
           "array of size 5*5.";
    ASSERT_EQ(print, expected)
        << "Error the array generated does not have the right format.";

    expected = "[3] = {in1[6], in1[7], in1[8]};";
    ASSERT_NO_THROW(print =
                        printer->printDataAt(*array1D, array, start, nameVar))
        << "Failed to extract a 1D array  of size 3 at address 4 from a 2D "
           "array of size 5*5.";

    ASSERT_THROW(print = printer->printDataAt(*array1D, matrix, start, nameVar),
                 std::invalid_argument)
        << "Error should fail to extract a 2D array of size 2*2 at address 4 "
           "from a 1D array.";

    ASSERT_THROW(print = printer->printDataAt(*array1D, matrix, 15, nameVar),
                 std::invalid_argument)
        << "Error should fail to extract a 1D array  of size 3 at address 15 "
           "from a 2D "
           "array of size 5*5.";

    ASSERT_THROW(printer->printDataAt(*fake3D, array, start, nameVar),
                 std::invalid_argument)
        << "Error should fail to extract data from 3D DataHandler";
}

TEST_F(DataHandlerPrinterTest, getDemangleTemplateType)
{
    ASSERT_EQ(printer->getDemangleTemplateType(*array1D), "double")
        << "Fail to retrieve the type (in a human readable format) of the "
           "PrimitiveTypeArray inside the DataHandlerPrinter.";

    ASSERT_EQ(printer->getDemangleTemplateType(*array2D), "double")
        << "Fail to retrieve the type (in a human readable format) of the "
           "PrimitiveTypeArray2D inside the DataHandlerPrinter.";
}

TEST_F(DataHandlerPrinterTest, getOperandSizes)
{
    ASSERT_EQ(Data::DataHandlerPrinter::getOperandSizes(scalar),
              std::vector<size_t>{});
    std::vector<size_t> size = {3};
    ASSERT_EQ(Data::DataHandlerPrinter::getOperandSizes(array), size);
    size = {2, 2};
    ASSERT_EQ(Data::DataHandlerPrinter::getOperandSizes(matrix), size);
    size = {2, 3};
    ASSERT_EQ(Data::DataHandlerPrinter::getOperandSizes(operandArray2D), size);
}
#endif // CODE_GENERATION
