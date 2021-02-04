#include<iostream>
#include "../qram.h"
#include "../qgates.h"
using namespace std;
using namespace qcpp;

class cpgate:public qgate
{
public:
	cpgate(const double& theta,const std::string& s):
		qgate(1,s),
		a(cos(theta),sin(theta))
	{
	}
	
	virtual void apply(std::complex<double>* out,const std::complex<double>* in) const
	{
		//first bit is control bit, second bit is phasor
		out[0]=in[0];
		out[1]=a*in[1];
	}
	std::complex<double> a;
};


void p1()
{
	const int xbit=1;
	const int ybit=2;
	const int zbit=4;
	
	cpgate T(M_PI/4.0,"T");
	cpgate Tconj(-M_PI/4.0,"Tconj");
	cpgate S(M_PI/2.0,"S");
	qhadamard H;
	
	for(int i=0;i<8;i++)
	{
		cout << "Is now encoding " << i << endl;
		qram r(3,std::clog,i);
		r.op(H,xbit);
		cout << 1 << r << endl;
		r.op(qcnot(),xbit | ybit);
		cout << 2 << r << endl;
		r.op(Tconj,xbit);
		cout << 3 << r << endl;
		r.op(qcnot(),xbit | zbit);
		cout << 4 << r << endl;
		r.op(T,xbit);
		cout << 5 << r << endl;
		r.op(qcnot(),xbit | ybit);
		cout << 6 << r << endl;
		r.op(Tconj,xbit);
		cout << 7 << r << endl;
		r.op(qcnot(),xbit | zbit);
		cout << 8 << r << endl;
		r.op(Tconj,ybit);
		cout << 9 << r << endl;
		r.op(T,xbit);
		cout << 10 << r << endl;
		r.op(qcnot(),ybit | zbit);
		cout << 11 << r << endl;
		r.op(Tconj,ybit);
		cout << 12 << r << endl;
		r.op(qcnot(),ybit | zbit);
		cout << 13 << r << endl;
		r.op(T,zbit);
		cout << 14 << r << endl;
		r.op(S,ybit);
		cout << 15 << r  << endl;
		r.op(H,xbit);
		cout << r << endl;
	}
}



int main(int,char**)
{
//	p1a();
//	p1b();
	p1();
	return 0;
}
