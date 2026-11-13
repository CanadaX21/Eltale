.include "macro.inc"

/* assembler directives */
.set noat      /* allow manual use of $at */
.set noreorder /* don't insert nops after branches */
.set gp=64     /* allow use of 64-bit general purpose registers */

.section .text, "ax"

/* The boot entry, kept by hand (the splat config types it hasm, so splat never rewrites it). It clears main's .bss
 * from gBootStack on, 8 bytes a step; the size is a linker symbol (undefined_syms.jp.txt, main_bss_clear_size), so
 * the clear follows a change in main's .bss, and the lui and the ori each take the low half of their own symbol,
 * which is exact for any size. Then it jumps to boot with the stack pointer at the top of the boot stack
 * (gBootStack, 0x2000 bytes, src/game/boot.c). */

glabel entrypoint
    lui        $t0, %hi(gBootStack)
    lui        $t1, %lo(main_bss_clear_size_hi)
    addiu      $t0, $t0, %lo(gBootStack)
    ori        $t1, $t1, %lo(main_bss_clear_size)
.L80000410:
    addi       $t1, $t1, -0x8
    sw         $zero, 0x0($t0)
    sw         $zero, 0x4($t0)
    bnez       $t1, .L80000410
     addi      $t0, $t0, 0x8
    lui        $t2, %hi(boot)
    lui        $sp, %hi(gBootStack + 0x2000)
    addiu      $t2, $t2, %lo(boot)
    jr         $t2
     addiu     $sp, $sp, %lo(gBootStack + 0x2000)
    nop
    nop
    nop
    nop
    nop
    nop
