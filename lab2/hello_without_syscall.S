.data

msg:
	.ascii "Hello world\n"
	len = . - msg
.text
	.global _start

_start:
	mov $4, %eax
	mov $1, %ebx
	mov $msg, %ecx
	mov $len, %edx
    int $0x80
	mov $1, %eax  
	xor %ebx, %ebx
	int $0x80