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
