#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#include "bonfire.h"
#include "monitor.h"

#include "console.h"
#include "uart.h"
#include "spi.h"
#include "spiffs_hal.h"

#define MAX_ARGS 16

typedef int (*t_shellfunc)(int argc,char **argv);

typedef struct {
  char * cmd_str;
  t_shellfunc cmd_func;
} t_shellcomand;


static char* option_error = "Invalid options/arguments\n";

static int ls_cmd(int argc,char **argv)
{
spiffs_DIR d;
struct spiffs_dirent e;
struct spiffs_dirent *pe = &e;

  if (argc==1) {
    SPIFFS_opendir(&fs, "/", &d);
    while ((pe = SPIFFS_readdir(&d, pe))) {
      printk("%s  %d bytes \n", pe->name,  pe->size);
    }
    SPIFFS_closedir(&d);
    return 0;
  } else {
    printk(option_error);
    return -1;
  }
  
}


static int fsinfo_cmd(int argc,char **argv)
{
  if (argc==1) {
    uint32_t total, used;
    SPIFFS_info(&fs,&total,&used);
    printk("Total: %lu bytes, used %lu bytes\n",total,used);    
    return 0;
  }  else   {
      if (argv[1][0]=='-' && argv[1][1]=='x') {
        SPIFFS_vis(&fs);
        return 0;
      } else {
        printk(option_error);
        return -1;
      }  
  }
}


static int rm_cmd(int argc,char **argv)
{
int32_t result;
char *fmt;

    if (argc==2) {
        result=SPIFFS_remove(&fs,argv[1]);
        if (result==SPIFFS_OK) {
            fmt="removed file %s\n";
        } else {
            fmt="remove %s failed (%ld)\n";
        }
        printk(fmt,argv[1],result);
        return result;
    } else  {
      printk(option_error);
      return -1;
    }
}


static int cat_cmd(int argc,char **argv)
{
spiffs_file fd;
char *fmt;
int len;
char buffer[257];

    if (argc==2) {
        fd=SPIFFS_open(&fs,argv[1],SPIFFS_O_RDONLY,0);
        if (fd>=0) {
            while (!SPIFFS_eof(&fs,fd)) {
                len = SPIFFS_read(&fs,fd,buffer,sizeof(buffer-1));
                if (len) {
                    buffer[len]='\0';
                    write_console(buffer);
                }    
            }
            SPIFFS_close(&fs,fd);
            return 0;
        } else {
            fmt="cannot open file %s\n";
            printk(fmt,argv[1]);
            return -1;
        }
        
    } else  {
      printk("usage rm <filename>\n");
      return -1;
    }
}

static int mv_cmd(int argc,char **argv)
{
    if (argc==3) {
        int32_t result = SPIFFS_rename(&fs,argv[1],argv[2]);
        if (result!=SPIFFS_OK) {
            printk("mv failed, error %ld\n",result);
        }
        return result;
    } else {
      printk("usage mv <source> <dest>\n");
      return -1; 
    }
}


static int fsck_cmd(int argc,char **argv)
{
    if (argc==1) {
        int32_t result = SPIFFS_check(&fs);
        if (result!=SPIFFS_OK) {
            printk("fsck failed, error %ld\n",result);
        }
        return result;
    } else {
      printk(option_error);
      return -1; 
    }    
}


static int format_cmd(int argc,char **argv)
{
    if (argc==1) {
        printk("All data will be deleted!\n");
        SPIFFS_unmount(&fs);
        int32_t result=SPIFFS_format(&fs);
        if (result!=SPIFFS_OK) {
            printk("fsck format, error %ld\n",result);
        } else {
          // remount
          result = spiffs_init(get_spiflash(),4096,false);
        }
        return result;
    } else {
      printk(option_error);
      return -1; 
    }    

}

static int run_cmd(int argc,char **argv)
{
spiffs_file fd;
char *fmt;
int len;


    if (argc==2) {
        fd=SPIFFS_open(&fs,argv[1],SPIFFS_O_RDONLY,0);
        if (fd>=0) {
           
          len = SPIFFS_read(&fs,fd,LOAD_BASE,LOAD_SIZE);
          if (len>0) {
            printk("Loaded %ld bytes\n",len);
            SPIFFS_close(&fs,fd);
            flush_dache();
            start_user((uint32_t)LOAD_BASE,(uint32_t)USER_STACK);
          } else {
            printk("Load failed or file empty\n");
          }
          
        } else {
            fmt="cannot open file %s\n";
            printk(fmt,argv[1]);
            return -1;
        }
        
    } else  {
      printk("usage rm <filename>\n");
      return -1;
    }
}


static t_shellcomand cmds[] = {
   {"ls", ls_cmd},
   {"fsinfo",fsinfo_cmd},
   {"rm",rm_cmd},
   {"cat",cat_cmd},
   {"mv",mv_cmd},
   {"fsck",fsck_cmd},
   {"format",format_cmd},
   {"run",run_cmd},
   {"exit", NULL},
   {NULL, NULL}
};


void shell()
{
char cmd[128];
char *p;
char *arg;

char *argv[MAX_ARGS];
int argc;
bool found;

  while(1) {
     write_console("\n$");
     readBuffer(cmd,sizeof(cmd));
     p = cmd;
     argc = 0;
     while ( (argc< MAX_ARGS) &&  (arg = parsenext_arg(p,&p)) != NULL) {
       //printk( "arg[%d]=%s (%lx)\n",argc, arg, (uintptr_t)arg );
       argv[argc++] = arg; 
     } 
     if (argc>=1) {
         found = false;
         for(int i=0;cmds[i].cmd_str!=NULL;i++) {
            t_shellcomand *c = &cmds[i]; 
            if (strcmp(c->cmd_str,argv[0])==0) {
                found = true;
                if (c->cmd_func) {
                    c->cmd_func(argc,argv);
                    break;    
                } else {
                    return; // exit function....
                }
            }
         }
         if (!found) {
             printk("invalid command\n");
         }
     }
  }
}