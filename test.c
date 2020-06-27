#include<stdio.h>

unsigned char check_prime(int number){
	int divisors = 0;
	for(int i = 1; i<= number; i++){
		if(number%i==0) divisors++;
	}
	if(divisors==2) return 1;
	else return 0;
}

void calculate_primes(int upsize){
	printf("calculating number of prime numbers...\n");
	int ctr = 0;
	int quant = upsize / 25;
	int treshold = quant;

	for(int i = 0; i<upsize; i++){
		ctr += check_prime(i);
		if(i == treshold){
			treshold += quant;
			printf("X");
		}
	}

	printf("\nCalculated! There are ");
	printf("%d numbers\n",ctr);

}

int main(){
	calculate_primes(10000);
	return 0;
}
