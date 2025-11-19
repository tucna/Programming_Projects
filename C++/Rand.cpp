#include <iostream>
#include <random>
#include <map>
#include <iomanip>
#include <cmath>         // for std::round, std::clamp, std::max
#include <type_traits>   // for std::is_integral_v

// Template function to fill the map with counts, handling both int and double distributions
template<typename Dist>
void fillMap(Dist& dist, std::mt19937& gen, int numSamples, std::map<int, int>& counts)
{
  for (int i = 0; i < numSamples; ++i)
  {
    auto val = dist(gen);
    int ival;
    if constexpr (std::is_integral_v<decltype(val)>)
    {
      ival = val;
    }
    else
    {
      ival = static_cast<int>(std::round(val));
    }

    ival = std::clamp(ival, 0, 100);
    counts[ival]++;
  }
}

// Function to print a simple text histogram for the map
void printHistogram(const std::map<int, int>& counts, int totalSamples)
{
  if (counts.empty()) return;

  int max_count = 0;
  for (const auto& pair : counts)
    max_count = std::max(max_count, pair.second);

  std::cout << "Histogram (values clamped/rounded to [0,100]):\n";

  for (const auto& pair : counts) 
  {
    int stars = (max_count > 0) ? static_cast<int>(std::round(static_cast<double>(pair.second) / max_count * 50.0)) : 0;
    std::cout << std::setw(3) << pair.first << ": " << std::string(stars, '*') << " (" << pair.second << ")\n";
  }

  std::cout << std::endl;
}

int main()
{
  std::random_device rd;
  std::mt19937 gen(rd()); // Mersenne Twister generator seeded with random_device

  const int numSamples = 100000; // Number of samples for each distribution

  // 1. Uniform Integer Distribution (0 to 100)
  std::uniform_int_distribution<int> uniformInt(0, 100);
  std::map<int, int> uniformIntCounts;
  fillMap(uniformInt, gen, numSamples, uniformIntCounts);
  std::cout << "Uniform Integer Distribution (0-100):\n";
  printHistogram(uniformIntCounts, numSamples);

  // 2. Bernoulli Distribution (p=0.5, results in 0 or 1)
  std::bernoulli_distribution bernoulli(0.5);
  std::map<int, int> bernoulliCounts;
  fillMap(bernoulli, gen, numSamples, bernoulliCounts);
  std::cout << "Bernoulli Distribution (p=0.5):\n";
  printHistogram(bernoulliCounts, numSamples);

  // 3. Binomial Distribution (n=100, p=0.5, mean=50)
  std::binomial_distribution<int> binomial(100, 0.5);
  std::map<int, int> binomialCounts;
  fillMap(binomial, gen, numSamples, binomialCounts);
  std::cout << "Binomial Distribution (n=100, p=0.5):\n";
  printHistogram(binomialCounts, numSamples);

  // 4. Poisson Distribution (lambda=20, mean=20)
  std::poisson_distribution<int> poisson(20.0);
  std::map<int, int> poissonCounts;
  fillMap(poisson, gen, numSamples, poissonCounts);
  std::cout << "Poisson Distribution (lambda=20):\n";
  printHistogram(poissonCounts, numSamples);

  // 5. Exponential Distribution (lambda=0.02, mean=50, rounded to int)
  std::exponential_distribution<double> exponential(0.02);
  std::map<int, int> exponentialCounts;
  fillMap(exponential, gen, numSamples, exponentialCounts);
  std::cout << "Exponential Distribution (lambda=0.02, mean=50):\n";
  printHistogram(exponentialCounts, numSamples);

  // 6. Normal Distribution (mean=50, stddev=15, rounded to int)
  std::normal_distribution<double> normal(50.0, 15.0);
  std::map<int, int> normalCounts;
  fillMap(normal, gen, numSamples, normalCounts);
  std::cout << "Normal Distribution (mean=50, stddev=15, rounded):\n";
  printHistogram(normalCounts, numSamples);

  return 0;
}