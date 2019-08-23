//{ 0xD2 , 0x05 , 0x00 , "Blinds Control for Position and Angle                                            ",  "Type 0x00                                                                        " },

// TITLE:CMD 1 - Go to Position and Angle
T_DATAFIELD D20500_CMD_1 [] = {
{  1 , 7 , "POS"      , "Position"},
{  9 , 7 , "ANG"      , "Angle"},
{ 17 , 3 , "REPO"     , "Repositioning"},//Value: 0 = Go directly to POS/ANG 
                                         //Value: 1 = Go up (0%), then to POS/ANG 
                                         //Value: 2 = Go down (100%), then to POS/ANG 
                                         //Value: 3 ... 7 = Reserved 
{ 21 , 3 , "LOCK"     , "Locking modes"},//Value: 0 = Do not change 
                                         //Value: 1 = Set blockage mode 
                                         //Value: 2 = Set alarm mode 
                                         //Value: 3 ... 6 = Reserved 
                                         //Value: 7 = Deblockage 
{ 24 , 4 , "CHN"      , "Channel"},//Value: 0 = Channel 1 
{ 28 , 4 , "CMD"      , "Command ID"},//Value: 1 = Goto command 
{  0 , 0 , 0          , 0           }
};

// Index of field
#define D20500_CMD_1_POS        0
#define D20500_CMD_1_ANG        1
#define D20500_CMD_1_REPO       2
#define D20500_CMD_1_LOCK       3
#define D20500_CMD_1_CHN        4
#define D20500_CMD_1_CMD        5
#define D20500_CMD_1_NB_DATA    6
#define D20500_CMD_1_DATA_SIZE  4

// TITLE:CMD 2 - Stop
T_DATAFIELD D20500_CMD_2 [] = {
{  0 , 4 , "CHN"      , "Channel"},//Value: 0 = Channel 1 
{  4 , 4 , "CMD"      , "Command ID"},//Value: 2 = Stop command 
{  0 , 0 , 0          , 0           }
};

// Index of field
#define D20500_CMD_2_CHN        0
#define D20500_CMD_2_CMD        1
#define D20500_CMD_2_NB_DATA    2
#define D20500_CMD_2_DATA_SIZE  1

// TITLE:CMD 3 - Query Position and Angle
T_DATAFIELD D20500_CMD_3 [] = {
{  0 , 4 , "CHN"      , "Channel"},//Value: 0 = Channel 1 
{  4 , 4 , "CMD"      , "Command ID"},//Value: 3 = Query command 
{  0 , 0 , 0          , 0           }
};

// Index of field
#define D20500_CMD_3_CHN        0
#define D20500_CMD_3_CMD        1
#define D20500_CMD_3_NB_DATA    2
#define D20500_CMD_3_DATA_SIZE  1

// TITLE:CMD 4 - Reply Position and Angle
T_DATAFIELD D20500_CMD_4 [] = {
{  1 , 7 , "POS"      , "Position"},
{  9 , 7 , "ANG"      , "Angle"},
{ 21 , 3 , "LOCK"     , "Locking modes"},//Value: 0 = Normal (no lock) 
                                         //Value: 1 = Blockage mode 
                                         //Value: 2 = Alarm mode 
                                         //Value: 3 ... 7 = Reserved 
{ 24 , 4 , "CHN"      , "Channel"},//Value: 0 = Channel 1 
{ 28 , 4 , "CMD"      , "Command ID"},//Value: 4 = Reply command 
{  0 , 0 , 0          , 0           }
};

// Index of field
#define D20500_CMD_4_POS        0
#define D20500_CMD_4_ANG        1
#define D20500_CMD_4_LOCK       2
#define D20500_CMD_4_CHN        3
#define D20500_CMD_4_CMD        4
#define D20500_CMD_4_NB_DATA    5
#define D20500_CMD_4_DATA_SIZE  4

// TITLE:CMD 5 - Set parameters
T_DATAFIELD D20500_CMD_5 [] = {
{  1 ,15 , "VERT"     , "Set vertical"},
{ 16 , 8 , "ROT"      , "Set rotation"},
{ 29 , 3 , "AA"       , "Set alarm action"},//Value: 0 = No action 
                                            //Value: 1 = Immediate stop 
                                            //Value: 2 = Go up (0%) 
                                            //Value: 3 = Go down (100%) 
                                            //Value: 4 ... 6 = Reserved 
                                            //Value: 7 = -> No change 
{ 32 , 4 , "CHN"      , "Channel"},//Value: 0 = Channel 1 
{ 36 , 4 , "CMD"      , "Command ID"},//Value: 5 = Set parameters command 
{  0 , 0 , 0          , 0           }
};

// Index of field
#define D20500_CMD_5_VERT       0
#define D20500_CMD_5_ROT        1
#define D20500_CMD_5_AA         2
#define D20500_CMD_5_CHN        3
#define D20500_CMD_5_CMD        4
#define D20500_CMD_5_NB_DATA    5
#define D20500_CMD_5_DATA_SIZE  5
