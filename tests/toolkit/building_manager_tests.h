#ifndef CYCLUS_TESTS_TOOLKIT_BUILDING_MANAGER_TESTS_H_
#define CYCLUS_TESTS_TOOLKIT_BUILDING_MANAGER_TESTS_H_

#include <gtest/gtest.h>

#include "toolkit/building_manager.h"
#include "commodity_test_helper.h"

namespace cyclus {
namespace toolkit {

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
class BuildingManagerTests : public ::testing::Test {
 public:
  virtual void SetUp();
  virtual void TearDown();
  void SetUpProblem();

 protected:
  CommodityTestHelper helper;
  Builder builder1;
  Builder builder2;
  BuildingManager manager;
  double demand, capacity1, capacity2, cost1, cost2;
  int build1, build2;
};

}  // namespace toolkit
}  // namespace cyclus

#endif  // CYCLUS_TESTS_TOOLKIT_BUILDING_MANAGER_TESTS_H_
