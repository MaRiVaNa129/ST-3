// Copyright 2021 GHA Test Team
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <cstdint>
#include <thread>
#include <chrono>
#include <future>
#include "TimedDoor.h"

using ::testing::_;
using ::testing::Mock;
using ::testing::Return;

class MockTimerClient : public TimerClient {
 public:
    MOCK_METHOD(void, Timeout, (), (override));
};

class MockDoor : public Door {
 public:
    MOCK_METHOD(void, lock, (), (override));
    MOCK_METHOD(void, unlock, (), (override));
    MOCK_METHOD(bool, isDoorOpened, (), (override));
};

class TimedDoorTest : public ::testing::Test {
 protected:
    void SetUp() override {
        door = new TimedDoor(100);
    }

    void TearDown() override {
        delete door;
    }

    TimedDoor* door;
};

class DoorTimerAdapterTest : public ::testing::Test {
 protected:
    void SetUp() override {
        timedDoor = new TimedDoor(100);
        adapter = new DoorTimerAdapter(*timedDoor);
    }

    void TearDown() override {
        delete adapter;
        delete timedDoor;
    }

    TimedDoor* timedDoor;
    DoorTimerAdapter* adapter;
};

TEST_F(TimedDoorTest, ConstructorTest) {
    EXPECT_EQ(door->getTimeOut(), 1);
    EXPECT_FALSE(door->isDoorOpened());
}

TEST_F(TimedDoorTest, LockDoorTest) {
    door->unlock();
    EXPECT_TRUE(door->isDoorOpened());
    door->lock();
    EXPECT_FALSE(door->isDoorOpened());
}

TEST_F(TimedDoorTest, UnlockDoorTest) {
    door->lock();
    EXPECT_FALSE(door->isDoorOpened());
    door->unlock();
    EXPECT_TRUE(door->isDoorOpened());
}

TEST_F(TimedDoorTest, NoExceptionWhenDoorIsClosed) {
    door->lock();
    EXPECT_NO_THROW(door->throwState());
}

TEST_F(TimedDoorTest, ExceptionWhenDoorIsOpen) {
    door->unlock();
    EXPECT_THROW(door->throwState(), std::runtime_error);
}

TEST_F(DoorTimerAdapterTest, AdapterTimeoutCallsThrowState) {
    timedDoor->unlock();
    EXPECT_THROW(adapter->Timeout(), std::runtime_error);
}

TEST_F(TimedDoorTest, TimerThrowsExceptionOnOpenDoor) {
    door->unlock();
    std::this_thread::sleep_for(std::chrono::seconds(2));
    SUCCEED();
}

TEST_F(TimedDoorTest, MultipleOpenCloseTest) {
    for (int i = 0; i < 5; i++) {
        door->unlock();
        EXPECT_TRUE(door->isDoorOpened());
        door->lock();
        EXPECT_FALSE(door->isDoorOpened());
    }
}

TEST(MockTimerClientTest, MockTimeoutCallTest) {
    MockTimerClient mockClient;
    EXPECT_CALL(mockClient, Timeout())
        .Times(1);

    mockClient.Timeout();
    Mock::VerifyAndClearExpectations(&mockClient);
}

TEST(TimedDoorDifferentTimeoutsTest, DifferentTimeoutValues) {
    TimedDoor door5(5);
    TimedDoor door10(10);

    EXPECT_EQ(door5.getTimeOut(), 5);
    EXPECT_EQ(door10.getTimeOut(), 10);
}

TEST_F(TimedDoorTest, NoExceptionAfterClosingDoor) {
    door->unlock();
    EXPECT_TRUE(door->isDoorOpened());
    door->lock();
    EXPECT_FALSE(door->isDoorOpened());
    EXPECT_NO_THROW(door->throwState());
}

TEST_F(DoorTimerAdapterTest, AdapterNoExceptionOnClosedDoor) {
    timedDoor->lock();
    EXPECT_NO_THROW(adapter->Timeout());
}
