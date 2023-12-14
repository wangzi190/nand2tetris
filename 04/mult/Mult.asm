// Multiplies R0 and R1 and stores the result in R2.
// (R0, R1, R2 refer to RAM[0], RAM[1], and RAM[2], respectively.)

// This program only needs to handle arguments that satisfy
// R0 >= 0, R1 >= 0, and R0*R1 < 32768.

// Initialize R2 to 0
@R2
M=0 // Memory[R2] = 0

// Load R1 into D, then set counter to R1
@R1
D=M // D = Memory[R1]
@counter
MD=D // Memory[counter] = Memory[R1]

//@END
//D;JEQ
// ^ ORIGINALLY DIDNT TAKE INTO ACCOUNT R1==0

(ADD)
    // Set counter to itself subtracted by 1
    @counter
    M=M-1 // Memory[counter] = Memory[counter] - 1

    D=M // D = Memory[counter]
    // ^ ORIGINALLY FORGOT TO LOAD COUNTER INTO D

    // Terminate program
    @END
    D;JLT // Jump to END if Memory[counter] <= 0

    // Load R2 into D, then set R2 to itself plus R0
    @R2
    D=M // D = Memory[R2]
    @R0
    D=D+M // Memory[R2] = Memory[R2] + Memory[R0]
    @R2
    M=D //<--ORIGINALLY FORGOT TO PUT CALCULATION BACK INTO R2

    // If not terminated, continue adding
    @ADD
    0;JMP // Jump to ADD if !(Memory[counter] <= 0)

(END)
    // Terminate program
    @END
    0;JMP