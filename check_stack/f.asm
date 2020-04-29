global f
global fff
extern __chkstk

%define STACK_SIZE (4096*6)

segment .text
f:
%ifdef USE_CHKSTK
	mov [rsp+8], ecx
	mov eax, STACK_SIZE
	call __chkstk
	sub rsp, rax
%else
	sub rsp, STACK_SIZE
%endif
	mov rax, [rsp]
	add rsp, STACK_SIZE
	ret

fff:
	ret