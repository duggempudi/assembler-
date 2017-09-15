#include<stdio.h>
#include<stdlib.h>
char p[2000];
int programlength;
int littleEndian=1;
int num_words;
int * eax , *ebx, *ecx, *edx, *esi, * esp, *ebp, *edi;
int pc;
char * re[]={"eax","ecx","edx","ebx","esp","ebp","esi","edi"};
int codes; 
void setup(){
	*eax=*ecx=*edx=*ebc=*esi=*edi=*esp=*ebp=pc=0;}
int parseFromASCII(char  h[])
    {
        int j = 0;
        programLength = 0;
        
        if (sizeof(h) == 0)
        {
            printf("There are no bytes in the input file\n");
            return 1;
        } else
        {
            // c0af   c 0af  c0 af  c0a f //
            // i ranges over the h array, j over the p array
            for (int i = 0; i < sizeof(h); i++)
            {
                char a = aToH(h[i]);
                char b = aToH(h[i + 1]);
                p[j] = (char) (a << 4 | b);
                printf("%x ", p[j] & 0xff);
                i++;
                j++;
            }
            programLength = j;
        }
        printf("\n");
        return 0;
    }
char aToH(char x)
    {
        if (x > '/' && x < ':')
            /* 0-9 */
            x = (char) (x - 48);
        else
            if (x > '`' && x < 'g')
                /* a-f */
                x = (char) (x - 87);
        return x;
    }
int decode(char p[])
    {
        /*
    take the first char
        - determine how many operands
        - call function with right operands
         */

        for (pc = 0; pc < programLength;)
        {
            printRegisters();
            printf("\n0x%x:\t", pc & 0xff);
            switch (p[pc] & 0xf0)
            {
                case 0x00:
                {
                    /* l=2, halt */
                    if ((p[pc] & 0x0f) == 0x0)
                    {
                        printf("%x\t\t", p[pc]);
                        halt();
                    } else
                    {
                        error("Error interpreting halt at pc=%x", pc);
                    }
                    break;
                }
                case 0x10:
                {
                    /* l=2, nop */
                    if ((p[pc] & 0x0f) == 0x0)
                    {
                        printf("%x\t\t", p[pc]);
                        nop();
                    } else
                    {
                        error("Error interpreting nop at pc=%x", pc);
                    }
                    break;
                }
                case 0x20:
                {
                    printf("%x %x \t\t", p[pc] & 0xff, p[pc + 1] & 0xff);
                    int reg = p[pc + 1];
                    /* l=4, mov
                rrmovl rA, rb     20 rArB
                cmovle rA, rb     21 rArB
                cmovl rA, rb      22 rArB
                cmove rA, rB      23 rArB
                cmovne rA, rB     24 rArB
                cmovge rA, rB     25 rArB
                cmovg rA, rB      26 rArB
                     */
                    switch (p[pc] & 0x0f)
                    {
                        case 0x0:
                            rrmovl(reg);
                            break;
                        case 0x1:
                            cmovle(reg);
                            break;
                        case 0x2:
                            cmovl(reg);
                            break;
                        case 0x3:
                            cmove(reg);
                            break;
                        case 0x4:
                            cmovne(reg);
                            break;
                        case 0x5:
                            cmovge(reg);
                            break;
                        case 0x6:
                            cmovg(reg);
                            break;
                        default:
                            setINS();
                            error("Error interpreting mov at pc=%x", pc);
                            break;
                    }
                    break;
                }
                case 0x30:
                {
                    // l=8, irmovl */
                    // irmovl V, rb      30 FrB V[4] */
                    if ((p[pc] & 0x0f) == 0x0)
                    {
                        printf("%x %x %x %x %x %x\t", 
                                p[pc] & 0xff, 
                                p[pc + 1] & 0xff,
                                p[pc + 2] & 0xff, 
                                p[pc + 3] & 0xff, 
                                p[pc + 4] & 0xff, 
                                p[pc + 5] & 0xff);
                        int val = getVal(p[pc + 2], p[pc + 3], p[pc + 4], p[pc + 5]);
                        int reg = p[pc+1];
                        irmovl(val, reg);
                    } else
                    {
                        setINS();
                        error("Error interpreting irmovl at pc=%x", pc);
                    }
                    break;
                }
                case 0x40:
                {   
                    // l=8, rmmovl
                    // rmmovl rA, D(rB)  40 rArB D[4] */
                    if ((p[pc] & 0x0f) == 0x0)
                    {
                        printf("%x %x %x %x %x %x\t", p[pc] & 0xff, p[pc + 1] & 0xff,
                                p[pc + 2] & 0xff, p[pc + 3] & 0xff, p[pc + 4] & 0xff, p[pc + 5] & 0xff);
                        int data = getVal(p[pc + 2], p[pc + 3], p[pc + 4], p[pc + 5]);
                        rmmovl(p[pc + 1], data);
                    } else
                    {
                        setINS();
                        error("Error interpreting rrmovl at pc=%x", pc);
                    }
                    break;
                }
                case 0x50:
                {
                    /* l=8, mrmovl */
                    /*     mrmovl D(rB), rA     50 rArB D[4] */
                    if ((p[pc] & 0x0f) == 0x0)
                    {
                        printf("%x %x %x %x %x %x\t", p[pc] & 0xff, p[pc + 1] & 0xff,
                                p[pc + 2] & 0xff, p[pc + 3] & 0xff, p[pc + 4] & 0xff, p[pc + 5] & 0xff);
                        int data = getVal(p[pc + 2], p[pc + 3], p[pc + 4], p[pc + 5]);
                        mrmovl(p[pc + 1], data);
                    } else
                    {
                        setINS();
                        error("Error interpreting mrmovl at pc=%x", pc);
                    }
                    break;
                }
                case 0x60:
                {
                    printf("%x %x \t\t", p[pc] & 0xff, p[pc + 1] & 0xff);
                    int reg = p[pc + 1];
                    /* l=4, op */
                /*
                addl rA, rB       60 rArB
                subl rA, rB       61 rArB
                andl rA, rB       62 rArB
                xorl rA, rB       63 rArB
                     */
                    switch (p[pc] & 0x0f)
                    {
                        case 0x0:
                            addl(reg);
                            break;
                        case 0x1:
                            subl(reg);
                            break;
                        case 0x2:
                            andl(reg);
                            break;
                        case 0x3:
                            xorl(reg);
                            break;
                        default:
                            setINS();
                            error("Problem parsing op at pc=%x", pc);
                            break;
                    }
                    break;
                }
                case 0x70:
                {
                    /* l=8, jmps */
                    printf("%x %x %x %x %x\t", p[pc] & 0xff, p[pc + 1] & 0xff,
                            p[pc + 2] & 0xff, p[pc + 3] & 0xff, p[pc + 4] & 0xff);
                    int dest = getVal(p[pc + 1], p[pc + 2], p[pc + 3], p[pc + 4]);
                    /*
                jmp Dest          70 Dest[4]
                jle Dest          71 Dest[4]
                jl Dest           72 Dest[4]
                je Dest           73 Dest[4]
                jne Dest          74 Dest[4]
                jge Dest          75 Dest[4]
                jg Dest           76 Dest[4]
                     */
                    switch (p[pc] & 0x0f)
                    {
                        case 0x0:
                            jmp(dest);
                            break;
                        case 0x1:
                            jle(dest);
                            break;
                        case 0x2:
                            jl(dest);
                            break;
                        case 0x3:
                            je(dest);
                            break;
                        case 0x4:
                            jne(dest);
                            break;
                        case 0x5:
                            jge(dest);
                            break;
                        case 0x6:
                            jg(dest);
                            break;
                        default:
                            setINS();
                            error("Error interpreting jump at pc=%x", pc);
                            break;
                    }
                    break;
                }
                case 0x80:
                {
                    /* l=8, call */
                    /*     call  80 Dest[4] */
                    if ((p[pc] & 0x0f) == 0x0)
                    {
                        printf("%x %x %x %x %x\t", p[pc] & 0xff, p[pc + 1] & 0xff,
                                p[pc + 2] & 0xff, p[pc + 3] & 0xff, p[pc + 4] & 0xff);
                        int dest = getVal(p[pc + 1], p[pc + 2], p[pc + 3], p[pc + 4]);
                        call(dest);
                    } else
                    {
                        setINS();
                        error("Error interpreting call at pc=%x", pc);
                    }
                    break;
                }
                case 0x90:
                {
                    /* l=2, ret */
                    /*     ret   90 */
                    if ((p[pc] & 0x0f) == 0x0)
                    {
                        printf("%x  \t\t", p[pc] & 0xff);
                        ret();
                    } else
                    {
                        setINS();
                        error("Error interpreting ret at pc=%x", pc);
                    }
                    break;
                }
                case 0xa0:
                {
                    /* l=4, pushl */
                    /* pushl rA          A0 rAF */
                    if ((p[pc] & 0x0f) == 0x0)
                    {
                        printf("%x %x \t\t", p[pc] & 0xff, p[pc + 1] & 0xff);
                        pushl((char) (p[pc + 1] & 0xf0));
                    } else
                    {
                        setINS();
                        error("Error interpreting pushl at pc=%x", pc);
                    }
                    break;
                }
                case 0xb0:
                {
                    /* l=4, popl */
                     /* popl rA           B0 rAF */
                    if ((p[pc] & 0x0f) == 0x0)
                    {
                        printf("%x %x \t\t", p[pc] & 0xff, p[pc + 1] & 0xff);
                        popl((char) (p[pc + 1] & 0xf0));
                    } else
                    {
                        setINS();
                        error("Error interpreting pushl at pc=%x", pc);
                    }
                    break;
                }
                default:
                {
                    setINS();
                    error("Error interpreting instruction at pc=%x", pc);
                }
            }
        }
        printRegisters();
        return 0;
    }
void setZF()
    {
        codes = codes | 64;
        /* sets the 01000000 flag */
        printf("   ZF set");
    }
void clearZF()
    {
        codes = codes &(255-64);
        /* clears 10111111 flag */
        // if set, clear
        // if clear, clear
    }
int getZF()
    {
        if ((codes & 64) == 64)
            return 1;
        return 0;
    }
void clearFlags()
    {
        clearSF();
        clearZF();
        clearOF();
    }





    /* Sign Flag operations */
void setSF()
    {
        codes = codes | 32;
        /* sets the 00100000 flag */
        printf("   SF set");
    }





    void clearSF()
    {
        codes = codes &(255-32);
    }





    int getSF()
    {
        if ((codes & 32) == 32)
            return 1;
        return 0;
    }





    /* Overflow flag operations */
    void setOF()
    {
        codes = codes | 16;
        /* sets the 00010000 flag */
       printf("   OF set");
    }





    void clearOF()
    {
        codes = codes &(255-16);
    }





    int getOF()
    {
        if ((codes & 16) == 16)
            return 1;
        return 0;
    }





    /* Status code operations (last four bits) */
    void setAOK()
    {
        clearStatus();
        codes = codes | 1;
        /* sets 00000001 */
        printf("   set status = AOK");
    }





    void setHLT()
    {
        clearStatus();
        codes = codes | 2;
        /* sets 00000010 */
        printf("   set status = HLT");
    }





    void setADR()
    {
        clearStatus();
        codes = codes | 3;
        /* sets 00000011 */
        printf("   set status = ADR");
    }





    void setINS()
    {
        clearStatus();
        codes = codes | 4;
        /* sets 00000100 */
        printf("   set status = INS");
    }





    int getStatus()
    {
        return codes & 0xf0;
    }





    void clearStatus()
    {
        codes = codes & (255 - 240);
        /*11110000 - clears last four bits */
    }





    /**
     ** Gets an integer value from four bytes (based on big * endian or little
     * endian encoding)
     *
     */
    int getVal(char a, char b, char c, char d)
    {
        int val;
        if (littleEndian)
        {
//        printf("      getVal: %x %x %x %x \n ",a,b,c,d);
            val = d;
            val = val | c << 8;
            val = val | b << 16;
            val = val | a << 24;
//        printf(": %x ", val);
        } else
        /* big endian */
        {
            val = a;
            val = val | b << 8;
            val = val | c << 16;
            val = val | d << 24;
        }
        return val;
    }





    /* 
    Computes the register from the first part of the byte 
    passed in. Returns a pointer to the register given 
    character code as input.
    0  eax; 1 ecx; 2 edx; 3  ebx
    4  esp; 5 ebp; 6 esi; 7  edi
    F  No register
     */
    int* r1(char a)
    {
        //    printf("r2: %x, r2 & 0x0f: %x", a, (a & 0xf0)); 
        switch (a & 0xf0)
        {
            case 0x00:
                return eax;
            case 0x10:
                return ecx;
            case 0x20:
                return edx;
            case 0x30:
                return ebx;
            case 0x40:
                return esp;
            case 0x50:
                return ebp;
            case 0x60:
                return esi;
            case 0x70:
                return edi;
            default:
                error("Error determining register value. pc=%x", pc);
        }
        return eax;
        /* shouldn't be hit */
    }





    /* 
    Computes the register from the second part of the byte 
    passed in. Returns a pointer to the register given 
    character code as input.
    0  eax; 1 ecx; 2 edx; 3  ebx
    4  esp; 5 ebp; 6 esi; 7  edi
    F  No register
     */
    int*r2(char a)
    {
//    printf("r2: %x, r2 & 0x0f %x", a, (a & 0x0f));
        switch (a & 0x0f)
        {
            case 0x00:
                return eax;
            case 0x01:
                return ecx;
            case 0x02:
                return edx;
            case 0x03:
                return ebx;
            case 0x04:
                return esp;
            case 0x05:
                return ebp;
            case 0x06:
                return esi;
            case 0x07:
                return edi;
            default:
                error("Error determining register value. pc=%x", pc);
        }
        return eax;
        /* shouldn't be hit */
    }





    /*
*  Prints out the values of the registers */
void printRegisters()
    {
        printf("\t%s: %x\n, ", "eax", *eax);
        printf("%s: %x\n, ", "ecx", *ecx);
        printf("%s: %x\n, ", "edx", *edx);
        printf("%s: %x\n, ", "ebx", *ebx);
        printf("%s: %x\n, ", "esi", *esi);
        printf("%s: %x\n, ", "edi", *edi);
        printf("%s: %x\n, ", "esp", *esp);
        printf("%s: %x\n, ", "ebp", *ebp);
        printf("PC: %x\n", pc);
    }
void error(String words, int pc)
    {
        printf(words);
        exit(1);
    }
void halt()
    {
        printf("halt");
        setHLT();
        pc += 1;
        printRegisters();
        exit(0);
    }
void nop()
    {
        printf("nop");
        pc += 1;
    }
void rrmovl(int reg)
    {
        int* src = r1(reg);
        int* dst = r2(reg);
        *dst = *src;
        printf("rrmovl %s, %s:  (%x)", re[reg>>4], re[reg&0x0f], *dst);
        pc += 2;
    }
void cmovle(char reg)
    {
        int *src = r1(reg);
        int *dst = r2(reg);
        if (getZF() == 1 || getSF() != getOF())
        {
            *dst = *src;
            printf("cmovle %s, %s (%x moved)",re[reg>>4], re[reg&0x0f], *dst);
        } else
            printf("cmovle %s, %s (%x not moved)"re[reg>>4], re[reg&0x0f], *src);
        pc += 2;
    }
void cmovl(char reg)
    {
        int *src = r1(reg);
        int *dst = r2(reg);
        if (getSF() != getOF())
        {
            *dst = *src;
            printf("cmovl %s, %s (%x moved)"re[reg>>4], re[reg&0x0f], *dst);
        } else
            printf("cmovl %s, %s (%x not moved)",re[reg>>4], re[reg&0x0f], *src);
        pc += 2;
    }
void cmove(char reg)
    {
        int *src = r1(reg);
        int *dst = r2(reg);
        if (getZF() == 1)
        {
            *dst = *src;
            printf("cmove %s, %s (%x moved)",re[reg>>4], re[reg&0x0f], *dst);
        } else
            printf("cmove %s, %s (%x not moved)", re[reg>>4], re[reg&0x0f], *src);
        pc += 2;
    }





    /**
     * cmovne rA, rB 24 rArB
     */
 void cmovne(char reg)
    {
        int *src = r1(reg);
        int *dst = r2(reg);
        if (getZF() == 0)
        {
            *dst = *src;
            printf("cmovne %s, %s (%x moved)", re[reg>>4], re[reg&0x0f], *dst);
        } else
            printf("cmovne %s, %s (%x not moved)",re[reg>>4], re[reg&0x0f], *src);
        pc += 2;
    }





    /**
     * cmovge rA, rB 25 rArB
     */
void cmovge(char reg)
    {
        int *src = r1(reg);
        int *dst = r2(reg);
        if (getSF() == getZF())
        {
            *dst = *src;
            printf("cmovge %s, %s, (%x moved)", re[reg>>4], re[reg&0x0f], *dst);
        } else
            printf("cmovge %s, %s (%x not moved)",re[reg>>4], re[reg&0x0f], *src);
        pc += 2;
    }





    /**
     * cmovg rA, rB 26 rArB
     */
  void cmovg(char reg)
    {
        Reg src = r1(reg);
        Reg dst = r2(reg);
        if (getSF() == 0 && getZF() == 0)
        {
            *dst = *src;
            printf("cmovg %s, %s (%x moved)",re[reg>>4], re[reg&0x0f], *dst);
        } else
            printf("cmovg %x, %x (%x not moved)",re[reg>>4], re[reg&0x0f], *src);
        pc += 2;
    }





    /**
     * irmovl V, rb 30 FrB Va Vb Vc Vd
     */
void irmovl(int val, char  reg)
    {
        int *rB = r2(reg);
        *rB = val;
        printf("irmovl %s, %x", re[reg&0x0f], *rB & 0xffff);
        pc += 6;
    }





    /**
     * rmmovl rA, D(rB) 40 rArB Da Db Dc Dd
     */
void rmmovl(char reg, int offset)
    {
        int *rA = r1(reg);
        int *rB = r2(reg);
        loadVal(*rA, *rB + offset);
        printf("rmmovl %s, %x(%x)", re[reg&0x0f], offset & 0xffff, *rB & 0xffff);
        pc += 6;
    }





    /**
     * mrmovl D(rB), rA 50 rArB Da Db Dc Dd
     */
void mrmovl(char  reg, int offset)
    {
        int *rA = r1(reg);
        int *rB = r2(reg);
        *rA = p[*rB + offset];
        printf("mrmovl %x(%s), %s: (%x)", offset & 0xffff, re[reg&0x0f], re[reg>>4], *rA);
        pc += 6;
    }





    /**
     * Sets flags based on the last result
     */
void setFlags(int a, int b, int result)
    {
        clearFlags();
        if (result == 0)
            setZF();
        if (result < 0)
            setSF();
        if (((a & 128) == 128 &&  (b & 128) == 128 &  (result & 128) == 0) ||
            ((a & 128) == 0   &&  (b & 128) == 0   &  (result & 128) == 128))
            setOF();
    }




    
    
    

    /**
     * addl rA, rB 60 rArB
     */
void addl(char reg)
    {
        int *src = r1(reg);
        int *dst = r2(reg);
        int *tmp = dst;
        *dst = *dst+*src;
        printf("addl %s, %s: (%x)",re[reg&0x0f], re[reg>>4], *dst);
        setFlags(*tmp, *src, *dst);
        pc += 2;
    }





    /**
     * subl rA, rB 61 rArB
     */
void subl(char  reg)
    {
        int *src = r1(reg);
        int *dst = r2(reg);
        int *tmp = dst;
        *dst = *dst - *src;
        printf("subl %s, %s: (%x)", re[reg&0x0f],re[reg>>4], *dst);
        setFlags(*tmp, *src, *dst);
        pc += 2;
    }





    /**
     * andl rA, rB 62 rArB
     */
void andl(char  reg)
    {
        int*  src = r1(reg);
        int * dst = r2(reg);
       char * tmp = dst;
        *dst = *dst & *src;
        printf("andl %s, %s: (%x)",re[reg&0x0f], re[reg>>4], *dst);
        setFlags(*tmp, *src, *dst);
        pc += 2;
    }





    /**
     * xorl rA, rB 63 rArB
     */
void xorl(char  reg)
    {
        int *src = r1(reg);
        int *dst = r2(reg);
        int *tmp = dst;
        *dst = *dst ^*src;
        printf("xorl %s, %s: (%x)", dst, src, *dst);
        setFlags(*tmp, *src., *dst);
        pc += 2;
    }





    /**
     * jmp Dest 70 Da Db Dc Dd
     */
void jmp(int dest)
    {
        printf("jmp %x", dest);
        pc = dest;
        printf(" (pc=%x)", dest);
        pc += 5;
    }





    /**
     * jle Dest 71 Da Db Dc Dd
     */
void jle(int dest)
    {
        printf("jle %x", dest);
        if (getZF() == 1 || getSF() != getOF())
        {
            pc = dest;
            printf(" (pc=%x)", dest);
        } else
        {
            printf(" (not taken)");
            pc += 5;
        }
    }





    /**
     * jl Dest 72 Da Db Dc Dd
     */
void jl(int dest)
    {
        printf("jl %x", dest);
        if (getZF() != getOF())
        {
            pc = dest;
            printf(" (pc=%x)", dest);
        } else
        {
            printf(" (not taken)");
            pc += 5;
        }
    }





    /**
     * je Dest 73 Da Db Dc Dd
     */
void je(int dest)
    {
        printf("je %x", dest);
        if (getZF() == 1)
        {
            pc = dest;
            printf(" (taken)");
        } else
        {
            printf(" (not taken)");
            pc += 5;
        }
    }





    /**
     * jne Dest 74 Da Db Dc Dd
     */
void jne(int dest)
    {
        printf("jne %x", dest);
        if (getZF() == 0)
        {
            pc = dest;
            printf(" (pc=%x)", pc & 0xff);
        } else
        {
            printf(" (not taken)");
            pc += 5;
        }
    }





    /**
     * jge Dest 75 Da Db Dc Dd
     */
void jge(int dest)
    {
        printf("jge %x", dest);
        if (getZF() == getSF())
        {
            pc = dest;
            printf(" (pc=%x)", dest);
        } else
        {
            printf(" (not taken)");
            pc += 5;
        }
    }





    /**
     * jg Dest 76 Da Db Dc Dd
     */
void jg(int dest)
    {
        printf("jg %x", dest);
        if (getZF() == 0 && getSF() == 0)
        {
            pc = dest;
           printf(" (pc=%x)", dest);
        } else
        {
            printf(" (not taken)");
            pc += 5;
        }
    }





    /**
     * call 80 Da Db Dc Dd
     */
void call(int addr)
    {
        printf("call %x", addr);
        // make entry on stack /
        /* push old pc onto stack */
        doPush(pc);
        pc = addr;
    }





    /*     ret               90   */
void ret()
    {
        printf("ret");
        pc = p[*esp];
        *esp = *esp + 0x4;   // restore pc from stack /
        pc += 5;
        /* for how many bytes it took to call */
    }





    /*     pushl rA          A0 rAF   */
 void pushl(char reg)
    {
        int *rA = r1(reg);
        printf("pushl %x", *rA);
        pc += 2;
        doPush(*rA);
    }




void doPush(int val)
    {
        *esp = *esp - 4; // make entry on the stack 
        loadVal(val, *esp);
    }





void loadVal(int val, int addr)
    {
        p[addr] = 	     (char) (val & 0xf000);
        p[addr + 1] = (char ) (val & 0x0f00);
        p[addr + 2] = (char ) (val & 0x00f0);
        p[addr + 3] = (char ) (val & 0x000f);
    }





    /*      popl rA           B0 rAF   */
void popl(char reg)
    {
        int *rA = r1(reg);
        *rA = p[*esp];
        /* pull top stack value into rA */
        *esp = *esp + 0x4;
        /* remove entry from the stack */
        printf("popl %s %x", re[reg>>4], *rA);
        pc += 2;
    }



int printMemory(int start)
    {
        int words_on_screen = 1000;

        for (int i = start; i < start + words_on_screen; i++)
        {
            if (i == (0 % 4))
                printf("");
            printf("%x\t", p[i]);
        }
        return 0;
    }




int main(int argc , char * argv[]){
     int i:
	for(i=1;i<argc;i++){
		printf("%s\n",argv[i]);}
	if(argv<2){
		printf("there are not enough arguments\n");
		exit(1);}
	FILE *fp;
	fp=fopen(argv[1],"rb");
	fseek(fp,0,SEEK_END);
	long int length=ftell(fp);
	fseek(fp,0,SEEK_SET);
	char h[length];
	fread(h,1,length,fp);
	fp.close();
	parseFromASCII(h);
	printf("Parsed %s\n",argv[1]);
	decode(p);
		
	
		


}
