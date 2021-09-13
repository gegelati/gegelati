/**
* This file is a template that can be filled to connect C code generated with
* the code generation feature from Gegelati with an
* AdversarialLearningEnvironment AND (optionally) to compare its behaviour
* with the TPG from Gegelati, imported from its dot format.
*
* To use this template, the following changes should be made, carefully read
* and replace all comments in this file.
*/

#include <iostream>

extern "C" {
/// include the header created by the CodeGen (only the one without _program.h is required)
// ex stickGame : #include "cmake-build-debug/Stick_game.h"

}
/// include the header with your learning environment
// ex stickGame : #include "StickGame/stickGameAdversarial.h"

/// instantiate global variable used to communicate between the TPG and the environment
/**type**/* in1;
//type in2; if required in3...

int main(){
    /// Import instruction set used during training(required only for gegelati Inference)
    Instructions::Set set;

    /// initialise your AdversarialLearningEnvironment le
    // ex stickGame : auto le = StickGameAdversarial();

    /// Instantiate an Environment and import (required only for gegelati Inference)
    Environment env(set, le.getDataSources(), /**number of register**/, /** number of constant**/);
    auto tpg = TPG::TPGGraph(env);
    TPG::TPGExecutionEngine tee(env);

    /// Import your trained TPG from the .dot file.
    File::TPGGraphDotImporter dotImporter(/**exported_TPG.dot **/, env, tpg);
    dotImporter.importGraph();
    auto root2 = tpg.getRootVertices().front();

    /// fetch data in the environment
    auto& st = le.getDataSources().at(0).get();
    in1 = st.getDataAt(typeid(/**type in1**/), 0).getSharedPointer</**type in1**/>().get();
//    auto& st2 = le.getDataSources().at(1).get();
//    in2 = st2.getDataAt(typeid(/**type in2**/), 0).getSharedPointer</**type in2**/>().get();

    /// set the number of game
    size_t nbGame = 1;
    uint64_t action;
    int playerNb = 0;

    /// Uncomment the following line (and the one in the loop) if your LearningEnvironment can be convert to a string
    //std::cout<<"Game :\n"<<le.toString()<<std::endl;

    // let's play, the only way to leave this loop is finish all game
    while(nbGame!= 0){
        // gets the action the TPG would decide in this situation
        /// to use gegelati Inference uncomment the following line
//        action=((const TPG::TPGAction *) tee.executeFromRoot(* root).back())->getActionID();

        /// to use inference with generated C files uncomment the 2 following lines
        action = executeFromVertex(root);
        reset();

        std::cout<<"TPG : "<<action<<std::endl;
        le.doAction(action);
        playerNb = !playerNb;
        /// prints the game board
        // std::cout<<"Game :\n"<<le.toString()<<std::endl;

        if(le.isTerminal()){
            std::cout<<"TPG nb" << playerNb <<" won !"<<std::endl;
            std::cout<<"Reseting game..."<<std::endl;
            le.reset();
            nbGame--;
            continue;
        }
    }
}