#include "StandardGates.h"
#include<sstream>
#include<algorithm>
#include <complex>
#include "QMachine.h"

std::string NOT::src=R"CLSRC(
	
size_t gatefunc(size_t input)
{
	return ~input;
}
)CLSRC";

std::string GTOFFOLI::src=R"CLSRC(
	
size_t gatefunc(size_t input)
{
	size_t prod=((input>>1) & (BUMASK>>1))==(BUMASK>>1) ? 0x1 : 0x0;//if ALL upper bumask bits are set, then prod=1
	return input ^ prod; //target = prod XOR input;
}
)CLSRC";

std::string GFREDKIN::src=R"CLSRC(
	
size_t gatefunc(size_t input)
{
	size_t output=input & 0x00;
	if(((input>>2) & (BUMASK>>2))==(BUMASK>>2))
	{
		output |= ((input << 1) & 0x2) | ((input >> 1) & 0x1);
	}//if ALL upper bumask bits are set, then prod=1
	else
	{
		output=input;
	}
	return output; //target = prod XOR input;
}
)CLSRC";

std::string ADDI::src=R"CLSRC(
	
size_t gatefunc(size_t input,long lit)
{
	return input+lit;
}
)CLSRC";

std::string XORI::src=R"CLSRC(
	
size_t gatefunc(size_t input,ulong lit)
{
	return input ^ lit;
}
)CLSRC";


std::string CIRCRSHIFT::src=R"CLSRC(
	
size_t gatefunc(size_t input,arg_t samount)
{
	const size_t rshift= (samount % BU + BU) % BU;
	const size_t low_order=input & ((1<<rshift)-1);
	return (input >> rshift) | (low_order << (BU-rshift));
}
)CLSRC";


std::string HADAMARD::src=R"CLSRC(
	
#define isqr2 0.70710678118f
__constant static const float2 H[2] = {(float2)(isqr2,isqr2), (float2)(isqr2,-isqr2)};
float2 gatefunc(size_t row,const __global float2* input)
{
	const float2 hr=H[row & 0x1];
	return hr.x*input[0]+hr.y*input[1];
}
)CLSRC";

std::string PHASESHIFT::src=R"CLSRC(
	
float2 gatefunc(size_t row,const __global float2* input,float phase)
{
	float sn,cs;
	sn=sincos(phase,&cs);
	const float2 rpart=(float2)(cs,-sn);
	const float2 ipart=(float2)(sn, cs);
	
	return (row & 0x1 ?  (rpart*input[1].x+ipart*input[1].y) : input[0] );
}
)CLSRC";

std::string ROTATOR::src=R"CLSRC(
	
float2 gatefunc(size_t row,const __global float2* input,float phase)
{
	float2 scs;
	scs.y=sincos(phase,&scs.x);
	hr=(row & 0x1 ? scs.yx*(float2)(-1.0f,0.0f) : scs.xy);
	return hr.x*input[0]+hr.y*input[1];
}
)CLSRC";



std::string PERMUTE::src=R"CLSRC(
		
size_t gatefunc(size_t input,arg_t permutation)
{
	size_t output=0;
	for(size_t i=0;i<BU;i++)
	{	//this is output[i]=input[index[i]]
		output |= ((input >> permutation.indices[i]) & 0x1) << i;
	}
	return output; //target = prod XOR input;
}
)CLSRC";

void PERMUTE::operator()()
{
	QClassicalGate::kernel.setArg(2,argval);
	QClassicalGate::operator()();
}
void PERMUTE::operator()(const permutation_t& a0)
{
	argval=a0;
	this->operator()();
}
void PERMUTE::operator()(QMachine& qm)
{
	QClassicalGate::kernel.setArg(2,argval);
	QClassicalGate::operator()(qm);
}
void PERMUTE::operator()(QMachine& qm,const permutation_t& a0)
{
	argval=a0;
	this->operator()(qm);
}


//this is output[i]=input[index[i]]
//{0,1,2,3,4,5,6,7,8,9,10,11,12}
//
permutation_t permutation_t::inverse() const
{
	permutation_t pt;
	for(cl_long i=0;i<(sizeof(cl_long)*8);i++)
	{
		pt.indices[indices[i]]=i;
	}
	return pt;
}

permutation_t::permutation_t()
{
	for(cl_long i=0;i<(sizeof(cl_long)*8);i++)
	{
		indices[i]=i;
	}
}
permutation_t::permutation_t(const std::initializer_list<cl_char>& inp)
{
	bool used[64];
	std::fill(used,used+64,false);
	cl_char *lst=indices;
	for(std::initializer_list<cl_char>::const_iterator ci=inp.begin();ci!=inp.end() && (lst < indices+64);++ci)
	{
		cl_char d=*ci & 0x3F;
		used[d]=true;
		*lst++=d;
	}
	for(int i=0;i<64;i++)
	{
		if(!used[i])
		{
			*lst++=i;
		}
	}
}

std::ostream& operator<<(std::ostream& out,const permutation_t& p)
{
	out << "{";
	for(int i=0;i<64;i++)
	{
		out << (int)p.indices[i] << ",";
	}
	out << "}";
	return out;
}
