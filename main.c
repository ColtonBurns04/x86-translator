#include <stdio.h>
#include <errno.h>
#include "xis.h"
#include "xtra.h"
int main(int argc, char **argv) {
    if(argc<2){
        //no file specified, stop program execution
        printf("No File Specified");
        return 1;
    }
    else if(argc>2) {
        printf("Too many files specified");
        return 1;
    }
    FILE *file = fopen(argv[1], "r");
    //pass opened file to Xtra to be translated
    //if file doesn't exist, exit program with code specified by errno
    if (file == NULL) {
        printf("Could not Open File: %s ERROR Number:%d \n",argv[1], errno);
        return 1;
    }
    //pass file to xtra
    xtra(file);
    //flush and close file stream
    fflush(file);
    fclose(file);
    return 0;
}

//function called when debug flag is true
void printDebug(){
    printf("call debug\n");
}
/* Xtra translator, takes in binary executable of complied x instructions and
 * outputs the x86 version of each instruction via usage of switch statements.
 * Created by Colton Burns on March 15th, 2024 currently supports: entire instruction
 * set mentioned in problem 2. runner.c and xis.h provided by
 * Dr. Alex Brodsky, Dalhousie University, Faculty of Computer Science*/
void xtra(FILE* file){
    //register mapping array (x86 regs are indexed by x regs)
    char* x86_registers[XIS_REGS] = {
            "%rax", "%rbx", "%rcx", "%rdx",
            "%rsi", "%rdi", "%r8",  "%r9",
            "%r10", "%r11", "%r12", "%r13",
            "%r14", "%r15", "%rbp", "%rsp"
    };
    //register mapping for loadb, storb instructions which require bytes
    char* x86_registersBYTE[XIS_REGS] = {
            "%al", "%bl", "%cl", "%dl",
            "%sil", "%rdi", "%r8b",  "%r9b",
            "%r10b", "%r11b", "%r12b", "%r13b",
            "%r14b", "%r15b", "%bpl", "%spl"
    };
    //output prologue
    printf(".globl test\n"
           "test:\n"
           "push %%rbp\n"
           "mov %%rsp, %%rbp\n");
    //buffer to read from file
    unsigned char buff[2];
    //flag to activate debug
    int debug_flag=0;
    //effective offset of current instruction(address)
    unsigned short label =0;
    // enter translation loop
    while(1){
        //read in two bytes from the file
        fread(buff,sizeof(unsigned char),2,file);
        /*output call debug if debug flag is set
        * and next instruction not cld and the zero byte is read, print debug
         * and exit loop */
        if(debug_flag && buff[0]!=I_CLD&&(buff[0]==0&&buff[1]==0)){
            printf(".L%4.4x:\n",label);
            printf("call debug\n");
            break;
        }
        //if both bytes read in are zero, terminate loop
        if(buff[0]==0&&buff[1]==0){
            break;
        }
        //store current operation
        unsigned char operation=buff[0];
        /* operation with  0 operands
        * NOTE: instructions are 2 bytes so the
        * current address(label) is incremented by 2*/
        if(XIS_NUM_OPS(operation)==0) {
            if(debug_flag && buff[0]!=I_CLD){
                printDebug();
            }
            switch (operation) {
                //return from current procedure
                case I_RET:
                    printf("ret\n");
                    label+=2;
                    break;
                    // std called, set debug flag to true
                case I_STD:
                    debug_flag=1;
                    label+=2;
                    break;
                    //cld called, unset debug flag
                case I_CLD:
                    printDebug();
                    debug_flag=0;
                    label+=2;
                    break;
                    //could not translate/invalid instruction
                default:
                    break;
            }
        }
            /*operations with 1 operand
            * NOTE: instructions are 2 bytes so the
            * current address(label) is incremented by 2*/
        else if(XIS_NUM_OPS(operation)==1) {
            //store value of register used
            unsigned short reg1= XIS_REG1(buff[1]);
            //print address of current instruction
            printf(".L%4.4x:\n",label);
            //check if debug flag set, if it is, called printDebug function
            if(debug_flag){
                printDebug();
            }
            switch (operation) {
                case I_NEG:
                    printf("neg %s\n",x86_registers[reg1]);
                    label+=2;
                    break;
                case I_NOT:
                    printf("not %s\n",x86_registers[reg1]);
                    label+=2;
                    break;
                case I_PUSH:
                    printf("push %s\n",x86_registers[reg1]);
                    label+=2;
                    break;
                case I_POP:
                    printf("pop %s\n",x86_registers[reg1]);
                    label+=2;
                    break;
                case I_INC:
                    printf("inc %s\n",x86_registers[reg1]);
                    label+=2;
                    break;
                case I_DEC:
                    printf("dec %s\n",x86_registers[reg1]);
                    label+=2;
                    break;
                case I_OUT:
                    /*as outlined in the assignment 4 document, the first parameter of a function call is stored
                    * in %rdi so, the current value of %rdi has to be saved on the stack via pushing, once the
                    * value is pushed to the stack, the byte(char) to be printed is loaded into %rdi,then the
                    * out char procedure is called and the original value of %rdi is popped off the stack and
                    * saved back into %rdi*/
                    printf("push %%rdi\n mov %s, %%rdi\n call outchar\n pop %%rdi\n ",x86_registers[reg1]);
                    label+=2;
                    break;
                case I_JR:
                    label+=2;
                    //label is the PC as in the specification: PC+L(buff[1])-2
                    printf("jr .L%4.4x\n",(label+buff[1]-2));
                    break;
                case I_BR:
                    label+=2;
                    //check if F is set before jumping
                    printf("test $0x0001, %%r15\n");
                    //label is the PC as in the specification: PC+L(buff[1])-2
                    printf("jne .L%4.4x\n",(label+buff[1]-2));

                    break;
                    //unidentifiable operations exit through default case
                default: break;
            }
        }
            /* operations that require 2 operands
             * NOTE: instructions are 2 bytes so the
             * current address(label) is incremented by 2*/
        else if(XIS_NUM_OPS(operation)==2){
            //store both registers
            unsigned short reg1= XIS_REG1(buff[1]);
            unsigned short reg2= XIS_REG2(buff[1]);
            //print current address/label
            printf(".L%4.4x:\n",label);
            //check if debug flag is set, if it is, print debug
            if(debug_flag){
                printDebug();
            }
            switch (operation) {
                case I_ADD:
                    printf("add %s, %s\n", x86_registers[reg1], x86_registers[reg2]);
                    label+=2;
                    break;
                case I_SUB:
                    printf("sub %s, %s\n", x86_registers[reg1], x86_registers[reg2]);
                    label+=2;
                    break;
                case I_MUL:
                    printf("imul %s, %s\n", x86_registers[reg1], x86_registers[reg2]);
                    label+=2;
                    break;
                case I_AND:
                    printf("and %s, %s\n", x86_registers[reg1], x86_registers[reg2]);
                    label+=2;
                    break;
                case I_XOR:
                    printf("xor %s, %s\n", x86_registers[reg1], x86_registers[reg2]);
                    label+=2;
                    break;
                case I_OR:
                    printf("or %s, %s\n", x86_registers[reg1], x86_registers[reg2]);
                    label+=2;
                    break;
                case I_TEST:
                    //set flag F (%r15) if test returns a non zero value
                    printf("test %s, %s\n", x86_registers[reg1], x86_registers[reg2]);
                    printf("setnz %%r15b\n");
                    label+=2;
                    break;
                case I_CMP:
                    //set flag F (%r15) if cmp returns that Rs1>Rs2
                    printf("cmp %s, %s\n", x86_registers[reg1], x86_registers[reg2]);
                    printf("setg %%r15b\n");
                    label+=2;
                    break;
                case I_EQU:
                    //set flag F (%r15) if cmp returns equal, in other words (Rs1-Rs2==0)
                    printf("cmp %s, %s\n", x86_registers[reg1], x86_registers[reg2]);
                    printf("sete %%r15b\n");
                    label+=2;
                    break;
                case I_MOV:
                    printf("mov %s, %s\n", x86_registers[reg1], x86_registers[reg2]);
                    label+=2;
                    break;
                case I_LOAD:
                    printf("mov (%s), %s\n",x86_registers[reg1],x86_registers[reg2]);
                    label+=2;
                    break;
                    //access the 8 lower bytes of the register specified by using the BYTES array
                case I_LOADB:
                    printf("movb (%s), %s\n",x86_registers[reg1],x86_registersBYTE[reg2]);
                    label+=2;
                    break;
                case I_STOR:
                    printf("mov %s, (%s)\n",x86_registers[reg1],x86_registers[reg2]);
                    label+=2;
                    break;
                case I_STORB:
                    //access the 8 lower bytes of the register specified by using the BYTES array
                    printf("movb %s, (%s)\n",x86_registersBYTE[reg1],x86_registers[reg2]);
                    label+=2;
                    break;
                    //unidentifiable operations exit through default case
                default:
                    break;
            }
        }
            /*extended operations
            * NOTE: extended instructions are 4 bytes in size
            * so the current address(label) is incremented by 4*/
        else if(XIS_NUM_OPS(operation)==3){
            //print current address/label
            printf(".L%4.4x:\n",label);
            if(debug_flag){
                printDebug();
            }
            //grab register only if one is used
            unsigned short reg=XIS_REG1(buff[1]);
            //store next word read
            unsigned char word[2];
            //read in next word
            fread(word,sizeof(unsigned char),2,file);
            //combine both bytes read into unsigned short
            unsigned short worded = (((word[0]<<8)))|(((word[1])));
            switch (operation){
                case I_LOADI:
                    //print mov operation with immediate value which is the same as loadi
                    printf("mov $%d, %s\n", worded, x86_registers[reg]);
                    label+=4;
                    break;
                case I_JMP:
                    printf("jmp .L%4.4x\n",worded);
                    label+=4;
                    break;
                case I_CALL:
                    printf("call .L%4.4x\n",worded);
                    label+=4;
                    break;
                default:
                    break;
            }
        }
    }
    //output epilogue
    printf("pop %%rbp\n"
           "ret");
}

