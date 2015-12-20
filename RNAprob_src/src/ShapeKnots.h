/*
 * ShapeKnots, a program that predicts RNA secondary structures with pseudoknots.
 *
 * (c) 2013 
 * Mathews Lab, University of Rochester Medical Center
 * Weeks Lab, The University at North Carolina at Chapel Hill
 * Code contributors: Wayne Higgins, Stanislav Bellaousov, David H. Mathews
 */

#include "pkHelix.h"
#include "PseudoParser.h"
#include <iomanip>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>
#include <sstream>


#include "../src/rna_library.h"
#include "../src/structure.h"
#include "../src/algorithm.h"
#include "../RNA_class/RNA.h"
#include "../src/ParseCommandLine.h"

//using namespace std;

void addtoAggregate(structure * folded, structure * psa, int r);
	//Function that takes a folded structure (after call to dynamic) and adds it to the final list of folded structures
	//folded is the newly folded structure
	//psa is a pointer to the final structure and is equal to pseudoStructAggregate.
	//NOTE!!! that only the first structure is added to the list, no matter how many tracebacks there are

void pseudoknotFold(pkHelix &pknot, RNA * st, RNA * psa, int energyPrune, datatable *data, int maxtracebacks, int percent, int window, int &numstructures, double P1, double P2, double Ss, double Si, string DMSFile, string SHAPEFile, double DSs, string DSHAPEFile, string doubleOffsetFile);
//Function that removes a helix (forces all nucleotides in that helix to be single stranded) from an RNA, folds the RNA, and then 
//	adds the helix back IF the addition of the helix to the folded RNA lowers the overall energy of the RNA below the energy of the
//	pseudoknot-free minimum free energy structure. 
//pknot is a pointer to the helix to be removed from the RNA (forced to be single stranded).
//st is a pointer to the RNA structure object that will be folded.
//psa is a pointer to the RNA structure object that will hold the pseudoknot free minimum free energy structure and 
//	all folded RNA structures that contain a pseudoknot.  This is equivalent to pseudoStructAggregate.
//energyPrune is the total energy of the minimum free energy structure
//data is a pointer to a datatable object (defined by DHM, needed for the call to Dynamic)
//maxtracebacks defines number of sub-optimal structures to be included in the dynamic function
//percent defines how close in energy sub-optimal structures can be
//numstructures indicates how many RNAs are in psa

//Print the file with helices
void printhelixListtoFile(vector<pkHelix> pkhelixList);

void printPseudoknotList(RNA* pseudoStructAggregate);
	//Function that checks through each of the final folded structures for the presence of pseuodoknots.  Checks by iterating through
	//	all helices looking for the condition that satisfies i<i'<j<j', where i is paired to j, and i' is paired to j'.
	//This algorithm is not exhaustive in that a helix might have interleaved basepairs with more than one helix.  Therefore,
	//	manual inspection of any structure which contains interleaved basepairs will have to be examined.
	//pseudoStructAggregate is the final list of structures, each structure is individually checked.

void convertToHundredHelices(vector<pkHelix> &pkHelixList, int finallistSize);
	//Function that "trims" the list of possible pseudoknots to the 100 most energetically stable (lowest energy).  Initial list can
	//	be a few to several thousand possibilities depending on the size of the RNA.
	//pkHelixList is a pointer to the list of helices, a list of pkHelix objects.
	//pkHelisListSize is the current size of the list of helices

void findhelix(int **arrayPointer, vector<pkHelix>  &pkHelixList, int x, int y, datatable *data, structure *ct);
	//Function to find helices in the 2-d array generated fill.	 
	//arrayPointer is the pointer to the 2-d array.
	//pkHelixList is a pointer to the list of helices, a list of pkHelix objects.
	//pkHelisListSize is the current size of the list of helices
	//x and y are coordinates in the 2-d array
	//data is a pointer to a data object defined by DHM, passed to the pkHelix constructor and ultimately used to calculate the energy of the new helix
	//ct is a pointer to a structure object defined by DHM, passed to the pkHelix constructor and ultimately used to calculate the energy of the new helix
	//This function is a little awkward.  Takes a cell (x,y) identified to hold a base pair in the defineHelixList function and looks to see if there are more
	//	basepairs on the diagonal with the same energy, indicating a helix.	 Only helices greater than 2nts and less than 10 nts are considered to be possible pseudoknots.

void defineHelixList(int **arrayPointer, vector<pkHelix> &pkHelixList, int & maxsize, int lowvalue, datatable *data, structure *ct);
	//Function that iterates through the 2-d array generated by the fill function, and looks for structures with total energy within 25% of the pseudoknot-free minimum free energy structure.
	//arrayPointer is the pointer to the 2-d array.
	//pkHelixList is a pointer to the list of helices, a list of pkHelix objects.
	//pkHelisListSize is the current size of the list of helices
	//data is a pointer to a data object defined by DHM, passed to findhelix function and then the pkHelix constructor and ultimately used to calculate the energy of the new helix
	//ct is a pointer to a structure object defined by DHM, passed to findhelix function and then the pkHelix constructor and ultimately used to calculate the energy of the new helix
	//max size is the maximum size of the list of helices -	 resized as needed
	//lowvalue is the energy of the pseudoknot-free minimum free energy structure.	This was calculated in fill and stored when the 2-d array was built

void convertStructureToHelixList(structure *st, datatable *data, vector<pkHelix> &structureHelices);
	//Function that converts the pseudoknot-free MFE (OR ANY STRUCTURE) to a list of helices.  This function uses a pointer to a list of pkHelix objects 
	//	to hold the list.  This list will use this to compare MFE helices to proposed pseudoknotted structure to look for false positives
	//	Only looks at lowest energy structure of each traceback, will only report first structure anyway
	//st is a pointer to a structure object, holds all data associated with an RNA structure 
	//data is a pointer to datatable object 
	//structureHelices is a pointer to the list of helices in the pk-free MFE
	//structureHelicesSize is the number of helices in the list

void comparepkHelixListToMFE(vector<pkHelix> &pkHelixList, structure *ct, vector<pkHelix> &MFEhelixList);
	//Function that removes any helices from the list of possible pseudoknots that will significantly repair the pseudknot-free MFE.
	//this is simply an empirically derived method to reduce the number of false positives in the prediction
	//pkHelixList is a pointer to the list of helices, a list of pkHelix objects. tHIS IS THE LIST OF POSSIBLE PSEUDOKNOTS
	//pkHelisListSize is the current size of the list of possible pseudoknots
	//ct is a pointer to a structure object, holds all data associated with an RNA structure (programmed by DHM)
	//data is a pointer to datatable object (programmed by DHM)
	//MFEhelixlist is a pointer to a list of helices, a list of pkHelix objects.  THIS IS THE LIST OF HELICES IN THE PK-FREE MFE.
	//MFEhelixListSize is the number of helices in the pk-free MFE

//Check for duplicate structures in a structure class and remove them.
void checkForDuplicates(structure* &pseudoStructAggregate);
	//function that checks the final folded list of structures to ensure that no structure is duplicated
	//pseudostructAggregate is a pointer to the final list of folded structures.

void copyStructure(structure * ct, structure* &pseudoStructAggregate, string DMSFile, string SHAPEFile);
	//Function that initializes pseudoStructAggregate to have some of the same attributes as the original structure ct
	//ct is allocated in main, and reads in the initial data
	//pseudoStructAggregate is a pointer to a structure that holds the final list of folded structures

void getdat(char *loop, char *stackf, char *tstackh, char *tstacki,
            char *tloop, char *miscloop, char *danglef, char *int22,
            char *int21,char *coax, char *tstackcoax,
            char *coaxstack, char *tstack, char *tstackm, char *triloop,
            char *int11, char *hexaloop, char *tstacki23, char *tstacki1n,
            char *datapath, bool isRNA);
//Function gets the names of data files to open

void pseudoknot(RNA* rnaCT, datatable *data, int maxtracebacks, int percent, int windowsize, string ctoutput, double P1, double P2, double Ss, double Si, string DMSFile, string SHAPEFile, double DSs, string DSHAPEFile, string doubleOffsetFile, int OutPercent, int OutWindowSize, int OutMaxStructures, bool ifWindowOptions, int finallistSize);
//Function that drives the rest of the code.  This function is called in main after the sequence file, shape file and shape parameters are
//	are read.
//ct is a pointer to a structure object, holds all data associated with an RNA structure (programmed by DHM)
//data is a pointer to datatable object (programmed by DHM)
//maxtracebacks defines number of sub-optimal structures to be included in the dynamic function
//percent defines how close in energy sub-optimal structures can be
//ctoutput is the name of the file where results are printed
//SHAPEslope and SHAPEintercept are the SHAPE parameters
	
