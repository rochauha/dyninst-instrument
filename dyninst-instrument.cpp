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

// Certain snippets can't be modeled on a GPU as we can't allocate temporaries
// on GPU. So only play with snippets that can be modeled on both CPU and GPU,
// and hack on X86 code generator appropriately. Basic snippet for initial
// testing: Pick an available register and multiply by 0xabc
//
void insertMulSnippet(BPatch_image *binaryImage,
                      std::vector<BPatch_point *> &insertionPoints) {
  std::vector<BPatch_register> availableRegs;
  BPatch_addressSpace *addressSpace = binaryImage->getAddressSpace();

  bool gotRegs = addressSpace->getRegisters(availableRegs);
  if (!gotRegs) {
    std::cout << "No dead registers available for instrumentation.\n";
    std::cout << "Exitting...\n";
    exit(1);
  }

  // create add snippet using the first available register
  BPatch_register r1 = availableRegs[0];
  BPatch_register r2 = availableRegs[1];

  // if (availableRegs.size() > 1)
  //   r2 = availableRegs[1];

  std::cout << "r1 = " << r1.name() << '\n';
  std::cout << "r2 = " << r2.name() << '\n';

  BPatch_registerExpr op1(r1);
  BPatch_registerExpr op2(r2);
  // BPatch_constExpr op2(0xabc); // TODO : change this to reg operand for SOP2
  BPatch_arithExpr mulExpr(BPatch_times, op1, op2);

  bool success = addressSpace->insertSnippet(mulExpr, insertionPoints);
  if (!success) {
    std::cout << "snippet insertion failed\n";
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
    insertMulSnippet(binaryImage, *entryPoints);
  }

  std::string newPath = std::string(argv[1]) + "-instr";
  if (!binary->writeFile(newPath.c_str())) {
    std::cout << "Rewriting binary failed\n";
  }
}
