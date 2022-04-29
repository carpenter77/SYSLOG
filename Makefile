config-build:
	g++ -std=c++11 -I./thirdlib/yaml-cpp/include ./*.h ./*.cc ./tests/test_config.cc -L./thirdlib/yaml-cpp/lib -lyaml-cpp -o test_config 
