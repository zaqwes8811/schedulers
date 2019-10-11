/*
 Copyright 2015 Adobe Systems Incorporated
 Distributed under the MIT License (see license at http://stlab.adobe.com/licenses.html)

 This file is intended as example code and is not production quality.
 */

#include <deque>
#include <iostream>
#include <memory>
#include <thread>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <type_traits>
#include <functional>

using namespace std;

#if 0
/**************************************************************************************************/

using lock_t = unique_lock<mutex>;

class notification_queue {
	deque<function<void()>> _q;
	bool _done { false };
	mutex _mutex;
	condition_variable _ready;

public:
	bool try_pop(function<void()>& x) {
		lock_t lock { _mutex, try_to_lock };

		if (!lock || _q.empty())
			return false;

		x = move(_q.front());
		_q.pop_front();
		return true;
	}

	template<typename F>
	bool try_push(F&& f) {
		{
			lock_t lock { _mutex, try_to_lock };
			if (!lock)
				return false;

			// About forwarding
			// http://stackoverflow.com/questions/3582001/advantages-of-using-forward
			// http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2002/n1385.htm
			_q.emplace_back(forward<F>(f));
		}
		_ready.notify_one();
		return true;
	}

	void done() {
		{
			unique_lock<mutex> lock { _mutex };
			_done = true;
		}
		_ready.notify_all();
	}

	bool pop(function<void()>& x) {
		lock_t lock { _mutex };
		while (_q.empty() && !_done)
			_ready.wait(lock);

		if (_q.empty())
			return false;

		x = move(_q.front());
		_q.pop_front();
		return true;
	}

	// Templ?
	template<typename F>
	void push(F&& f) {
		{
			lock_t lock { _mutex };
			_q.emplace_back(forward<F>(f));
		}
		_ready.notify_one();
	}
};

/**************************************************************************************************/

class task_system {
	const unsigned _count { thread::hardware_concurrency() };
	vector<thread> _threads;
	vector<notification_queue> _q { _count };
	atomic<unsigned> _index { 0 };

	void run(unsigned i) {
		while (true) {
			function<void()> f;

			for (unsigned n = 0; n != _count * 32; ++n) {
				if (_q[(i + n) % _count].try_pop(f))
					break;
			}
			if (!f && !_q[i].pop(f))
				break;

			f();
		}
	}

public:
	task_system() {
		for (unsigned n = 0; n != _count; ++n) {
			_threads.emplace_back([&, n] {run(n);});
		}
	}

	~task_system() {
		for (auto& e : _q)
			e.done();
		for (auto& e : _threads)
			e.join();
	}

	template<typename F>
	void async_(F&& f) {
		auto i = _index++;

		for (unsigned n = 0; n != _count; ++n) {
			if (_q[(i + n) % _count].try_push(forward<F>(f)))
				return;
		}

		_q[i % _count].push(forward<F>(f));
	}
};

/**************************************************************************************************/

task_system _system;

/**************************************************************************************************/

template<typename >
struct result_of_;

template<typename R, typename ... Args>
struct result_of_<R(Args...)> {
	using type = R;
};

template<typename F>
using result_of_t_ = typename result_of_<F>::type;

/**************************************************************************************************/

template<typename R>
struct shared_base {
	vector<R> _r; // optional
	mutex _mutex;
	condition_variable _ready;
	vector<function<void()>> _then;

	virtual ~shared_base() {
	}

	void set(R&& r) {
		vector<function<void()>> then;
		{
			lock_t lock { _mutex };
			_r.push_back(move(r));
			swap(_then, then);
		}
		_ready.notify_all();
		for (const auto& f : then)
			_system.async_(move(f));
	}

	template<typename F>
	void then(F&& f) {
		bool resolved { false };
		{
			lock_t lock { _mutex };
			if (_r.empty())
				_then.push_back(forward<F>(f));
			else
				resolved = true;
		}
		if (resolved)
			_system.async_(move(f));
	}

	const R& get() {
		lock_t lock { _mutex };
		while (_r.empty())
			_ready.wait(lock);
		return _r.back();
	}
};

template<typename > struct shared;
// not defined

template<typename R, typename ... Args>
struct shared<R(Args...)> : shared_base<R> {
	function<R(Args...)> _f;

	template<typename F>
	shared(F&& f) :
			_f(forward<F>(f)) {
	}

	template<typename ... A>
	void operator()(A&&... args) {
		this->set(_f(forward<A>(args)...));
		_f = nullptr;
	}
};

template<typename > class packaged_task;
//not defined
template<typename > class future;

template<typename S, typename F>
auto package(F&& f) -> pair<packaged_task<S>, future<result_of_t_<S>>>;

template<typename R>
class future {
	shared_ptr<shared_base<R>> _p;

	template<typename S, typename F>
	friend auto package(
			F&& f) -> pair<packaged_task<S>, future<result_of_t_<S>>>;

	explicit future(shared_ptr<shared_base<R>> p) :
			_p(move(p)) {
	}
public:
	future() = default;

	template<typename F>
	auto then(F&& f) {
		auto pack = package<result_of_t<F(R)>()>([p = _p, f = forward<F>(f)]() {
			return f(p->_r.back());
		});
		_p->then(move(pack.first));
		return pack.second;
	}

	const R& get() const {
		return _p->get();
	}
};

template<typename R, typename ...Args>
class packaged_task<R(Args...)> {
	weak_ptr<shared<R(Args...)>> _p;

	template<typename S, typename F>
	friend auto package(
			F&& f) -> pair<packaged_task<S>, future<result_of_t_<S>>>;

	explicit packaged_task(weak_ptr<shared<R(Args...)>> p) :
			_p(move(p)) {
	}

public:
	packaged_task() = default;

	template<typename ... A>
	void operator()(A&&... args) const {
		auto p = _p.lock();
		if (p)
			(*p)(forward<A>(args)...);
	}
};

template<typename S, typename F>
auto package(F&& f) -> pair<packaged_task<S>, future<result_of_t_<S>>> {
	auto p = make_shared<shared<S>>(forward<F>(f));
	return make_pair(packaged_task<S>(p), future<result_of_t_<S>>(p));
}

/**************************************************************************************************/

template<typename F, typename ...Args>
auto async(F&& f, Args&&... args) {
	using result_type = result_of_t<F (Args...)>;
	using packaged_type = packaged_task<result_type()>;

	auto pack = package<result_type()>(
			bind(forward<F>(f), forward<Args>(args)...));

	_system.async_(move(get<0>(pack)));
	return get<1>(pack);
}

/**************************************************************************************************/

//// https://stackoverflow.com/questions/44355747/how-to-implement-stdwhen-any-without-polling
//// when all
//template<class ... Futures>
//struct when_any_shared_state {
//	promise<tuple<Futures...>> m_promise;
//	tuple<Futures...> m_tuple;
//	std::atomic<bool> m_done;
//	std::atomic<bool> m_count_to_two;
//
//	when_any_shared_state(promise<tuple<Futures...>> p) :
//			m_promise(std::move(p)), m_done(false), m_count_to_two(false) {
//	}
//};
//
//template<class ... Futures>
//auto when_any(Futures ... futures) -> future<tuple<Futures...>> {
//	using shared_state = when_any_shared_state<Futures...>;
//	using R = tuple<Futures...>;
//	promise<R> p;
//	future<R> result = p.get_future();
//
//	auto sptr = make_shared<shared_state>(std::move(p));
//	auto satisfy_combined_promise = [sptr](auto f) {
//		if (sptr->m_done.exchange(true) == false) {
//			if (sptr->m_count_to_two.exchange(true)) {
//				sptr->m_promise.set_value(std::move(sptr->m_tuple));
//			}
//		}
//		return f.get();
//	};
//	sptr->m_tuple = tuple<Futures...>(
//			futures.then(satisfy_combined_promise)...);
//	if (sptr->m_count_to_two.exchange(true)) {
//		sptr->m_promise.set_value(std::move(sptr->m_tuple));
//	}
//	return result;
//}
// slab
// https://stlab.cc/libraries/concurrency/future/when_any.html
// https://github.com/stlab/libraries/tree/develop/stlab/concurrency
// see this
// https://meetingcpp.com/mcpp/slides/2018/Continuable.pdf

#endif

#include <stlab/concurrency/future.hpp>
#include <stlab/concurrency/default_executor.hpp>

using namespace stlab;

int main() {
	// http://open-std.org/JTC1/SC22/WG21/docs/papers/2017/p0701r0.html

//	// https://calcul.math.cnrs.fr/attachments/spip/Documents/Journees/janv2017/parallelism_in_cpp_sc15.pdf
//	future<int> x = async([] {return 100;});
//	future<int> y = x.then([](const int& x) {return x * 2;});
////	y.wait();
//	cout << y.get() << endl;

	future<int> argument1 = async(default_executor, [] {return 42;});
	future<int> argument2 = async(default_executor, [] {return 815;});

	auto result = when_any(default_executor, [](int x, std::size_t index) {
		cout << "The current result is " << x << " " << index << '\n';
	}, argument1, argument2);

	// Waiting just for illustration purpose
	while (!result.get_try()) {
		this_thread::sleep_for(chrono::milliseconds(1));
	}

	size_t p = 0;
	size_t r = 0;
	std::vector<stlab::future<int>> futures;
	futures.push_back(async(default_executor, [] {return 1;}));
	futures.push_back(async(default_executor, [] {return 2;}));
	futures.push_back(async(default_executor, [] {return 3;}));
	futures.push_back(async(default_executor, [] {return 5;}));

	auto sut = when_all(default_executor,
			[& _p = p, &_r = r](std::vector<int> v) {
				_p = v.size();
				for (auto i : v) {
					_r += i;
				}
			}, std::make_pair(futures.begin(), futures.end()));

	while (!sut.get_try()) {
		this_thread::sleep_for(chrono::milliseconds(1));
	}

	cout << p << " " << r << endl;

}
