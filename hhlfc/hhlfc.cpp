#include <iostream>
#include <iterator>
#include <optional>
#include <mutex>
#include <list>

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

template<class T>
void ConsumeData(T const& data)
{

}


int main()
{
	std::list<int> numbers;
	numbers.push_back(2); numbers.push_back(3);

	auto begin = numbers.begin();
	std::mutex mutex;

	ProcessData(begin, numbers.end(), mutex, ConsumeData<decltype(numbers)::value_type>);

	return 0;
}
