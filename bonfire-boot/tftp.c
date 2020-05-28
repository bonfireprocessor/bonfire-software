#include <stdbool.h>
#include <stdint.h>
#include "console.h"
#include "monitor.h"

#include "pico_stack.h"
#include "pico_ipv4.h"
#include "pico_tftp.h"


#define tftplog printk//temporary 

#define MAX_FILESIZE (16 * 1024 * 1024) // 16 Megabyte

typedef  struct {
  uint16_t opcode;
  uint8_t *buffer;
  unsigned long size;
} t_file_ctx;

#define BUFFER_ADDRESS LOAD_BASE // temporary hack !!!!!
                                        

static int cb_tftp_txrx(struct pico_tftp_session *session, uint16_t event,
                        uint8_t *block, int32_t _len, void *arg)
{
t_file_ctx *ctx = (t_file_ctx *)arg;  
const char *trans_error = "out of memory error, aborting transfer\n"; 

long new_size;


switch (event) {

    case PICO_TFTP_EV_OK:
     
      if ( ctx->opcode==PICO_TFTP_RRQ ) {
          pico_tftp_abort(session,-1,trans_error);
          return 0;
      } else {
         //recevice
         if ( _len>0 ) {
            new_size = ctx->size + _len;
            if (new_size>MAX_FILESIZE) {
              tftplog(trans_error);
              pico_tftp_abort(session,-1,trans_error);
              return 0;
            } else {
                //printk("%lx<-%lx (%d) \n",ctx->buffer+ctx->size,block,_len);
                memcpy(ctx->buffer+ctx->size,block,_len);
                ctx->size = new_size;
            }
        }
      }
      break;
    case PICO_TFTP_EV_ERR_PEER:
    case PICO_TFTP_EV_ERR_LOCAL:
      tftplog("tftp error  %s at session %lx\n",(char*)block, (uint32_t)session);
      break;
    case  PICO_TFTP_EV_SESSION_CLOSE:
      tftplog("tftp  session %lx closed transfered %ld bytes to %lx\n",(uint32_t)session,ctx->size,ctx->buffer);
      //free(ctx->buffer);
      free(ctx);
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

    if (buffer) {
        tftplog("Allocated buffer at %lx\n",buffer);
        ctx=malloc(sizeof(ctx));
        ctx->opcode = opcode;
        ctx->buffer = buffer;
        ctx->size = 0;
        return ctx;
    } else
    {
        return NULL;
    }
}


static void tftp_server_callback( union pico_address *addr, uint16_t port,
                                  uint16_t opcode, char *filename, int32_t len )
{

struct pico_tftp_session *session;
t_file_ctx *ctx;
char ip_s[16];

  pico_ipv4_to_string(ip_s,addr->ip4.addr);
  tftplog("new request from remote address %s port %d opcode %d\n", ip_s,short_be(port),opcode);
  
  switch(opcode) {
    case PICO_TFTP_RRQ:
        pico_tftp_reject_request(addr,port,-1,"tftp get not supported");
        break;
    case PICO_TFTP_WRQ:
        ctx = open_file(filename,"w",opcode);
        if (!ctx) { 
          pico_tftp_reject_request(addr,port,-1,"out of memory");
          return;
        } else {
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
