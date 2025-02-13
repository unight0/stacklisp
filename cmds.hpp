#ifndef CMDS_HPP
#define CMDS_HPP

enum {
    CMD_PUSH = 0, // Push to stack
    CMD_BLKB, // Begin block (commands are saved, not interpreted)
    CMD_BLKE, // End block
    CMD_LAM,  // Lambda (function definition) (lambda args body)
    CMD_DROP, // Drop from stack
    CMD_TD,   // Push to data stack >D
    CMD_FD,   // Pop from data stack D>
    CMD_D,    // Nth from data stack
    CMD_ND,   // Push n to data stack >ND
    CMD_APPL, // Apply function
    CMD_ENVV, // Get value by ID (from Environment)
    
    //Datasection commands//////////////
    CMDD_BD, // Begin data(string) block
    CMDD_ED, // End data block
    CMDD_ID  // Declare ID of data block
};

#endif
