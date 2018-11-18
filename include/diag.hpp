#ifndef DIAG_HPP
#define DIAG_HPP

#include <iostream>
#include <string>
#include <sstream>
#include <map>

#include "utils.hpp"

#define SUMMARY_EVERY_US 1000000

struct Diag
{
private:
    struct Counter
    {
    private:
        std::string name;
        int count;
        int previous_count;

        uint64_t _interval;
        bool _enabled;
    public:
        Counter(const std::string& name, bool enabled):
            name(name), count(0), previous_count(0), _enabled(enabled) {}

        void updateInterval(uint64_t interval) { this->_interval = interval; }

        int intervalCount() {
            return this->count - this->previous_count;
        }

        double intervalRate(uint64_t interval) {
            if (interval>0) {
                return (double)this->intervalCount() / (interval / 1000000.0);
            }
            return 0.0f;
        }

        void intervalReset() {
            this->previous_count = this->count;
        }

        void reset() {
            this->count = 0;
            this->previous_count = 0;
        }

        void inc() { if (this->_enabled) this->count++; }

        friend std::ostream& operator<<(std::ostream& stream, Counter& c) {
            stream << c.name << " ^"
                << c.count << " +" << c.intervalCount()
                << " ~" << c.intervalRate(c._interval) << "Hz";
            return stream;
        }
    };

    std::map<std::string, Counter> counters;

    uint64_t start_time = now_microseconds();
    uint64_t previous_report_time = start_time;
    uint64_t next_summary_time = start_time + SUMMARY_EVERY_US;
    bool enabled;
public:
    Diag(bool enabled=false):
        enabled(enabled) {}

    void registerCounter(const char* name)
    {
        counters.emplace(name, Counter(name, this->enabled));
    }

    void reset() {
        for(auto& counter : this->counters) {
            counter.second.reset();
        }

        start_time = now_microseconds();
        previous_report_time = start_time;
        next_summary_time = start_time + SUMMARY_EVERY_US;
    }

    void report()
    {
        if (!this->enabled) {
            return;
        }
        uint64_t now;
        now = now_microseconds();
        if (now > next_summary_time) {
            uint64_t interval = now - previous_report_time;
            std::ostringstream os;
            os << ((int)(now - start_time) / 1000) << " ms: ";
            bool firstCounter = true;
            for(auto& counter : this->counters) {
                counter.second.updateInterval(interval);
                if (!firstCounter) {
                    os << " | " << counter.second;
                } else {
                    firstCounter = false;
                    os << counter.second;
                }
                counter.second.intervalReset();
            }
            os << std::endl;
            std::cerr << os.str();

            previous_report_time = now;
            next_summary_time += SUMMARY_EVERY_US;
        }
    }

    void counterInc(const char* counterName) {
        this->counters.at(counterName).inc();
    }
};

#endif
