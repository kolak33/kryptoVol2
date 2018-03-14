#include <iostream>
#include <stdlib.h>
#include <cstdio>
#include <vector>
#include <ctime>

#define MAX 400
#define seed 1



int main()
{
  srand(time(NULL));
/*int r[MAX];
int i;
r[0] = seed;

for (i = 1; i<31; i++) {
    r[i] = (16807LL * r[i - 1]) % 2147483647;
    if (r[i] < 0) {
       r[i] += 2147483647;
    }
}

for (i = 31; i<34; i++) {
    r[i] = r[i - 31];
}

for (i = 34; i<344; i++) {
    r[i] = r[i - 31] + r[i - 3];
} 

for (i = 344; i<MAX; i++) {
    r[i] = r[i - 31] + r[i - 3];
    printf("%u =?= %u\n", ((unsigned int)r[i]) >> 1, random());
} */


std::vector<int> randOutputs;
for(int i=0; i<33; ++i)
	randOutputs.push_back(random());

int iPredicted = 0;
int iTestsCount = 500;

for(int i=0; i<iTestsCount; ++i)
{
	int iPrediction = randOutputs[randOutputs.size() - 31] + randOutputs[randOutputs.size() - 3];
	iPrediction = iPrediction & ((1u << 31) - 1);
	
	int iGenerated = random();
	std::cout << "prediction: "<< iPrediction << ", rand:" << iGenerated << std::endl; 
	if(iPrediction == iGenerated)
		++iPredicted;
	randOutputs.push_back(iGenerated);
}

std::cout << "predicted: " << iPredicted << " / " << iTestsCount << std::endl;
std::cout << "predicted: " << ((float)iPredicted/iTestsCount) * 100.0 << "%\n";


return 0;
}
