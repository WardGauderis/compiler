
###############
#  stdio.asm  #
###############

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


scanf:
	addu $sp, $sp, -20
	sw $t0, 0($sp)
	sw $t1, 4($sp)
	sw $a0, 8($sp)
	sw $a1, 12($sp)
	swc1 $f0, 16($sp)

	lw $t0, -4($sp)		#load fmt counter
	addi $t1, $sp, -8	#arg counter
	li $a1, 0x7ffffffe	#set string length counter

scanf_loop:
	lb $a0, 0($t0)		#load char
	addu $t0, $t0, 1	#inc counter
	beq $a0, '%', scanf_fmt
	beqz $a0, scanf_end

scanf_put:
	li $v0, 12
	syscall
	j scanf_loop

scanf_fmt:
	lb $a0, 0($t0)		#load char
	addu $t0, $t0, 1	#inc counter
	beq $a0, 'd', scanf_int
	beq $a0, 'i', scanf_int
	beq $a0, 's', scanf_str
	beq $a0, 'c', scanf_char
	beq $a0, 'f', scanf_float
	subu $a0, $a0, '0'
	bleu $a0, 9, scanf_length
	j scanf_put

scanf_length:
	bne $a1, 0x7ffffffe, scanf_no_reset
	li $a1, 0
scanf_no_reset:
	mulu $a1, $a1, 10
	addu $a1, $a1, $a0
	j scanf_fmt

scanf_shift:
	li $a1, 0x7ffffffe	#reset string length counter
	add $t1, $t1, -4
	j scanf_loop

scanf_int:
	li $v0, 5
	syscall
	lw $a0, 0($t1)
	sw $v0, 0($a0)
	j scanf_shift

scanf_str:
	lw $a0, 0($t1)
	addi $a1, $a1,1
	li $v0, 8
	syscall
	j scanf_shift

scanf_char:
	li $v0, 12
	syscall
	lw $a0, 0($t1)
	sb $v0, 0($a0)
	j scanf_shift

scanf_float:
	li $v0, 6
	syscall
	lw $a0, 0($t1)
	swc1 $f0, 0($a0)
	j scanf_shift

scanf_end:
	swc1 $f0, 16($sp)
	sw $a1, 12($sp)
	sw $a0, 8($sp)
	sw $t1, 4($sp)
	sw $t0, 0($sp)
	addu $sp, $sp, 20
	li $v0, 0
	jr $ra

###############
#  stdio.asm  #
###############
