#ifndef QGATE_H
#define QGATE_H

#define __CL_ENABLE_EXCEPTIONS 
#include<CL/cl.hpp>
#include<cstdlib>
#include<string>
#include<cstdint>
#include<initializer_list>
class QMachine;


struct permutation_t
{
	////this is output[i]=input[index[i]]
	cl_char indices[64];
	permutation_t inverse() const;
	permutation_t();
	permutation_t(const std::initializer_list<cl_char>&);
};

std::ostream& operator<<(std::ostream& out,const permutation_t&);

class QGate
{
protected:
	cl::Program prog;
	cl::Kernel kernel;
	QMachine& qm;
	
	virtual void apply(QMachine& mac);
public:
	const std::size_t bits_used;
	
	QGate(QMachine& machine,
		const std::size_t& bu,
		const std::string& templatesrc,
       		const std::string& gatesrc,
		const std::string& argtype="",
		const std::string& flags=""
     	);
	virtual void operator()(QMachine& mac);
	virtual void operator()();
	virtual void operator()(const permutation_t&);
	virtual void operator()(QMachine& mac,const permutation_t&);
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
		GateType(m,bu,gatesrc,argtype,flags)
	{}
	virtual void operator()()
	{
		GateType::kernel.setArg(2,argval);
		GateType::operator()();
	}
	virtual void operator()(const AT& a0)
	{
		argval=a0;
		this->operator()();
	}
	virtual void operator()(QMachine& qm)
	{
		GateType::kernel.setArg(2,argval);
		GateType::operator()(qm);
	}
	virtual void operator()(QMachine& qm,const AT& a0)
	{
		argval=a0;
		this->operator()(qm);
	}

	virtual void operator()(const permutation_t& p)
	{
		GateType::kernel.setArg(2,argval);
		GateType::operator()(p);
	}
	virtual void operator()(const AT& a0,const permutation_t& p)
	{
		argval=a0;
		this->operator()(p);
	}
	virtual void operator()(QMachine& qm,const permutation_t& p)
	{
		GateType::kernel.setArg(2,argval);
		GateType::operator()(qm,p);
	}
	virtual void operator()(QMachine& qm,const AT& a0,const permutation_t& p)
	{
		argval=a0;
		this->operator()(qm,p);
	}
};



#endif
