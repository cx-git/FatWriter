# FatWriter

FatWriter provides a multi-thread-safe user interface method for c style file stream writing:

virtual void printf(const char * format, ...) = 0;

FatWriter is designed for the following scenes：
1. Unlimited output files
2. Limited file stream
3. Thread thrifty

FatWriter's implement is with the following points:
1. Adaptive buffering
2. file stream pool management
3. Dynamic flushing speed
