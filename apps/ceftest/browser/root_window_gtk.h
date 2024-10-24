// Copyright (c) 2015 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#ifndef CEF_TESTS_CEFCLIENT_BROWSER_ROOT_WINDOW_GTK_H_
#define CEF_TESTS_CEFCLIENT_BROWSER_ROOT_WINDOW_GTK_H_
#pragma once

#include <gtk/gtk.h>

#include <memory>
#include <string>

#include "ceftest/browser/browser_window.h"
#include "ceftest/browser/root_window.h"

namespace client {

// GTK implementation of a top-level native window in the browser process.
// The methods of this class must be called on the main thread unless otherwise
// indicated.
class RootWindowGtk : public RootWindow, public BrowserWindow::Delegate {
 public:
  // Constructor may be called on any thread.
  explicit RootWindowGtk(bool use_alloy_style);
  ~RootWindowGtk();

  // RootWindow methods.
  void Init(RootWindow::Delegate* delegate,
            std::unique_ptr<RootWindowConfig> config,
            const CefBrowserSettings& settings) override;
  void InitAsPopup(RootWindow::Delegate* delegate,
                   bool with_controls,
                   bool with_osr,
                   const CefPopupFeatures& popupFeatures,
                   CefWindowInfo& windowInfo,
                   CefRefPtr<CefClient>& client,
                   CefBrowserSettings& settings) override;
  void Show(ShowMode mode) override;
  void Hide() override;
  void SetBounds(int x, int y, size_t width, size_t height) override;
  void Close(bool force) override;
  void SetDeviceScaleFactor(float device_scale_factor) override;
  float GetDeviceScaleFactor() const override;
  CefRefPtr<CefBrowser> GetBrowser() const override;
  ClientWindowHandle GetWindowHandle() const override;
  bool WithWindowlessRendering() const override;

 private:
  void CreateBrowserWindow(const std::string& startup_url);
  void CreateRootWindow(const CefBrowserSettings& settings,
                        bool initially_hidden);

  // BrowserWindow::Delegate methods.
  bool UseAlloyStyle() const override { return IsAlloyStyle(); }
  void OnBrowserCreated(CefRefPtr<CefBrowser> browser) override;
  void OnBrowserWindowClosing() override;
  void OnBrowserWindowDestroyed() override;
  void OnSetAddress(const std::string& url) override;
  void OnSetTitle(const std::string& title) override;
  void OnSetFullscreen(bool fullscreen) override;
  void OnAutoResize(const CefSize& new_size) override;
  void OnSetLoadingState(bool isLoading,
                         bool canGoBack,
                         bool canGoForward) override;
  void OnSetDraggableRegions(
      const std::vector<CefDraggableRegion>& regions) override;
  void OnSetDomWalkResult(const std::string& message) override;

  void NotifyMoveOrResizeStarted();
  void NotifySetFocus();
  void NotifyVisibilityChange(bool show);
  void NotifyMenuBarHeight(int height);
  void NotifyContentBounds(int x, int y, int width, int height);
  void NotifyLoadURL(const std::string& url);
  void NotifyButtonClicked(int id);
  void NotifyMenuItem(int id);
  void NotifyForceClose();
  void NotifyCloseBrowser();
  void NotifyDestroyedIfDone(bool window_destroyed, bool browser_destroyed);

  // Signal handlers for the top-level GTK window.
  static gboolean WindowFocusIn(GtkWidget* widget,
                                GdkEventFocus* event,
                                RootWindowGtk* self);
  static gboolean WindowState(GtkWidget* widget,
                              GdkEventWindowState* event,
                              RootWindowGtk* self);
  static gboolean WindowConfigure(GtkWindow* window,
                                  GdkEvent* event,
                                  RootWindowGtk* self);
  static void WindowDestroy(GtkWidget* widget, RootWindowGtk* self);
  static gboolean WindowDelete(GtkWidget* widget,
                               GdkEvent* event,
                               RootWindowGtk* self);

  // Signal handlers for the GTK Vbox containing all UX elements.
  static void GridSizeAllocated(GtkWidget* widget,
                                GtkAllocation* allocation,
                                RootWindowGtk* self);

  // Signal handlers for the GTK toolbar.
  static void ToolbarSizeAllocated(GtkWidget* widget,
                                   GtkAllocation* allocation,
                                   RootWindowGtk* self);
  static void InfobarSizeAllocated(GtkWidget* widget,
                                   GtkAllocation* allocation,
                                   RootWindowGtk* self);
  static void BackButtonClicked(GtkButton* button, RootWindowGtk* self);
  static void ForwardButtonClicked(GtkButton* button, RootWindowGtk* self);
  static void StopButtonClicked(GtkButton* button, RootWindowGtk* self);
  static void DomWalkButtonClicked(GtkButton* button, RootWindowGtk* self);

  // Signal handlers for the GTK URL entry field.
  static void URLEntryActivate(GtkEntry* entry, RootWindowGtk* self);
  static gboolean URLEntryButtonPress(GtkWidget* widget,
                                      GdkEventButton* event,
                                      RootWindowGtk* self);

  // After initialization all members are only accessed on the main thread.
  // Members set during initialization.
  bool with_controls_;
  bool always_on_top_;
  bool with_osr_;
  bool is_popup_;
  CefRect start_rect_;
  std::unique_ptr<BrowserWindow> browser_window_;
  bool initialized_;

  // Main window.
  GtkWidget* window_;

  // Buttons.
  GtkToolItem* back_button_;
  GtkToolItem* forward_button_;
  GtkToolItem* domwalk_button_;

  // URL text field.
  GtkWidget* url_entry_;

  // Info labels
  GtkWidget* count_label_;
  GtkWidget* height_label_;
  GtkWidget* width_label_;

  // Height of UX controls that affect browser window placement.
  int toolbar_height_;
  int infobar_height_;

  CefRect browser_bounds_;

  bool window_destroyed_;
  bool browser_destroyed_;

  // JavaScript data
  std::string dom_walk_js_;

  // Members only accessed on the UI thread because they're needed for
  // WindowDelete.
  bool force_close_;
  bool is_closing_;

  DISALLOW_COPY_AND_ASSIGN(RootWindowGtk);
};

}  // namespace client

#endif  // CEF_TESTS_CEFCLIENT_BROWSER_ROOT_WINDOW_GTK_H_
