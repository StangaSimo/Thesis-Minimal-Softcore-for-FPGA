	.text
	.align	2
	.globl	main
	.ent	main
	.type	main, @function        #r4=155 #r7=2147483647  #carry=1  #r1=157

main:

	addi	r1,r0,14
	addi	r9,r0,45
	addi	r8,r0,1 
	addi  r2,r0,0
	lwi	 r3,r2,0	
	or r3,r3,r9
	swi  r3,r2,0   
	rsub  r1,r8,r1
	addi  r2,r2,1
	bgei  r1,-20

	.end	main
