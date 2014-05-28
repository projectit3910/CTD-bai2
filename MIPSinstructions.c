
#include <stdarg.h>
#include "MIPSinstructions.h"
#include "instructions.h"

FILE* stream;
nlabel labelNumber = 0;
nlabel instructionCount = 0;

nlabel nextLabel() {
	return labelNumber++;
}
nlabel nextInstruction() {
	return instructionCount++;
}

void print(const char * format, ...) {
	va_list args;
	va_start (args, format);
	if (stream == NULL)	{
		vprintf(format, args);
	} else {
		vfprintf(stream, format, args);
	}
	va_end (args);
}

void mipsLA(WORD p, WORD q) {
	mipsINT(1);
	print("\
	li a0, %d // a0 = p\r\n\
	jal base\r\n\
	add v0, v0, %d // v0 = base(p) + q\r\n\
	sll t1,s0,2\r\n\
	add t0,sp,t1 // t0 = &s[t]\r\n\
	sw v0,0(t0) // s[t] = base(p) + q\r\n",p,q);	
}

void mipsLV(WORD p, WORD q) {
	mipsINT(1);
	print("\
	li a0, %d // a0 = p\r\n\
	jal base // v0 = base(p)\r\n\
	addi v0, v0, %d // v0 = base(p) + q\r\n\
	sll t1, s0, 2\r\n\
	add t0,t1,sp // t0 = &s[t]\r\n\
	sll v0,v0,2\r\n\
	add v0,v0,sp // v0 = &s[base(p) + q]\r\n\
	lw t1,0(v0) // t1 = s[base(p) + q]\r\n\
	sw t1,0(t0) // s[t] = s[base(p) + q]\r\n",p,q);
}

void mipsLC(WORD q) {
	mipsINT(1);
	print("\
	sll t1, s0, 2\r\n\
	add t0,t1,sp // t0 = &s[t]\r\n\
	li t1, %d // t1 = q\r\n\
	sw t1,0(t0) // s[t] = q\r\n",q);
}

void mipsLI() {
	print("\
	sll t1, s0, 2\r\n\
	add t0, t1, sp // t0 = &s[t]\r\n\
	lw t1, 0(t0) // t1 = s[t]\r\n\
	sll t1, t1, 2\r\n\
	add t1, t1, sp // t1 = &s[s[t]]\r\n\
	lw t2, 0(t1) // t2 = s[s[t]]\r\n\
	sw t2, 0(t0) // s[t] = s[s[t]]\r\n");
}

void mipsINT(WORD q) {
	print("	addi s0,s0,%d // t = t + %d\r\n",q,q);
}

void mipsDCT(WORD q) {
	if (q >= 0) print("	addi s0,s0,-%d // t = t - q\r\n",q,q);
	else print("	addi s0,s0,%d// t = t - q\r\n",0-q,q);
}

void mipsJ(WORD q) {
	print("	j i_%lu\r\n", q);
}

void mipsFJ(WORD q) {
	nlabel l = nextLabel();
	print("\
	sll t1, s0, 2\r\n\
	add t0, t1, sp // t0 = &s[t]\r\n\
	addi s0, s0, -1 // t = t - 1\r\n\
	lw t1, 0(t0) // t1 = s[t]\r\n\
	bne t1,zero,label_%lu // skip jump if s[t] != 0\r\n\
	j i_%lu\r\n\
label_%lu:\r\n",l,q,l);
}

void mipsHL() {
	print("\
	la a0, string_hl\r\n\
	jal printf\r\n\
	jal getchar\r\n\
	li a0, '\\n'\r\n\
	jal putchar\r\n");
}

void mipsST() {
	print("\
	sll t1, s0, 2\r\n\
	add t0, t1, sp // t0 = &s[t]\r\n\
	lw t1, -4(t0) // t1 = s[t-1]\r\n\
	sll t1,t1,2\r\n\
	add t2, t1, sp // t2 = &s[s[t-1]]\r\n\
	lw t0, 0(t0) // t0 = s[t]\r\n\
	sw t0, 0(t2) // s[s[t-1]] = s[t]\r\n");
	mipsDCT(2);
}

void mipsSTS() {
  int i;
  print("\
	sll t1, s0, 2\r\n\
	add t0, t1, sp // t0 = &s[t]\r\n\
	lw t1, -%d(t0) // t1 = s[t-%d]\r\n\
	sll t1,t1,2\r\n\
	add t2, t1, sp // t2 = &s[s[t-%d]]\r\n",STRING_SIZE*4,STRING_SIZE,STRING_SIZE);
  for (i=1;i<=STRING_SIZE;i++) {
    print("\
	lw t1, -%d(t0) // t1 = s[t-%d]\r\n\
	sw t1, %d(t2) // s[s[t-%d]+%d] = s[t-%d]\r\n",4*(STRING_SIZE-i),STRING_SIZE-i,(i-1)*4,STRING_SIZE,i-1,STRING_SIZE-i);
  }
  mipsDCT(1 + STRING_SIZE);
}

void mipsCALL(WORD p, WORD q) {
	nlabel l = nextLabel();
	print("\
	sll t1, s0, 2\r\n\
	add t0, t1, sp // t0 = &s[t]\r\n\
	sw fp, 8(t0) // s[t+2] = b\r\n\
	la t2, label_%lu\r\n\
	sw t2, 12(t0) // s[t+3] = pc\r\n\
	li a0, %d // a0 = p\r\n\
	jal base\r\n\
	sw v0, 16(t0) // s[t+4] = base(p)\r\n\
	addi fp, s0, 1 // b = t+1\r\n",l,p);
	mipsJ(q);
	print("	label_%lu:\r\n",l);
}

void mipsEP() {
	print("\
	addi s0,fp,-1 // t = b-1\r\n\
	sll t1, s0, 2\r\n\
	add t0, t1, sp // t0 = &s[b-1]\r\n\
	lw t2, 12(t0) // t2 = s[b+2]\r\n\
	lw fp, 8(t0) // b = s[b+1]\r\n\
	jr t2 // pc = s[b+2]\r\n");
}

void mipsEF() {
	print("\
	move s0, fp // t = b\r\n\
	sll t1, s0, 2\r\n\
	add t0, t1, sp // t0 = &s[b]\r\n\
	lw t2, 8(t0) // t2 = s[b+2]\r\n\
	lw fp, 4(t0) // b = s[b+1]\r\n\
	jr t2 // pc = s[b+2]\r\n");
}

void mipsRC() {
	mipsINT(1);
	print("\
	jal getchar // v0 = read one character\r\n\
	sll t1, s0, 2\r\n\
	add t0, t1, sp // t0 = &s[t]\r\n\
	sw v0, 0(t0) // s[t] = v0\r\n");
}

void mipsRI() {
	mipsINT(1);
	print("\
	jal inputNumber // v0 = read one integer\r\n\
	sll t1, s0, 2\r\n\
	add t0, t1, sp // t0 = &s[t]\r\n\
	sw v0, 0(t0) // s[t] = v0\r\n");
}

void mipsRS() {
  mipsINT(1);
  print("\
  // comming soon\r\n");
}

void mipsWRC() {
	print("\
	sll t1, s0, 2\r\n\
	add t0, t1, sp // t0 = &s[t]\r\n\
	lw a0, 0(t0) // a0 = s[t]\r\n\
	jal putchar\n");
	mipsDCT(1);
}

void mipsWRI() {
	print("\
	sll t1, s0, 2\r\n\
	add t0, t1, sp // t0 = &s[t]\r\n\
	lw a1, 0(t0) // a1 = s[t]\r\n\
	lui t1, 0x2564 // t1 = '%%d'\r\n\
	sw t1, 0(t0)\r\n\
	move a0, t0\r\n\
	jal printf\r\n");
	mipsDCT(1);
}

void mipsWRF() {
	print("\
	sll t1, s0, 2\r\n\
	add t0, t1, sp // t0 = &s[t]\r\n\
	// coming soon");
	mipsDCT(1);
}

void mipsWRS() {
	print("\
	sll t1, s0, 2\r\n\
	add t0, t1, sp // t0 = &s[t]\r\n\
	lui t1, 0x2573 // t1 = '%%s'\r\n\
	sw t1, 4(t0) // s[t+1] = '%%s'\r\n\
	addi a0, t0, 4\r\n\
	addi a1, t0, -%d // a1 = s[t-%d]\r\n\
	jal printf\r\n",(STRING_SIZE-1)*4,STRING_SIZE-1);
	mipsDCT(1);
}

void mipsWLN() {
	print("\
	li a0, '\\r'\r\n\
	jal putchar\r\n\
	li a0, '\\n'\r\n\
	jal putchar\n");
}

void mipsAD() {
	mipsDCT(1);
	print("\
	sll t1, s0, 2\r\n\
	add t0, t1, sp // t0 = &s[t]\r\n\
	lw t1, 0(t0) // t1 = s[t]\r\n\
	lw t2, 4(t0) // t2 = s[t+1]\r\n\
	add t1, t1, t2 // t1 = s[t] + s[t+1]\r\n\
	sw t1, 0(t0) // s[t] = s[t] + s[t+1]\r\n");
}

void mipsADS() {
	mipsDCT(STRING_SIZE);
	print("\
	sll t1, s0, 2\r\n\
	add t0, t1, sp // t0 = &s[t]\r\n\
	addi a0, t0, -%d\r\n\
	addi a1, t0, 4\r\n\
	li a2, %d\r\n\
	jal strcat\r\n",(STRING_SIZE-1)*4,STRING_SIZE*4-1);
}

void mipsSB() {
	mipsDCT(1);
	print("\
	sll t1, s0, 2\r\n\
	add t0, t1, sp // t0 = &s[t]\r\n\
	lw t1, 0(t0) // t1 = s[t]\r\n\
	lw t2, 4(t0) // t2 = s[t+1]\r\n\
	sub t1, t1, t2 // t1 = s[t] - s[t+1]\r\n\
	sw t1, 0(t0) // s[t] = s[t] - s[t+1]\r\n");
}

void mipsML() {
	mipsDCT(1);
	print("\
	sll t1, s0, 2\r\n\
	add t0, t1, sp // t0 = &s[t]\r\n\
	lw t1, 0(t0) // t1 = s[t]\r\n\
	lw t2, 4(t0) // t2 = s[t+1]\r\n\
	mult t1, t2\r\n\
	mflo t1 // t1 = s[t] * s[t+1]\r\n\
	sw t1, 0(t0) // s[t] = s[t] * s[t+1]\r\n");
}

void mipsDV() {
	mipsDCT(1);
	print("\
	sll t1, s0, 2\r\n\
	add t0, t1, sp // t0 = &s[t]\r\n\
	lw t1, 0(t0) // t1 = s[t]\r\n\
	lw t2, 4(t0) // t2 = s[t+1]\r\n\
	div t1, t2\r\n\
	mflo t1 // t1 = s[t] / s[t+1]\r\n\
	sw t1, 0(t0) // s[t] = s[t] / s[t+1]\r\n");
}

void mipsDI() {
	mipsDCT(1);
	print("\
	sll t1, s0, 2\r\n\
	add t0, t1, sp // t0 = &s[t]\r\n\
	lw t1, 0(t0) // t1 = s[t]\r\n\
	lw t2, 4(t0) // t2 = s[t+1]\r\n\
	div t1, t2\r\n\
	mfhi t1 // t1 = s[t] %% s[t+1]\r\n\
	sw t1, 0(t0) // s[t] = s[t] %% s[t+1]\r\n");
}

void mipsNEG() {
	print("\
	sll t1, s0, 2\r\n\
	add t0, t1, sp // t0 = &s[t]\r\n\
	lw t1, 0(t0) // t1 = s[t]\r\n\
	sub t1, zero, t1 // t1 = -s[t]\r\n\
	sw t1, 0(t0) // s[t] = -s[t]\r\n");
}

void mipsCV() {
	print("\
	sll t1, s0, 2\r\n\
	add t0, t1, sp // t0 = &s[t]\r\n\
	lw t1, 0(t0) // t1 = s[t]\r\n\
	sw t1, 4(t0) // s[t+1] = s[t]\r\n");
	mipsINT(1);
}

void mipsEQ() {
	nlabel l1 = nextLabel(), l2 = nextLabel();
	mipsDCT(1);
	print("\
	sll t1, s0, 2\r\n\
	add t0, t1, sp // t0 = &s[t]\r\n\
	lw t1, 0(t0) // t1 = s[t]\r\n\
	lw t2, 4(t0) // t2 = s[t+1]\r\n\
	beq t1, t2, label_%lu\r\n\
	li t1, 0 // not equal\r\n\
	b label_%lu\r\n\
label_%lu:\r\n\
	li t1, 1 // equal\r\n\
label_%lu:\r\n\
	sw t1, 0(t0) // s[t] = s[t] == s[t+1] ? 1 : 0\r\n",l1,l2,l1,l2);
}

void mipsNE() {
	nlabel l1 = nextLabel(), l2 = nextLabel();
	mipsDCT(1);
	print("\
	sll t1, s0, 2\r\n\
	add t0, t1, sp // t0 = &s[t]\r\n\
	lw t1, 0(t0) // t1 = s[t]\r\n\
	lw t2, 4(t0) // t2 = s[t+1]\r\n\
	beq t1, t2, label_%lu\r\n\
	li t1, 1 // equal\r\n\
	b label_%lu\r\n\
label_%lu:\r\n\
	li t1, 0 // not equal\r\n\
label_%lu:\r\n\
	sw t1, 0(t0) // s[t] = s[t] != s[t+1] ? 1 : 0\r\n",l1,l2,l1,l2);
}

void mipsGT() {
	nlabel l1 = nextLabel(), l2 = nextLabel();
	mipsDCT(1);
	print("\
	sll t1, s0, 2\r\n\
	add t0, t1, sp // t0 = &s[t]\r\n\
	lw t1, 0(t0) // t1 = s[t]\r\n\
	lw t2, 4(t0) // t2 = s[t+1]\r\n\
	sub t1, t1, t2\r\n\
	bgtz t1, label_%lu\r\n\
	li t1, 0\r\n\
	b label_%lu\r\n\
label_%lu:\r\n\
	li t1, 1\r\n\
label_%lu:\r\n\
	sw t1, 0(t0) // s[t] = s[t] > s[t+1] ? 1 : 0\r\n",l1,l2,l1,l2);
}

void mipsLT() {
	nlabel l1 = nextLabel(), l2 = nextLabel();
	mipsDCT(1);
	print("\
	sll t1, s0, 2\r\n\
	add t0, t1, sp // t0 = &s[t]\r\n\
	lw t1, 0(t0) // t1 = s[t]\r\n\
	lw t2, 4(t0) // t2 = s[t+1]\r\n\
	sub t1, t1, t2\r\n\
	bltz t1, label_%lu\r\n\
	li t1, 0\r\n\
	b label_%lu\r\n\
label_%lu:\r\n\
	li t1, 1\r\n\
label_%lu:\r\n\
	sw t1, 0(t0) // s[t] = s[t] < s[t+1] ? 1 : 0\r\n",l1,l2,l1,l2);
}

void mipsGE() {
	nlabel l1 = nextLabel(), l2 = nextLabel();
	mipsDCT(1);
	print("\
	sll t1, s0, 2\r\n\
	add t0, t1, sp // t0 = &s[t]\r\n\
	lw t1, 0(t0) // t1 = s[t]\r\n\
	lw t2, 4(t0) // t2 = s[t+1]\r\n\
	sub t1, t1, t2\r\n\
	bgez t1, label_%lu\r\n\
	li t1, 0\r\n\
	b label_%lu\r\n\
label_%lu:\r\n\
	li t1, 1\r\n\
label_%lu:\r\n\
	sw t1, 0(t0) // s[t] = s[t] >= s[t+1] ? 1 : 0\r\n",l1,l2,l1,l2);
}

void mipsLE() {
	nlabel l1 = nextLabel(), l2 = nextLabel();
	mipsDCT(1);
	print("\
	sll t1, s0, 2\r\n\
	add t0, t1, sp // t0 = &s[t]\r\n\
	lw t1, 0(t0) // t1 = s[t]\r\n\
	lw t2, 4(t0) // t2 = s[t+1]\r\n\
	sub t1, t1, t2\r\n\
	blez t1, label_%lu\r\n\
	li t1, 0\r\n\
	b label_%lu\r\n\
label_%lu:\r\n\
	li t1, 1\r\n\
label_%lu:\r\n\
	sw t1, 0(t0) // s[t] = s[t] <= s[t+1] ? 1 : 0\r\n",l1,l2,l1,l2);
}

void mipsCSW() {
  print("\
	sll t1, s0, 2\r\n\
	add t0, t1, sp // t0 = &s[t]\r\n\
	lw a0, 0(t0) // a0 = s[t]\r\n\
	jal single2word\r\n\
	sll t1, s0, 2\r\n\
	add t0, t1, sp // t0 = &s[t]\r\n\
	sw v0, 0(t0) // s[t] = (short)s[t]\r\n");
}

void startMIPScode() {
	print("\
#include <iregdef.h>\r\n\
.data\r\n\
string_hl: .ascii \"\\nPress any key to continue...\"\r\n\
.text\r\n\
.set reorder\r\n\
.globl start\r\n\
.ent start\r\n\
start:\r\n\
	move s7, ra\r\n\
	li s0, 0 // t = 0\r\n\
	li fp, 0 // b = 0\r\n");
}

void mipsFuncBase() {
	print("\
.ent base\r\n\
base:\r\n\
	move v0, fp // b' = b\r\n\
base_loop:\r\n\
	beq a0, zero, base_return // p = 0 -> return\r\n\
	addi a0, a0, -1 // p--\r\n\
  sll t0, v0, 2\r\n\
	add v0, t0, sp // b' = &s[b']\r\n\
	lw v0, 12(v0) // b' = s[b'+3]\r\n\
	j base_loop\r\n\
base_return:\r\n\
	jr ra\r\n\
.end base\r\n");
}

void mipsFuncStrcat() {
  print("// a0 = str1 address\r\n// a1 = str2 address\r\n// a2 = max strlen\r\n// v0 -> strlen\r\n.ent strcat\r\nstrcat:\r\n	li v0, 0 // strlen\r\n	\r\nstrcat_loop1:\r\n	lb v1, 0(a0)\r\n	beq v1, zero, strcat_endLoop1\r\n	addi v0, v0, 1\r\n	addi a0, a0, 1\r\n	j strcat_loop1\r\nstrcat_endLoop1:\r\n	\r\n	slt a3, a2, v0 // check if strlen exceed\r\n	bne a3, zero, strcat_endLoop2\r\n	\r\nstrcat_loop2:\r\n	lb v1, 0(a1)\r\n	beq v1, zero, strcat_endLoop2\r\n	sb v1, 0(a0)\r\n	addi v0, v0, 1\r\n	addi a0, a0, 1\r\n	addi a1, a1, 1\r\n	slt a3, v0, a2 // check if strlen exceed\r\n	bne a3, zero, strcat_loop2\r\nstrcat_endLoop2:\r\n	li v1, 0\r\n	sb v1, 0(a0)\r\n	jr ra\r\n.end strcat\r\n");
}

void mipsFuncSingle2Word() {
  print(".ent single2word\r\nsingle2word:\r\n	lui t0, 0x8000 // sign bit\r\n	lui t1, 0x7F80 // exponent bit\r\n	li t2, 0x7FFFFF // fraction bit\r\n	li v0, 0\r\n	\r\n	and t2, a0, t2\r\n	and t0, a0, t0\r\n	and t1, a0, t1\r\n	\r\n	srl t1, t1, 23\r\n	lui t3, 0x80\r\n	or t2, t3, t2\r\n	\r\n	addi t1, t1, -127\r\n	bltz t1, single2word_return\r\n	addi t3, t1, -32\r\n	bgez t3, single2word_return // overflowed\r\n\r\n	addi t1, t1, -23\r\n	bgtz t1, single2word_shiftLeft\r\n	sub t1, zero, t1\r\n	srlv v0, t2, t1\r\n	j single2word_next\r\nsingle2word_shiftLeft:\r\n	sllv v0, t2, t1\r\nsingle2word_next:\r\n	\r\n	beq t0, zero, single2word_return\r\n	sub v0, zero, v0\r\n	\r\nsingle2word_return:\r\n	jr ra\r\n.end single2word\r\n");
}

void mipsFuncInput() {
	print("// v0 = value\r\n.ent inputNumber\r\ninputNumber:\r\n	sw ra, -4(sp)\r\n	sw s0, -8(sp)\r\n	sw s1, -12(sp)\r\n	addi sp, sp, -12\r\n\r\n	li t0,0 	// return value\r\n	li v1,0 	// length of number input\r\n	li s0,10	// base	\r\n	li s1,0		// 1 if negative, 0 if not\r\n	\r\ninputNumber_while:\r\n	jal getchar\r\n	move a0, v0\r\ninputNumber_analyseChar:\r\n	beq a0,'-', inputNumber_minus // minus\r\n	beq a0,'\\n', inputNumber_Enter // return when user input line break\r\n	beq a0,8, inputNumber_backSpace // backspace\r\n	addi t1,a0,-'9'\r\n	bgtz t1, inputNumber_while // if input > '9' then wait next char\r\n	addi t1,a0,-'0'	// t1 = input digit value\r\n	bltz t1, inputNumber_while // if input < '0' then wait next char\r\n	\r\n	multu t0,s0 // mult current data to base\r\n	\r\ninputNumber_checkOverflow:\r\n	mfhi t3\r\n	bne t3,zero,inputNumber_while // if mult value >= 2^32 then overflowed, skip character\r\n	mflo t3\r\n	slt t2, t3, zero\r\n	bne t2, zero, inputNumber_while // if value < 0 then overflowed, skip character\r\n	addu t1,t3,t1 // t1 = temp value\r\n	slt t2,t1,t3\r\n	bne t2,zero,inputNumber_while // if add result lower than last value then overflowed, skip character\r\n	\r\n	jal putchar // not overflow, show character\r\n	addi v1,v1,1 // length++\r\n	move t0,t1 // update value\r\n	j inputNumber_while\r\n\r\ninputNumber_Enter:\r\n	bne s1, zero, inputNumber_Enter2\r\n	bne v1,zero,inputNumber_return // if length > 0 then return\r\n	j inputNumber_while // else wait for next input\r\ninputNumber_Enter2:\r\n	addi v1, v1, -1\r\n	bne v1,zero,inputNumber_return // if length > 0 then return\r\n	addi v1, v1, 1\r\n	j inputNumber_while // else wait for next input\r\n	\r\ninputNumber_backSpace:\r\n	beq v1,zero,inputNumber_while // if input don't have any character then wait for next input\r\n	addi v1,v1,-1 // length--\r\n	divu t0,s0\r\n	mflo t0 // divide current value to base\r\n	jal putchar // backspace 1 character\r\n	beq v1, zero, inputNumber_plus\r\n	j inputNumber_while\r\n	\r\ninputNumber_plus:\r\n	li s1, 0\r\n	j inputNumber_while\r\n\r\ninputNumber_minus:\r\n	bne v1, zero, inputNumber_while\r\n	li s1, 1\r\n	addi v1,v1,1 // length++\r\n	jal putchar\r\n	j inputNumber_while\r\n	\r\ninputNumber_negative:\r\n	sub t0, zero, t0\r\n	li s1, 0\r\n	// j inputNumber_return\r\n	\r\ninputNumber_return:\r\n	bne s1, zero, inputNumber_negative\r\n	\r\n	jal putchar // print line break\r\n	\r\n	move v0,t0 // return value\r\n	lw ra, 8(sp)\r\n	lw s0, 4(sp)\r\n	lw s1, 0(sp)\r\n	addi sp, sp, 12\r\n	jr ra\r\n.end inputNumber\r\n");
}

void endMIPScode() {
	print("\
	move ra, s7\r\n\
	jr ra\r\n\
.end start\r\n");
	mipsFuncBase();
	mipsFuncInput();
  mipsFuncStrcat();
  mipsFuncSingle2Word();
}

void MIPSinstruction(Instruction* inst) {
	switch (inst->op) {
  case OP_LA: print("i_%lu: // LA %lu,%lu\r\n", nextInstruction(), inst->p, inst->q); mipsLA(inst->p, inst->q); break;
  case OP_LV: print("i_%lu: // LV %lu,%lu\r\n", nextInstruction(), inst->p, inst->q); mipsLV(inst->p, inst->q); break;
  case OP_LC: print("i_%lu: // LC %lu\r\n", nextInstruction(), inst->q); mipsLC(inst->q); break;
  case OP_LI: print("i_%lu: // LI\r\n", nextInstruction()); mipsLI(); break;
  case OP_INT: print("i_%lu: // INT %lu\r\n", nextInstruction(), inst->q); mipsINT(inst->q); break;
  case OP_DCT: print("i_%lu: // DCT %lu\r\n", nextInstruction(), inst->q); mipsDCT(inst->q); break;
  case OP_J: print("i_%lu: // J %lu\r\n", nextInstruction(), inst->q); mipsJ(inst->q); break;
  case OP_FJ: print("i_%lu: // FJ %lu\r\n", nextInstruction(), inst->q); mipsFJ(inst->q); break;
  case OP_HL: print("i_%lu: // HL\r\n", nextInstruction()); mipsHL(); break;
  case OP_ST: print("i_%lu: // ST\r\n", nextInstruction()); mipsST(); break;
  case OP_STS: print("i_%lu: // STS\r\n", nextInstruction()); mipsSTS(); break;
  case OP_CALL: print("i_%lu: // CALL %lu,%lu\r\n", nextInstruction(), inst->p, inst->q); mipsCALL(inst->p, inst->q); break;
  case OP_EP: print("i_%lu: // EP\r\n", nextInstruction()); mipsEP(); break;
  case OP_EF: print("i_%lu: // EF\r\n", nextInstruction()); mipsEF(); break;
  case OP_RC: print("i_%lu: // RC\r\n", nextInstruction()); mipsRC(); break;
  case OP_RI: print("i_%lu: // RI\r\n", nextInstruction()); mipsRI(); break;
  case OP_RS: print("i_%lu: // RS\r\n", nextInstruction()); mipsRS(); break;
  case OP_WRC: print("i_%lu: // WRC\r\n", nextInstruction()); mipsWRC(); break;
  case OP_WRI: print("i_%lu: // WRI\r\n", nextInstruction()); mipsWRI(); break;
  case OP_WRF: print("i_%lu: // WRF\r\n", nextInstruction()); mipsWRF(); break;
  case OP_WRS: print("i_%lu: // WRS\r\n", nextInstruction()); mipsWRS(); break;
  case OP_WLN: print("i_%lu: // WLN\r\n", nextInstruction()); mipsWLN(); break;
  case OP_AD: print("i_%lu: // AD\r\n", nextInstruction()); mipsAD(); break;
  case OP_ADS: print("i_%lu: // ADS\r\n", nextInstruction()); mipsADS(); break;
  case OP_SB: print("i_%lu: // SB\r\n", nextInstruction()); mipsSB(); break;
  case OP_ML: print("i_%lu: // ML\r\n", nextInstruction()); mipsML(); break;
  case OP_DV: print("i_%lu: // DV\r\n", nextInstruction()); mipsDV(); break;
  case OP_DI: print("i_%lu: // DI\r\n", nextInstruction()); mipsDI(); break;
  case OP_NEG: print("i_%lu: // NEG\r\n", nextInstruction()); mipsNEG(); break;
  case OP_CV: print("i_%lu: // CV\r\n", nextInstruction()); mipsCV(); break;
  case OP_EQ: print("i_%lu: // EQ\r\n", nextInstruction()); mipsEQ(); break;
  case OP_NE: print("i_%lu: // NE\r\n", nextInstruction()); mipsNE(); break;
  case OP_GT: print("i_%lu: // GT\r\n", nextInstruction()); mipsGT(); break;
  case OP_LT: print("i_%lu: // LT\r\n", nextInstruction()); mipsLT(); break;
  case OP_GE: print("i_%lu: // GE\r\n", nextInstruction()); mipsGE(); break;
  case OP_LE: print("i_%lu: // LE\r\n", nextInstruction()); mipsLE(); break;
  case OP_CSW: print("i_%lu: // CSW\r\n", nextInstruction()); mipsCSW(); break;

  //case OP_BP: mipsBP(); break;
  default: break;
  }
}

void printMIPScode(CodeBlock* codeBlock) {
	Instruction* pc = codeBlock->code;
	int i;
	startMIPScode();
	instructionCount = 0;
	for (i = 0 ; i < codeBlock->codeSize; i ++) {
		MIPSinstruction(pc);
		pc ++;
	}
	endMIPScode();
}

void saveMIPS(CodeBlock* codeBlock, FILE* f) {
	stream = f;
	printMIPScode(codeBlock);
	stream = NULL;
}