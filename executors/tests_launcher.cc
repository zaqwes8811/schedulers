#include <gtest/gtest.h>

#include <iostream>

#include <unistd.h>
#include <stdio.h>

int main(int argc, char* argv[]) {
  // http://stackoverflow.com/questions/3019630/c-how-to-redirect-stderr-from-system-command-to-stdout-or-file
  dup2(1, 2);  //redirects stderr to stdout below this line.

  testing::InitGoogleTest(&argc, argv);
  testing::GTEST_FLAG(print_time) = true;
  return RUN_ALL_TESTS();
}
