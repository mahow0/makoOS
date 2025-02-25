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

void kernel_main(void) {

    printf("\n\n%s%s", "Welcome", " to Mako OS!\n");
    printf("1 + 2 = %d, %x\n", 1 + 2, 0xff901);

    for (;;) {
        __asm__ __volatile__("wfi");
    }
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



