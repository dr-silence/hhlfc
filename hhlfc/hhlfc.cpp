#include <fstream>
#include <iostream>
#include <iterator>
#include <optional>
#include <mutex>
#include <list>
#include <functional>
#include <thread>

// todo: Использовать boost::synchronized_value вместо явного использования мьютекса?
template<class I, class OI, class M, class Consumer>
void ProcessData(I& next, I end, OI& out, M& mutex, Consumer consumeData)
{
	using value_type = typename std::iterator_traits<I>::value_type;

	while (true)
	{
		std::optional<value_type> value;
		{
			std::lock_guard lock(mutex);
			if (next != end)
			{
				*out++ = *next;
				value = *next++;
			}
		}

		if (!value) break;

		consumeData(*value);
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

// todo: Реализовать класс представления простого числа: порядковый номер простого числа, значение простого числа.
using data_type = int;

int main()
{
	std::list<data_type> numbers;
	[&numbers]() {
		std::fstream numbersFile("Input.txt", std::ios_base::in);

		int number;
		while (numbersFile >> number)
			numbers.push_back(number);
	}();

	auto begin = numbers.begin();
	std::mutex mutex;

	std::list<data_type> data1, data2;

	auto consumeData = [](auto& data, auto value) { data.push_back(value); DelayThisThread(); };

	using consume_data = std::function<void(data_type)>;
	consume_data consumeData1 = std::bind(consumeData, std::ref(data1), std::placeholders::_1);
	consume_data consumeData2 = std::bind(consumeData, std::ref(data2), std::placeholders::_1);

	std::vector<decltype(numbers)::value_type> result;
	result.reserve(numbers.size());
	auto resultInserter = std::back_inserter(result);
	// todo: const_iterator?
	auto processData = std::bind(ProcessData<decltype(begin), decltype(resultInserter), decltype(mutex), consume_data>, std::ref(begin), numbers.end(), std::ref(resultInserter), std::ref(mutex), std::placeholders::_1);

	auto processData1 = std::bind(processData, consumeData1);
	auto processData2 = std::bind(processData, consumeData2);

	std::thread thread1(processData1), thread2(processData2);

	thread1.join();
	thread2.join();

	auto printNumbers = [](auto const& name, auto& numbers)
	{
		std::cout << name << std::endl;
		int i = 0;
		std::for_each(numbers.begin(), numbers.end(), [&i](auto number) { std::cout << ((i++ > 0) ? ", " : "") << number; });
		std::cout << std::endl;
	};

	printNumbers("data1", data1);

	printNumbers("data2", data2);

	return 0;
}
