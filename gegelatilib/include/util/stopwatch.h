#ifndef STOPWATCH
#define STOPWATCH

#include <chrono>
#include <vector>

/**
 * \brief Stopwatch to measure time alongside evolution
 */
class Stopwatch
{
    public:
        /**
         * @brief Constructs a stopwatch and starts measuring elapsed time.
         */
        Stopwatch() { 
            reset(); 
        }

        /**
         * @brief Resets the stopwatch and clears all pending checkpoints.
         */
        void reset() { 
            this->start = now();
            this->lastCheckpoint = this->start; 
            this->checkpoints.clear(); 
        }

        /**
         * @brief Restarts the measure of the next checkpoint and clears all pending checkpoints, without resetting the total elapsed time.
         */
        void checkpointReset() { 
            this->lastCheckpoint = now(); 
            this->checkpoints.clear(); 
        }

        /**
         * @brief Records the duration since the previous checkpoint or reset.
         */
        void checkpoint() { 
            const auto current = now(); 
            this->checkpoints.push_back(std::chrono::duration<double, std::milli>(current - this->lastCheckpoint).count() / 1000); 
            this->lastCheckpoint = current; }

        /**
         * @brief Returns all pending checkpoint durations in seconds and removes them.
         * @return Checkpoint durations in chronological order, expressed in seconds.
         */
        std::vector<double> getCheckpoints() { 
            std::vector<double> result; 
            result.swap(this->checkpoints); 
            return result; 
        }

        /**
         * @brief Returns the total elapsed time since the last reset, in seconds.
         * @return Elapsed time in seconds.
         */
        double elapsed() const { 
            return std::chrono::duration<double, std::milli>(now() - this->start).count() / 1000; 
        }

    private:
        /**
         * @brief Returns the current steady-clock time point at nanosecond precision.
         */
        static std::chrono::time_point<std::chrono::steady_clock, std::chrono::nanoseconds> now() { 
            return std::chrono::time_point_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now()); 
        }

        /// @brief Start
        std::chrono::time_point<std::chrono::steady_clock, std::chrono::nanoseconds> start;

        /// @brief The last checkpoint
        std::chrono::time_point<std::chrono::steady_clock, std::chrono::nanoseconds> lastCheckpoint;

        /// @brief List of checkpoints
        std::vector<double> checkpoints;
};

#endif