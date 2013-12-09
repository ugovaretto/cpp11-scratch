dataIn = (BYTE *) clEnqueueMapBuffer(clComQueue, bufferIn, CL_TRUE, CL_MAP_WRITE, 0,  imageW * imageH * 4, 0, NULL, NULL, &ciErr)

...

clEnqueueUnmapMemObject(clComQueue, bufferIn, imageIn, 0, NULL, NULL);


class CLByteBuffer {
    cl_command_queue cmdQueue_;
    cl_mem clbuffer_;
    size_t bufferSize_;
    ...
public:
    ...    
    template < F >
    void operator(F&& f) {
        cl_int err;
        unsigned char* data = 
                  clEnqueueMapBuffer(cmdQueue_,
                                     clbuffer_,
                                     CL_TRUE,
                                     CL_MAP_WRITE,
                                     0,
                                     bufferSize_,
                                     0, NULL, NULL, &err);
        if(err != CL_SUCCESS) throw(CLException(err));                               
        f(data);
        err = clEnqueueUnmapMemObject(cmdQueue_, clbuffer_, data, 0, NULL, NULL);
        if(err != CL_SUCCESS) throw(CLException(err));                            
    }
    ...
};
