#include <cassert>
#include <iostream>

#include "BPatch.h"
#include "BPatch_addressSpace.h"
#include "BPatch_binaryEdit.h"
#include "BPatch_function.h"
#include "BPatch_point.h"
#include "BPatch_process.h"
// #include "registers/MachRegister.h"

using namespace std;
using namespace Dyninst;

enum SnippetKind {
  MulSnippet,
  LoadSnippet,
  StoreSnippet,
  IfSnippet,
  WhileSnippet,
};

void insertMulSnippet(BPatch_point *point) {
  BPatch_addressSpace *addressSpace = point->getAddressSpace();
  std::vector<BPatch_register> allRegs;
  assert(addressSpace->getRegisters(allRegs) && "Must get all regs");

  BPatch_register r1 = allRegs[0];

  BPatch_registerExpr op1(r1);
  BPatch_constExpr op2(0xabc);
  BPatch_arithExpr mulExpr(BPatch_times, op1, op2);

  BPatchSnippetHandle *handle = addressSpace->insertSnippet(mulExpr, *point);

  if (!handle) {
    std::cout << "couldn't insert snippet\n";
  }
}

void insertLoadSnippet(BPatch_point *point) {}

void insertStoreSnippet(BPatch_point *point) {}

void insertIfSnippet(BPatch_point *point) {}

void insertWhileSnippet(BPatch_point *point) {}

SnippetKind getSnippetKind(const char *str) {
  if (std::string(str) == "-mul")
    return MulSnippet;
  else if (str == "-load")
    return LoadSnippet;
  else if (str == "-store")
    return StoreSnippet;
  else if (str == "-if")
    return IfSnippet;
  else if (str == "-while")
    return WhileSnippet;
  else {
    std::cerr << "Invalid snippet kind\n";
    exit(2);
  }
}

void insertSnippet(SnippetKind sk,
                   std::vector<BPatch_point *> &insertionPoints) {
  // The compiler will probably do loop switching here :P
  for (size_t i = 0; i < insertionPoints.size(); ++i) {
    switch (sk) {
    case MulSnippet: {
      insertMulSnippet(insertionPoints[i]);
      break;
    }
    case LoadSnippet: {
      insertLoadSnippet(insertionPoints[i]);
      break;
    }
    case StoreSnippet: {
      insertStoreSnippet(insertionPoints[i]);
      break;
    }
    case IfSnippet: {
      insertIfSnippet(insertionPoints[i]);
      break;
    }
    case WhileSnippet: {
      insertWhileSnippet(insertionPoints[i]);
      break;
    }
    default:
      std::cerr << "invalid snippet kind!\n";
      break;
    }
  }
}

int main(int argc, char **argv) {
  assert(argc == 3);
  SnippetKind snippetKind = getSnippetKind(argv[1]);

  BPatch BPatch;
  const char *binaryPath = argv[2];

  BPatch_binaryEdit *binary = BPatch.openBinary(binaryPath);
  assert(binary);

  BPatch_image *binaryImage = binary->getImage();
  assert(binaryImage);

  BPatch_Vector<BPatch_function *> functions;
  assert(binaryImage->getProcedures(functions));

  for (auto *function : functions) {
    std::vector<BPatch_point *> *entryPoints = function->findPoint(BPatch_entry);
    insertSnippet(snippetKind, *entryPoints);
  }

  std::string newPath = std::string(argv[2]) + "-instr";
  if (!binary->writeFile(newPath.c_str())) {
    std::cout << "Rewriting binary failed\n";
  }
}
