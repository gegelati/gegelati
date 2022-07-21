# Information

This folder adds the required file to compile the unit tests for the class TPGGenerationEngine. Each test needs a dedicated main file, a CMakeLists.txt and links with the header _externalHeader_. Some tests require Data stored into a CSV. The first value is the expected action returned by the TPG, other values of a line correspond to the input data of the TPG.

## Main files :

Depending on the test, main files can do several things : 

- If the TPG generated has one leaf there is only one path to check. In this case no CSV file is required hence the main file reads the input data given as argument when the executable is called.
- If we do not know the list of input value that we want to test the main file reads the input data given as argument when the executable is called. This is the case for the tests StickGame and TicTacToe where we compare the inference with the code gen and gegelati.
- If the TPG has several leaves, the main file is able to read a CSV file. The filename is given as argument when the executable is called. The CSV file contain a list of input value to check to test all the possible path of the generated TPG.

## CSV files

They store several tests vectors to check all the possibles path for a TPG.
Each line of a CSV file correspond to a test vector construct as follow.
The first element of the line is the expected integer returned after the execution of the TPG.
All the other elements of the line are the values given as input to the TPG programs.

## Golden references files

Source and header files compared with the generated files to check if the generated files are correct.

## CMakeLists.txt

These files are almost all the same, they allow to compile the generated code with the main files. Their main difference is the name of their variable ```target```. Some also include the directory with the CSV parser or link with math.h if necessary.

## Test purposes
### OneLeafNoInstruction
This test checks if the structure of the generated file is correctly generated : the function that execute the teams, the management of the stack of visited edges ...

### OneLeaf
This test checks if a simple TPG with 1 root and 1 leaf can be generated and executed. 

### TwoLeaves
This test checks if the choice between 2 destinations is correct. The two destinations are leaves in this test.

### ThreeLeaves
This test checks more conditions to choose the destination, one of the is if the bid of the program P2 executed is equal to the actual max result we follow the edge that contain P1.

### OneTeamOneLeaf
This test is composed of 1 root, 1 team (destination of the root) and 1 leaf. It checks if a non root team is properly executed. 

### OneTeamTwoLeaves
This test is composed of 1 root, 1 team (destination of the root) and 2 leaves. It checks if a non root team do the right choice between two leaves. 

### TwoTeamsOneCycle
This test is composed of 1 root, 2 team (destination of the root) and 2 leaves. It checks if a cycle is correctly handle by a generated TPG. 

### TwoTeamsOneCycleNegativeBid
This test is composed of 1 root, 2 team (destination of the root) and 3 leaves. It checks if a cycle is correctly handle by a generated TPG when some program bids can be negative.

### ThreeTeamsOneCycleThreeLeaves
This test is composed of 1 root, 3 team (destination of the root) and 3 leaves. It checks if a more complex cycle is correctly handle by a generated TPG. 
