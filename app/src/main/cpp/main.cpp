//
// Created by sarap on 8/5/2024.
//

#include <iostream>
#include <vector>
#include <string>
#include <cstdio>
#include <unistd.h>
#include <pthread.h>
#include "functions.c"

using namespace std;
extern "C" ResourceUsage*  Sara();
//ResourceUsage test(const char* process_name); // Prototip funkcije


int FunkcijaUCppKojaSePozivaIzC() {

    printf("uspeh funkcije\n\n" );
    return 0;
}
int (*func_ptr)() = FunkcijaUCppKojaSePozivaIzC;

//extern int (*func_ptr)();
int FunkcijaUC(int (*func_ptr)()) ;

//extern "C" int Sara(int argc, char* argv[]);  // Deklaracija funkcije kao extern "C"

//int Sara(int argc, char* argv[])

ResourceUsage*  Sara()
{
    /* if (argc < 2) {
         fprintf(stderr, "Usage: %s <process_name>\n", argv[0]);
         return 1;
     }

     const char* process_name = argv[1];*/
    const char* process_name="helloworld";



    //prikaziZauzetostCpu();
    // sleep(20);
    printf("\nNakon ovoga test\n");
    printf("\n\n\n");
    ResourceUsage* usage=test(process_name);
    //sleep(10);
    printf("\nKraj maina, nakon ovog while(2) \n");

    return usage;
    //while(2){sleep(12);}
}


//int (*PFunkcijaUCppKojaSePozivaIzC)() = &FunkcijaUCppKojaSePozivaIzC;
