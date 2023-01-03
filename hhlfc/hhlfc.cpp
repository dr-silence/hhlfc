#include <fstream>
#include <iostream>
#include <iterator>
#include <optional>
#include <mutex>
#include <list>
#include <map>
#include <functional>
#include <thread>

// todo: Использовать boost::synchronized_value вместо явного использования мьютекса?
template<class I, class M, class OI, class OM, class Consumer>
void ProcessData(I& next, I end, M& mutex, OI& out, OM& outMutex, Consumer consumeData)
{
	while (true)
	{
		I current;
		{
			std::lock_guard lock(mutex);
			current = next;

			if (next != end)
				next++;
		}

		{
			std::lock_guard lock(outMutex);
			if (current != end)
				*out++ = *current;
		}

		if (current == end) break;

		consumeData(*current);
	}
}

void DelayThisThread()
{
#if false
	std::this_thread::sleep_for(std::chrono::milliseconds(1));
#elif false
	std::this_thread::yield();
#endif
};

using number_value_type = int;
using number_position_type = int;

using prime_numbers = std::map<number_position_type, number_value_type>;

int main()
{
	prime_numbers primeNumbers;
	{
		std::list<number_value_type> numbers;
		[&numbers]() {
			std::fstream numbersFile("Input.txt", std::ios_base::in);

			int number;
			while (numbersFile >> number)
				numbers.push_back(number);
		}();

		numbers.sort();
		{
			number_position_type position{ 1 };

			std::transform(numbers.begin(), numbers.end(), std::inserter(primeNumbers, primeNumbers.end()), [&position](number_value_type number) {return std::make_pair(position++, number); });
		}
	}

	auto begin = primeNumbers.begin();
	std::mutex mutex;

	prime_numbers data1, data2;

	auto consumeData = [](auto& data, auto value) { data.insert(value); DelayThisThread(); };

	using consume_data = std::function<void(prime_numbers::value_type)>;
	consume_data consumeData1 = std::bind(consumeData, std::ref(data1), std::placeholders::_1);
	consume_data consumeData2 = std::bind(consumeData, std::ref(data2), std::placeholders::_1);

	prime_numbers result;
	auto resultInserter = std::inserter(result, result.end());
	std::mutex resultMutex;

	// todo: const_iterator?
	auto processData = std::bind(ProcessData<decltype(begin), decltype(mutex), decltype(resultInserter), decltype(resultMutex), consume_data>, std::ref(begin), primeNumbers.end(), std::ref(mutex), std::ref(resultInserter), std::ref(resultMutex), std::placeholders::_1);

	auto processData1 = std::bind(processData, consumeData1);
	auto processData2 = std::bind(processData, consumeData2);

	std::thread thread1(processData1), thread2(processData2);

	thread1.join();
	thread2.join();

	auto writePrimesToFile = [](prime_numbers const& primes, auto os)
	{
		std::ostream_iterator<number_value_type>	outputIt(os, " ");
		std::transform(std::begin(primes), std::end(primes), outputIt, [](auto p) { return p.second; });
	};

	writePrimesToFile(data1, std::ofstream("Thread1.txt"));
	writePrimesToFile(data2, std::ofstream("Thread2.txt"));

	writePrimesToFile(result, std::ofstream("Result.txt"));

	return 0;
}
