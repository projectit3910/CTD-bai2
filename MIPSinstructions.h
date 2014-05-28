#ifndef __MIPSINSTRUCTIONS_H__
#define __MIPSINSTRUCTIONS_H__

#include <stdio.h>
#include "instructions.h"

typedef unsigned long nlabel;
nlabel nextLabel();
nlabel nextInstruction();

// s0 -> t
// fp -> b
// sp -> s
// s7 -> return address, DO NOT change

void mipsLA(WORD p, WORD q);
void mipsLV(WORD p, WORD q);
void mipsLC(WORD q);
void mipsLI();
void mipsINT(WORD q);
void mipsDCT(WORD q);
void mipsJ(WORD q);
void mipsFJ(WORD q);
void mipsHL();
void mipsST();
void mipsSTS();
void mipsCALL(WORD p, WORD q);
void mipsEP();
void mipsEF();
void mipsRC();
void mipsRI();
void mipsRS();
void mipsWRC();
void mipsWRI();
void mipsWRF();
void mipsWRS();
void mipsWLN();
void mipsAD();
void mipsSB();
void mipsML();
void mipsDV();
void mipsDI();
void mipsNEG();
void mipsCV();
void mipsEQ();
void mipsNE();
void mipsGT();
void mipsLT();
void mipsGE();
void mipsLE();
void mipsCSW();
void startMIPScode();
void endMIPScode();
void mipsFuncBase();
void mipsFuncInput();
void mipsFuncStrcat();
void mipsFuncSingle2Word();

void printMIPScode(CodeBlock* codeBlock);
void MIPSinstruction(Instruction* inst);
void saveMIPS(CodeBlock* codeBlock, FILE* f);

#endif