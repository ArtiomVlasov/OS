int main(int argc, char const *argv[])
{
    const char msg[] = "Hello world\n";
    const long len = 13;
    __asm__ volatile(
        "movq $1, %%rax\n"
	    "movq $1, %%rdi\n"
	    "leaq %0, %%rsi\n"
	    "movq %1, %%rdx\n"
        "syscall\n"
        :: "m"(msg), "r"(len)
        : "%rax", "%rdi", "%rsi", "%rdx"
    );
    
    __asm__ volatile(
        "movq $60, %%rax\n"
        "xorq %%rdi, %%rdi\n"
        "syscall\n"
        ::: "%rax", "%rdi"
    );   
}
