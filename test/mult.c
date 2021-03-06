#include "bonfire.h"
#include "uart.h"
#include "bonfire_gpio.h"
#include "mem_rw.h"


#include <stdlib.h>
#include <stdio.h>
#include "console.h"

void readnumstr(char *b,int sz) {
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


void newline() {
  write_console("\n");    
}

int main() {
char buff[80];
int x,y,result;
uint32_t impid;


 
  setBaudRate(PLATFORM_BAUDRATE);
  write_console("\nWelcome to Bonfire\n");
  
  
  impid=get_impid();
  
  printk("\nProcessor ID: %x\n",impid);
  
  while(1) {
    writestr("Enter x (max 10 digits):");
    writechar('\x1a'); // End simulation
    readnumstr(buff,11);
    x=atoi(buff);
    newline();

    writestr("Enter y (max 10 digits):");
    readnumstr(buff,11);
    y=atoi(buff);
    newline();

    result=x*y;
     
    printk(" %d * %d = %d\n",x,y,result);

  }
  return 0;

}
