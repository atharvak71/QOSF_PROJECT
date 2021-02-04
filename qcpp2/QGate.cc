#include "QGate.h"
#include "QMachine.h"
#include <sstream>
#include <algorithm>
#include <vector>
#include <iostream>
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
	qm(machine),
	bits_used(bu==0 ? machine.num_bits : std::min(machine.num_bits,bu))
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
	
	std::string tmplatsrc=templatesrc;
	tmplatsrc=findreplace(tmplatsrc,"ARG",arg);
	tmplatsrc=findreplace(tmplatsrc,"ACALL",acall);
	tmplatsrc=findreplace(tmplatsrc,"ATYPE",atype);
	
	tmplatsrc+=gatesrc+"\n\n\n";
	
	srcs.push_back(std::make_pair(tmplatsrc.c_str(),tmplatsrc.size()));

	std::ostringstream flgstream;
	size_t bumsk=1;
	bumsk<<=bits_used;
	bumsk--;
	
	
	flgstream << flags << " -DBU=" << bits_used << " -DBUMASK=" << bumsk;
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

void QGate::operator()(QMachine& mac)
{		
	this->apply(mac);
}

void QGate::apply(QMachine& mac)
{
	mac.run_gate_kernel(kernel);
}

void QGate::operator()()
{
	this->operator()(qm);
}
void QGate::operator()(QMachine& mac,const permutation_t& p)
{		
	mac.permutation_gate->operator()(mac,p);
	this->operator()(mac);
	mac.permutation_gate->operator()(mac,p.inverse());
	//inverse
}
void QGate::operator()(const permutation_t& p)
{
	this->operator()(qm,p);
}

string QClassicalGate::src=R"CLSRC(
ATYPE
size_t gatefunc(size_t input ARG);
__kernel void gateapply(__global float2* outbits,const __global float2* inbits ARG)
{
	const size_t bumask=BUMASK;
	size_t input=get_global_id(0);
	size_t output=gatefunc(input & bumask ACALL) & bumask	| (input & ~bumask);
	outbits[output]=inbits[input];
	//outbits[input]=(float2)(0.0,1.0) * output;
}
)CLSRC";

string QQuantumGate::src=R"CLSRC(
ATYPE
float2 gatefunc(size_t row,const __global float2* instate ARG);
__kernel void gateapply(__global float2* outbits,const __global float2* inbits ARG)
{
	const size_t bumask=BUMASK;
	size_t outputid=get_global_id(0);
	size_t inputid=outputid & ~bumask;
	outbits[outputid]=gatefunc(outputid & bumask,inbits+inputid ACALL);
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
