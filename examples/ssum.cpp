#include "../qram.h"
#include "../qgates.h"
#include <iostream>
using namespace std;
using namespace qcpp;

template<unsigned as>
class qadder: public qgate
{
public:
	qadder():qgate(2*as,"add")
	{
	}
	virtual void apply(std::complex<double>* out,const std::complex<double>* in) const
	{
		const std::size_t sz=1 << (2*as);
		const std::size_t lmask=(1 << as)-1;
		for(std::size_t i=0;i < sz;i++)
		{
			std::size_t y=(i >> as) & lmask;
			std::size_t z=i & lmask;
			z=(z+y) & lmask;
			z|=(y << as);
			
			out[z]=in[i];
		}
	}
};

// x
//00->random
//01->random
//10->random
//11->random


void load4(qram& r,unsigned int rloc,unsigned int val)
{
	for(unsigned int i=0;i<4;i++)
	{
		unsigned int msk=1 << i;
		if(val & msk)
		{
			r.op(qnot(),msk << (rloc*4));
		}
	}
}

int main(int,char**)
{
	qram r(16,std::clog);
	load4(r,0,3);
	load4(r,1,5);
	load4(r,2,4);
	load4(r,3,1);
	
	//cout << r << endl;
	r.op(qadder<4>(),0xFF);
	cout << r << endl;
	/*
	load4(r,1,5);
	r.op(qadder<4>(),0xFF);
	cout << r.measure(0xF) << endl;*/
	return 0;
}
