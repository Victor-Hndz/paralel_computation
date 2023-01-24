#include <stdio.h>


double sqroot(double n)
{
    // Max and min are used to take into account numbers less than 1
    double lo = 1, hi = n, mid;

    // Update the bounds to be off the target by a factor of 10
    while(100 * lo * lo < n) lo *= 10;
    while(0.01 * hi * hi > n) hi *= 0.1;

    for(int i = 0 ; i < 100 ; i++){
        mid = (lo+hi)/2;
        if(mid*mid == n) return mid;
        if(mid*mid > n) hi = mid;
        else lo = mid;
    }
    return mid;
}

int main() 
{

    double n=1;
    printf("%f\n",sqroot(n));
    printf("Hello, World!");
    return 0;
}