section .data ;declare section
memA DD 0
memD DD 0
section .text
global main
main:
mov eax , 10
mov ebx , 15
mov eax , eax
mov ebx , ebx
mov [memD] , edx
mov [memA] , eax
mov eax , dword [memA]
imul ebx
mov dword [memA] , eax
mov eax , [memA]
mov edx , [memD]
mov ebx , eax
exit_program:
mov eax, 1 ;Linux system call to return
 int 0x80
