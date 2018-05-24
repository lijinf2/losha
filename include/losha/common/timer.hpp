#include <chrono>
using namespace std::chrono;
namespace husky {
namespace losha {
class AccTimer {
public:
    AccTimer() {
        start_ = steady_clock::now();
        last_ = steady_clock::now();
    }
    double getDelta() {
        steady_clock::time_point current = steady_clock::now();
        duration<double> time_span = duration_cast<duration<double>> (current- last_); 
        last_ = steady_clock::now();
        return time_span.count();
    };
    double getTotal() {
        steady_clock::time_point current = steady_clock::now();
        duration<double> time_span = duration_cast<duration<double>> (current - start_); 
        return time_span.count();
    };
private:
    steady_clock::time_point start_;
    steady_clock::time_point last_;
};    
}
}
