#include <cppunit/CompilerOutputter.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/TestResult.h>
#include <cppunit/TestResultCollector.h>
#include <cppunit/TestRunner.h>
#include <cppunit/BriefTestProgressListener.h>

#ifdef _MSC_VER
#include <crtdbg.h>
#endif

int main() {
#ifdef _MSC_VER
  _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

  CPPUNIT_NS::TestResult controller;

  // Add a listener to collect the test results.
  CPPUNIT_NS::TestResultCollector collected_results;
  controller.addListener(&collected_results);

  // Add a listener to show progress.
  CPPUNIT_NS::BriefTestProgressListener progress;
  controller.addListener(&progress);

  // Run all the tests.
  CPPUNIT_NS::TestRunner test_runner;
  test_runner.addTest(CPPUNIT_NS::TestFactoryRegistry::getRegistry().makeTest());
  test_runner.run(controller);

  // Write the results out to stdout.
  CPPUNIT_NS::CompilerOutputter outputter(&collected_results, CPPUNIT_NS::stdCOut());
  outputter.write();

  return collected_results.wasSuccessful() ? 0 : 1;
}
