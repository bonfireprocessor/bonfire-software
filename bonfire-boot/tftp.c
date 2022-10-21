#include <stdbool.h>
#include <stdint.h>
#include "bonfire.h"
#include "console.h"
#include "monitor.h"
#include "littlefs_hal.h"


#include "pico_stack.h"
#include "pico_ipv4.h"
#include "pico_tftp.h"


#define tftplog printk//temporary 

#define MAX_FILESIZE (16 * 1024 * 1024) // 16 Megabyte

typedef  struct {
  char filename[64];
  uint16_t opcode;
  uint8_t *buffer;
  unsigned long size;
  lfs_file_t fd;
  char cmd;
} t_file_ctx;

#define BUFFER_ADDRESS LOAD_BASE // temporary hack !!!!!

static bool buffer_busy = false;
static bool transfer_inprogress = false;         


static int save(t_file_ctx *ctx)
{
int res;

   fd_init(&ctx->fd);
   res=lfs_file_open(&lfs,&ctx->fd,ctx->filename,LFS_O_CREAT|LFS_O_TRUNC|LFS_O_WRONLY);
   if (res!=LFS_ERR_OK) return res;
   res=lfs_file_write(&lfs,&ctx->fd,ctx->buffer,ctx->size);
   lfs_file_close(&lfs,&ctx->fd);
   return res;

}

static void tftp_write_finish(t_file_ctx *ctx)
{
    switch (ctx->cmd) {
    case '\0':
      tftplog("writing to flash, please wait...\n");
      int res=save(ctx);
      if (res>0) {
          tftplog("file %s written to flash\n",ctx->filename);     
      } else {
          tftplog("file write failed with errorcode %ld\n",res);
      }
      break;
    case '@':
      tftplog("File written to memory @%lx\n",ctx->buffer);
      break;
    case '!':
      tftplog("run image at %lx\n",ctx->buffer);
      clear_csr(mstatus,MSTATUS_MIE);
      flush_dache();
      start_user((uint32_t)(ctx->buffer),USER_STACK );
      break;  
    default:
      break;
    }
    
}



static int cb_tftp_txrx(struct pico_tftp_session *session, uint16_t event,
                        uint8_t *block, int32_t _len, void *arg)
{
t_file_ctx *ctx = (t_file_ctx *)arg;  
const char *trans_error = "interal error, aborting transfer\n"; 

long new_size;


switch (event) {

    case PICO_TFTP_EV_OK:
     
      if ( ctx->opcode==PICO_TFTP_RRQ ) {
          int32_t result;
          uint8_t buffer[PICO_TFTP_PAYLOAD_SIZE];

          result = lfs_file_read(&lfs,&ctx->fd,buffer,PICO_TFTP_PAYLOAD_SIZE);
          if ( result<=0) {
             lfs_file_close(&lfs,&ctx->fd);
          }

          if ( result<0 ) {
              pico_tftp_abort(session,-1,trans_error);
              return 0;
          }            
          pico_tftp_send(session,buffer,result);
          ctx->size += result;
          
      } else {
         //recevice
         if ( _len>0 ) {
            new_size = ctx->size + _len;
            if (new_size>MAX_FILESIZE) {
              tftplog(trans_error);
              pico_tftp_abort(session,-1,trans_error);
              return 0;
            } else {
                //printk("size %ld, %lx<-%lx (%d) \n",ctx->size,ctx->buffer+ctx->size,block,_len);
                memcpy(ctx->buffer+ctx->size,block,_len);
                ctx->size = new_size;
            }
        }
      }
      break;
    case PICO_TFTP_EV_ERR_PEER:
    case PICO_TFTP_EV_ERR_LOCAL:
      transfer_inprogress = false;
      buffer_busy = false;
      tftplog("tftp error %s at session %lx\n",(char*)block, (uint32_t)session);
      break;
    case  PICO_TFTP_EV_SESSION_CLOSE:
      tftplog("tftp  session %lx closed transfered %ld bytes\n",(uint32_t)session,ctx->size);
      if (ctx->opcode==PICO_TFTP_WRQ && ctx->size>0) {
         tftp_write_finish(ctx); 
      } // else if (ctx->opcode==PICO_TFTP_RRQ && ctx->fd>=0) {
      //    lfs_file_close(&lfs,&ctx->fd);
      // }
      buffer_busy = false;
      free(ctx);
      transfer_inprogress = false;
      break;
    default:
      tftplog("unsupported event: %d\n",event);  
  } 
  return 0;
}                        


static t_file_ctx *open_file(char * filename, char* mode, uint16_t opcode)
{
t_file_ctx *ctx;
uint8_t * buffer = (uint8_t*)BUFFER_ADDRESS; // malloc(MAX_FILESIZE);
char cmd ='\0';

 
 

  switch(opcode) {
    case PICO_TFTP_RRQ:
        ctx=malloc(sizeof(t_file_ctx)); 
        if (!ctx) return NULL;
        fd_init(&ctx->fd);
        int result = lfs_file_open(&lfs,&ctx->fd,filename,LFS_O_RDONLY);
        if (result!=LFS_ERR_OK) {
          free(ctx);
          return NULL;
        } 
        break;

    case PICO_TFTP_WRQ:
        if (buffer_busy) return NULL;
        if (filename[0]=='@' || filename[0]=='!') {
          cmd = filename[0];
          char * dummy;
          uint32_t addr;
          int res = hstrtolx(filename+1,&dummy,&addr);
          if (res<0) {
            tftplog("Invalid target address in %s\n",filename);
            return NULL;
          } else {
            tftplog("Writing to memory address %lx\n",addr);
            buffer = (uint8_t*)addr;
          }
        }
        ctx=malloc(sizeof(t_file_ctx)); 
        if (!ctx) 
           return NULL;
        else    
          buffer_busy = true;
        break;

    default:
      return NULL;  
      
  }
  
  
  // Complete setup of File Context
  strncpy(ctx->filename,filename,63);
  ctx->cmd = cmd;
  ctx->opcode = opcode;
  ctx->buffer = buffer;
  //tftplog("ctx buffer: %lx filename: %s\n",ctx->buffer,ctx->filename);
  ctx->size = 0;
  return ctx;
  

}



static void tftp_server_callback( union pico_address *addr, uint16_t port,
                                  uint16_t opcode, char *filename, int32_t len )
{

struct pico_tftp_session *session;
t_file_ctx *ctx;
char ip_s[16];

  pico_ipv4_to_string(ip_s,addr->ip4.addr);
  tftplog("new request from remote address %s port %d opcode %d\n", ip_s,short_be(port),opcode);

  if (transfer_inprogress) {
    pico_tftp_reject_request(addr,port,-1,"already transfer in progress");
    return;
  }
  
  switch(opcode) {
    case PICO_TFTP_RRQ:
        ctx = open_file(filename,"r",opcode);
        if (!ctx) {
          pico_tftp_reject_request(addr,port,-1,"file could not be opened");
        } else {
          transfer_inprogress = true;
          session = pico_tftp_session_setup(addr,PICO_PROTO_IPV4);
          pico_tftp_start_tx(session,port,filename,cb_tftp_txrx,ctx);
          tftplog("tftp get session %lx for %s\n", (uint32_t)session, filename);  
        }
        break;
    case PICO_TFTP_WRQ:
        ctx = open_file(filename,"w",opcode);
        if (!ctx) { 
          pico_tftp_reject_request(addr,port,-1,"out of memory");
          return;
        } else {
          transfer_inprogress = true;
          session = pico_tftp_session_setup(addr,PICO_PROTO_IPV4);
          pico_tftp_start_rx(session,port,filename,cb_tftp_txrx,ctx);
          tftplog("tftp put session %lx for %s\n", (uint32_t)session, filename);  
        }
        break;
    default:
        return;    
  }
 
}


void tftp_server_main()
{

   int err=pico_tftp_listen(PICO_PROTO_IPV4,tftp_server_callback);
   tftplog(err?"tfp start failed\n":"tftp start sucessfull\n");
}
