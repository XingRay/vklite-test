//
// Created by leixing on 2025-07-22.
//

#include <memory>
#include "sandbox/Sandbox.h"
#include "Test05.h"

int main(int argc, const char** argv) {
    std::unique_ptr<test::TestBase> test = std::make_unique<test::Test05>();
    test::Sandbox sandbox(std::move(test));
    sandbox.run();
}
