#include <iostream>
#include <iterator>
#include <optional>
#include <mutex>
#include <list>
#include <functional>

template<class I, class M, class Consumer>
void ProcessData(I& next, I end, M& mutex, Consumer consumeData)
{
	using value_type = typename std::iterator_traits<I>::value_type;

	while(true)
	{
		std::optional<value_type> value;
		{
			std::lock_guard lock(mutex);
			if (next != end) value = *next++;
		}

		if (!value) break;

		consumeData(*value);
	}
}

template<class M, class T>
void ConsumeData(M& mutex, T const& data)
{
	std::lock_guard lock(mutex);
}


int main()
{
	std::list<int> numbers;
	numbers.push_back(2); numbers.push_back(3);

	auto begin = numbers.begin();
	std::mutex mutex;
	std::mutex consumeMutex;

	auto consumeData = std::bind(ConsumeData<decltype(consumeMutex), decltype(numbers)::value_type>, std::ref(consumeMutex), std::placeholders::_1);
	ProcessData(begin, numbers.end(), mutex, consumeData);

	return 0;
}
