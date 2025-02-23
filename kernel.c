typedef unsigned char uint8_t;
typedef unsigned int uint32_t;
typedef uint32_t size_t;


// Declaration of linker script symbols, achieved using ``extern``
// set linker symbols to array type to ensure that we use the address instead of value
extern char __bss[], __bss_end[], __stack_top[];

void *memset(void *buf,  char val, size_t size) {
    uint8_t *p = (uint8_t *) buf;
    while (size) {
        *p = val;
        p++;
        size--;
    }

    return buf;
}

void kernel_main(void) {

    memset(__bss, 0, (size_t) __bss_end - (size_t) __bss);
    for (;;);
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



