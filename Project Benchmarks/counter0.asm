init:
    xor $t0, $t0, $t0 ; Iteration register.
    xor $t1, $t1, $t1 ; Counter value.
    xor $t2, $t2, $t2 ; Core Index.
    xor $s0, $s0, $s0 ; Lock address.
    xor $s1, $s1, $s1 ; Lock value.

set:
    add $t0, $zero, $imm, 128 ; Counter register set to 128 iterations.
    add $t2, $zero, $imm, 0   ; Core index.
    add $s0, $zero, $imm, 10  ; Lock at address 10 (arbitrary).

loop:
    lw  $s1,  $s0,   $zero       ; Read lock index value.
    bne $imm, $s1,   $t2,  loop  ; While (t2 != core).
    lw  $t1,  $zero, $zero       ; Load from address 0.
    add $t1,  $t1,   $imm,  1    ; Increment value by 1.
    sw  $t1,  $zero, $zero       ; Save to address 0.
    sub $t0,  $t0,   $imm,  1    ; Decrement counter by 1.
    add $s1,  $zero, $imm,  1    ; Set index to 1.
    sw  $s1,  $s0,   $zero       ; Save new index.
    bgt $imm, $t0,   $zero, loop ; While (t0 > 0).

end:
    halt $zero, $zero, $zero, 0 ; End simulation.