#ifndef __BOUNDED_QUEUE_HPP__
#define __BOUNDED_QUEUE_HPP__

#include <condition_variable>
#include <mutex>
#include <optional>
#include <queue>

template<typename T> class Bounded_queue {
public:
	explicit Bounded_queue(size_t capacity): _capacity{capacity}, _closed{false} {}
	Bounded_queue& operator=(Bounded_queue&) = delete;
	Bounded_queue(Bounded_queue&&) = delete;
	Bounded_queue& operator=(const Bounded_queue&) = delete;
	Bounded_queue& operator=(Bounded_queue&&) = delete;

	void push(const T& value)
	{
		std::unique_lock<std::mutex> lock{this->_mut};

		// Wait until the queue is no longer full.
		this->_not_full.wait(lock, [this] { return this->_data.size() < this->_capacity; });
		this->_data.push(value);
		lock.unlock();

		this->_not_empty_or_closed.notify_one(); // Notify a waiting consumer.
	}

	std::optional<T> pop()
	{
		std::unique_lock<std::mutex> lock(this->_mut);

		// Wait until the queue is no longer empty or is closed.
		this->_not_empty_or_closed.wait(lock, [this] { return !this->_data.empty() || _closed; });

		// if the queue is empty and is closed then there will be no more data
		if (this->_data.empty() && _closed) {
			return std::nullopt;
		}

		T value = this->_data.front();
		this->_data.pop();
		lock.unlock();

		this->_not_full.notify_one(); // Notify the producer.
		return value;
	}

	void close()
	{
		{
			std::lock_guard lock{_mut};
			_closed = true;
		}
		_not_empty_or_closed.notify_all();
	}

private:
	std::queue<T> _data;
	size_t _capacity;
	std::mutex _mut;
	std::condition_variable _not_empty_or_closed;
	std::condition_variable _not_full;
	bool _closed;
};

#endif // __BOUNDED_QUEUE_HPP__