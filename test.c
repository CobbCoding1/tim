#include <stdio.h>

double power(double base, double num2){
    double result = 1;
    for(int i = 0; i < (int)num2; i++){
        result *= base; 
    }
    return result;
}

int main(){
    printf("%f\n", power(2.3, 10.5));
}
