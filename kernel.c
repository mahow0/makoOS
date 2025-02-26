#include "kernel.h"
#include "common.h"

typedef unsigned char uint8_t;
typedef unsigned int uint32_t;
typedef uint32_t size_t;




// Declaration of linker script symbols, achieved using ``extern``
// set linker symbols to array type to ensure that we use the address instead of value
extern char __bss[], __bss_end[], __stack_top[];



struct sbiret sbi_call(long reg0, long reg1, long reg2, long reg3, long reg4,
                        long reg5, long fid, long eid) {
        
        register long a0 __asm__("a0") = reg0;
        register long a1 __asm__("a1") = reg1;
        register long a2 __asm__("a2") = reg2;
        register long a3 __asm__("a3") = reg3;
        register long a4 __asm__("a4") = reg4;
        register long a5 __asm__("a5") = reg5;
        register long a6 __asm__("a6") = fid;
        register long a7 __asm__("a7") = eid;

        __asm__ __volatile__ ("ecall"
                                : "=r"(a0), "=r"(a1)
                                : "r"(a0), "r"(a1), "r"(a2), "r"(a3), "r"(a4), "r"(a5),
                                "r"(a6), "r"(a7)
                                : "memory");
        
        struct sbiret ret = {.error = a0, .value = a1};
        return ret;
}


void putchar(char c) {
    sbi_call(c, 0, 0, 0, 0, 0, 0, 1);
}



// Exception handler entry point -- this saves CPU registers onto the stack, jumps to the exception handler, and then restores those registers before returning
__attribute__((naked))
__attribute__((aligned(4)))
void kernel_entry(void) {
    __asm__ __volatile__(
        "csrw sscratch, sp\n"
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

        "csrr a0, sscratch\n"
        "sw a0, 4 * 30(sp)\n"

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

// Exception handler,
// scause: type of exception
// stval: additional info about the exception (value is exception-dependent)
// sepc: program counter at point which exception was triggered
// to handle exceptions, we dump the values of CSR and panic
void handle_trap(struct trap_frame *f) {
    uint32_t scause = READ_CSR(scause);
    uint32_t stval = READ_CSR(stval);
    uint32_t user_pc = READ_CSR(sepc);

    PANIC("unexpected trap scause=%x, stval=%x, sepc=%x\n", scause, stval, user_pc);
}

// Memory management
extern char __free_ram[], __free_ram_end[];

paddr_t alloc_pages(uint32_t n) {
    // ``next_paddr`` -> beginning of newly unallocated space
    // static keyword indicates that ``next_paddr`` retains its value across function calls
    static paddr_t next_paddr = (paddr_t) __free_ram;
    paddr_t paddr = next_paddr;
    next_paddr += n * PAGE_SIZE;

    if (next_paddr > (paddr_t) __free_ram_end)
        PANIC("out of memory");
    
    memset((void *) paddr, 0, n * PAGE_SIZE);
    return paddr;
}

void kernel_main(void) {

    memset(__bss, 0, (size_t) __bss_end - (size_t) __bss);

    paddr_t paddr0 = alloc_pages(2);
    paddr_t paddr1 = alloc_pages(1);
    printf("__free_ram: %x\n", __free_ram);
    printf("alloc_pages test: paddr0=%x\n", paddr0);
    printf("alloc_pages test: paddr1=%x\n", paddr1);

    PANIC("booted!");
}




// this places ``boot`` in the address denoted by the ``.text.boot`` of the linker script 
// for OpenSBI, this inserts the entry point code at 0x80200000 
__attribute__((section(".text.boot")))
// instructs the compiler to not generate unnecessary code before or after the function body
// necessary for ensuring that the function body is identical to the asm body
__attribute__((naked))
void boot(void) {
    __asm__ __volatile__ (
        "mv sp, %[stack_top]\n" // set stack pointer (why is this necessary? why doesn't linker do this for us?)
        "j kernel_main\n" // jump to the kernel_main function
        :
        : [stack_top] "r" (__stack_top) // pass the stack_top symbol into the asm code
    );
}



