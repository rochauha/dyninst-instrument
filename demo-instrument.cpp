#include <iostream>

#include "BPatch.h"
#include "BPatch_addressSpace.h"
#include "BPatch_binaryEdit.h"
#include "BPatch_flowGraph.h"
#include "BPatch_function.h"
#include "BPatch_point.h"

using namespace std;
using namespace Dyninst;

void insertBasicBlockCounters(BPatch_function *function) {
  BPatch_addressSpace *addressSpace = function->getAddSpace();

  BPatch_flowGraph *cfg = function->getCFG();

  BPatch_Set<BPatch_basicBlock *> basicBlockSet;
  cfg->getAllBasicBlocks(basicBlockSet);
  assert(!basicBlockSet.empty());

  std::vector<BPatch_basicBlock *> basicBlocks;
  basicBlockSet.elements(basicBlocks);
  assert(basicBlocks.size() == basicBlockSet.size());

  std::cout << "number of basic blocks = " << basicBlocks.size() << '\n';

  std::vector<BPatch_variableExpr *> bbCounters;
  std::vector<BPatch_point *> bbEntryPoints;

  for (int i = 0; i < basicBlocks.size(); ++i) {
    BPatch_point *entryPoint = basicBlocks[i]->findEntryPoint();
    assert(entryPoint);
    bbEntryPoints.push_back(entryPoint);

    // now create a counter for this
    std::string varName = std::string("block-") + std::to_string(i);
    BPatch_variableExpr *counter = addressSpace->malloc(4, varName);

    std::cerr << "mutator : created variable " << varName << '\n';

    assert(counter);
    bbCounters.push_back(counter);
  }

  assert(basicBlocks.size() == bbCounters.size() &&
         basicBlocks.size() == bbEntryPoints.size());

  // Create an assignment counter = counter + 1 for each basic block counter
  // Insert correspnonding assignment at beginning of the basic block
  for (int i = 0; i < basicBlocks.size(); ++i) {
    BPatch_constExpr one(0x1);
    BPatch_arithExpr addExpr(BPatch_plus, *(bbCounters[i]), one);
    BPatch_arithExpr assignExpr(BPatch_assign, *(bbCounters[i]), addExpr);

    BPatchSnippetHandle *handle = addressSpace->insertSnippet(
        assignExpr, *(bbEntryPoints[i]), BPatch_callBefore);
    assert(handle);
  }
}

int main(int argc, char **argv) {
  assert(argc == 2);

  BPatch BPatch;
  const char *binaryPath = argv[1];

  BPatch_binaryEdit *binary = BPatch.openBinary(binaryPath);
  assert(binary);

  BPatch_image *binaryImage = binary->getImage();
  assert(binaryImage);

  BPatch_Vector<BPatch_function *> functions;
  assert(binaryImage->getProcedures(functions));

  for (auto *function : functions) {
    insertBasicBlockCounters(function);
  }

  std::string newPath(std::string(argv[1]) + std::string("-instr"));
  if (!binary->writeFile(newPath.c_str())) {
    std::cout << "Rewriting binary failed\n";
  }
}
