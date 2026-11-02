//defining data types of our os
#include "kernel.h"
#include "common.h"
#include "process.h"
#include "process.c"

typedef unsigned char uint8_t;
typedef unsigned int uint32_t;
typedef uint32_t size_t; //means 32 bit system. size_t is the return type of sizeof function. its tha max value the system can hold and an unsigned int

//extern mean the variables come frfom other files

extern char __bss[],__bss_end[],__stack_top[] ;



void kernel_entry(void);
void handle_trap(struct trap_frame *f);
paddr_t alloc_pages(uint32_t n);



//ENTRY(boot) in kernel.ld happens here then this function uses assembly code to go to kernel_main function


__attribute__((section(".text.boot")))
__attribute__((naked))
void boot(void){
    __asm__ __volatile__(
        "mv sp, %[stack_top]\n" // not __stack_top as its convention for c variable  but here its assembly var
        "j kernel_main\n"      //jump to kernel_main function
        :
        :[stack_top] "r" (__stack_top)    //relating assembly var stack_top to the c var __stack_top

    );
}


struct sbiret sbi_call(long arg0, long arg1, long arg2, long arg3, long arg4, long arg5, long fid, long eid){ // its a function with return type struct sbiret
    register long a0 __asm__("a0")=arg0;
    register long a1 __asm__("a1")=arg1;
    register long a2 __asm__("a2")=arg2;
    register long a3 __asm__("a3")=arg3;
    register long a4 __asm__("a4")=arg4;
    register long a5 __asm__("a5")=arg5;   //in a0 register a0 points  such is ensured by this line as next function used rewuired precisely these 7 actual registers
    register long a6 __asm__("a6")=fid;
    register long a7 __asm__("a7")=eid;

    __asm__ __volatile__("ecall"
                         : "=r"(a0), "=r"(a1)
                         : "r"(a0), "r"(a1), "r"(a2), "r"(a3), "r"(a4), "r"(a5),
                           "r"(a6), "r"(a7)
                         : "memory");

    //inline assembly has format (assemblycode in "" : output variables : input variables: and something i forgot)
    return (struct sbiret){.error=a0 , .value= a1};
    
    
}

void putchar(char ch){ //puts a character on the console kinda like printing
    sbi_call(ch, 0, 0, 0, 0, 0, 0, 1 /* Console Putchar */); //the specific parameters lead to putchar functionality
}

void kernel_main(void){
    memset(__bss,0,(size_t)__bss_end - (size_t)__bss );
    printf("\n\nHello %s\n", "World!");

    const char *s = "\n\nHellow World\n";
    WRITE_CSR(stvec, (uint32_t) kernel_entry); // exception
    __asm__ __volatile__("unimp"); // new


    for (int i=0;s[i]!='\0';i++){
        putchar(s[i]);
    }
    for(;;){
        __asm__ __volatile__("wfi");
    }; //ensures that the function runs forever as its te main function

    idle_process = create_process((uint32_t) NULL);
    idle_process->pid = 0; // idle
    current_process = idle_process;

    proc_a = create_process((uint32_t) proc_a_entry);
    proc_b = create_process((uint32_t) proc_b_entry);

    yield();
    PANIC("switched to idle process");
}


__attribute__((naked))
__attribute__((aligned(4)))
void kernel_entry(void) {
    __asm__ __volatile__(
        // Retrieve the kernel stack of the running process from sscratch.
        "csrrw sp, sscratch, sp\n"

        "addi sp, sp, -4 * 31\n"
        "sw ra,  4 * 0(sp)\n"
        "sw gp,  4 * 1(sp)\n"
        "sw tp,  4 * 2(sp)\n"
        "sw t0,  4 * 3(sp)\n"
        "sw t1,  4 * 4(sp)\n"
        "sw t2,  4 * 5(sp)\n"
        "sw t3,  4 * 6(sp)\n"
        "sw t4,  4 * 7(sp)\n"
        "sw t5,  4 * 8(sp)\n"
        "sw t6,  4 * 9(sp)\n"
        "sw a0,  4 * 10(sp)\n"
        "sw a1,  4 * 11(sp)\n"
        "sw a2,  4 * 12(sp)\n"
        "sw a3,  4 * 13(sp)\n"
        "sw a4,  4 * 14(sp)\n"
        "sw a5,  4 * 15(sp)\n"
        "sw a6,  4 * 16(sp)\n"
        "sw a7,  4 * 17(sp)\n"
        "sw s0,  4 * 18(sp)\n"
        "sw s1,  4 * 19(sp)\n"
        "sw s2,  4 * 20(sp)\n"
        "sw s3,  4 * 21(sp)\n"
        "sw s4,  4 * 22(sp)\n"
        "sw s5,  4 * 23(sp)\n"
        "sw s6,  4 * 24(sp)\n"
        "sw s7,  4 * 25(sp)\n"
        "sw s8,  4 * 26(sp)\n"
        "sw s9,  4 * 27(sp)\n"
        "sw s10, 4 * 28(sp)\n"
        "sw s11, 4 * 29(sp)\n"

        // Retrieve and save the sp at the time of exception.
        "csrr a0, sscratch\n"
        "sw a0,  4 * 30(sp)\n"

        // Reset the kernel stack.
        "addi a0, sp, 4 * 31\n"
        "csrw sscratch, a0\n"

        "mv a0, sp\n"
        "call handle_trap\n"

        "lw ra,  4 * 0(sp)\n"
        "lw gp,  4 * 1(sp)\n"
        "lw tp,  4 * 2(sp)\n"
        "lw t0,  4 * 3(sp)\n"
        "lw t1,  4 * 4(sp)\n"
        "lw t2,  4 * 5(sp)\n"
        "lw t3,  4 * 6(sp)\n"
        "lw t4,  4 * 7(sp)\n"
        "lw t5,  4 * 8(sp)\n"
        "lw t6,  4 * 9(sp)\n"
        "lw a0,  4 * 10(sp)\n"
        "lw a1,  4 * 11(sp)\n"
        "lw a2,  4 * 12(sp)\n"
        "lw a3,  4 * 13(sp)\n"
        "lw a4,  4 * 14(sp)\n"
        "lw a5,  4 * 15(sp)\n"
        "lw a6,  4 * 16(sp)\n"
        "lw a7,  4 * 17(sp)\n"
        "lw s0,  4 * 18(sp)\n"
        "lw s1,  4 * 19(sp)\n"
        "lw s2,  4 * 20(sp)\n"
        "lw s3,  4 * 21(sp)\n"
        "lw s4,  4 * 22(sp)\n"
        "lw s5,  4 * 23(sp)\n"
        "lw s6,  4 * 24(sp)\n"
        "lw s7,  4 * 25(sp)\n"
        "lw s8,  4 * 26(sp)\n"
        "lw s9,  4 * 27(sp)\n"
        "lw s10, 4 * 28(sp)\n"
        "lw s11, 4 * 29(sp)\n"
        "lw sp,  4 * 30(sp)\n"
        "sret\n"
    );
}

void handle_trap(struct trap_frame *f) {
    uint32_t scause = READ_CSR(scause);
    uint32_t stval = READ_CSR(stval);
    uint32_t user_pc = READ_CSR(sepc);

    PANIC("unexpected trap scause=%x, stval=%x, sepc=%x\n", scause, stval, user_pc);
}

extern char __free_ram[], __free_ram_end[];

paddr_t alloc_pages(uint32_t n) {
    static paddr_t next_paddr = (paddr_t) __free_ram;
    paddr_t paddr = next_paddr;
    next_paddr += n * PAGE_SIZE;

    if (next_paddr > (paddr_t) __free_ram_end)
        PANIC("out of memory");

    memset((void *) paddr, 0, n * PAGE_SIZE);
    return paddr;
}



