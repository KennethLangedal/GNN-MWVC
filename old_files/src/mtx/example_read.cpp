#include <fstream>
#include <iostream>
#include <string>

extern "C" int mm_read_mtx_crd_size(FILE *f, int *M, int *N, int *nz );
using namespace std;

int main(int argc, char * argv[]) {

    if (argc != 2) {
        cout << "Usage: example_read_mtx_using_library [graph]" << endl;
        return 0;
    }

    printf("ARGC: %i \n", argc);
    printf("ARGV: %s \n", argv[1]);
    printf("THIS IS A TEST \n");

    FILE *file;
    int ret_code;
    int M, N, nz; 

    string graph_path = argv[1];
    file = fopen(graph_path.c_str(), "r");


    if ((ret_code = mm_read_mtx_crd_size(file, &M, &N, &nz)) !=0)
        exit(1);

    printf("M %i  \n", M); 
    printf("N %i  \n", N); 
    printf("nz %i \n", nz); 


}