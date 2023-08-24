#pragma once
#include <array>
#include <iostream>
#include <fstream>

#include "utilDefs.h"
#include "voiengine.h"

class CHIP8 : public voi::VoiEngine {
	std::array<ui8, 4096> mem = { 0 };
	std::array<ui8, 16> reg = { 0 };
	std::array<ui16, 256> stack = { 0 };
	std::array<ui64, 32> screen = { 0 };

	ui8 dt = 0;
	ui8 sp = 0;
	ui8 st = 0;
	ui16 I = 0;
	ui16 pc = 0x200;
	ui16 fntAddr = 0x050;

	float ftime = 1.f/60.f;
	float clockt = 1.f / 1000.f;

public:
	CHIP8(HINSTANCE instance){
		if (this->Construct(instance, L"Chip8 emulator", 64, 32, 16, 16)) this->Start();
	}

private:

// ############################################################
// ############################################################
// ##                                                        ##
// ##             Console Engine virtual functions           ##
// ##                     IMPLEMMENTATION                    ##
// ##                                                        ##
// ############################################################
// ############################################################

	void OnCreate() override{
		std::string addr = "PONG2";
		std::cout << "Rom address:" << std::endl;

		std::getline(std::cin, addr);

		std::ifstream file(addr.c_str(), std::ios::binary);
		file.seekg(0, file.end);
		size_t size = file.tellg();
		file.seekg(0, file.beg);

		file.read((char*)(&mem.data()[0x200]), size);

		ui8 fontData[16 * 5] = {
			0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
			0x20, 0x60, 0x20, 0x20, 0x70, // 1
			0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
			0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
			0x90, 0x90, 0xF0, 0x10, 0x10, // 4
			0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
			0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
			0xF0, 0x10, 0x20, 0x40, 0x40, // 7
			0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
			0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
			0xF0, 0x90, 0xF0, 0x90, 0x90, // A
			0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
			0xF0, 0x80, 0x80, 0x80, 0xF0, // C
			0xE0, 0x90, 0x90, 0x90, 0xE0, // D
			0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
			0xF0, 0x80, 0xF0, 0x80, 0x80, // F
		};

		for (ui16 c = 0; c < (16 * 5); c++) {
			mem[fntAddr + c] = fontData[c];
		}
	}
	void OnUpdate(f32 deltaTime) override {
		clockt -= deltaTime;
		if (clockt <= 0.f) {
			clockt = 1.f / 1000.f;
			ui16 opCode = fetch();
			execute(opCode);
		}

		ftime -= deltaTime;
		if (ftime <= 0.f) {
			dt -= (dt > 0 ? 1 : 0);
			st -= (st > 0 ? 1 : 0);
			ftime = 1.f / 60.f;
			Clear();
			for (int y = 0; y < screen.size(); y++) {
				ui64 mask = ui64(1) << 63;
				for (int x = 0; x < 64; x++) {
					if (screen[y] & mask) Point(x, y);
					mask >>= 1;
				}
			}
		}
	}


// ############################################################
// ############################################################
// ##                                                        ##
// ##              CPU fetch-decode-execute cycle            ##
// ##                     IMPLEMMENTATION                    ##
// ##                                                        ##
// ############################################################
// ############################################################
	
	// stack

	void stackPush(ui16 addr) {
		stack[sp++] = addr;
	}

	ui16 stackPop() {
		return stack[--sp];
	}

	//input implementation

	bool chip8Pressed(ui8 k) {
		switch (k) {
		case 0:
			return IsKeyPressed(voi::X);
		case 1:
			return IsKeyPressed(voi::K1);
		case 2:
			return IsKeyPressed(voi::K2);
		case 3:
			return IsKeyPressed(voi::K3);
		case 4:
			return IsKeyPressed(voi::Q);
		case 5:
			return IsKeyPressed(voi::W);
		case 6:
			return IsKeyPressed(voi::E);
		case 7:
			return IsKeyPressed(voi::A);
		case 8:
			return IsKeyPressed(voi::S);
		case 9:
			return IsKeyPressed(voi::D);
		case 10:
			return IsKeyPressed(voi::Z);
		case 11:
			return IsKeyPressed(voi::C);
		case 12:
			return IsKeyPressed(voi::K4);
		case 13:
			return IsKeyPressed(voi::R);
		case 14:
			return IsKeyPressed(voi::F);
		case 15:
			return IsKeyPressed(voi::V);
		}
	}

	ui16 fetch() {
		ui8 msb = mem[pc++ & 0x0FFF];
		ui8 lsb = mem[pc++ & 0x0FFF];
		pc = pc & 0x0FFF;

		ui16 opCode = (ui16(msb) << 8) | ui16(lsb);

		return opCode;
	}

	void execute(ui16 opCode) {
		switch ((opCode & 0xF000)) {
		case 0x0000:
			switch (opCode) {
			case 0x00E0:
				_00E0();
				break;
			case 0x00EE:
				_00EE();
				break;
			default:
				_0NNN(opCode & 0x0FFF);
			}
			break;
		case 0x1000:
			_1NNN(opCode & 0x0FFF);
			break;
		case 0x2000:
			_2NNN(opCode & 0x0FFF);
			break;
		case 0x3000:
			_3XNN(ui8((opCode & 0x0F00) >> 8), ui8(opCode & 0x00FF));
			break;
		case 0x4000:
			_4XNN(ui8((opCode & 0x0F00) >> 8), ui8(opCode & 0x00FF));
			break;
		case 0x5000:
			_5XY0(ui8((opCode & 0xF00) >> 8), ui8((opCode & 0x00F0) >> 4));
			break;
		case 0x6000:
			_6XNN(ui8((opCode & 0x0F00) >> 8), ui8(opCode & 0x00FF));
			break;
		case 0x7000:
			_7XNN(ui8((opCode & 0x0F00) >> 8), ui8(opCode & 0x00FF));
			break;
		case 0x8000:
			switch (opCode & 0x000F) {
			case 0x0:
				_8XY0(ui8((opCode & 0x0F00) >> 8), ui8((opCode & 0x00F0) >> 4));
				break;
			case 0x1:
				_8XY1(ui8((opCode & 0x0F00) >> 8), ui8((opCode & 0x00F0) >> 4));
				break;
			case 0x2:
				_8XY2(ui8((opCode & 0x0F00) >> 8), ui8((opCode & 0x00F0) >> 4));
				break;
			case 0x3:
				_8XY3(ui8((opCode & 0x0F00) >> 8), ui8((opCode & 0x00F0) >> 4));
				break;
			case 0x4:
				_8XY4(ui8((opCode & 0x0F00) >> 8), ui8((opCode & 0x00F0) >> 4));
				break;
			case 0x5:
				_8XY5(ui8((opCode & 0x0F00) >> 8), ui8((opCode & 0x00F0) >> 4));
				break;
			case 0x6:
				_8XY6(ui8((opCode & 0x0F00) >> 8), ui8((opCode & 0x00F0) >> 4));
				break;
			case 0x7:
				_8XY7(ui8((opCode & 0x0F00) >> 8), ui8((opCode & 0x00F0) >> 4));
				break;
			case 0xE:
				_8XYE(ui8((opCode & 0x0F00) >> 8), ui8((opCode & 0x00F0) >> 4));
				break;
			}
			break;
		case 0x9000:
			_9XY0(ui8((opCode & 0x0F00) >> 8), ui8((opCode & 0x00F0) >> 4));
			break;
		case 0xA000:
			_ANNN(opCode & 0x0FFF);
			break;
		case 0xB000:
			_BNNN(opCode & 0x0FFF);
			break;
		case 0xC000:
			_CXNN(ui8((opCode & 0xF00) >> 8), ui8(opCode & 0x00FF));
			break;
		case 0xD000:
			_DXYN(ui8((opCode & 0x0F00) >> 8), ui8((opCode & 0x00F0) >> 4), ui8(opCode & 0x000F));
			break;
		case 0xE000:
			switch (ui8(opCode & 0x00FF)) {
			case 0x9E:
				_EX9E(ui8((opCode & 0x0F00) >> 8));
				break;
			case 0xA1:
				_EXA1(ui8((opCode & 0x0F00) >> 8));
				break;
			}
			break;
		case 0xF000:
			switch (ui8(opCode & 0x00FF)) {
			case 0x07:
				_FX07(ui8((opCode & 0x0F00) >> 8));
				break;
			case 0x0A:
				_FX0A(ui8((opCode & 0x0F00) >> 8));
				break;
			case 0x15:
				_FX15(ui8((opCode & 0x0F00) >> 8));
				break;
			case 0x18:
				_FX18(ui8((opCode & 0x0F00) >> 8));
				break;
			case 0x1E:
				_FX1E(ui8((opCode & 0x0F00) >> 8));
				break;
			case 0x29:
				_FX29(ui8((opCode & 0x0F00) >> 8));
				break;
			case 0x33:
				_FX33(ui8((opCode & 0x0F00) >> 8));
				break;
			case 0x55:
				_FX55(ui8((opCode & 0x0F00) >> 8));
				break;
			case 0x65:
				_FX65(ui8((opCode & 0x0F00) >> 8));
				break;
			}
			break;
		}
	}

// ############################################################
// ############################################################
// ##                                                        ##
// ##                   OPCODES INSTRUCTIONS                 ##
// ##                     IMPLEMMENTATION                    ##
// ##                                                        ##
// ############################################################
// ############################################################

	void _0NNN(ui16 nnn) {
		stackPush(pc);
		pc = nnn;
	}

	void _00E0() {
		screen = { 0 };
	}

	void _00EE() {
		pc = stackPop();
	}

	void _1NNN(ui16 nnn) {
		pc = nnn;
	}

	void _2NNN(ui16 nnn) {
		stackPush(pc);
		pc = nnn;
	}

	void _3XNN(ui8 x, ui8 nn) {
		if (reg[x] == nn) pc = (pc + 2) & 0x0FFF;
	}

	void _4XNN(ui8 x, ui8 nn) {
		if (reg[x] != nn) pc = (pc + 2) & 0x0FFF;
	}

	void _5XY0(ui8 x, ui8 y) {
		if (reg[x] == reg[y]) pc = (pc + 2) & 0x0FFF;
	}

	void _6XNN(ui8 x, ui8 nn) {
		reg[x] = nn;
	}

	void _7XNN(ui8 x, ui8 nn) {
		reg[x] += nn;
	}

	void _8XY0(ui8 x, ui8 y) {
		reg[x] = reg[y];
	}
	void _8XY1(ui8 x, ui8 y) {
		reg[x] |= reg[y];
	}
	void _8XY2(ui8 x, ui8 y) {
		reg[x] &= reg[y];
	}
	void _8XY3(ui8 x, ui8 y) {
		reg[x] ^= reg[y];
	}
	void _8XY4(ui8 x, ui8 y) {
		ui16 xn = reg[x];
		ui16 yn = reg[y];

		xn += yn;
		reg[x] = ui8(xn & 0x00FF);

		if (xn & 0x0F00) reg[0xF] = 1;
	}
	void _8XY5(ui8 x, ui8 y) {
		ui8 xn = reg[x];
		reg[x] -= reg[y];
		reg[0xF] = (xn >= reg[y]);
	}
	void _8XY6(ui8 x, ui8 y) {
		reg[x] = reg[y] >> 1;
		reg[0xF] = reg[y] & 0x1;
	}
	void _8XY7(ui8 x, ui8 y) {
		ui8 xn = reg[x];
		reg[x] = reg[y] - xn;
		reg[0xF] = (reg[y] >= xn);
	}
	void _8XYE(ui8 x, ui8 y) {
		reg[x] = reg[y] << 1;
		reg[0xF] = (reg[y] & 0x80) >> 7;
	}
	void _9XY0(ui8 x, ui8 y) {
		if (reg[x] != reg[y]) pc = (pc + 2) & 0x0FFF;
	}
	void _ANNN(ui16 nnn) {
		I = nnn;
	}
	void _BNNN(ui16 nnn) {
		pc = ((nnn + reg[0]) & 0x0FFF);
	}
	void _CXNN(ui8 x, ui8 nn) {
		reg[x] = ui8(rand() & nn);
	}
	void _DXYN(ui8 x, ui8 y, ui8 n) {
		reg[0xF] = 0;
		ui8 yp = reg[y];
		ui16 xof = 56 - reg[x];

		for (ui8 i = 0; i < n; i++) {
			ui64 spr = 0;
			ui16 yof = yp + i;
			if (xof & 0x8000) spr = ui64(mem[(I + i) & 0x0FFF]) >> ((-1) * xof);
			else spr = ui64(mem[(I + i) & 0x0FFF]) << xof;

			if (yof >= 0 && yof < 32) {
				reg[0xF] = ((screen[yof] & spr) > 0);
				screen[yof] ^= spr;
			}
		}
	}
	void _EX9E(ui8 x) {
		if (chip8Pressed(reg[x])) pc = (pc + 2) & 0x0FFF;
	}
	void _EXA1(ui8 x) {
		if (!chip8Pressed(reg[x])) pc = (pc + 2) & 0x0FFF;
	}
	void _FX07(ui8 x) {
		reg[x] = dt;
	}
	void _FX0A(ui8 x) {
		bool wait = true;
		while (wait) {
			for (ui8 k = 0; k < 16; k++) {
				if (chip8Pressed(k)) {
					reg[x] = k;
					wait = false;
					break;
				}
			}
		}
	}
	void _FX15(ui8 x) {
		dt = reg[x];
	}
	void _FX18(ui8 x) {
		st = reg[x];
	}
	void _FX1E(ui8 x) {
		I = ((I + reg[x]) & 0x0FFF);
	}
	void _FX29(ui8 x) {
		I = (fntAddr + (reg[x] * 5)) & 0xFFF;
	}
	void _FX33(ui8 x) {
		ui8 xn = reg[x];
		mem[I] = ui8(xn / 100);
		mem[(I + 1) & 0x0FFF] = ui8((xn - (mem[I] * 100)) / 10);
		mem[(I + 2) & 0x0FFF] = ui8(xn - (mem[I] * 100) - (mem[(I + 1) & 0x0FFF] * 10));
	}
	void _FX55(ui8 x) {
		for (ui8 of = 0; of <= x; of++) mem[(I + of) & 0x0FFF] = reg[of];
	}
	void _FX65(ui8 x) {
		for (ui8 of = 0; of <= x; of++) reg[of] = mem[(I + of) & 0x0FFF];
	}
};
