#define CATCH_CONFIG_MAIN
#include "../../vendor/catch/catch.hpp"

#include "cf_utility.hpp"
#include "logger.hpp"

namespace {
class run_loop_thread_test final {
public:
  run_loop_thread_test(void) : count1_(0),
                               count2_(0) {
    for (int i = 0; i < 5000; ++i) {
      count1_ = 0;
      count2_ = 0;

      thread1_ = std::make_shared<krbn::cf_utility::run_loop_thread>();
      thread2_ = std::make_shared<krbn::cf_utility::run_loop_thread>();

      // thread1 (loop)

      for (int j = 0; j < 5; ++j) {
        CFRunLoopPerformBlock(thread1_->get_run_loop(),
                              kCFRunLoopCommonModes,
                              ^{
                                ++count1_;
                                // krbn::logger::get_logger().info("thread1 {0} {1}", j, count1);
                              });
      }
      thread1_->wake();

      // thread2 (recursive)

      enqueue2();
      thread2_->wake();

      // Verify counts

      thread1_->terminate();
      thread1_ = nullptr;
      thread2_->terminate();
      thread2_ = nullptr;

      REQUIRE(count1_ == 5);
      REQUIRE(count2_ == 3);
    }
  }

private:
  void enqueue2(void) {
    CFRunLoopPerformBlock(thread2_->get_run_loop(),
                          kCFRunLoopCommonModes,
                          ^{
                            ++count2_;
                            if (count2_ < 3) {
                              enqueue2();
                            }
                          });
  }

private:
  std::shared_ptr<krbn::cf_utility::run_loop_thread> thread1_;
  std::shared_ptr<krbn::cf_utility::run_loop_thread> thread2_;

  int count1_;
  int count2_;
};
} // namespace

TEST_CASE("run_loop_thread") {
  run_loop_thread_test();
}
