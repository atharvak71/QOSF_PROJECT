#include "QGate.h"
#include "QMachine.h"
#include <sstream>
#include <algorithm>
#include <vector>
#include <iostream>
#include <stdexcept>
using namespace std;

static std::string findreplace(const std::string& src,const std::string& pattern,const std::string& repl)
{
	std::string newstr=src;
	for(	std::size_t found = newstr.find(pattern.c_str());
			found!=std::string::npos;
			found=newstr.find(pattern.c_str()))
	{
		newstr.replace(found,pattern.size(),repl);
	}
	return newstr;
}

//((input >> permutation.indices[i]) & 0x1) << i;
//This implies a[i]=a[p[i]] is the operation
//{45,46,47,48}

permutation_t combine_registers(const std::initializer_list<QRegister>& lst)
{
	//Detect register overlap here...
	bool used[REG_MAX_SIZE];
	std::fill(used,used+REG_MAX_SIZE,false);
	permutation_t output;
	
	//Get the permutation indices for each used register, put in order
	size_t coutput=0;
	for(const QRegister& l : lst)
	{
		for(size_t j=0;j<l.num_bits;j++)
		{
			unsigned char ni=l.definition.indices[j];
			if(ni)
			{
				throw std::runtime_error("Error, two different registers in an operation had overlapping bits.  This is not right.");
			}
			else
			{
				used[ni]=true;
				output.indices[coutput++]=ni;
			}
		}
	}
	for(size_t ui=0;ui<REG_MAX_SIZE && coutput<REG_MAX_SIZE;ui++)
	{
		if(!used[ui])
		{
			output.indices[coutput++]=ui;
		}
	}
	return output;
}
cl_ulong compute_regspec(const std::initializer_list<QRegister>& lst)
{
	cl_ulong regspecout=0;
	size_t i=0;
	for(const QRegister& l : lst)
	{
		regspecout |= (l.num_bits & REG_MAX_MASK) << (REG_MAX_SIZE_BITS*i++);	//size of each register
	}
	return regspecout;
}

/*
static vector<string> parseargs(const std::string& arglist)
{
	string word;
	vector<string> args;
	stringstream stream(arglist);
	while( getline(stream, word, ',') )
	        args.push_back(word);
	return args;
}*/

class clBuildError: public cl::Error
{
public:
	std::string whatstr;
	
	clBuildError(const cl::Error& e,const std::string& buildlog):
		cl::Error(e.err(),e.what())
	{
		whatstr=cl::Error::what();
		whatstr+="Build Error:\n";
		whatstr+=buildlog;
	}
	
	virtual const char * what() const throw ()
	{
		return whatstr.c_str();
	}
};

QGate::QGate(QMachine& machine,
	const std::size_t& bu,
	const std::string& templatesrc,
	const std::string& gatesrc,
	const std::string& argtype,
	const std::string& flags
    	):
	qm(machine)
{
	cl::Program::Sources srcs;
	
	std::string atype="";
	std::string arg="";
	std::string acall="";
	
	if(argtype!="")
	{
		atype="typedef "+argtype+" arg_t;";
		arg=",arg_t a0";
		acall=",a0";
	}
	
	std::string tmplatsrc=QGate::standard_library_src+"\n\n\n"+templatesrc;
	tmplatsrc=findreplace(tmplatsrc,"ARG",arg);
	tmplatsrc=findreplace(tmplatsrc,"ACALL",acall);
	tmplatsrc=findreplace(tmplatsrc,"ATYPE",atype);
	
	tmplatsrc+=gatesrc+"\n\n\n";
	
	srcs.push_back(std::make_pair(tmplatsrc.c_str(),tmplatsrc.size()));

	std::ostringstream flgstream;
/*	size_t bumsk=1;
	bumsk<<=bits_used;
	bumsk--;*/
	
	
	flgstream << flags 
	//	<< " -DBU=" << bits_used 
	//	<< " -DBUMASK=" << bumsk 
		<< " -DREG_MAX_SIZE_BITS=" << REG_MAX_SIZE_BITS
		<< " -DREG_MAX_SIZE=" << REG_MAX_SIZE
		<< " -DREG_MAX_MASK=" << REG_MAX_MASK; 
	prog=cl::Program(machine.context,srcs);
	
	try
	{
		prog.build(machine.devices,flgstream.str().c_str());
	}
	catch(const cl::Error& e)
	{
		string buildlog;
		prog.getBuildInfo(machine.devices[0], (cl_program_build_info)CL_PROGRAM_BUILD_LOG, &buildlog );
		buildlog+="\nSource:\n";
		buildlog+=tmplatsrc;
		throw clBuildError(e,buildlog);
	}
	
	kernel=cl::Kernel(prog,"gateapply");
}

void QGate::apply(const cl_ulong& regspec)
{
	qm.run_gate_kernel(kernel,regspec);
}

QOperationRecord QGate::operator()(const permutation_t& p,const cl_ulong& regspec)
{
	qm.permutation_gate->operator()(p,REG_MAX_SIZE);
	this->apply(regspec);
	qm.permutation_gate->operator()(p.inverse(),REG_MAX_SIZE);
	return QRegister(qm,p,regspec & REG_MAX_MASK);
}
QOperationRecord QGate::operator()(const::std::initializer_list<QRegister>& regs)
{
	permutation_t perm=combine_registers(regs);
	cl_ulong spec=compute_regspec(regs);
	return operator()(perm,spec);
}

string QGate::standard_library_src=R"CLSRC(
	//TODO: Stuff with regspec unpack
	size_t regspec_length_total(ulong regspec)
	{
		size_t clen=0;
		while(regspec != 0)
		{
			size_t ilen=regspec & REG_MAX_MASK;
			clen+=ilen;
			respec >>= REG_MAX_SIZE_BITS;
		}
		return clen;
	}
	size_t regspec_reg_read(size_t input,ulong regspec,uchar id)
	{
		uint cpos=0;
		for(uint i=0;i<id;i++)
		{
			cpos+=regspec & REG_MAX_MASK;
			
			regspec >>= REG_MAX_SIZE_BITS;
		}
		uint clen=regspec & REG_MAX_MASK;
		return (input >> cpos) & ((1<<clen)-1);
	}
	void regspec_reg_write(size_t* input,ulong regspec,size_t val,uchar id)
	{
		uint cpos=0;
		for(uint i=0;i<id;i++)
		{
			cpos+=regspec & REG_MAX_MASK;
			regspec >>= REG_MAX_SIZE_BITS;
		}
		uint clen=regspec & REG_MAX_MASK;
		size_t lmask=((1<<clen)-1) << cpos;
		*input &=~mask;
		*input |= (val << cpos) & mask;
	}
	size_t regspec_reg_size(ulong regspec,uchar id)
	{
		return (regspec >> (id*REG_MAX_SIZE_BITS)) & REG_MAX_MASK;
	}
)CLSRC";

string QClassicalGate::src=R"CLSRC(
ATYPE
size_t gatefunc(size_t input,const ulong regspec ARG);
__kernel void gateapply(__global float2* outbits,const __global float2* inbits,const ulong regspec ARG)
{
	const size_t bumask=regspec_length_total(regspec);
	size_t input=get_global_id(0);
	size_t output=gatefunc(input & bumask ACALL) & bumask	| (input & ~bumask);
	outbits[output]=inbits[input];
	//outbits[input]=(float2)(0.0,1.0) * output;
}
)CLSRC";

string QQuantumGate::src=R"CLSRC(
ATYPE
float2 gatefunc(size_t row,const ulong regspec,const __global float2* instate ARG);
__kernel void gateapply(__global float2* outbits,const __global float2* inbits,const ulong regspec ARG)
{
	const size_t bumask=regspec_length_total(regspect);
	size_t outputid=get_global_id(0);
	size_t inputid=outputid & ~bumask;
	outbits[outputid]=gatefunc(outputid & bumask,inbits+inputid ACALL);
}
)CLSRC";
		
string QEmptyGate::src=R"CLSRC(
size_t gatefunc(size_t input,const ulong regspec)
{
	return input;
}
	
)CLSRC";
		
/*
string QClassicalGate::debug_src=R"CLSRC(
ATYPE
size_t gatefunc(size_t ARG);
__kernel void gateapply(__global float2* outbits,const __global float2* inbits,__global unsigned int* debugbits ARG)
{
	const size_t bumask=BUMASK;
	size_t input=get_global_id(0);
	
	size_t output=gatefunc(input & bumask ACALL) & bumask	| (input & ~bumask);
	outbits[output]=inbits[input];
	//outbits[input]=(float2)(0.0,1.0) * output;
	
	const size_t N=get_global_size(0);
	
	//debugbits is an N bitarray stored in an array of unsigned ints.
	//writing to them is simple
	atom_or(outbits+output>>5,1 << (output & 0x1F)); //writes to the bit for output.  If gate is reversable, then all outputs will be present.
	
	mem_fence (CLK_GLOBAL_MEM_FENCE);
	
	mem_fence (CLK_GLOBAL_MEM_FENCE);
	//they should be cleared in the lower threads after summation is done...
	if(input < (N >> 5)) //if input is less than N/32
	{
		debugbits[input]=0; //clear
	}
	
}
)CLSRC";*/
