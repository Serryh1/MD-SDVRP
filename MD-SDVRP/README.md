### Environment

1. C++ 17;
2. cmake VERSION 3.5.0

### Run instructions
Create build folder

	mkdir build
Then use the following commands to generate the executable:

	cd build
	cmake .. 
	make 
Finally, execute the program using the following command, which requires three arguments:

	./program_name input_file_path output_file_path algorithm_name

an example is:

	cd ./build
	./MDSDVRP "../../dataset/G_set/g1.txt" "../Result/G_set/g1.txt" "Match" 

Please notice that, in our code, **Match** corresponds to **ALG1+** in the paper, while **GW** corresponds to **ALG2**.
