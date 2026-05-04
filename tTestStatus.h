#pragma once

enum class tTestStatus {
    None,         // No test status expected. May be used for groups
    Skipped,      // Test skipped. This should not impact the upper level status.
    Passed,       // Test passed. All good!
    Failed,       // Test failed as did not meet Test Specs
    TestError,    // Something went wrong with test equipment or process.
    Tested,       // Test finished. Make decision now. This is not a final status. Must be switched to PASSED or FAILED
    Pending,      // Should be tested later. Used for Auto tests and Complex tests
    Testing,      // Under test now
    Interrupted,  // Test interrupted by user
    LAST          // Never used
};
