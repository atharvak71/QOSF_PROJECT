#include<cstdlib>
#include<complex>
#include<vector>
#include<cstdint>
#include<iostream>

class QMachine
{
//protected:
	std::vector<std::complex<double> > state1;
	std::vector<std::complex<double> > state2;
	
	bool flipped;
	const std::uint_fast8_t num_bits;
	const std::size_t N;
public:	
	QMachine(const std::uint_fast8_t bits,const std::size_t initialstate=0):
		state1(1 << bits,std::complex<double>(0.0,0.0) ),
		state2(1 << bits,std::complex<double>(0.0,0.0) ),
		flipped(false),
		num_bits(bits),
		N(1 << bits)
	{
		state1[initialstate]=1.0;
	}
	
	template<class ClassicalOp>  //T is a callable that takes in a register and returns a register
	void classical_op(ClassicalOp op)
	{
		const std::vector<std::complex<double> >& inputvec=(flipped ? state2 : state1);
		std::vector<std::complex<double> >& outputvec=(flipped ? state1 : state2);
		
		for(std::size_t input=0;input<N;++input)
		{
			std::size_t output=op(input);
			outputvec[output]=inputvec[input];
		}
		flipped=!flipped;
	}
};

std::size_t cnot(const std::size_t& input)
{
	//assume input[1] is control
	//assume input[0] is target
	return (input & 0x2) ? input ^ 0x1 : input;
	
}

std::size_t toffoli(const std::size_t& input)
{
	//assume input[1],input[2] are control
	//assume input[0] is target
	return (input & 0x4) && (input & 0x2) ? input ^ 0x1 : input;
}

template<std::size_t N>
std::size_t gtoffoli(const std::size_t& input)
{
	//top N-1 bits are control
	//last bit is 
	std::size_t check=1;
	for(std::size_t i=0;i<N;i++)
	{
		input>>=1;
		check&=input;
	}
	return (check & 0x1) ? input ^ 0x1 : input; 
}

template<std::size_t N,std::size_t c>
std::size_t addi(const std::size_t& input)
{
	return (input+c) & (1 << N);
}

template<std::size_t N,
int main()
{
	for(int i=0;i<4;i++)
	{
		//QMachine qm(2,i);
		//qm.classical_op(cnot);
		std::cout << cnot(i) << std::endl;
	}	
	return 0;
}
