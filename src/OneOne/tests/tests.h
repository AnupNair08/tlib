#define RED "\033[1;31m"
#define RESET "\033[0m"
#define GREEN "\e[0;32m"
#define BLUE "\033[1;34m"

#define TESTPASS \
        printf(GREEN"Test Passed\n\n"RESET); \

#define TESTFAIL \
        printf(RED"Test Failed\n\n"RESET); \

#define LINE printf("---------------------------------------------------------------------------------------------\n")

