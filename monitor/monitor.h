#ifndef __MONITOR_H
#define __MONITOR_H

#define LOAD_BASE ((void*)0x010000)


typedef struct
{
  long gpr[32];
  long status;
  long epc;
  long badvaddr;
  long cause;
  long insn;
} trapframe_t;


typedef struct {
  uint32_t magic;
  uint32_t nPages;  // File length in Number of 4096 Byte Units
  uint32_t loadAddress; // Address to load 
  uint32_t brkAddress;
  uint8_t  hash[32]; // SHA-256 Hash over code (without this header)
} t_flash_header_data;

typedef struct {
  t_flash_header_data header;
  uint8_t padding[256-sizeof(t_flash_header_data)];

} t_flash_header;

#define HEADER_BASE ((void*)(LOAD_BASE-sizeof(t_flash_header))) // Place Flash Header  below LOAD_BASE
#define flash_header (((t_flash_header*)HEADER_BASE)->header) // Macro to simplify header access

#define C_MAGIC 0x55aaddcc // new MAGIC...


void do_break(uint32_t arg0,...);

void start_user(uint32_t pc,uint32_t sp);

long sys_time(long* loc); // implemented in syscall.c

// RAM independant delay loop which takes 6 clocks/count
void delay_loop(uint32_t count);

#define LOOP_TIME (CLK_PERIOD * 6)

void test_dcache(int n);
void print_cache_size();

#endif
