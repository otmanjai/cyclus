
#include <gtest/gtest.h>

#include "context.h"
#include "event_manager.h"
#include "mock_facility.h"
#include "timer.h"

using cyclus::Timer;
using cyclus::Model;
using cyclus::Trader;
using cyclus::EventManager;
using cyclus::Context;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
class ContextTests: public ::testing::Test {
 protected:
  Timer ti;
  EventManager em;
  Context* ctx;
  MockFacility* fac;
  
  virtual void SetUp() {
    ctx = new Context(&ti, &em);
    fac = new MockFacility(ctx);
  };
  
  virtual void TearDown() {
    delete fac;
    delete ctx;
  };
  
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(ContextTests, traders) {
  EXPECT_TRUE(ctx->traders().empty());
  Model* clone = fac->Clone();
  clone->Deploy(clone);
  Trader* exr = dynamic_cast<Trader*>(clone);
  EXPECT_EQ(ctx->traders().size(), 1);
  EXPECT_EQ(*ctx->traders().begin(), exr);
  clone->Decommission();
  EXPECT_TRUE(ctx->traders().empty());
}  