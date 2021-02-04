#include"QGate.h"
#include"StandardGates.h"
#include"QMachine.h"
#include<complex>
#include<random>
#include<algorithm>
#include<cmath>
#include<limits>
#include<iostream>
using namespace std;

template<class T>
class StableAccumulator
{
public:
	std::vector<T> classes;
	
	StableAccumulator():
		classes(std::numeric_limits<T>::max_exponent-std::numeric_limits<T>::min_exponent,0.0)
	{}
	
	void insert(const T& tin)
	{
		int inclass;
		frexp(tin,&inclass);
		inclass-=std::numeric_limits<T>::min_exponent;
		
		float Tnew=classes[inclass]+tin;
		
		int repclass;
		frexp(Tnew,&repclass);
		repclass-=std::numeric_limits<T>::min_exponent;
		
		if(repclass !=  inclass)
		{
			classes[inclass]=0.0;
			insert(Tnew);
		}
		else
		{
			classes[inclass]=Tnew;
		}
	}
	
	operator T() const
	{
		return std::accumulate(classes.begin(),classes.end(),T(0.0));
	}
};

//wavefunction collapse
static std::vector<std::size_t> discrete_real_samples_collapse(const std::complex<float>* pdf, std::size_t n, std::size_t num_samples)
{
	// Select num_samples random samples from the count distribution, relative the numerical precision of the count.  i.e. 10.
	
	
	std::random_device rd;
	std::mt19937_64 mt(rd());
	std::uniform_real_distribution<float> rdist(0,1);
	std::vector<float> samplecnts(num_samples);
	for(int i = 0; i < num_samples; i++)
	{
		samplecnts[i]=rdist(mt);
	}
	sort(samplecnts.begin(), samplecnts.end());

	// Map the random count samples to its bin
	std::vector<std::size_t> samples;
	samples.reserve(num_samples);
	
	StableAccumulator<float> accum;
	std::vector<float>::const_iterator sampleiter = samplecnts.begin();
	for(std::size_t sample=0; sample < n && (sampleiter != samplecnts.end()); sample++)
	{
		float a=std::abs(pdf[sample]);
		accum.insert(a*a);
		float distsum=accum;
		// Whenever the current sum of the counts exceeds a count sample, map to that bin.
		for(; (distsum > (*sampleiter)) && (sampleiter != samplecnts.end()); sampleiter++)
		{
			samples.push_back(sample);
		}
	}
	return samples;
}


MEASUREMENT::MEASUREMENT(QMachine& m,const std::size_t& bu):
		QGate(m,bu,MEASUREMENT::tsrc,MEASUREMENT::gatesrc),
		msbshift(m,bu),
		mresult(0),
		result(mresult)

	{}

void MEASUREMENT::apply(const cl_ulong& regspec)
{
	//shift the measurement bits to the msb...this is so the state counter can work over the state sum...even withing the mask
	size_t bits_used=regspec & REG_MAX_MASK;
	msbshift(permutation_t(),bits_used);
	
	const std::complex<float>* states=qm.lock();
	
	//implement sarah's O(n) time O(1) space quartile function implementation
	
	std::vector<std::size_t> samp=discrete_real_samples_collapse(states,qm.num_states,1);
	
	//We don't care about the lsb..because they are not relevant..they don't represent possible states..this is a clever trick using the histogram to avoid re-binning.
	//the lsb states get clamped to a single bin for the msb.
	//we only care about the msb for the result.
	mresult=samp[0] >> (qm.num_bits-bits_used); 
	

	//this is the probability inside the bin.
	StableAccumulator<float> Px;
	
	//the probability is going to be the sum of all the sub-bins inside the msb category...because we did the lsb->msb translation at the start
	//then this is just a linear sum over the sub-section of the array
	const std::size_t result_begin=mresult << (qm.num_bits-bits_used);
	const std::size_t result_end=(mresult + 1) << (qm.num_bits-bits_used);
	
	//find the sum over the relevant parts; this is the probability of only the parts
	//that remain.  Do this on the CPU for simplicity and stability.
	
	for(std::size_t i=result_begin;i!=result_end;i++)
	{
		float a=std::abs(states[i]);
		Px.insert(a*a);
	}
	
	float norm_factor=Px;
	
	qm.unlock();
	
	//if the msb of the index is true, then the sample falls into the bin chosen by the quartile algorithm...
	//that means it should be normalized.
	//otherwise, its not a possible result, under the measurement, so set this probability to 0.
	//this is entanglement, really.
	//which is what the kernel does
	
	uint64_t mask=((1<<bits_used)-1) << (qm.num_bits - bits_used);
	uint64_t rbegin=result_begin;
	
	this->arg(0,mask);
	this->arg(1,rbegin);
	this->arg(2,1.0f/sqrt(norm_factor));
	
	QGate::apply(qm.num_bits);
		
	msbshift(permutation_t(),-bits_used);		//unshift to msb
}

std::string MEASUREMENT::gatesrc="";
std::string MEASUREMENT::tsrc=R"CLSRC(
__kernel void gateapply(__global float2* outbits,const __global float2* inbits,const ulong regspec,const ulong mask,const ulong resultmask,const float Px)
{
	const ulong inp=get_global_id(0);
	float f=((inp & mask)==resultmask) ? Px : 0.0;
	outbits[inp]=inbits[inp]*f;
}
)CLSRC";
