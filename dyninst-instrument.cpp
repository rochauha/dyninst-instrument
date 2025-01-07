
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
  procedureCounter
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


void insertprocedureCounter(BPatch_function *function) {
  // Strategy:
  // Use same memory allocated from the host for all kernels.
  // After each kernel launch the variable must be re-initialized.
  BPatch_addressSpace *addressSpace = function->getAddSpace();
  //
  // SymtabAPI::Type symType("int", SymtabAPI::dataScalar);
  // BPatch_type bType(&symType);

  BPatch_variableExpr *procedureCounter = addressSpace->malloc(4, std::string("procedureCounter"));
  BPatch_constExpr one(0x1);
  BPatch_arithExpr addExpr(BPatch_plus, *procedureCounter, one);
  BPatch_arithExpr assignExpr(BPatch_assign, *procedureCounter, addExpr);
  std::vector<BPatch_point *> *procedureEntryPoints = function->findPoint(BPatch_locEntry);
  if (!procedureEntryPoints) {
    std::cout << "didn't find procedure entry point\n";
    exit(0);
  }
  std::cout << procedureEntryPoints->size() << '\n';
  for (auto it = procedureEntryPoints->begin(); it != procedureEntryPoints->end(); ++it) {
    BPatch_point* point = *it;
    BPatchSnippetHandle *handle = addressSpace->insertSnippet(assignExpr, *point, BPatch_callBefore);
    std::cout << handle << '\n';
    if (!handle) {
      std::cout << "couldn't insert snippet for point " << '(' << point <<  " )\n";
    }
  }
}

void insertLoadSnippet(BPatch_point *point) {}

void insertStoreSnippet(BPatch_point *point) {}

void insertIfSnippet(BPatch_point *point) {
  BPatch_addressSpace *addressSpace = point->getAddressSpace();
  std::vector<BPatch_register> allRegs;
  assert(addressSpace->getRegisters(allRegs) && "Must get all regs");

  BPatch_register r1 = allRegs[0];
  BPatch_register r2 = allRegs[2];

  BPatch_registerExpr op1(r1);
  BPatch_constExpr op2(0xabc);
  BPatch_registerExpr op3(r2);
  BPatch_constExpr op4(0xbeef);

  // condition
  BPatch_boolExpr ltExpr(BPatch_lt, op1, op2);

  // then
  BPatch_arithExpr mulExpr(BPatch_times, op3, op4);

  BPatch_ifExpr ifExpr(ltExpr, mulExpr);

  BPatchSnippetHandle *handle = addressSpace->insertSnippet(ifExpr, *point);

  if (!handle) {
    std::cout << "couldn't insert snippet\n";
  }
}

void insertWhileSnippet(BPatch_point *point) {}

SnippetKind getSnippetKind(const char *str) {
  if (std::string(str) == "-mul")
    return MulSnippet;
  else if (std::string(str) == "-load")
    return LoadSnippet;
  else if (std::string(str) == "-store")
    return StoreSnippet;
  else if (std::string(str) == "-if")
    return IfSnippet;
  else if (std::string(str) == "-while")
    return WhileSnippet;
  else if (std::string(str) == "-procedure-count") {
    return procedureCounter;
  }
  else {
    std::cerr << "Invalid snippet kind\n";
    exit(2);
  }
}

void insertSnippet(SnippetKind sk,
                   std::vector<BPatch_point *> &insertionPoints) {
  for (size_t i = 0; i < insertionPoints.size(); ++i) {
    switch (sk) {
    case MulSnippet:
      insertMulSnippet(insertionPoints[i]);
      break;

    case LoadSnippet:
      insertLoadSnippet(insertionPoints[i]);
      break;

    case StoreSnippet:
      insertStoreSnippet(insertionPoints[i]);
      break;

    case IfSnippet:
      insertIfSnippet(insertionPoints[i]);
      break;

    case WhileSnippet:
      insertWhileSnippet(insertionPoints[i]);
      break;

    default:
      std::cerr << "invalid snippet kind!\n";
      break;
    }
  }
}

int main(int argc, char **argv) {
  assert(argc == 3);
  std::cerr << argv[1] << '\n';
  SnippetKind snippetKind = getSnippetKind(argv[1]);

  BPatch BPatch;
  const char *binaryPath = argv[2];

  BPatch_binaryEdit *binary = BPatch.openBinary(binaryPath);
  assert(binary);

  BPatch_image *binaryImage = binary->getImage();
  assert(binaryImage);

  BPatch_Vector<BPatch_function *> functions;
  assert(binaryImage->getProcedures(functions));

  if (snippetKind == procedureCounter) {
    for (auto *function : functions) {
      insertprocedureCounter(function);
    }
  } else {
    for (auto *function : functions) {
      std::vector<BPatch_point *> *entryPoints = function->findPoint(BPatch_entry);
      insertSnippet(snippetKind, *entryPoints);
    }
  }

  std::string newPath = std::string(argv[2]) + "-instr";
  if (!binary->writeFile(newPath.c_str())) {
    std::cout << "Rewriting binary failed\n";
  }
}
