// "Borrowed" from RISC-V proxy kernel
// See LICENSE for license details.



#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>

#include <stdlib.h>
#include <ctype.h>
#include "uart.h"
#include "monitor.h"
#include "console.h"




void vprintk(const char* s, va_list vl)
{
char out[256];

  vsnprintf(out, sizeof(out), s, vl);
  write_console(out);
}

void printk(const char* s, ...)
{
  va_list vl;
  va_start(vl, s);

  vprintk(s, vl);

  va_end(vl);
}

void dump_tf(trapframe_t* tf)
{
  static const char*  regnames[] = {
    "z ", "ra", "sp", "gp", "tp", "t0",  "t1",  "t2",
    "s0", "s1", "a0", "a1", "a2", "a3",  "a4",  "a5",
    "a6", "a7", "s2", "s3", "s4", "s5",  "s6",  "s7",
    "s8", "s9", "sa", "sb", "t3", "t4",  "t5",  "t6"
  };

  tf->gpr[0] = 0;

  for(int i = 0; i < 32; i+=4)
  {
    for(int j = 0; j < 4; j++)
      printk("%s %lx%c",regnames[i+j],tf->gpr[i+j],j < 3 ? ' ' : '\n');
  }
  printk("pc %lx va %lx op %x sr %lx\n\n\n", tf->epc, tf->badvaddr,
         (uint32_t)tf->insn, tf->status);
}

void do_panic(const char* s, ...)
{
  va_list vl;
  va_start(vl, s);

  vprintk(s, vl);
  while(1); // endless loop

}

void kassert_fail(const char* s)
{
  register uintptr_t ra asm ("ra");
  do_panic("assertion failed @ %p: %s\n", ra, s);
}



void read_hex_str(char *b,int sz) {
char c;
char *p;

   p=b;
   c=toupper(readchar());
   while (c!='\r') {

      if (c==8 && p>b) {// backspace
        p--;
        writestr("\b \b");
      } else if ( ((c>='0' && c<='9') || (c>='A' && c<='F')) && p<(b+sz-1) ) {
          *p++=c;
          writechar(c); // echo
      }
      else
        writechar('\a'); // beep

      c=toupper(readchar());
   }
   *p='\0';

}

void read_num_str(char *b,int sz) {
char c;
char *p;

   p=b;
   c=readchar();
   while (c!='\r') {

      if (c==8 && p>b) {// backspace
        p--;
        writestr("\b \b");
      } else if (((c>='0' && c<='9') || c=='-') && p<(b+sz-1) ) {
          *p++=c;
          writechar(c); // echo
      }
      else
        writechar('\a'); // beep

      c=readchar();
   }
   *p='\0';

}


void hex_dump(void *mem,int numWords)
{
uint32_t *pmem = mem;
int i;

    for(i=0;i<numWords;i++) {
      if ((i % 4)==0) { // Write Memory address for every four words
        printk("\n%lx    ",(uint32_t)&pmem[i]);
      }
      printk("%lx ",pmem[i]);
      //writeHex(pmem[i]);
    }
}


long hstrtol(char *p, char **pp)
{
uint32_t v=0;
char c;
int digit;


  while(*p!='\0' ) {
    c=*p;
   
    if (c>='a' && c<='f') digit=c-'a'+10;
    else if (c>='A' && c<='F') digit=c-'A'+10;
    else if (c>='0' && c<='9') digit=c-'0';
    else {
      // Invalid char
      *pp=p;   // let pp point to it
      return v;
    }
    v= (v << 4 ) | digit;
    //printk("digit=%x  v=%x\n",digit,v);
    p++;
  }
  *pp=p;
  return v;
}


// Extended version. Returns error code 
// -1: invalid char
// -2 number to long
// 0 : no error
// retruns converted value in result
// accets nul or space as delimiter 

int hstrtolx(char *p, char **pp, uint32_t *result)
{
uint32_t v=0;
char c;
int digit;
int numdigits = 0;


  while(*p!='\0' && *p!=' ') {
    c=*p;
   
    if (c>='a' && c<='f') digit=c-'a'+10;
    else if (c>='A' && c<='F') digit=c-'A'+10;
    else if (c>='0' && c<='9') digit=c-'0';
    else {
      // Invalid char
      *pp=p;   // let pp point to it
      return -1;
    }
    v= (v << 4 ) | digit;
    if (++numdigits > 8) {
       return -2;
    }
    p++;
  }
  *pp=p;
  *result = v;
  return 0;
}



bool parseNext(char *p,char **p1,uint32_t *pV)
{
  if (p) {

   skipWhiteSpace(&p);
   if (*p=='\0')  {
     *p1=p;
     return false;
   }

   *pV=hstrtol(p,p1);
   return p!=*p1; // true when chars are consumed

  } else
   return false;

}

void skipWhiteSpace(char **pc)
{
char *p;

   p=*pc;
   while((*p==' ' || *p=='\t') && *p!='\0') p++; // skip white space
   *pc=p;
}

// get next string argument from command line
// p : pointer to current position in command line
// **p1: will be filled with pointer to first char after current argument, used for next call
// returns NULL when no argument found anymore, otherwise returns ptr to argument as zero terminated string
// as side effect will replace the first space in the buffer after the returned argument with 0
// returns the found argument (stripped from quotes) or NULL if there is no remaining arguemmt
// p1 is only updatet when an argument is returned

char* parsenext_arg(char *p, char **p1)
{
char delimiter;
char *start;
bool quoted;

  if (p && *p) {
    skipWhiteSpace(&p);
    if (!*p) return NULL; // end of buffer reached 
    // check of quotes
    quoted = *p=='\'' || *p=='"';
    if ( quoted ) {
       delimiter = *p;
       start = ++p;
    }  else {
       delimiter = ' ';
       start = p;  
    }
    while (*p && *p!=delimiter ) p++; 
    if (quoted && !*p) return NULL; // detect open quote...
    *p1 = (*p)?p+1:p;
    *p = '\0'; // Mark end of argument
    return start;
  }
  return NULL;
}


int readBuffer(char *b,int sz)
{
char c;
char *p;

   p=b;
   c=readchar();
   while (c!='\r') {

      if (c==8) {// backspace
        if (p>b) {
          p--;
          writestr("\b \b");
        }
      } else if ( p<(b+sz-1) ) {
          *p++=c;
          writechar(c); // echo
      }
      else
        writechar('\a'); // beep

      c=readchar();

   }
   *p='\0';
   write_console("\n");
   return p-b; // len
}
