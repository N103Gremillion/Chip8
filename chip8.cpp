#include "chip8.hpp"
#include <fstream>
#include <vector>


void load_rom(const std::string& fileName, Chip8& chip) {
  // Open file in binary mode
  ifstream romFile(fileName, ios::binary);

  if (!romFile) {
    cerr << "Error: could not open ROM file " << fileName << endl;
    return;  
  }

  // Get the size of the file
  romFile.seekg(0, ios::end);
  streampos size = romFile.tellg();
  romFile.seekg(0, ios::beg);

  // Allocate memory for the buffer using a vector for automatic memory management
  vector<char> buffer(size);

  // Read the contents of the file into the buffer
  romFile.read(buffer.data(), size);
  
  if (!romFile) {
    std::cerr << "Error: failed to read the entire ROM file" << std::endl;
    return;
  }

  romFile.close();

  // Copy the ROM data into chip's RAM starting at ROM_START
  for (std::size_t i = 0; i < size; ++i) {
    chip.ram[ROM_START + i] = buffer[i];
  }
}

// HELPERS FUNCTIONS
void printHex(u16 instruction) {
  cout<< hex << uppercase << static_cast<int>(instruction) << endl;
}

void printStack(Chip8& chip) {
  for (int i = 0; i < 16; i++) {
    cout<< "|" << chip.stack[i] << "|" << endl;
  }
}

// MAIN FUNCTIONS
void run(Chip8& chip) {

  bool running = true;
  SDL_Event event;

  while(running) {
    // Handle use input
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        running = false;
      }
    }
    // execute an instruction
    u16 instruction = fetch_instruction(chip);
    perform_instruction(instruction, chip);

    // update timers

    // draw pixels / update screen
    update_screen(chip.screen);

    
  }
}

void put_value_in_Vreg(int regNum, u8 value, Chip8& chip) {
    if (regNum > 15 || regNum < 0) {
      return;
    }
    switch (regNum) {
      case 0:
        chip.regs->v0 = value;
        break;
      case 1:
        chip.regs->v1 = value;
        break;
      case 2:
        chip.regs->v2 = value;
        break;
      case 3:
        chip.regs->v3 = value;
        break;
      case 4:
        chip.regs->v4 = value;
        break;
      case 5:
        chip.regs->v5 = value;
        break;
      case 6:
        chip.regs->v6 = value;
        break;
      case 7:
        chip.regs->v7 = value;
        break;
      case 8:
        chip.regs->v8 = value;
        break;
      case 9:
        chip.regs->v9 = value;
        break;
      case 10:
        chip.regs->vA = value;
        break;
      case 11:
        chip.regs->vB = value;
        break;
      case 12:
        chip.regs->vC = value;
        break;
      case 13:
        chip.regs->vD = value;
        break;
      case 14:
        chip.regs->vE = value;
        break;
      case 15:  
        chip.regs->vF = value;
        break;
    }
}

u8 get_value_in_Vreg(int regNum, Chip8& chip) {
  if (regNum < 0 || regNum > 15) {
    return -1;
  }
  switch (regNum) {
      case 0:
        return chip.regs->v0;
        break;
      case 1:
        return chip.regs->v1;
        break;
      case 2:
        return chip.regs->v2;
        break;
      case 3:
        return chip.regs->v3;
        break;
      case 4:
        return chip.regs->v4;
        break;
      case 5:
        return chip.regs->v5;
        break;
      case 6:
        return chip.regs->v6;
        break;
      case 7:
        return chip.regs->v7;
        break;
      case 8:
        return chip.regs->v8;
        break;
      case 9:
        return chip.regs->v9;
        break;
      case 10:
        return chip.regs->vA;
        break;
      case 11:
        return chip.regs->vB;
        break;
      case 12:
        return chip.regs->vC;
        break;
      case 13:
        return chip.regs->vD;
        break;
      case 14:
        return chip.regs->vE;
        break;
      case 15:  
        return chip.regs->vF;
        break;
      default:
        return -1;
        break;
  }
}

u16 fetch_instruction(Chip8& chip) {
  u16 pc = chip.regs->pc;
  u16 instruction = (chip.ram[pc] << 8) | chip.ram[pc + 1];
  chip.regs->pc+=2;
  return instruction;  
}

void printMemory(Chip8& chip) {
  for (int i = 0; i < RAM_SIZE; ++i) {
    cout << "0x" << hex << (int)(unsigned char)chip.ram[i] << " ";
  }
  cout << std::dec << std::endl;
}

void free_chip(Chip8& chip) {
  // free everything from the stack
  free(chip.ram);
  delete chip.regs;
  free(chip.stack);
  free_screen(chip.screen);
}

  
void perform_instruction(u16 instruction, Chip8& chip) {
  // niblets = (4 bits)
  u8 opcode = instruction >> 12; // defines the type of instruction

  switch (opcode) {

    case 0x0:
      switch (instruction) {

        // clear screen
        case 0x00E0: 
          clear_screen(chip.screen);
          break;

        // return from subroutine sets pc to address at top of stck then sp --
        case 0x00EE:
          chip.regs->sp--;
          chip.regs->pc = chip.stack[chip.regs->sp];
          // chip.stack[chip.regs->sp] = chip.regs->pc;
          break;
      }

    // jump to the nnn register
    case 0x1: {
      u16 nnn = (instruction & 0xFFF);
      chip.regs->pc = nnn;
      break;
    }

    // call instruction
    case 0x2: {
      if (chip.regs->sp >= 16 - 1) {
        std::cerr<<"Stack overflow"<<std::endl;
      } else {
        chip.regs->sp++;
        chip.stack[chip.regs->sp] = chip.regs->pc;
        chip.regs->pc = (instruction & 0xFFF);
      }
      break;
    }

    // skip next instruction Vx = kk
    case 0x3: {
      u8 kk = (instruction & 0xFF);
      int reg_num = (int) ((instruction >> 8) & 0xF);
      if (kk == get_value_in_Vreg(reg_num, chip)) {
        chip.regs->pc += 2;
      }
      break;
    }

    // skip next instruction if Vx != kk
    case 0x4: {
      u8 kk = (instruction & 0xFF);
      int reg_num = (int) ((instruction >> 8) & 0xF);
      if (kk != get_value_in_Vreg(reg_num, chip)) {
        chip.regs->pc += 2;
      }
      break;
    }

    // skip next instruction if Vx = Vy
    case 0x5: {
      int x_reg_num = ((instruction >> 8) & 0xF);
      int y_reg_num = ((instruction >> 4) & 0xF);
      if (get_value_in_Vreg(x_reg_num, chip) == get_value_in_Vreg(y_reg_num, chip)) {
        chip.regs->pc += 2;
      }
      printf("check skip \n");
      break;
    }

    // put value kk into register Vx
    case 0x6: {
      u8 kk = (instruction & 0xFF);
      int x = (int) ((instruction >> 8) & 0xF);
      put_value_in_Vreg(x, kk, chip);
      break;
    }

    // add kk to the value of register Vx, then store result in Vx
    case 0x7: {
      u8 kk = (instruction & 0xFF);
      int x_reg_num = ((instruction >> 8) & 0xF);
      u8 new_reg_val = get_value_in_Vreg(x_reg_num, chip) + kk;
      put_value_in_Vreg(x_reg_num, new_reg_val, chip);
      break;
    }

    case 0x8: {
      u8 finalNib = (instruction & 0xF);
      int x_reg_num = ((instruction >> 8) & 0xF);
      int y_reg_num = ((instruction >> 4) & 0xF);
      u8 Vy = get_value_in_Vreg(y_reg_num, chip);
      u8 Vx = get_value_in_Vreg(x_reg_num, chip);

      switch (finalNib) {
        
        // store value in Vy in Vx register
        case 0x0:
          put_value_in_Vreg(x_reg_num, Vy, chip);
          break;

        // performs bitwise OR on the values of Vx and Vy, and stores the result in Vx   
        case 0x1: 
          put_value_in_Vreg(x_reg_num, (Vx | Vy), chip);
          break;

        // performs bitwise AND on values in Vx and Vy and stores in Vx
        case 0x2:
          put_value_in_Vreg(x_reg_num, (Vx & Vy), chip);
          break;

        // performs bitwise exclusive OR on values in Vx and Vy and stores in Vx
        case 0x3:
          put_value_in_Vreg(x_reg_num, (Vx ^ Vy), chip);
          break;

        // ADD Vx and Vy values; if result > 8 bits, VF is set to 1, else 0
        case 0x4: {
          u16 sum = (u16)Vx +(u16)Vy;
          u8 result = (u8)sum;
          u8 carry = (sum > 255) ? 1 : 0;
          put_value_in_Vreg(x_reg_num, result, chip);
          put_value_in_Vreg(0xF, carry, chip);
          break;
        }

        // if Vx > Vy, VF is set to 1, else 0
        case 0x5: {
          u8 carry = (Vx > Vy) ? 1 : 0;
          put_value_in_Vreg(x_reg_num, (Vx - Vy), chip);
          put_value_in_Vreg(0xF, carry, chip);
          break;
        }

        // if LSB of Vx is 1, then VF is set to 1, else 0. Then Vx is divided by 2
        case 0x6: {
          u8 lsb = (Vx & 0x1);
          put_value_in_Vreg(0xF, lsb, chip);
          put_value_in_Vreg(x_reg_num, (Vx >> 1), chip);
          break;
        }

        // if Vy > Vx, VF is set to 1, else 0. Then Vx is subtracted from Vy
        case 0x7: {
          u8 carry = (Vy > Vx) ? 1 : 0;
          put_value_in_Vreg(x_reg_num, (Vy - Vx), chip);
          put_value_in_Vreg(0xF, carry, chip);
          break;
        }

        // if MSB of Vx is 1, VF is set to 1, else 0. Then Vx is multiplied by 2
        case 0xE: {
          u8 msb = (Vx >> 7);
          put_value_in_Vreg(x_reg_num, (Vx << 1), chip);
          put_value_in_Vreg(0xF, msb, chip);
          break;
        }
      }
    }
      break;
    case 0x9:
      // skip next instruction if Vx != Vy
      printf("If Vx != Vy pc += 2 \n");
      break;
    case 0xA: {
      // value of register I is set to nnn
      u16 nnn = (instruction & 0xFFF);
      chip.regs->I = nnn;
      break;
    }
    case 0xB:
      // Jump to location nnn + V0
      printf("pc = nnn + V0 \n");
      break;
    case 0xC:
      // generate random num (0 - 255), AND with value kk, store in Vx
      printf("AND random num with kk and store in Vx \n");
      break;
    case 0xD: {
      // display n-byte sprite starting at memory location I at (Vx, Vy), set VF = collision
      int x_reg_num = (int) ((instruction >> 8) & 0xF);
      int y_reg_num = (int) ((instruction >> 4) & 0xF);
      int n = (int) (instruction & 0xF);
      int x = get_value_in_Vreg(x_reg_num, chip) & 63; // % 64 helps it wrap 
      int y = get_value_in_Vreg(y_reg_num, chip) & 31; // %32 helps it wrap
      u16 memory_location = chip.regs->I;

      put_value_in_Vreg(15, 0x00, chip);

      while (n > 0) {
        u8 cur_byte = chip.ram[memory_location];
        bool set_Vf_to_1 = draw_pixel_row(x, y, chip.screen, cur_byte);
        if (set_Vf_to_1) {
          put_value_in_Vreg(15, 1, chip); // if collision occurs set VF to 1
        }
        // Update coordinates for the next row of pixels
        y++;
        memory_location++;
        n--;
        // stop when you hit the bottom of screen
        if (y >= 32) {
          break;
        }
      } 
      break;
    }
    case 0xE: {
      u8 last8 = (instruction & 0xFF);
      switch (last8) {
        case 0x9E:
          // skip next instruction if key with value of Vx is pressed
          printf("check keyboard and if key is pressed that is in Vx skip next instruction \n");
          break;
        case 0xA1:
          // skip next instruction if key with value of Vx is not pressed
          printf("checks keyboard and if key is not pressed that is in Vx skip next instruction \n");
          break;
      }
      break;
    }
    case 0xF: {
      u8 last8 = (instruction & 0xFF);
      switch (last8) {
        case 0x07:
          // set Vx = delay_timer value
          printf("Vx = delay_timer\n");
          break;
        case 0x0A:
          // Wait for key press, store value in Vx
          printf("wait for key press and store in Vx \n");
          break;
        case 0x15:
          // set delay timer to Vx
          printf("delay_timer = Vx \n");
          break;
        case 0x18:
          // set sound timer = Vx
          printf("sound_timer = Vx \n");
          break;
        case 0x1E:
          // set I = I + Vx
          printf("I = I + Vx \n");
          break;
        case 0x29:
          // value of I is set to location of the hex sprite corresponding to the value of Vx
          printf("I set to sprite hex in Vx \n");
          break;
        case 0x33:
          // take decimal value of Vx, place hundreds, tens, and ones digit in memory at I, I+1, I+2
          printf("Some decimal val in Vx are placed in I register locations\n");
          break;
        case 0x55:
          // copies values of V0 through Vx into memory, starting at the address in I
          printf("value of V0 is copied into memory through Vx starting at the address in I \n");
          break;
        case 0x65:
          // read values from memory starting at I into registers V0 through Vx
          printf("copy values from memory starting at location I into V0 through Vx\n");
          break;
        }
      }
    break;
  }
}





