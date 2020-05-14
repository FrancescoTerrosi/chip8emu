import sys

if __name__ == '__main__':
    if len(sys.argv) == 1:
        print("Usage: python {} [rom.ch8] >> [output.csv]".format(sys.argv[0]))
        exit()
    
    rom = sys.argv[-1]
    
    bytes = bytearray()
    with open(rom, 'rb') as r:
        byte = r.read(1)
        while byte != "":
            bytes.append(byte)
            byte = r.read(1)
    
    disassemble = []
    byteindex = 0
    while byteindex < len(bytes):
        disassemble.append(bytes[byteindex] << 8 | bytes[byteindex + 1])
        byteindex += 2
    
    pc = 0x200
    print("address,instruction,opcode,x,y,n,kk,nnn,description")
    for ip in range(0x00, len(disassemble)):
        mem_addr = '0x{0:0{1}X}'.format(pc, 4)
        instr = '0x{0:0{1}X}'.format(disassemble[ip], 4)
        opcode = '0x{0:0{1}X}'.format(disassemble[ip] & 0xF000, 4)
        x = '0x{0:0{1}X}'.format((disassemble[ip] >> 8) & 0x000F, 4)
        y =  '0x{0:0{1}X}'.format((disassemble[ip] >> 4) & 0x000F, 4)
        n =  '0x{0:0{1}X}'.format(disassemble[ip] & 0x000F, 4)
        kk =  '0x{0:0{1}X}'.format(disassemble[ip] & 0x0FF, 4)
        nnn =  '0x{0:0{1}X}'.format(disassemble[ip] & 0x0FFF, 4)
        desc = ""
        kk
        if int(opcode, 16) == 0x0000:
            if int(kk, 16) == 0x00E0:
                desc = "CLEAR_SCREEN"
            elif int(kk, 16) == 0x00EE:
                desc = "SUBROUTINE_RETURN"
            else:
                desc = "UNKNOWN_0x00_INSTRUCTION"
        elif int(opcode, 16) == 0x1000:
            desc = "JUMP_{}".format(nnn)
        elif int(opcode, 16) == 0x2000:
            desc = "SUBROUTINE_CALL_{}".format(nnn)
        elif int(opcode, 16) == 0x3000:
            desc = "SKIP_NEXT_INSTRUCTION_IF_V[{}]_EQUALS_{}".format(x, kk)
        elif int(opcode, 16) == 0x4000:
            desc = "SKIP_NEXT_INSTRUCTION_IF_V[{}]_NOT_EQUALS_{}".format(x, kk)
        elif int(opcode, 16) == 0x5000:
            desc = "SKIP_NEXT_INSTRUCTION_IF_V[{}]_EQUALS_V[{}]".format(x, y)
        elif int(opcode, 16) == 0x6000:
            desc = "SET_V[{}]_EQUALSTO_{}".format(x, kk)
        elif int(opcode, 16) == 0x7000:
            desc = "SET_V[{}]_EQUALSTO_V[{}]+{}".format(x, x, kk)
        elif int(opcode, 16) == 0x8000:
            if int(n, 16) == 0x0000:
                desc = "SET_V[{}]_EQUALSTO_V[{}]".format(x, y)
            elif int(n, 16) == 0x0001:
                desc = "SET_V[{}]_EQUALSTO_V[{}]_OR_V[{}]".format(x, x, y)
            elif int(n, 16) == 0x0002:
                desc = "SET_V[{}]_EQUALSTO_V[{}]_AND_V[{}]".format(x, x, y)
            elif int(n, 16) == 0x0003:
                desc = "SET_V[{}]_EQUALSTO_V[{}]_XOR_V[{}]".format(x, x, y)
            elif int(n, 16) == 0x0004:
                desc = "SET_V[{}]_EQUALSTO_V[{}]_PLUS_V[{}]".format(x, x, y)
            elif int(n, 16) == 0x0005:
                desc = "SET_V[{}]_EQUALSTO_V[{}]_MINUS_V[{}]".format(x, x, y)
            elif int(n, 16) == 0x0006:
                desc = "SET_V[{}]_EQUALSTO_V[{}] >> 1".format(x, x)
            elif int(n, 16) == 0x0007:
                desc = "SET_V[{}]_EQUALSTO_V[{}]_MINUS_V[{}]".format(x, y, x)
            elif int(n, 16) == 0x000E:
                desc = "SET_V[{}]_EQUALSTO_V[{}] << 1".format(x, x)
            else:
                desc = "UNKNOWN_0x8000_INSTRUCTION"
        elif int(opcode, 16) == 0x9000:
            desc = "SKIP_NEXT_INSTRUCTION_IF_V[{}]_NOT_EQUALS_V[{}]".format(x, y)
        elif int(opcode, 16) == 0xA000:
            desc = "SET_I_EQUALSTO_{}".format(nnn)
        elif int(opcode, 16) == 0xB000:
            desc = "JUMP_V0_PLUS_{}".format(nnn)
        elif int(opcode, 16) == 0xC000:
            desc = "SET_V[{}]_EQUALSTO_RANDOM".format(x)
        elif int(opcode, 16) == 0xD000:
            desc = "DRAW_SPRITE_AT_COORDINATES_V[{}]V[{}]_WIDTH_8_HEIGHT_{}".format(x, y, n)
        elif int(opcode, 16) == 0xE000:
            if int(kk, 16) == 0x009E:
                desc = "SKIP_NEXT_INSTRUCTION_IF_V[{}]_PRESSED".format(x)
            elif int(kk, 16) == 0x00A1:
                desc = "SKIP_NEXT_INSTRUTION_IF_V[{}]_NOTPRESSED".format(x)
            else:
                desc = "UNKNOWN_0xE000_INSTRUCTION"
        elif int(opcode, 16) == 0xF000:
            if int(kk, 16) == 0x0007:
                desc = "SET_V[{}]_EQUALSTO_DELAYTIMER".format(x)
            elif int(kk, 16) == 0x00A:
                desc = "KEYPRESS_WAIT_AND_STOREKEY_IN_V[{}]".format(x)
            elif int(kk, 16) == 0x0015:
                desc = "SET_DELAYTIMER_EQUALSTO_V[{}]".format(x)
            elif int(kk, 16) == 0x0018:
                desc = "SET_SOUNDTIMER_EQUALSTO_V[{}]".format(x)
            elif int(kk, 16) == 0x001E:
                desc = "SET_I_EQUALSTO_I_PLUS_V[{}]".format(x)
            elif int(kk, 16) == 0x0029:
                desc = "SET_I_EQUALSTO_SPRITE_ADDRESS_OF_CHAR_IN_V[{}]".format(x)
            elif int(kk, 16) == 0x0033:
                desc = "SET_BCD_V[{}]".format(x)
            elif int(kk, 16) == 0x0055:
                desc = "STORE_V[0-{}]_STARTINGAT_MEM[I]".format(x)
            elif int(kk, 16) == 0x0065:
                desc = "LOAD_MEM[I - I+{}]_IN_V[0-{}]".format(x, x)
            else:
                desc = "UNKNOWN_0xF000_INSTRUCTION"
        else:
            desc = "UNKNOWN_{}_OPCODE".format(opcode)
        pc += 2
        
        print("{},{},{},{},{},{},{},{},{}".format(mem_addr, instr, opcode, x, y, n, kk, nnn, desc))
    
            
        
        
