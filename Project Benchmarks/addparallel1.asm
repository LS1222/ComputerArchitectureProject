init:
    xor $t0, $t0, $t0 ; Index of the vector.
    xor $s0, $s0, $s0 ; Value of first vector.
    xor $s1, $s1, $s1 ; Value of second vector.
    xor $s2, $s2, $s2 ; Value of sum.
    xor $a0, $a0, $a0 ; Address of first vector.
    xor $a1, $a1, $a1 ; Address of second vector.
    xor $s3, $s3, $s3 ; Address of result vector. 
set:
    add $t0, $zero, $imm, 1024  ; Number of elements to sum.
    add $a0, $zero, $imm, 1024  ; Address of first  vector @ 0.
    add $a1, $zero, $imm, 5120  ; Address of second vector @ 5120.
    add $s3, $zero, $imm, 9216  ; Address of result vector @ 9216.

loop:
    lw  $s0, $a0, $zero        ; Load from first vector.
    lw  $s1, $a1, $zero        ; Load from second vector.
    add $a0, $a0, $imm, 1      ; Next first  vector address.
    add $a1, $a1, $imm, 1      ; Next second vector address.
    add $s2, $s0, $s1          ; Sum = First + Second.
    sw  $s2, $a2, $zero        ; Save to result address.
    add $a2, $a2, $imm, 1      ; Next result vector address.
    sub $t0, $t0, $imm, 1      ; Decrement index.
    bne $imm, $t0, $zero, loop ; While (t0 > 0).

end:
    halt $zero, $zero, $zero, 0 ; End simulation.