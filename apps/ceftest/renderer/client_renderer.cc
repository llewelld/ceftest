// Copyright (c) 2012 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "ceftest/renderer/client_renderer.h"

#include <sstream>
#include <string>

#include "include/cef_crash_util.h"
#include "include/cef_dom.h"
#include "include/wrapper/cef_helpers.h"
#include "include/wrapper/cef_message_router.h"

namespace client::renderer {

namespace {

class DomWalkHandler : public CefV8Handler {
 public:
  DomWalkHandler(CefRefPtr<CefBrowser> browser)
    : browser(browser) {}

  ~DomWalkHandler() {
    browser = nullptr;
  }

  bool Execute(const CefString& name,
               CefRefPtr<CefV8Value> object,
               const CefV8ValueList& arguments,
               CefRefPtr<CefV8Value>& retval,
               CefString& exception) override;

  CefRefPtr<CefBrowser> browser;

  IMPLEMENT_REFCOUNTING(DomWalkHandler);
};


bool DomWalkHandler::Execute(const CefString& name,
           CefRefPtr<CefV8Value> object,
           const CefV8ValueList& arguments,
           CefRefPtr<CefV8Value>& retval,
           CefString& exception) {
  if (!arguments.empty()) {
    // Create the message object.
    CefRefPtr<CefProcessMessage> msg = CefProcessMessage::Create("dom_walk");

    // Retrieve the argument list object.
    CefRefPtr<CefListValue> args = msg->GetArgumentList();

    // Populate the argument values.
    args->SetString(0, arguments[0]->GetStringValue());

    // Send the process message to the main frame in the render process.
    // Use PID_BROWSER instead when sending a message to the browser process.
    browser->GetMainFrame()->SendProcessMessage(PID_BROWSER, msg);
  }
  return true;
}


// Must match the value in client_handler.cc.
const char kFocusedNodeChangedMessage[] = "ClientRenderer.FocusedNodeChanged";

class ClientRenderDelegate : public ClientAppRenderer::Delegate {
 public:
  ClientRenderDelegate() = default;

  void OnWebKitInitialized(CefRefPtr<ClientAppRenderer> app) override {
    if (CefCrashReportingEnabled()) {
      // Set some crash keys for testing purposes. Keys must be defined in the
      // "crash_reporter.cfg" file. See cef_crash_util.h for details.
      CefSetCrashKeyValue("testkey_small1", "value1_small_renderer");
      CefSetCrashKeyValue("testkey_small2", "value2_small_renderer");
      CefSetCrashKeyValue("testkey_medium1", "value1_medium_renderer");
      CefSetCrashKeyValue("testkey_medium2", "value2_medium_renderer");
      CefSetCrashKeyValue("testkey_large1", "value1_large_renderer");
      CefSetCrashKeyValue("testkey_large2", "value2_large_renderer");
    }

    // Create the renderer-side router for query handling.
    CefMessageRouterConfig config;
    message_router_ = CefMessageRouterRendererSide::Create(config);
  }

  void OnContextCreated(CefRefPtr<ClientAppRenderer> app,
                        CefRefPtr<CefBrowser> browser,
                        CefRefPtr<CefFrame> frame,
                        CefRefPtr<CefV8Context> context) override {
    message_router_->OnContextCreated(browser, frame, context);
    if (!dom_walk_handler) {
      dom_walk_handler = new DomWalkHandler(browser);
    }

    CefRefPtr<CefV8Context> v8_context = frame->GetV8Context();
    if (v8_context.get() && v8_context->Enter()) {
      CefRefPtr<CefV8Value> global = v8_context->GetGlobal();
      CefRefPtr<CefV8Value> dom_walk = CefV8Value::CreateFunction("dom_walk", dom_walk_handler);
      global->SetValue("dom_walk", dom_walk, V8_PROPERTY_ATTRIBUTE_READONLY);

      CefV8ValueList args;
      dom_walk->ExecuteFunction(global, args);

      v8_context->Exit();
    }
  }

  void OnContextReleased(CefRefPtr<ClientAppRenderer> app,
                         CefRefPtr<CefBrowser> browser,
                         CefRefPtr<CefFrame> frame,
                         CefRefPtr<CefV8Context> context) override {
    message_router_->OnContextReleased(browser, frame, context);
    dom_walk_handler = nullptr;
  }

  void OnFocusedNodeChanged(CefRefPtr<ClientAppRenderer> app,
                            CefRefPtr<CefBrowser> browser,
                            CefRefPtr<CefFrame> frame,
                            CefRefPtr<CefDOMNode> node) override {
    bool is_editable = (node.get() && node->IsEditable());
    if (is_editable != last_node_is_editable_) {
      // Notify the browser of the change in focused element type.
      last_node_is_editable_ = is_editable;
      CefRefPtr<CefProcessMessage> message =
          CefProcessMessage::Create(kFocusedNodeChangedMessage);
      message->GetArgumentList()->SetBool(0, is_editable);
      frame->SendProcessMessage(PID_BROWSER, message);
    }
  }

  bool OnProcessMessageReceived(CefRefPtr<ClientAppRenderer> app,
                                CefRefPtr<CefBrowser> browser,
                                CefRefPtr<CefFrame> frame,
                                CefProcessId source_process,
                                CefRefPtr<CefProcessMessage> message) override {
    return message_router_->OnProcessMessageReceived(browser, frame,
                                                     source_process, message);
  }

 private:
  bool last_node_is_editable_ = false;
  CefRefPtr<CefV8Handler> dom_walk_handler;

  // Handles the renderer side of query routing.
  CefRefPtr<CefMessageRouterRendererSide> message_router_;

  DISALLOW_COPY_AND_ASSIGN(ClientRenderDelegate);
  IMPLEMENT_REFCOUNTING(ClientRenderDelegate);
};

}  // namespace

void CreateDelegates(ClientAppRenderer::DelegateSet& delegates) {
  delegates.insert(new ClientRenderDelegate);
}

}  // namespace client::renderer
