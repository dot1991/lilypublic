.code
vmcall_intel PROC
mov rax,r8
vmcall
ret
vmcall_intel ENDP

vmcall_amd PROC
mov rax,r8
vmmcall
ret
vmcall_amd ENDP
END