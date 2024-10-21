// Copyright (c) 2024 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#ifndef CEF_TESTS_CEFCLIENT_BROWSER_HANG_TEST_H_
#define CEF_TESTS_CEFCLIENT_BROWSER_HANG_TEST_H_
#pragma once

#include "ceftest/browser/test_runner.h"

namespace client::hang_test {

// Create message handlers. Called from test_runner.cc.
void CreateMessageHandlers(test_runner::MessageHandlerSet& handlers);

}  // namespace client::hang_test

#endif  // CEF_TESTS_CEFCLIENT_BROWSER_HANG_TEST_H_
