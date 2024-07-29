// Runs an infinite loop that listens to the keyboard input.
// When a key is pressed (any key), the program blackens the screen,
// i.e. writes "black" in every pixel;
// the screen should remain fully black as long as the key is pressed. 
// When no key is pressed, the program clears the screen, i.e. writes
// "white" in every pixel;
// the screen should remain fully clear as long as no key is pressed.

@SCREEN
D=A
@COUNTER
M=D // Set COUNTER to SCREEN aka RAM[16384], base address of screen memory map

// Gap between SCREEN and KBD RAM addresses :
// 24576 - 16384 = 8192, hence the 8192 below
@8192
D=A

@SCREEN
D=D+A // 8192 + SCREEN

@COUNTER_MAX
M=D // Set COUNTER_MAX to (SCREEN + 8192) aka end address of the screen memory map

(CHECK)
    @KBD
    D=M // D = KBD aka RAM[24576], base address of the keyboard memory map

    @FILL
    D;JNE // Jump to FILL if KBD != 0
@EMPTY
0;JMP // Otherwise, jump to EMPTY

(FILL)
    @COUNTER
    D=M
    A=D
    M=-1 // Memory[ Memory[COUNTER] ] = -1; turns the attributed pixel black
    // I first mistakenly had this set to Memory[COUNTER] = -1, which would just screw up the counter value and not change the color of a pixel
@ADD
0;JMP

(EMPTY)
    @COUNTER
    D=M
    A=D
    M=0 // Memory[ Memory[COUNTER] ] = 0; turns the attributed pixel white
@ADD
0;JMP

(ADD)
    @COUNTER
    D=M+1
    M=D // Increment counter

    @COUNTER_MAX
    D=M
    @COUNTER
    D=M-D // Stores COUNTER - (SCREEN + 8192) in D

    @CHECK
    D;JLT // If [COUNTER - (SCREEN + 8192)] < 0, jumps back to CHECK which will jump to either FILL or COUNT with the incremented counter value
    // The program should keep jumping back to CHECK to detect if a key is being held

@SCREEN // Otherwise, resets COUNTER and then jumps back to CHECK
D=A
@COUNTER
M=D

@CHECK
0;JMP