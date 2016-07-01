global b
global c
global f
global g

segment .data
a dq 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0
b dq 0, 0, 0, 0, 0, 0, 0, 0
c dq 0, 0, 0, 0, 0, 0, 0, 0

segment .text
f:
	vxorpd zmm0, zmm0, zmm0
	mov rax, a
	vaddpd zmm0, zmm0, [rax]
	mov rax, b
	vmovdqu64 [rax], zmm0
	ret

g:
	vxorpd zmm0, zmm0, zmm0
	mov rax, a
	vaddpd zmm0, zmm0, [rax]{1to8}
	mov rax, c
	vmovdqu64 [rax], zmm0
	ret
