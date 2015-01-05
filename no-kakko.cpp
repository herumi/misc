#if 1
#include <stdio.h>
#include <vector>
#include <stdexcept>
#include <sstream>

struct Reg {
	int idx_;
	Reg(int idx) : idx_(idx) { }
	std::string toString() const
	{
		return idx_ == 0 ? "eax" : "ecx";
	}
} eax(0), ecx(1);

struct Mov {
	std::vector<std::string> op;
	Mov& operator,(int val)
	{
		std::ostringstream os;
		os << val;
		op.push_back(os.str());
		return *this;
	}
	Mov& operator,(const Reg& reg)
	{
		op.push_back(reg.toString());
		return *this;
	}
	~Mov()
	{
		if (op.size() != 2) throw std::runtime_error("need 2op");
		printf("mov %s, %s\n", op[0].c_str(), op[1].c_str());
	}
};

#define mov Mov(),

int main()
	try
{
	mov eax, ecx;
	mov ecx, 5;

} catch (std::exception& e) {
	printf("err=%s\n", e.what());
}
#else
#include <stdio.h>
#include <vector>
#include <stdexcept>
#include <iostream>
#include <sstream>

enum {
	type_Imm, type_Reg
};

struct Operand {
	int type_;
	Operand(int type) : type_(type) { }
	virtual std::string toString() const = 0;
};

struct Imm : Operand {
	int val_;
	Imm(int val) : Operand(type_Imm), val_(val) { }
	std::string toString() const
	{
		std::ostringstream os;
		os << val_;
		return os.str();
	}
};

struct Reg : Operand {
	int idx_;
	Reg(int idx) : Operand(type_Reg), idx_(idx) { }
	std::string toString() const
	{
		return (idx_ == 0 ? "eax" : "ecx");
	}
} eax(0), ecx(1);

struct Mov {
	std::vector<Operand*> op;
	Mov& operator,(int val) { op.push_back(new Imm(val)); return *this; }
	Mov& operator,(const Reg& reg) { op.push_back(new Reg(reg)); return *this; }
	~Mov()
	{
		if (op.size() != 2) throw std::runtime_error("need 2op");
		std::cout << "mov " << op[0]->toString() << ", " << op[1]->toString() << std::endl;
		for (size_t i = 0; i < op.size(); i++) {
			delete op[i];
		}
	}
};

#define mov Mov(),

int main()
	try
{
	mov eax, ecx;
	mov ecx, 5;

} catch (std::exception& e) {
	printf("err=%s\n", e.what());
}
#endif
