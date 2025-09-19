#include <iostream>

#include "BPatch.h"
#include "BPatch_addressSpace.h"
#include "BPatch_binaryEdit.h"
#include "BPatch_flowGraph.h"
#include "BPatch_function.h"
#include "BPatch_point.h"

using namespace std;
using namespace Dyninst;

void insertEndProgramCounter(BPatch_function *function) {
  // if (function->getDemangledName() != "foo")
    // return;

  BPatch_addressSpace *addressSpace = function->getAddSpace();
  std::vector<BPatch_point *> exitPoints;
  function->getExitPoints(exitPoints);

  BPatch_variableExpr *counter = addressSpace->malloc(4, "endpgm_counter");
  BPatch_constExpr one(0x1);
  BPatch_arithExpr addExpr(BPatch_plus, *counter, one);
  BPatch_arithExpr assignExpr(BPatch_assign, *counter, addExpr);

  BPatchSnippetHandle *handle =
    addressSpace->insertSnippet(assignExpr, exitPoints, BPatch_callAfter);
  assert(handle);
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
    insertEndProgramCounter(function);
  }

  std::string newPath(std::string(argv[1]) + std::string("-instr"));
  if (!binary->writeFile(newPath.c_str())) {
    std::cout << "Rewriting binary failed\n";
  }
}
