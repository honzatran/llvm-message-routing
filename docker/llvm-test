# Taken from JitFromScratch repository 
# https://github.com/weliveindetail/JitFromScratch/tree/master/docker
#
#
FROM ubuntu:18.04

# Install tools for apt-add-repository
RUN apt-get update && \
        apt-get install -y --no-install-recommends \
        ca-certificates wget gnupg software-properties-common

# Add LLVM apt repository
RUN wget https://apt.llvm.org/llvm-snapshot.gpg.key && \
    apt-key add llvm-snapshot.gpg.key && \
    apt-add-repository "deb http://apt.llvm.org/bionic/ llvm-toolchain-bionic-8 main" && \
    apt-get update

# Install tools and libs for JitFromScratch build
RUN apt-get install -y --no-install-recommends \
    build-essential git cmake cmake-data \
    ninja-build clang zlib1g-dev 


RUN git clone --depth=1 --branch=release/8.x https://github.com/llvm/llvm-project.git llvm-release80
RUN mkdir llvm-release80-release && \
    cd llvm-release80-release && \
        CC=clang CXX=clang++ \
                cmake -GNinja -DCMAKE_BUILD_TYPE=Release -DLLVM_TARGETS_TO_BUILD=host -DLLVM_INCLUDE_TESTS=OFF \
                       -DLLVM_INCLUDE_BENCHMARKS=OFF ../llvm-release80/llvm

RUN cd llvm-release80-release && ninja llvm-config FileCheck
RUN cd llvm-release80-release && ninja LLVMSupport LLVMDemangle LLVMCore LLVMBinaryFormat
RUN cd llvm-release80-release && ninja LLVMRuntimeDyld LLVMObject LLVMMCParser LLVMBitReader LLVMMC LLVMDebugInfoCodeView LLVMDebugInfoMSF
RUN cd llvm-release80-release && ninja LLVMExecutionEngine LLVMTarget LLVMAnalysis LLVMProfileData LLVMOrcJIT LLVMTransformUtils
RUN cd llvm-release80-release && ninja LLVMGlobalISel LLVMSelectionDAG LLVMScalarOpts LLVMInstCombine LLVMAggressiveInstCombine
RUN cd llvm-release80-release && ninja LLVMAsmPrinter LLVMCodeGen LLVMMCDisassembler LLVMBitWriter LLVMBinaryFormat
RUN cd llvm-release80-release && ninja LLVMX86AsmPrinter LLVMX86CodeGen LLVMX86Disassembler LLVMX86AsmParser LLVMX86Desc LLVMX86Info LLVMX86Utils
RUN cd llvm-release80-release && ninja LLVMipo LLVMInstrumentation LLVMVectorize LLVMLinker LLVMIRReader LLVMAsmParser

# taken from teeks99/boost-cpp-docker
RUN add-apt-repository -y ppa:git-core/ppa \
#Install pre-reqs
     && apt-get update \
     && apt-get install -y \
       ca-certificates \
       build-essential \
       wget \
       python \
       python2.7 \
       python2.7-dev \
       python-numpy \
       python3 \
       python3-dev \
       python3-numpy \
       libjpeg-dev \
       libpng-dev \
       libssl-dev \
       libtiff-dev \
       libbz2-dev \
       liblzma-dev \
       libicu-dev \
       libopenmpi-dev \
       zlib1g-dev \
       # Speed up fetch
       && git config --global submodule.fetchJobs 40



# install boost 1.70.0
RUN wget --max-redirect 3 https://dl.bintray.com/boostorg/release/1.70.0/source/boost_1_70_0.tar.gz
RUN mkdir /tmp/boost && tar zxf boost_1_70_0.tar.gz -C /tmp/boost --strip-component 1
ADD ./docker/project-config.jam /tmp/project-config.jam

RUN cd /tmp/boost/ \
    && ./bootstrap.sh \
    && cp -f /tmp/project-config.jam . \
    && echo "Start compilation" \
    && ./b2 install --prefix=/usr/local -a \
    && echo "All done"
    
    
