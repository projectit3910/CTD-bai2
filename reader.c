/* 
 * @copyright (c) 2008, Hedspi, Hanoi University of Technology
 * @author Huu-Duc Nguyen
 * @version 1.0
 */

#include <stdio.h>
#include "reader.h"

savePoint breakPoints[MAXBREAKPOINTS];
int currentBreakPoint = 0;

FILE *inputStream;
int lineNo, colNo;
int currentChar;

int readChar(void) {
  currentChar = getc(inputStream);
  colNo ++;
  if (currentChar == '\n') {
    lineNo ++;
    colNo = 0;
  }
  return currentChar;
}

int openInputStream(char *fileName) {
  inputStream = fopen(fileName, "rt");
  if (inputStream == NULL)
    return IO_ERROR;
  lineNo = 1;
  colNo = 0;
  readChar();
  return IO_SUCCESS;
}

void closeInputStream() {
  fclose(inputStream);
}

void saveBreakPoint() {
  if (currentBreakPoint < MAXBREAKPOINTS) {
    breakPoints[currentBreakPoint].lineNo = lineNo;
    breakPoints[currentBreakPoint].colNo = colNo;
    breakPoints[currentBreakPoint].currentChar = currentChar;
    fgetpos(inputStream,&(breakPoints[currentBreakPoint].filePos));
    currentBreakPoint++;
  } else {
    //TODO bao loi
  }
}

int loadBreakPoint() {
  if (currentBreakPoint > 0) {
    currentBreakPoint--;
    lineNo = breakPoints[currentBreakPoint].lineNo;
    colNo = breakPoints[currentBreakPoint].colNo;
    currentChar = breakPoints[currentBreakPoint].currentChar;
    fsetpos(inputStream,&(breakPoints[currentBreakPoint].filePos));
    return 1;
  } else {
    return 0; //load break point failed
  }
}

void deleteBreakPoint() {
  if (currentBreakPoint > 0) {
    currentBreakPoint--;
  }
}
