#include <gtest/gtest.h>
#include <chrono>
#include <thread>
#include <vector>

#include "util/stopwatch.h"

class StopwatchTest : public ::testing::Test
{
    protected:
        Stopwatch stopwatch;
};

TEST_F(StopwatchTest, StartsAutomatically)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    ASSERT_GE(stopwatch.elapsed(), 0.005) << "Elapsed time should be greater than zero";
}

TEST_F(StopwatchTest, ResetRestartsTimer)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    stopwatch.reset();

    ASSERT_LT(stopwatch.elapsed(), 0.01) << "Elapsed time should restart after reset";
    ASSERT_TRUE(stopwatch.getCheckpoints().empty()) << "Reset should clear all checkpoints";
}

TEST_F(StopwatchTest, CheckpointRecordsElapsedTime)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    stopwatch.checkpoint();

    const std::vector<double> checkpoints = stopwatch.getCheckpoints();

    ASSERT_EQ(checkpoints.size(), 1u) << "One checkpoint should be recorded";
    ASSERT_GE(checkpoints.at(0), 0.01) << "Checkpoint duration should be at least 10 ms";
    ASSERT_LT(checkpoints.at(0), 1.0) << "Checkpoint duration should be reasonable";
}

TEST_F(StopwatchTest, MultipleCheckpointsMeasureIntervals)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    stopwatch.checkpoint();

    std::this_thread::sleep_for(std::chrono::milliseconds(30));
    stopwatch.checkpoint();

    const std::vector<double> checkpoints = stopwatch.getCheckpoints();

    ASSERT_EQ(checkpoints.size(), 2u) << "Two checkpoints should be recorded";
    ASSERT_GE(checkpoints.at(0), 10.0 / 1000.0) << "First checkpoint should measure the first interval";
    ASSERT_GE(checkpoints.at(1), 20.0 / 1000.0) << "Second checkpoint should measure the second interval";
    ASSERT_LT(checkpoints.at(0), 1000.0 / 1000.0) << "First checkpoint duration should be reasonable";
    ASSERT_LT(checkpoints.at(1), 1000.0 / 1000.0) << "Second checkpoint duration should be reasonable";
}

TEST_F(StopwatchTest, GetCheckpointsClearsRecordedCheckpoints)
{
    stopwatch.checkpoint();
    stopwatch.checkpoint();

    const std::vector<double> checkpoints = stopwatch.getCheckpoints();
    const std::vector<double> emptyCheckpoints = stopwatch.getCheckpoints();

    ASSERT_EQ(checkpoints.size(), 2u) << "Both checkpoints should be returned";
    ASSERT_TRUE(emptyCheckpoints.empty()) << "Retrieving checkpoints should clear the internal list";
}

TEST_F(StopwatchTest, CheckpointResetClearsCheckpointsAndRestartsInterval)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    stopwatch.checkpoint();

    const double elapsedBeforeReset = stopwatch.elapsed();
    stopwatch.checkpointReset();

    ASSERT_TRUE(stopwatch.getCheckpoints().empty()) << "Checkpoint reset should clear recorded checkpoints";
    ASSERT_GE(stopwatch.elapsed(), elapsedBeforeReset) << "Checkpoint reset should not reset total elapsed time";

    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    stopwatch.checkpoint();

    const std::vector<double> checkpoints = stopwatch.getCheckpoints();

    ASSERT_EQ(checkpoints.size(), 1u) << "Only the checkpoint recorded after checkpoint reset should remain";
    ASSERT_GE(checkpoints.at(0), 10.0 / 1000.0) << "Checkpoint should measure time since checkpoint reset";
    ASSERT_LT(checkpoints.at(0), 1000.0 / 1000.0) << "Checkpoint duration should be reasonable";
}

TEST_F(StopwatchTest, CheckpointResetDoesNotResetTotalElapsedTime)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(20));

    const double elapsedBeforeReset = stopwatch.elapsed();
    stopwatch.checkpointReset();
    const double elapsedAfterReset = stopwatch.elapsed();

    ASSERT_GE(elapsedAfterReset, elapsedBeforeReset) << "Total elapsed time should continue after checkpoint reset";
    ASSERT_GE(elapsedAfterReset, 10.0 / 1000.0) << "Total elapsed time should retain the initial measurement";
}

TEST_F(StopwatchTest, ResetClearsBothTimerAndCheckpointState)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    stopwatch.checkpoint();

    stopwatch.reset();

    ASSERT_TRUE(stopwatch.getCheckpoints().empty()) << "Reset should clear all recorded checkpoints";
    ASSERT_LT(stopwatch.elapsed(), 10.0 / 1000.0) << "Reset should restart total elapsed time";

    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    stopwatch.checkpoint();

    const std::vector<double> checkpoints = stopwatch.getCheckpoints();

    ASSERT_EQ(checkpoints.size(), 1u) << "A new checkpoint should be recorded after reset";
    ASSERT_GE(checkpoints.at(0), 10.0 / 1000.0) << "New checkpoint should measure time since reset";
}