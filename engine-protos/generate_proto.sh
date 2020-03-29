
#!/bin/bash

../build/bin/protoc --grpc_out=src --plugin=protoc-gen-grpc=`which grpc_cpp_plugin` --cpp_out=src -I. *.proto

