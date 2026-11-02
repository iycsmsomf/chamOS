// #include "kernel.c"
#include "process.h"
#include "common.h"

void proc_a_entry(void);
void proc_b_entry(void);

__attribute__((naked)) void switch_context(uint32_t *prev_sp,
                                           uint32_t *next_sp) {
    __asm__ __volatile__(
        // Save callee-saved registers onto the current process's stack.
        "addi sp, sp, -13 * 4\n" // Allocate stack space for 13 4-byte registers
        "sw ra,  0  * 4(sp)\n"   // Save callee-saved registers only
        "sw s0,  1  * 4(sp)\n"
        "sw s1,  2  * 4(sp)\n"
        "sw s2,  3  * 4(sp)\n"
        "sw s3,  4  * 4(sp)\n"
        "sw s4,  5  * 4(sp)\n"
        "sw s5,  6  * 4(sp)\n"
        "sw s6,  7  * 4(sp)\n"
        "sw s7,  8  * 4(sp)\n"
        "sw s8,  9  * 4(sp)\n"
        "sw s9,  10 * 4(sp)\n"
        "sw s10, 11 * 4(sp)\n"
        "sw s11, 12 * 4(sp)\n"

        // Switch the stack pointer.
        "sw sp, (a0)\n"         // *prev_sp = sp;
        "lw sp, (a1)\n"         // Switch stack pointer (sp) here

        // Restore callee-saved registers from the next process's stack.
        "lw ra,  0  * 4(sp)\n"  // Restore callee-saved registers only
        "lw s0,  1  * 4(sp)\n"
        "lw s1,  2  * 4(sp)\n"
        "lw s2,  3  * 4(sp)\n"
        "lw s3,  4  * 4(sp)\n"
        "lw s4,  5  * 4(sp)\n"
        "lw s5,  6  * 4(sp)\n"
        "lw s6,  7  * 4(sp)\n"
        "lw s7,  8  * 4(sp)\n"
        "lw s8,  9  * 4(sp)\n"
        "lw s9,  10 * 4(sp)\n"
        "lw s10, 11 * 4(sp)\n"
        "lw s11, 12 * 4(sp)\n"
        "addi sp, sp, 13 * 4\n"  // We've popped 13 4-byte registers from the stack
        "ret\n"
    );
}

struct process procs[PROCS_MAX]; 

struct process *create_process(uint32_t pc){
    struct process *proc=NULL;
    int i;
    for (i=0;i<PROCS_MAX;i++){
        if (procs[i].state==PROC_UNUSED){
            proc = &procs[i];
            break;
        }
    }

    if (!proc)
        PANIC("no free process slots");

    //creating a fake process or emulated process
    uint32_t *sp = (uint32_t *)&proc->stack[sizeof(proc->stack)]; // so we are pointing sp to the last bit of the stack and then putting zero from last to first in the 13 registers
    *--sp = 0;                      // s11
    *--sp = 0;                      // s10
    *--sp = 0;                      // s9
    *--sp = 0;                      // s8
    *--sp = 0;                      // s7
    *--sp = 0;                      // s6
    *--sp = 0;                      // s5
    *--sp = 0;                      // s4
    *--sp = 0;                      // s3
    *--sp = 0;                      // s2
    *--sp = 0;                      // s1
    *--sp = 0;                      // s0
    *--sp = (uint32_t) pc;          // ra

    proc->pid=i+1;
    proc->sp=(uint32_t) sp;
    proc->state = PROC_RUNNABLE;

    return proc;



    
}
//delay function implements a busy wait to prevent the character output from becoming too fast, which would make your terminal unresponsive. nop instruction is a "do nothing"
void delay(void) {
    for (int i = 0; i < 30000000; i++)
        __asm__ __volatile__("nop"); // do nothing
}

struct process *proc_a;
struct process *proc_b;





void proc_a_entry(void){
    printf("Starting process A ");
    while(1){
        putchar('A');
        switch_context(&(proc_a->sp),&(proc_b->sp));
        delay();
    }

}
void proc_b_entry(void){
    printf("Starting process B ");
    while(1){
        putchar('B');
        switch_context(&(proc_b->sp),&(proc_a->sp));
        delay();
    }

}

struct process *current_process;
struct process *idle_process;

void yield(void){
    struct process *next=idle_process;
    for (int i=0;i<PROCS_MAX;i++){
        struct process *proc=&(procs[(current_process->pid + i)% PROCS_MAX]);
        if (proc->state== PROC_RUNNABLE){
            next=proc;
            break;
        }
    }

    if (next==current_process){
        return;
    }
        __asm__ __volatile__(
        "csrw sscratch, %[sscratch]\n"
        :
        : [sscratch] "r" ((uint32_t) &next->stack[sizeof(next->stack)])
    );
    
    struct process *prev=current_process;
    current_process=next;
    
    switch_context(&prev->sp,&current_process->sp);

}

