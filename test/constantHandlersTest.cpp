/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2019 - 2020) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2019 - 2020)
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

#include <gtest/gtest.h>

#include "data/constantHandler.h"
#include "data/primitiveTypeArray.h"

TEST(ConstantHandlersTest, Constructor)
{
    ASSERT_NO_THROW({
        Data::ConstantHandler* d = new Data::ConstantHandler(5);
        delete d;
    }) << "Call to ConstantHandler constructor failed.";
}

TEST(ConstantHandlersTest, ID)
{
    Data::ConstantHandler d0(5);
    Data::ConstantHandler d1(5);

    ASSERT_NE(d0.getId(), d1.getId())
        << "Id of two ConstantHandlers created one "
           "after the other should not be equal.";
}

TEST(ConstantHandlersTest, ConstantHandlerCanProvideTemplateType)
{
    Data::DataHandler* d = new Data::ConstantHandler(4);
    Data::DataHandler* d2 = new Data::ConstantHandler(0);

    ASSERT_FALSE(d->canHandle(typeid(int32_t)))
        << "ConstantHandler wrongfully say it can provide "
           "32 bit int data.";
    ASSERT_TRUE(d->canHandle(typeid(Data::Constant)))
        << "ConstantHandler wrongfully say it can not provide "
           "\"Data::Constant\" "
           "data.";
    ASSERT_FALSE(d->canHandle(typeid(Data::UntypedSharedPtr)))
        << "ConstantHandler wrongfully say it can provide "
           "UntypedSharedPtr data.";
    ASSERT_FALSE(d->canHandle(typeid(float)))
        << "ConstantHandler wrongfully say it can provide "
           "float data.";
    ASSERT_FALSE(d->canHandle(typeid(double)))
        << "ConstantHandler wrongfully say it can provide "
           "double data.";
    ASSERT_TRUE(d->canHandle(typeid(Data::Constant[3])))
        << "ConstantHandler wrongfully say it can not provide "
           "dataConstant array.";
    delete d;
    delete d2;
}