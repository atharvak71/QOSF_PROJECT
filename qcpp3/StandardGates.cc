#include "StandardGates.h"
#include<sstream>
#include<algorithm>
#include <complex>
#include "QMachine.h"

std::string NOT::src=R"CLSRC(
	
size_t gatefunc(size_t input,const ulong regspec)
{
	return ~input;
}
)CLSRC";

std::string GTOFFOLI::src=R"CLSRC(
	
size_t gatefunc(size_t input,const ulong regspec)
{
	size_t a0=regspec_reg_read(input,regspec,0);
	size_t a1=regspec_reg_read(input,regspec,1);
	
	size_t a1sizemask=(1 << regspec_reg_size(regspec,1))-1;
	size_t a0sizemask=(1 << regspec_reg_size(regspec,0))-1;
	size_t prod=((a1 & a1sizemask) == a1sizemask)  ? 0x1 : 0x0;//if ALL upper bumask bits are set, then prod=1
	return input ^ (a0sizemask*prod); //target = prod XOR input; //then flip all target bits
}
)CLSRC";
//*THIS IS PROBLEMATIC
std::string GFREDKIN::src=R"CLSRC(
	
size_t gatefunc(size_t input,const ulong regspec)
{
	size_t output;
	size_t a2=regspec_reg_read(input,regspec,2);
	size_t a2sizemask=(1 << regspec_reg_size(regspec,2))-1;
	if((a2 & a2sizemask) == a2sizemask)
	{
		size_t a0=regspec_reg_read(input,regspec,0);
		size_t a1=regspec_reg_read(input,regspec,1);
		regspec_reg_write(input,a0,1);
		regspec_reg_write(input,a1,0);
	}//if ALL upper bumask bits are set, then prod=1
	output=input;
	return output; //target = prod XOR input;
}
)CLSRC";

std::string ADDI::src=R"CLSRC(
	
size_t gatefunc(size_t input,const ulong regspec,long lit)
{
	return input+lit;
}
)CLSRC";

std::string XORI::src=R"CLSRC(
	
size_t gatefunc(size_t input,const ulong regspec,ulong lit)
{
	return input ^ lit;
}
)CLSRC";


std::string CIRCRSHIFTI::src=R"CLSRC(
	
size_t gatefunc(size_t input,const ulong regspec,arg_t samount)
{
	const size_t RMM=REG_MAX_MASK >> 1;
	const size_t BUM=regspec & RMM;
	const size_t rshift= ((samount % BUM) + BUM) % BUM;
	const size_t low_order=input & ((1<<rshift)-1);
	return (input >> rshift) | (low_order << (BU-rshift));
}
)CLSRC";


std::string HADAMARD1::src=R"CLSRC(
	
#define isqr2 0.70710678118f
__constant static const float2 H[2] = {(float2)(isqr2,isqr2), (float2)(isqr2,-isqr2)};
float2 gatefunc(size_t row,const __global float2* input,const ulong regspec)
{
	const float2 hr=H[row & 0x1];
	return hr.x*input[0]+hr.y*input[1];
}
)CLSRC";

std::string PHASESHIFT::src=R"CLSRC(
	
float2 gatefunc(size_t row,const __global float2* input,const ulong regspec,float phase)
{
	float sn,cs;
	sn=sincos(phase,&cs);
	const float2 rpart=(float2)(cs,-sn);
	const float2 ipart=(float2)(sn, cs);
	
	return (row & 0x1 ?  (rpart*input[1].x+ipart*input[1].y) : input[0] );
}
)CLSRC";

std::string ROTATOR::src=R"CLSRC(
	
float2 gatefunc(size_t row,const __global float2* input,const ulong regspec,float phase)
{
	float2 scs;
	scs.y=sincos(phase,&scs.x);
	hr=(row & 0x1 ? scs.yx*(float2)(-1.0f,0.0f) : scs.xy);
	return hr.x*input[0]+hr.y*input[1];
}
x=
y=sin(p)
)CLSRC";



std::string PERMUTE::src=R"CLSRC(
		
size_t gatefunc(size_t input,const ulong regspec, arg_t permutation)
{
	size_t output=0;
	for(size_t i=0;i<(regspec & REG_MAX_MASK);i++)
	{	//this is output[i]=input[index[i]]
		output |= ((input >> permutation.indices[i]) & 0x1) << i;
	}
	return output; //target = prod XOR input;
}
)CLSRC";

QOperationRecord PERMUTE::operator()(const permutation_t& p,const cl_ulong& regspec)
{
	QClassicalGate::arg(0,p);
	this->apply(regspec);
	return QRegister(qm,p,regspec & REG_MAX_MASK);
}
QOperationRecord PERMUTE::operator()(const::std::initializer_list<QRegister>& regs)
{
	return QClassicalGate::operator()(regs);
}

void BIT_1_TO_N::apply(const cl_ulong& regspec)
{
	for(int i=0;i < (regspec & REG_MAX_MASK);i++)
	{
		one_bit_gate({i},1);
	}
}
