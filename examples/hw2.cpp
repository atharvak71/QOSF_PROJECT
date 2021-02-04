#include<iostream>
#include "../qram.h"
#include "../qgates.h"
using namespace std;
using namespace qcpp;

class cphasor:public qgate
{
public:
	cphasor(const double& theta):
		qgate(2,"Controlled Rotate-"),
		a(cos(theta),sin(theta))
	{
		
	}
	
	virtual void apply(std::complex<double>* out,const std::complex<double>* in) const
	{
		//first bit is control bit, second bit is phasor
		out[0]=in[0];
		out[1]=a*in[1];
		out[2]=in[2];
		out[3]=a*in[3];
	}
	std::complex<double> a;
};

void p1a()
{
	const int xbit=1;
	const int ybit=2;
	
	double phi=M_PI/8.0;
	
	qram r(2,std::clog);
	r.op(qhadamard(),xbit);
	r.op(cphasor(phi),xbit | ybit);
	r.op(qhadamard(),xbit);
	qram::measurement m=r.measure(xbit);
	
	//cout << "P1a:\n" << m << endl;
	cout<< r << endl;
}

void p1b()
{
	const int xbit=1;
	const int ybit=2;
	
	double phi=0.0;
	
	qram r(2,std::clog);
	r.op(qhadamard(),xbit);
	r.op(cphasor(phi),xbit | ybit);
	r.op(qshift(3.0*M_PI/2.0),xbit);
	r.op(qhadamard(),xbit);
	qram::measurement m=r.measure(xbit);
	
	cout << "P1b:\n" << m << endl;
}

void p2()
{
	const int xbit=1;
	const int y1bit=2;
	const int y2bit=4;
	
	qram r(3,std::clog);
	//r.op(qnot(),y1bit);
	clog << r;
	r.op(qnot(),y2bit);
	clog << r;
	
	r.op(qhadamard(),xbit);
	clog << r;
	r.op(qcswap(),xbit | y1bit | y2bit);
	clog << r;
	r.op(qhadamard(),xbit);
	clog << r;
	qram::measurement m=r.measure(xbit);
	
	clog << "P2:\n" << m << endl;
}


int main(int,char**)
{
//	p1a();
//	p1b();
	p2();
	return 0;
}
