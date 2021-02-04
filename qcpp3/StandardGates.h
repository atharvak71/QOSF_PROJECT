#ifndef STANDARD_GATES_H
#define STANDARD_GATES_H

#include "QGate.h"

class NOT: public QClassicalGate
{
private:
	static std::string src;
public:
    NOT(QMachine& m,const std::size_t& bu=0):QClassicalGate(m,bu,NOT::src)
    {}
};

class GTOFFOLI: public QClassicalGate
{
private:
	static std::string src;
public:
    GTOFFOLI(QMachine& m,const std::size_t& bu=3):QClassicalGate(m,bu,GTOFFOLI::src)
    {}
};

class TOFFOLI: public GTOFFOLI
{
public:
	TOFFOLI(QMachine& m):GTOFFOLI(m,3)
	{}
};

class CNOT: public GTOFFOLI
{
public:
	CNOT(QMachine& m):GTOFFOLI(m,2)
	{}
};

class GFREDKIN: public QClassicalGate
{
private:
	static std::string src;
public:
	GFREDKIN(QMachine& m,const std::size_t& bu=3):
		QClassicalGate(m,bu,GFREDKIN::src)
	{}
};
class FREDKIN: public GFREDKIN
{
public:
	FREDKIN(QMachine& m):
		GFREDKIN(m,3)
	{}
};

typedef FREDKIN CSWAP;

class ADDI: public QGate1Arg<cl_long,QClassicalGate>
{
private:
	static std::string src;
public:
	ADDI(QMachine& m, const cl_long& lit=1,const std::size_t& bu=0):
		QGate1Arg<cl_long,QClassicalGate>(m,bu,ADDI::src,"long",lit)
	{}
};

class XORI: public QGate1Arg<cl_ulong,QClassicalGate>
{
private:
	static std::string src;
public:
	XORI(QMachine& m, const cl_ulong& lit=0,const std::size_t& bu=0):
		QGate1Arg<cl_ulong,QClassicalGate>(m,bu,XORI::src,"ulong",lit)
	{}
};

class CIRCRSHIFTI: public QGate1Arg<cl_short,QClassicalGate>
{
private:
	static std::string src;
public:
	CIRCRSHIFTI(QMachine& m,const cl_short& lit=0,const std::size_t& bu=0):
		QGate1Arg<cl_short,QClassicalGate>(m,bu,CIRCRSHIFTI::src,"short",lit)
	{}
};


class HADAMARD1: public QQuantumGate
{
private:
	static std::string src;
public:
        HADAMARD1(QMachine& m):
	        QQuantumGate(m,1,HADAMARD1::src)
	{}
};

class PHASESHIFT: public QGate1Arg<cl_float,QQuantumGate>
{
private:
	static std::string src;
public:
	PHASESHIFT(QMachine& m,const cl_float& lit=0):
		QGate1Arg<cl_float,QQuantumGate>(m,1,PHASESHIFT::src,"float",lit)
		{}
};

class ROTATOR: public QGate1Arg<cl_float,QQuantumGate>
{
private:
	static std::string src;
public:
	ROTATOR(QMachine& m,const cl_float& lit=0):
		QGate1Arg<cl_float,QQuantumGate>(m,1,ROTATOR::src,"float",lit)
		{}
};

class PERMUTE: public QClassicalGate
{
private:
	static std::string src;
public:
	permutation_t argval;
	PERMUTE(QMachine& m,const permutation_t& lit=permutation_t(),const std::size_t& bu=0):
		QClassicalGate(m,bu,PERMUTE::src,"struct { char indices[REG_MAX_SIZE]; }"),
		argval(lit)
	{}
	
	virtual QOperationRecord operator()(const::std::initializer_list<QRegister>& regs);
	virtual QOperationRecord operator()(const permutation_t& regperm,const cl_ulong& regspec);
};

class MEASUREMENT: public QGate
{
private:
	static std::string tsrc;
	static std::string gatesrc;
	std::uint64_t mresult;
	CIRCRSHIFTI msbshift;
protected:
	virtual void apply(const cl_ulong& regspec);
public:
	MEASUREMENT(QMachine& m,const std::size_t& bu=0);
	
	const std::uint64_t& result;
};

class BIT_1_TO_N: public QEmptyGate
{
private:
	QGate& one_bit_gate;
	virtual void apply(const cl_ulong& regspec);
public:
	BIT_1_TO_N(QMachine& m,QGate& obg):
		QEmptyGate(m,1),
		one_bit_gate(obg)
	{}
};

#endif 
