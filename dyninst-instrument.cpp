#include <cassert>
#include <iostream>

#include "BPatch.h"
#include "BPatch_addressSpace.h"
#include "BPatch_binaryEdit.h"
#include "BPatch_function.h"
#include "BPatch_point.h"
#include "BPatch_process.h"

using namespace std;
using namespace Dyninst;

void createAndInsertMulSnippet(BPatch_point *point) {
  std::vector<BPatch_register> liveRegs;
  if (!point->getLiveRegisters(liveRegs)) {
    std::cout << "No registers available for instrumentation.\n";
    return;
  }

  std::cout << "#live regs : " << liveRegs.size() << '\n';
  BPatch_register r1 = liveRegs[0];
  BPatch_register r2 = r1;
  std::cout << "r1 = " << r1.name() << '\n';
  std::cout << "r2 = " << r2.name() << '\n';

  BPatch_registerExpr op1(r1);
  BPatch_constExpr op2(0xabc);
  // BPatch_reisterExpr op2(r2);
  BPatch_arithExpr mulExpr(BPatch_times, op1, op2);

  BPatch_addressSpace *addressSpace = point->getAddressSpace();
  BPatchSnippetHandle *handle = addressSpace->insertSnippet(mulExpr, *point);

  if (!handle) {
    std::cout << "couldn't insert snippet\n";
  }
}

void insertMulSnippets(BPatch_image *binaryImage,
                      std::vector<BPatch_point *> &insertionPoints) {
  for (size_t i = 0; i < insertionPoints.size(); ++i) {
    createAndInsertMulSnippet(insertionPoints[i]);
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
    std::vector<BPatch_point *> *entryPoints = function->findPoint(BPatch_entry);
    insertMulSnippets(binaryImage, *entryPoints);
  }

  std::string newPath = std::string(argv[1]) + "-instr";
  if (!binary->writeFile(newPath.c_str())) {
    std::cout << "Rewriting binary failed\n";
  }
}
