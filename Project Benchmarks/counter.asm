init:
    add $t0, $zero, $imm, 128 ; Counter register set to 128 iterations.
    xor $t1, $t1,   $t1       ; Value from memory.

loop:
    lw  $t1, $zero, $zero      ; Load from address 0.
    add $t1, $t1,   $imm, 1    ; Increment value by 1.
    sw  $t1, $zero, $zero      ; Save to address 0.
    sub $t0, $t0,   $imm, 1    ; Decrement counter by 1.
    bne $imm, $t0, $zero, loop ; While (t0 > 0).

end:
    halt $zero, $zero, $zero, 0 ; End simulation.