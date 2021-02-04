#include "QMachine.h"
#include<cstdlib>
#define __CL_ENABLE_EXCEPTIONS 
#include<CL/cl.hpp>
#include<string>
#include<algorithm>
#include<stdexcept>
#include<iterator>
#include "QGate.h"
#include "StandardGates.h"

QMachine::QMachine(std::size_t nb,bool deb):
	num_bits(nb),
	num_states(1 << nb),
	debug(deb),
	lockptr(nullptr)
{
	std::vector<cl::Platform> platforms;
	cl::Platform::get(&platforms);
	cl_context_properties properties[] = 
	{ 
		CL_CONTEXT_PLATFORM, (cl_context_properties)(platforms[0])(), 0
	};
	context=cl::Context(CL_DEVICE_TYPE_GPU, properties); 
	
	devices = context.getInfo<CL_CONTEXT_DEVICES>();
	
	queue=cl::CommandQueue(context, devices[0],0);
	
	//Add debug mode later..
	if(debug)
	{
		//TODO
	}
	std::size_t bufsize=num_states*sizeof(float)*2;//complex std::float
	//TODO: if(bufsize*2 > device max, thrwo exception
	
	for(int i=0;i<2;i++)
	{
		//two buffers
		buf[i]=cl::Buffer(context,CL_MEM_READ_WRITE,bufsize);
	}
	prev=&buf[0];
	next=&buf[1];
	
	float* writebuf=reinterpret_cast<float*>(queue.enqueueMapBuffer(*prev,true,CL_MAP_WRITE,0,bufsize));
	std::fill(writebuf,writebuf+num_states*2,0.0f);
	writebuf[0]=1.0f;
	
	queue.enqueueUnmapMemObject(*prev,writebuf,NULL,&prevevent);		
	permutation_gate.reset(new PERMUTE(*this,permutation_t(),num_bits));
}

void QMachine::run_gate_kernel(cl::Kernel& kern,cl_ulong regspec)
{
	if(lockptr)
		throw std::runtime_error("Cannot run any kernels, machine is currently locked for reading");
	
	kern.setArg(0,*next);
	kern.setArg(1,*prev);
	kern.setArg(2,regspec);
	
	std::vector<cl::Event> peventv(&prevevent,&prevevent+1);
	queue.enqueueNDRangeKernel(kern,cl::NullRange,cl::NDRange(num_states),cl::NullRange,&peventv,&prevevent);	
	std::swap(prev,next);
}

const std::complex<float>* QMachine::lock()
{
	if(!lockptr)
	{
		std::vector<cl::Event> peventv(&prevevent,&prevevent+1);
		std::size_t bufsize=num_states*sizeof(float)*2;//complex std::float
		lockptr=reinterpret_cast<std::complex<float>* >(queue.enqueueMapBuffer(*prev,true,CL_MAP_WRITE,0,bufsize,&peventv,&prevevent));
	}
	return lockptr;
	
}

void QMachine::unlock()
{
	if(lockptr)
	{
		std::vector<cl::Event> peventv(&prevevent,&prevevent+1);
		queue.enqueueUnmapMemObject(*prev,lockptr,&peventv,&prevevent);
		lockptr=nullptr;
	}
}	

std::ostream& operator<<(std::ostream& out,QMachine& qm)
{
	const std::complex<float>* state=qm.lock();
	copy(state,state+qm.num_states,std::ostream_iterator<std::complex<float> >(out));
	qm.unlock();
	return out;
}
