/* 
 * @copyright (c) 2008, Hedspi, Hanoi University of Technology
 * @author Huu-Duc Nguyen
 * @version 1.0
 */

#include <stdio.h>
#include "reader.h"
#include "codegen.h"  

#define CODE_SIZE 10000
extern SymTab* symtab;

extern Object* readiFunction;
extern Object* readcFunction;
extern Object* readsFunction;
extern Object* writeiProcedure;
extern Object* writefProcedure;
extern Object* writesProcedure;
extern Object* writecProcedure;
extern Object* writelnProcedure;

CodeBlock* codeBlock;

int computeNestedLevel(Scope* scope) {
  // TODO
  int level = 0;
  Scope* tmp = symtab->currentScope;
  while (tmp != scope)  {
    tmp = tmp->outer;
    level++;
  }
  return level;
}

int float2WORD(float value) {
  int *pv;
  pv = (int*)(&value);
  return *pv;
}

void genVariableAddress(Object* var) {
  genLA(computeNestedLevel(var->varAttrs->scope),var->varAttrs->localOffset);  
}

void genVariableValue(Object* var) {
  genLV(computeNestedLevel(var->varAttrs->scope),var->varAttrs->localOffset);
}

void genParameterAddress(Object* param) {
  if (param->paramAttrs->kind == PARAM_VALUE) {
    genLA(computeNestedLevel(param->paramAttrs->scope),param->paramAttrs->localOffset);
  } else {
    genLV(computeNestedLevel(param->paramAttrs->scope),param->paramAttrs->localOffset);
  }
}

void genParameterValue(Object* param) {
  if (param->paramAttrs->kind == PARAM_VALUE) {
    genLV(computeNestedLevel(param->paramAttrs->scope),param->paramAttrs->localOffset);
  } else {
    genLV(computeNestedLevel(param->paramAttrs->scope),param->paramAttrs->localOffset);
    genLI();
  }
}

void genReturnValueAddress(Object* func) {
  genLA(computeNestedLevel(func->funcAttrs->scope),0);
}

void genReturnValueValue(Object* func) {
  genLV(computeNestedLevel(func->funcAttrs->scope),0);
}

void genPredefinedProcedureCall(Object* proc) {
  if (proc == writeiProcedure)
    genWRI();
  else if (proc == writefProcedure)
    genWRF();
  else if (proc == writecProcedure)
    genWRC();
  else if (proc == writelnProcedure)
    genWLN();
  else if (proc == writesProcedure)
    genWRS();
}

void genProcedureCall(Object* proc) {
  genCALL(computeNestedLevel(proc->procAttrs->scope->outer),proc->procAttrs->codeAddress);
}

void genPredefinedFunctionCall(Object* func) {
  if (func == readiFunction)
    genRI();
  else if (func == readcFunction)
    genRC();
  else if (func == readsFunction)
    genRS();
}

void genFunctionCall(Object* func) {
  genCALL(computeNestedLevel(func->funcAttrs->scope->outer),func->funcAttrs->codeAddress);
}

// STRING UNITY

void genParameterStringValue(Object* param) {
  if (param->paramAttrs->kind == PARAM_VALUE) {
    genLVS(computeNestedLevel(param->paramAttrs->scope),param->paramAttrs->localOffset);
  } else {
    genLV(computeNestedLevel(param->paramAttrs->scope),param->paramAttrs->localOffset);
    genLIS();
  }
}
void genVariableStringValue(Object* var) {
  genLVS(computeNestedLevel(var->varAttrs->scope),var->varAttrs->localOffset);
}

void genLVS(int level, int offset) {
  emitLV(codeBlock, level, offset++);
  emitLV(codeBlock, level, offset++);
  emitLV(codeBlock, level, offset++);
  emitLV(codeBlock, level, offset++);
}

void genLCS(char* value) {
  int i,j;
  for (i=0;i<STRING_SIZE;i++) {
    j = i * 4;
    genLC((value[j] << 24) | (value[j+1] << 16) | (value[j+2] << 8) | (value[j+3]));
  }
}

void genLIS() {
  int i;
  
  for (i=1;i<STRING_SIZE;i++) {
    genCV();
    genDCT(1);
    genLI();
    genINT(1);
    genLC(1);
    genAD();
  }
  genLI();
}

void genSTS(void) {
  emitSTS(codeBlock);
}

void genLA(int level, int offset) {
  emitLA(codeBlock, level, offset);
}

void genLV(int level, int offset) {
  emitLV(codeBlock, level, offset);
}

void genLC(WORD constant) {
  emitLC(codeBlock, constant);
}

void genLI(void) {
  emitLI(codeBlock);
}

void genINT(int delta) {
  emitINT(codeBlock,delta);
}

void genDCT(int delta) {
  emitDCT(codeBlock,delta);
}

Instruction* genJ(CodeAddress label) {
  Instruction* inst = codeBlock->code + codeBlock->codeSize;
  emitJ(codeBlock,label);
  return inst;
}

Instruction* genFJ(CodeAddress label) {
  Instruction* inst = codeBlock->code + codeBlock->codeSize;
  emitFJ(codeBlock, label);
  return inst;
}

void genHL(void) {
  emitHL(codeBlock);
}

void genST(void) {
  emitST(codeBlock);
}

void genCALL(int level, CodeAddress label) {
  emitCALL(codeBlock, level, label);
}

void genEP(void) {
  emitEP(codeBlock);
}

void genEF(void) {
  emitEF(codeBlock);
}

void genRC(void) {
  emitRC(codeBlock);
}

void genRI(void) {
  emitRI(codeBlock);
}

void genRS(void) {
  emitRS(codeBlock);
}

void genWRC(void) {
  emitWRC(codeBlock);
}

void genWRI(void) {
  emitWRI(codeBlock);
}

void genWRF(void) {
  emitWRF(codeBlock);
}

void genWRS(void) {
  emitWRS(codeBlock);
}

void genWLN(void) {
  emitWLN(codeBlock);
}

void genAD(void) {
  emitAD(codeBlock);
}

void genADS(void) {
  emitADS(codeBlock);
}

void genSB(void) {
  emitSB(codeBlock);
}

void genML(void) {
  emitML(codeBlock);
}

void genDV(void) {
  emitDV(codeBlock);
}

void genDI(void) {
  emitDI(codeBlock);
}

void genNEG(void) {
  emitNEG(codeBlock);
}

void genCV(void) {
  emitCV(codeBlock);
}

void genEQ(void) {
  emitEQ(codeBlock);
}

void genNE(void) {
  emitNE(codeBlock);
}

void genGT(void) {
  emitGT(codeBlock);
}

void genGE(void) {
  emitGE(codeBlock);
}

void genLT(void) {
  emitLT(codeBlock);
}

void genLE(void) {
  emitLE(codeBlock);
}

void updateJ(Instruction* jmp, CodeAddress label) {
  jmp->q = label;
}

void updateFJ(Instruction* jmp, CodeAddress label) {
  jmp->q = label;
}

CodeAddress getCurrentCodeAddress(void) {
  return codeBlock->codeSize;
}

int isPredefinedFunction(Object* func) {
  return ((func == readiFunction) || (func == readcFunction) || (func == readsFunction));
}

int isPredefinedProcedure(Object* proc) {
  return ((proc == writeiProcedure) || (proc == writefProcedure) || (proc == writecProcedure) || (proc == writesProcedure) || (proc == writelnProcedure));
}

void initCodeBuffer(void) {
  codeBlock = createCodeBlock(CODE_SIZE);
}

void printCodeBuffer(void) {
  printCodeBlock(codeBlock);
}

void cleanCodeBuffer(void) {
  freeCodeBlock(codeBlock);
}

int serialize(char* fileName) {
  FILE* f;

  f = fopen(fileName, "wb");
  if (f == NULL) return IO_ERROR;
  saveCode(codeBlock, f);
  fclose(f);
  return IO_SUCCESS;
}
