#include "qram.h"
#include<stdexcept>
#include<cstring>
#include<iostream>
using namespace qcpp;

static inline void print_binary(std::ostream& oss,const std::size_t& val,const unsigned int& bits)
{
	for(unsigned int bi=0;bi<bits;bi++)
	{
		unsigned int rbi=bits-bi-1;
		oss << ( (val >> rbi) & 1 );
		//oss << (int)((val >> (sizeof(std::size_t)*8-bi-1)) & 1);
	}
}
class private_qnot:public qgate
{
public:
	private_qnot():qgate(1,"NOT")
	{}
	virtual void apply(std::complex<double>* out,const std::complex<double>* in) const
	{
		out[0]=in[1];
		out[1]=in[0];
	}
};
qram::qram(const unsigned int& nb,std::ostream& os,const unsigned int& initialstate):
	num_entries(1 << nb),
	num_bits(nb),
	oss(os)
{
	if(num_bits > sizeof(std::size_t)*8)
	{
		throw std::range_error("Cannot allocate a qregister larger than the size of a machine pointer");
	}

	state=new std::complex<double>[num_entries];
	state_back=new std::complex<double>[num_entries];
	/*for(std::size_t index=0;index<num_entries/2;index++)
	{
		state[2*index]=1.0/sqrt((double)num_entries/2);
		state[2*index+1]=0.0;
	}*/
	memset(state,0,num_entries*sizeof(std::complex<double>));
	//memset(state_back,0,num_entries*sizeof(std::complex<double>));
	state[0]=1.0;//100% of being in the |0000...00> state
	
	private_qnot pqn;
	for(unsigned int i=0;i<num_bits;i++)
	{
		unsigned int cmask = 1 << i;
		if(initialstate & cmask)
		{
			op(pqn,cmask);
		}
	}
	
	
	for(unsigned int i=0;i<num_bits;i++)
	{
		os << "+-";
	}
	os << "--Initialization complete--\n";

}

qram::~qram()
{
	delete [] state;
	delete [] state_back;
}

static inline std::size_t shift(std::size_t value,std::size_t mask)
{
	std::size_t result=0;
	for(unsigned bi=0;bi<sizeof(std::size_t);bi++)
	{
		if(mask==0)
		{
			break;
		}
		if(mask & 1)
		{
			result |= (value & 1) << bi;
			value >>= 1;
		}
		mask >>= 1;
	}
	return result;
}
static inline std::size_t popcount(std::size_t x)
{
	std::size_t cnt=0;
	while(x != 0)
	{
		if(x & 1)
		{
			cnt++;
		}
		x>>=1;		
	}
	return cnt;
}

void qram::permute_internal_down(std::size_t mask)
{
	std::swap(state,state_back);
	const std::size_t mask_bits=popcount(mask);
	const std::size_t lowermask=(1 << mask_bits)-1;
	
	#pragma omp parallel for 
	for(std::size_t i=0;i<num_entries;i++)
	{
		std::size_t instance=i >> mask_bits;
		std::size_t ibase=shift(instance,~mask);
		std::size_t location=i & lowermask;
		std::size_t lindex=ibase+shift(location,mask);
		state[i]=state_back[lindex];
	}
}

void qram::permute_internal_up(std::size_t mask)
{	
	std::swap(state,state_back);
	const std::size_t mask_bits=popcount(mask);
	const std::size_t lowermask=(1 << mask_bits)-1;
	
	#pragma omp parallel for 
	for(std::size_t i=0;i<num_entries;i++)
	{
		std::size_t instance=i >> mask_bits;
		std::size_t ibase=shift(instance,~mask);
		std::size_t location=i & lowermask;
		std::size_t lindex=ibase+shift(location,mask);
		state[lindex]=state_back[i];
	}
}

void qram::op(const qgate& g,std::size_t mask)
{
	const std::size_t mask_bits=popcount(mask);
	const std::size_t instances=num_entries >> mask_bits;
	
	if(g.gate_bits!=mask_bits)
	{
		throw std::invalid_argument("The mask specified a different number of bits then the gate needs");
	}
	permute_internal_down(mask);
	
	std::swap(state,state_back);
	
	#pragma omp parallel for
	for(std::size_t instance=0;instance<instances;instance++)
	{
		std::size_t index=instance << mask_bits;
		g.apply(state+index,state_back+index);
	}
	
	permute_internal_up(mask);
	
	for(unsigned i=0;i<num_bits;i++)
	{
		oss << "| ";
	}
	oss << "\n";

	for(unsigned i=0;i<num_bits;i++)
	{
		oss << (((mask >> (num_bits-1-i)) & 1) ? "X " : "| ");
	}
	oss << "[" << g.gate_name << "]\n";
}

qram::measurement qram::measure(const std::size_t mask) const
{
	const std::size_t mask_bits=popcount(mask);
	const std::size_t lowermask=(1 << mask_bits)-1;
	qram::measurement ms(1 << mask_bits);
	ms.num_bits=mask_bits;
        std::complex<double> mag2=0.0;
	for(std::size_t mi=0;mi<(lowermask+1);mi++)
	{
		std::complex<double> sm=0.0;
		for(std::size_t i=0;i<(num_entries / (lowermask+1));i++)
		{
			std::size_t ibase=shift(i,~mask);
			std::size_t index=ibase+shift(mi,mask);
			sm+=state[index];
		}
		ms.state[mi]=sm;
                mag2+=sm;
        }
        mag2=std::sqrt(mag2);
        for(std::vector<std::complex<double> >::iterator i=ms.state.begin();i!=ms.state.end();++i)
	{
		*i /= mag2;
	}
	/*
	for(std::size_t i=0;i<num_entries;i++)
	{
		std::size_t instance=i >> mask_bits;
		std::size_t ibase=shift(instance,~mask);
		std::size_t location=i & lowermask;
		std::size_t lindex=ibase+shift(location,mask);
		state[i]=state_back[lindex];
	}*/
	return ms;
}

std::ostream& operator<<(std::ostream& oss,const qram& qc)
{
	for(std::size_t i=0;i<qc.num_entries;i++)
	{
		print_binary(oss,i,qc.num_bits);
		oss << " : " << qc.state[i] << "\n";
	}
	return oss;
}
std::ostream& operator<<(std::ostream& oss,const qram::measurement& ms)
{
	for(std::size_t i=0;i<ms.state.size();i++)
	{
		print_binary(oss,i,ms.num_bits);
		oss << " : " << ms.state[i] << "\n";
	}
	return oss;
}
