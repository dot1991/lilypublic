.code
RunWithKernelStack PROC
stac
swapgs
mov gs:[10h],rsp
mov rsp,gs:[1A8h]
sub rsp,40h					;The kernel stack must be aligned as 0x10 bytes
sti
call rdx
cli
add rsp,40h
mov rsp,gs:[10h]
swapgs
clac
ret
RunWithKernelStack ENDP
END