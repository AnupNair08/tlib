/**
 * @file mangle.h
 * @author Anup Nair & Hrishikesh Athalye
 * @brief glibc pointer mangling handling
 * @version 0.1
 * @date 2021-05-02
 * 
 * @copyright Copyright (c) 2021
 * 
 */

/**
 * @brief Mangled pointer address
 */
typedef unsigned long address_t;

#ifdef __x86_64__
#define JB_BP 5
#define JB_SP 6
#define JB_PC 7
#else
#define JB_BP 3
#define JB_SP 4
#define JB_PC 5
#endif

/* A translation is required when using an address of a variable.
   Use this as a black box in your code. 
   This code was referenced from : https://sites.cs.ucsb.edu/~chris/teaching/cs170/projects/proj2.html*/
address_t translate_address(address_t addr)
{
    address_t ret;
    asm volatile("xor    %%fs:0x30,%0\n"
                 "rol    $0x11,%0\n"
                 : "=g"(ret)
                 : "0"(addr));
    return ret;
}