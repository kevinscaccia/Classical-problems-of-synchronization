gcc -Wall -o teste teste.c -lpthread &&
gcc -Wall -shared -o my_library.so my_library.c -ldl -fPIC &&
LD_PRELOAD=./my_library.so ./teste
