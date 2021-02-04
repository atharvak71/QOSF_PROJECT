#include<iostream>
#include "../qram.h"
#include "../qgates.h"
using namespace std;
using namespace qcpp;

typedef bool (*FUNCPTR)(const std::size_t&);
//lowest bit is y
//all other bits are x
class Uf:public qgate
{
public:
	Uf(const FUNCPTR& ft,int nxb):qgate(nxb+1,"Uf"),
		f(ft),
		nxbits(nxb)
	{
	}
	virtual void apply(std::complex<double>* out,const std::complex<double>* in) const
	{
		std::size_t mx=1;
		mx<<=nxbits+1;
		for(std::size_t i=0;i<mx;i++)
		{
			std::size_t x=i>>1;
			bool y=(i & 1) == 1;
			bool fx=f(x);
			std::size_t ni=(i & (~(std::size_t)1));
			if(fx ^ y)
				ni++;
			out[ni]=in[i];
		}
	}
protected:
	FUNCPTR f;
	int nxbits;
};

bool sim_deutch_jozsa(const FUNCPTR& f,int num_bits)
{
	std::size_t ybit=1;
	std::size_t xbits=(1 << (num_bits+1))-2;
	
	qram r(num_bits+1,std::clog);
	
	qhadamard H;
	r.op(qnot(),ybit);//not the y bit
	
	for(int b=0;b<(num_bits+1);b++)
	{
		r.op(H,1 << b); //Apply a hadamard gate to the entire system
	}
	
	Uf uf(f,num_bits);
	r.op(uf,xbits | ybit);
	
	for(int b=0;b<(num_bits+1);b++)
	{
		r.op(H,1 << b); //Apply a hadamard gate to entire system.
	}

	qram::measurement m=r.measure(xbits);
	return m==0;
}

bool constant(const std::size_t& x)
{
	return true;
}

bool not_constant(const std::size_t& x)
{
	return (x & 1) == 0;
}

int main(int argc,char**)
{
	if(sim_deutch_jozsa(&not_constant,23))
		cout << "The function is a constant function" << endl;
	else
		cout << "The function is NOT a constant function" << endl;
	
	return 0;

}
