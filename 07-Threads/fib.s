	.syntax unified
//	.arch armv7-a
	.text
	.align 2
	.thumb
	.thumb_func

	.global fibonacci
	.type fibonacci, function

fibonacci:
	@ ADD/MODIFY CODE BELOW
	@ PROLOG
	push {r3, r4, r5, r6, lr}
    mov r3,#-1  @previous
    mov r4,#1   @result
    mov r5,#0   @i
    mov r6,#0   @sum
loop:
    
    add r6,r3,r4    @sum=result+previous
    add r3,r4,#0    @previous=result
    add r4,r6,#0       @result=sum

    add r5,r5,#1 @++i
    cmp r5,r0 @i <= x 
    ble loop
    
    mov r0,r4       @RETUTN RESULT
	pop {r3, r4, r5, r6, pc}		@EPILOG

	@ END CODE MODIFICATION

	.size fibonacci, .-fibonacci
	.end
