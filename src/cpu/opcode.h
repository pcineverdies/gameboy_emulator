#ifndef __OPCODE_H
#define __OPCODE_H

#define INVALID_OPCODE_1  0xd3
#define INVALID_OPCODE_2  0xe3
#define INVALID_OPCODE_3  0xe4
#define INVALID_OPCODE_4  0xf4
#define INVALID_OPCODE_5  0xdb
#define INVALID_OPCODE_6  0xeb
#define INVALID_OPCODE_7  0xec
#define INVALID_OPCODE_8  0xfc
#define INVALID_OPCODE_9  0xdd
#define INVALID_OPCODE_10 0xed
#define INVALID_OPCODE_11 0xfd

#define LH_INDEX 6
#define LD_r_U8_OPCODE "00xxx110"
#define LD_m_A_OPCODE "00xxx010"
#define LD_ff00_u8_OPCODE "111x0000"
#define LD_ff00_C_OPCODE "111x0010"
#define LD_u16_A_OPCODE "111x1010"

#define LD_r16_u16_OPCODE "00xx0001"
#define LD_BC_u16_OPCODE 0x01
#define LD_DE_u16_OPCODE 0x11
#define LD_HL_u16_OPCODE 0x21
#define LD_SP_u16_OPCODE 0x31

#define POP_OPCODE "11xx0001"
#define POP_BC_OPCODE 0xc1
#define POP_DE_OPCODE 0xd1
#define POP_HL_OPCODE 0xe1
#define POP_SP_OPCODE 0xf1

#define PUSH_OPCODE "11xx0101"
#define PUSH_BC_OPCODE 0xc5
#define PUSH_DE_OPCODE 0xd5
#define PUSH_HL_OPCODE 0xe5
#define PUSH_SP_OPCODE 0xf5

#define LD_u16_SP_OPCODE 0x08
#define LD_SP_HL_OPCODE 0xf9

#define LD_r_r 0b01

#define HALT_OPCODE 0x76

#define ALU_ADD_YYY 0
#define ALU_ADC_YYY 1
#define ALU_SUB_YYY 2
#define ALU_SBC_YYY 3
#define ALU_AND_YYY 4
#define ALU_XOR_YYY 5
#define ALU_OR_YYY 6
#define ALU_CP_YYY 7

#define ALU_INC_DEC_OPCODE "00xxx10x"
#define ALU_OPCODE "10xxxxxx"
#define ALU_u8_OPCODE "11xxx110"

#define RLCA_OPCODE 0x07
#define RLA_OPCODE  0x17
#define RRCA_OPCODE 0x0f
#define RRA_OPCODE  0x1f
#define DAA_OPCODE  0x27
#define CPL_OPCODE  0x2f
#define SCF_OPCODE  0x37
#define CCF_OPCODE  0x3f

#define ALU_16_INC_DEC_OPCODE "00xxx011"
#define INC_BC_OPCODE 0x03
#define INC_DE_OPCODE 0x13
#define INC_HL_OPCODE 0x23
#define INC_SP_OPCODE 0x33
#define DEC_BC_OPCODE 0x0b
#define DEC_DE_OPCODE 0x1b
#define DEC_HL_OPCODE 0x2b
#define DEC_SP_OPCODE 0x3b

#define ALU_16_r_r_OPCODE "00xx1001"
#define ADD_HL_BC_OPCODE 0x09
#define ADD_HL_DE_OPCODE 0x19
#define ADD_HL_HL_OPCODE 0x29
#define ADD_HL_SP_OPCODE 0x39

#define ADD_SP_i8_OPCODE 0xe8
#define LD_HL_SP_i8_OPCODE 0xf8

#define JR_COND_OPCODE "001xx000"
#define JR_i8_OPCODE 0x18

#define RET_COND_OPCODE "110xx000"
#define JP_COND_OPCODE "110xx010"
#define JP_OPCODE 0xc3

#define CALL_COND_OPCODE "110xx100"
#define CALL_OPCODE 0xcd

#define RST_OPCODE "11xxx111"

#define RET_OPCODE 0xc9
#define RETI_OPCODE 0xd9

#define CB_OPCODE 0xcb
#define CB_BLOCK0_OPCODE "00xxxxxx"

#define CB_RLC_OPCODE "00000xxx"
#define CB_RRC_OPCODE "00001xxx"
#define CB_RL_OPCODE "00010xxx"
#define CB_RR_OPCODE "00011xxx"
#define CB_SLA_OPCODE "00100xxx"
#define CB_SRA_OPCODE "00101xxx"
#define CB_SWAP_OPCODE "00110xxx"
#define CB_SRL_OPCODE "00111xxx"
#define CB_BIT_OPCODE "01xxxxxx"
#define CB_SET_RES_OPCODE "1xxxxxxx"

#endif
