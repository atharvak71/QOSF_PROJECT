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

class CIRCRSHIFT: public QGate1Arg<cl_short,QClassicalGate>
{
private:
	static std::string src;
public:
	CIRCRSHIFT(QMachine& m,const cl_short& lit=0,const std::size_t& bu=0):
		QGate1Arg<cl_short,QClassicalGate>(m,bu,CIRCRSHIFT::src,"short",lit)
	{}
};


class HADAMARD: public QQuantumGate
{
private:
	static std::string src;
public:
        HADAMARD(QMachine& m):
	        QQuantumGate(m,1,HADAMARD::src)
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
		QClassicalGate(m,bu,PERMUTE::src,"struct { char indices[64]; }"),
		argval(lit)
	{}
	
	virtual void operator()();
	virtual void operator()(const permutation_t& a0);
	virtual void operator()(QMachine& qm);
	virtual void operator()(QMachine& qm,const permutation_t& a0);
};

class MEASUREMENT: public QGate
{
private:
	static std::string tsrc;
	static std::string gatesrc;
	std::uint64_t mresult;
	CIRCRSHIFT msbshift;
	virtual void apply(QMachine& qm);
public:
	MEASUREMENT(QMachine& m,const std::size_t& bu=0);
	
	const std::uint64_t& result;
};


#endif
