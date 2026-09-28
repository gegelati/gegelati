/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2019 - 2025) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2019 - 2022)
 * Mickaël Dardaillon <mdardail@insa-rennes.fr> (2022)
 * Quentin Vacher <qvacher@insa-rennes.fr> (2025)
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

//#ifdef CODE_GENERATION

#include "codeGen/codeGenerationExporter.h"
#include "data/demangle.h"
#include "util/timestamp.h"

CodeGen::CodeGenerationExporter::CodeGenerationExporter(const std::string& filename, std::vector<std::string> subAlgoNames,
                                                  const std::string& path)
    : AbstractEngine()
{

    this->fileMain.open(path + filename + ".c", std::ofstream::out);
    this->fileMainH.open(path + filename + ".h", std::ofstream::out);
    if (!fileMain.is_open() || !fileMainH.is_open()) {
        throw std::runtime_error("Error can't open " +
                                 std::string(path + filename) + ".c or " +
                                 std::string(path + filename) + ".h");
    }

    fileMain << "/**\n"
             << " * File generated with GEGELATI v" GEGELATI_VERSION "\n"
             << " * On the " << Util::getCurrentDate() << "\n"
             << " * With the " << DEMANGLE_TYPEID_NAME(typeid(*this).name())
             << ".\n"
             << " */\n\n";

    fileMain   << "#include \"" << filename << ".h\"\n";
    for(std::string subAlgoName: subAlgoNames) {
        fileMain  << "#include \"" << subAlgoName << ".h\"\n";
    }

    fileMain 
        << "#include <limits.h>\n"
        << "#include <assert.h>\n"
        << "#include <float.h>\n"
        << "#include <stdbool.h>\n"
        << "#include <stdio.h>\n"
        << "#include <stdint.h>\n"
        << "#include <math.h>\n" << std::endl;

    fileMainH << "/**\n"
              << " * File generated with GEGELATI v" GEGELATI_VERSION "\n"
              << " * On the " << Util::getCurrentDate() << "\n"
              << " * With the " << DEMANGLE_TYPEID_NAME(typeid(*this).name())
              << ".\n"
              << " */\n\n";
    fileMainH << "#ifndef C_" << filename << "_H" << std::endl;
    fileMainH << "#define C_" << filename << "_H\n" << std::endl;
    fileMainH << "#include <stdlib.h>\n\n" << std::endl;
};

CodeGen::CodeGenerationExporter::~CodeGenerationExporter()
{
    fileMainH << "\n#endif" << std::endl;
    fileMain.close();
    fileMainH.close();
}


void CodeGen::CodeGenerationExporter::initActivationFunction(const Representation::Representation& representation)
{
    // TODO NEED UPDATE FOR CUSTOM RANGES
    if(representation.getOutputs().sizeContinuous() == 0) {
        throw std::runtime_error("CodeGenerationExporter::initActivationFunction: should not be called if no continuous actions");
    } 

    fileMainH << "void activationFunction_"<< representation.getRepresentationName() << representation.getRepresentationID() <<"(double *outputs);\n";

    fileMain << "void activationFunction_"<< representation.getRepresentationName() << representation.getRepresentationID() <<"(double *outputs) {\n"
             << "\tfor (size_t i = 0; i < "
             << representation.getOutputs().sizeContinuous()
             << "; i++) {\n"
             << "\t\tif(isnan(outputs[i])) outputs[i] = -INFINITY;\n\n";

    if (representation.getParams().activationFunction == "none") {
        fileMain << "\t\tif (outputs[i] > 1.0) outputs[i] = 1.0;\n"
                 << "\t\telse if (outputs[i] < -1.0) outputs[i] = -1.0;\n";
    }

    else if (representation.getParams().activationFunction ==
             "sigmoid") {
        fileMain << "\t\toutputs[i] = 1.0 / (1.0 + exp(-outputs[i]));\n";
    }

    else if (representation.getParams().activationFunction ==
             "tanh") {
        fileMain << "\t\toutputs[i] = tanh(outputs[i]);\n";
    }
    else {
        throw std::runtime_error("Activation function for codeGen not found.");
    }

    fileMain << "\t}\n}\n" << std::endl;
}

void CodeGen::CodeGenerationExporter::exportMainIndividual(const Representation::Individual& individual, const Representation::Representation& representation, std::map<uint64_t, std::set<std::reference_wrapper<const Representation::Individual>>>& subIndividuals) 
{
    
    fileMainH 
        << "void inference(double* actions);\n" << std::endl;

    fileMain
        << "\n"
        << "void inference(double* actions) {\n"
        << "\t"<<representation.getRepresentationName() << representation.getRepresentationID() << "_" << individual.getIndividualID()<<"(actions);\n";


    // If do need activation function
    if(representation.getOutputs().sizeContinuous() > 0) {
        bool doActivationFunction = true;
        for(const Output::Output& output: representation.getOutputs().getContinuousOutputs()) {
            if(output.getRangeMin() == - std::numeric_limits<double>::infinity() || output.getRangeMax() == std::numeric_limits<double>::infinity()) {
                doActivationFunction = false;
            }
        }

        if(doActivationFunction) {
            fileMain 
                << "\tactivationFunction_"<< representation.getRepresentationName() << representation.getRepresentationID() <<"(actions);\n"
                << "}\n" << std::endl;
            this->initActivationFunction(representation);
        } else {
            fileMain 
                << "}\n" << std::endl;
        }
    } else {
        fileMain 
            << "}\n" << std::endl;
    }

    fileMainH
        << "void " << representation.getRepresentationName() << representation.getRepresentationID() << "_" << individual.getIndividualID() << "(double* outputs);\n";

    // Print the individual by calling representation.printIndividual
    std::set<std::reference_wrapper<const Representation::Individual>> individuals{individual};
    representation.printCodeGenIndividuals(fileMain, fileMainH, individuals, subIndividuals);
}


void CodeGen::CodeGenerationExporter::exportIndividuals(std::set<std::reference_wrapper<const Representation::Individual>> individuals, const Representation::Representation& representation, std::map<uint64_t, std::set<std::reference_wrapper<const Representation::Individual>>>& subIndividuals) 
{

    // If do need activation function
    if(representation.getOutputs().sizeContinuous() > 0) {
        bool doActivationFunction = true;
        for(const Output::Output& output: representation.getOutputs().getContinuousOutputs()) {
            if(output.getRangeMin() == - std::numeric_limits<double>::infinity() || output.getRangeMax() == std::numeric_limits<double>::infinity()) {
                doActivationFunction = false;
            }
        }

        if(doActivationFunction) {
            this->initActivationFunction(representation);
        }
    }

    for(const Representation::Individual& individual: individuals) {
        fileMainH
            << "void " << representation.getRepresentationName() << representation.getRepresentationID() << "_" << individual.getIndividualID() << "(double* outputs);\n";
    }
    fileMainH << std::endl;

    representation.printCodeGenIndividuals(fileMain, fileMainH, individuals, subIndividuals);
}

//#endif // CODE_GENERATION
