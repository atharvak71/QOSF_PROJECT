#ifndef QGATE_H
#define QGATE_H

#define __CL_ENABLE_EXCEPTIONS 
#include<CL/cl.hpp>
#include<cstdlib>
#include<string>
#include<cstdint>
#include<initializer_list>
#include"QRegister.h"
class QMachine;

std::ostream& operator<<(std::ostream& out,const permutation_t&);

permutation_t combine_registers(const std::initializer_list<QRegister>&);
cl_ulong      compute_regspec(const std::initializer_list<QRegister>&);

class QGate
{
protected:
	cl::Program prog;
	cl::Kernel kernel;
	QMachine& qm;
	
	virtual void apply(const cl_ulong& regspec);
	
	template<class AT>
	void arg(size_t i,const AT& a)
	{
		kernel.setArg(i+3,a);
	}
	static std::string standard_library_src;
public:
	QGate(QMachine& machine,
		const std::size_t& regspec,
		const std::string& templatesrc,
       		const std::string& gatesrc,
		const std::string& argtype="",
		const std::string& flags=""
     	);
	virtual QOperationRecord operator()(const::std::initializer_list<QRegister>& regs);
	virtual QOperationRecord operator()(const permutation_t& regperm,const cl_ulong& regspec);

};

class QClassicalGate: public QGate
{
private:
	//classic gate func takes in a size_t-size integer where the first bu bits are used in the computation.
	//outputs a same integer with the results of the computation stored.
	static std::string src;
public:
	QClassicalGate(QMachine& m,const std::size_t& bu,const std::string& gatesrc,const std::string& argtype="",const std::string& flags=""):
		QGate(m,bu,QClassicalGate::src,gatesrc,argtype,flags)
	{}
};

class QQuantumGate: public QGate
{
private:
	//quantum gate func takes in a size_t integer where the first bu bits are the row
	//of the matrix, and a global float2* pointer pointing to the input state vector
	//it outputs a float2 for the dot product be;tween the matrix at that point and the 
        //corresponding row of the matrix.
	static std::string src;
public:
	QQuantumGate(QMachine& m,const std::size_t& bu,const std::string& gatesrc,const std::string& argtype="",const std::string& flags=""):
		QGate(m,bu,QQuantumGate::src,gatesrc,argtype,flags)
	{}
};

//A special type of gate that accepts exactly 1 argument....maybe this could be expanded to a type of gate that accepts n arguments...hm
template<class AT,class GateType>
class QGate1Arg: public GateType
{
public:
	AT argval;
	QGate1Arg(QMachine& m, const std::size_t& bu, const std::string& gatesrc, const std::string& argtype = "", const AT& arginitial=AT(),const std::string& flags = ""):
		GateType(m,bu,gatesrc,argtype,flags),argval(arginitial)
	{}
	
	virtual QOperationRecord operator()(const std::initializer_list<QRegister>& regs,const AT& a0)
	{
		argval=a0;
		GateType::arg(0,argval);
		return GateType::operator()(regs);
	}
	virtual QOperationRecord operator()(const permutation_t& p,const cl_ulong& regspec,const AT& a0)
	{
		argval=a0;
		GateType::arg(0,argval);
		return GateType::operator()(p,regspec);
	}
	virtual QOperationRecord operator()(const std::initializer_list<QRegister>& regs)
	{
		GateType::arg(0,argval);
		return GateType::operator()(regs);
	}
	virtual QOperationRecord operator()(const permutation_t& p,const cl_ulong& regspec)
	{
		GateType::arg(0,argval);
		return GateType::operator()(p,regspec);
	}
	void set_arg(const AT& a0)
	{
		argval=a0;
	}
};

class QEmptyGate:public QClassicalGate
{
protected:
	static std::string src;
public:
	QEmptyGate(QMachine& m,const std::size_t& regspec):
		QClassicalGate(m,regspec,src)
	{}
};



#endif
