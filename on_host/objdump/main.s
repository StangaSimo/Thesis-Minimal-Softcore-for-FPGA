	.text
	.align	2
	.globl	main
	.ent	main
	.type	main, @function  

main:
	addi	r2,r0,55 
	addi	r3,r0,100
	cmp     r4,r2,r3 
	addi	r5,r0,2147483640
	
	.end	main
