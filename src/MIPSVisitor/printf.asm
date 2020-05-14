.data
string: .asciiz "hallo %%%i%c %s %f %p"
str: .asciiz "test"
f: .float 0.25
.text

la $t0, string
sw $t0, -20($sp)

li $t0, 5
sw $t0, -24($sp)

li $t0, 65
sw $t0, -28($sp)

la $t0, str
sw $t0, -32($sp)

lw $t0, f
sw $t0, -36($sp)

li $t0, 0x50
sw $t0, -40($sp)

jal printf

li $v0, 10
syscall

printf:
	addu $sp, $sp, -16	#buffer for char (4) and \0 (8)
	sw $t0, 0($sp)
	sw $t1, 4($sp)
	sw $a0, 8($sp)
	swc1 $f12, 12($sp)
	
	lw $t0, -4($sp)		#load fmt counter
	addi $t1, $sp, -8	#arg counter
	
printf_loop:
	lb $a0, 0($t0)		#load char
	addu $t0, $t0, 1	#inc counter
	beq $a0, '%', printf_fmt
	beqz $a0, printf_end

printf_put:
	li $v0, 11
	syscall
	j printf_loop
	
printf_fmt:
	lb $a0, 0($t0)		#load char
	addu $t0, $t0, 1	#inc counter
	beq $a0, 'd', printf_int
	beq $a0, 'i', printf_int
	beq $a0, 's', printf_str
	beq $a0, 'c', printf_char
	beq $a0, 'f', printf_float
	beq $a0, 'p', printf_hex
	j printf_put
	
printf_shift:
	add $t1, $t1, -4
	j printf_loop
	
printf_int:
	lw $a0, 0($t1)
	li $v0, 1
	syscall
	j printf_shift
	
printf_str:
	lw $a0, 0($t1)
	li $v0, 4
	syscall
	j printf_shift
	
printf_char:
	lb $a0, 0($t1)
	li $v0, 11
	syscall
	j printf_shift
	
printf_float:
	lwc1 $f12, 0($t1)
	li $v0, 2
	syscall
	j printf_shift
	
printf_hex:
	lw $a0, 0($t1)
	li $v0, 34
	syscall
	j printf_shift
	
printf_end:
	lwc1 $f12, 12($sp)
	lw $a0, 8($sp)
	lw $t1, 4($sp)
	lw $t0, 0($sp)
	addu $sp, $sp, 16
	li $v0, 0
	jr $ra