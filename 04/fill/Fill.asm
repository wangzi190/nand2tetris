// Runs an infinite loop that listens to the keyboard input.
// When a key is pressed (any key), the program blackens the screen,
// i.e. writes "black" in every pixel;
// the screen should remain fully black as long as the key is pressed. 
// When no key is pressed, the program clears the screen, i.e. writes
// "white" in every pixel;
// the screen should remain fully clear as long as no key is pressed.

(CHECK)
    @24576
    D=M // D = RAM[24576]

    // Jump to (ERASE) if !(key pressed)
    @ERASE
    D;JEQ

    D=1

    (ERASE)
    D=0

    @131072
    D=A
    @counter
    M=D // 256*512 = 131,072 pixels

    @16384
    D=A
    @pixelNum
    M=1 // Screen starts at RAM[16384]

    @CHECK
    0;JMP

(DRAW)
    D=1

    (DRAWLOOP)
        // blacken 1 pixel
        @pixelNum
        M=D // Memory[pixelNum] = 1
        M=A+1 // pixelNum + 1

        @counter
        M=M-1 // Memory[counter] = Memory[counter] - 1
        D=M // D = Memory[counter]

        @CHECK
        D;JLE

        @DRAWLOOP
        0;JMP

    @CHECK
    0;JMP // Loop back to (CHECK)

(ERASE)
    // Initialization

    @131072
    D=A
    @counter
    M=D // 256*512 = 131,072 pixels

    @16384
    D=A
    @pixelNum
    M=1 // Screen starts at RAM[16384]

    D=0

    @ERASELOOP
    0;JMP

   (ERASELOOP)
        @pixelNum
        M=D // Memory[pixelNum] = 0
        M=A+1 // pixelNum + 1

        @counter
        M=M-1 // Memory[counter] = Memory[counter] - 1
        D=M // D = Memory[counter]

        @CHECK
        D;JLE

        @ERASELOOP
        0;JMP

    @CHECK
    0;JMP // Loop back to (CHECK)