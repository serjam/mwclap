#include "overlay.h"
#include "xor.h"

static HWND win;

auto FOverlay::window_set_style() -> void {
  int i = 0;

  i = (int)GetWindowLong(win, -20);

  SetWindowLongPtr(win, -20, (LONG_PTR)(i | 0x20)); //make transparent
  SetWindowDisplayAffinity(win, WDA_MONITOR | WDA_EXCLUDEFROMCAPTURE); //prevent capture
}

auto FOverlay::window_set_transparency() -> void {
  MARGINS margin;
  UINT opacity_flag, color_key_flag, color_key = 0;
  UINT opacity = 0;

  margin.cyBottomHeight = -1;
  margin.cxLeftWidth = -1;
  margin.cxRightWidth = -1;
  margin.cyTopHeight = -1;

  DwmExtendFrameIntoClientArea(win, &margin);

  opacity_flag = 0x02;
  color_key_flag = 0x01;
  color_key = 0x000000;
  opacity = 0xFF;

  SetLayeredWindowAttributes(win, color_key, opacity, opacity_flag);
}

auto FOverlay::window_set_top_most() -> void {
  SetWindowPos(win, HWND_TOPMOST, 0, 0, 0, 0, 0x0002 | 0x0001);
}

auto FOverlay::retrieve_window() -> HWND { return win; }


auto FOverlay::window_init() -> BOOL {
  win = FindWindow(xorstr_("CEF-OSC-WIDGET"), xorstr_("NVIDIA GeForce Overlay"));
  if (!win)
    return FALSE;

  FOverlay::window_set_style();
  FOverlay::window_set_transparency();
  FOverlay::window_set_top_most();

  ShowWindow(win, SW_SHOW);

  return TRUE;
}

/*
Overlay functions
*/

ID2D1Factory* d2d_factory;
ID2D1HwndRenderTarget* tar;
IDWriteFactory* write_factory;
ID2D1SolidColorBrush* brush;
ID2D1SolidColorBrush* red_brush;
ID2D1SolidColorBrush* green_brush;
IDWriteTextFormat* format;

auto FOverlay::d2d_shutdown() -> void {
  // Release
  tar->Release();
  write_factory->Release();
  brush->Release();
  red_brush->Release();
  green_brush->Release();
  d2d_factory->Release();
}

auto FOverlay::init_d2d() -> BOOL {
  HRESULT ret;
  RECT rc;

  // Initialize D2D here
  ret = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &d2d_factory);
  if (FAILED(ret))
    return FALSE;

  ret =
    DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory),
      (IUnknown**)(&write_factory));
  if (FAILED(ret))
    return FALSE;

  write_factory->CreateTextFormat(
    L"Consolas", NULL, DWRITE_FONT_WEIGHT_REGULAR, DWRITE_FONT_STYLE_NORMAL,
    DWRITE_FONT_STRETCH_NORMAL, 13.0, L"en-us", &format);

  GetClientRect(FOverlay::retrieve_window(), &rc);

  ret = d2d_factory->CreateHwndRenderTarget(
    D2D1::RenderTargetProperties(
      D2D1_RENDER_TARGET_TYPE_DEFAULT,
      D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN,
        D2D1_ALPHA_MODE_PREMULTIPLIED)),
    D2D1::HwndRenderTargetProperties(
      FOverlay::retrieve_window(),
      D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top)),
    &tar);
  if (FAILED(ret))
    return FALSE;

  tar->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), &brush);
  tar->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Red), &red_brush);
  tar->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Green), &green_brush);

  return TRUE;
}

auto FOverlay::begin_scene() -> void { tar->BeginDraw(); }

auto FOverlay::end_scene() -> void { tar->EndDraw(); }

auto FOverlay::clear_scene() -> void { tar->Clear(); }

auto FOverlay::draw_text_white(float fsize, int x, int y, const char* str, ...) -> void {
  char buf[4096];
  int len = 0;
  wchar_t b[256];

  // if (!draw) // no need for it.
  //	 return;

  va_list arg_list;
  va_start(arg_list, str);
  vsnprintf(buf, sizeof(buf), str, arg_list);
  va_end(arg_list);

  len = strlen(buf);
  mbstowcs(b, buf, len);

  if (format) {
    format->Release();
    format = nullptr;
  }

  write_factory->CreateTextFormat(
    L"Consolas", NULL, DWRITE_FONT_WEIGHT_REGULAR, DWRITE_FONT_STYLE_NORMAL,
    DWRITE_FONT_STRETCH_NORMAL, fsize, L"en-us", &format);


  tar->DrawText(b, len, format, D2D1::RectF(x, y, 1920, 1080), brush,
    D2D1_DRAW_TEXT_OPTIONS_NONE, DWRITE_MEASURING_MODE_NATURAL);
}

auto FOverlay::draw_text_red(int x, int y, const char* str, ...) -> void {
  char buf[4096];
  int len = 0;
  wchar_t b[256];

  // if (!draw) // no need for it.
  //	 return;

  va_list arg_list;
  va_start(arg_list, str);
  vsnprintf(buf, sizeof(buf), str, arg_list);
  va_end(arg_list);

  len = strlen(buf);
  mbstowcs(b, buf, len);

  tar->DrawText(b, len, format, D2D1::RectF(x, y, 1920, 1080), red_brush,
    D2D1_DRAW_TEXT_OPTIONS_NONE, DWRITE_MEASURING_MODE_NATURAL);
}

auto FOverlay::draw_text_green(int x, int y, const char* str, ...) -> void {
  char buf[4096];
  int len = 0;
  wchar_t b[256];

  // if (!draw) // no need for it.
  //	 return;

  va_list arg_list;
  va_start(arg_list, str);
  vsnprintf(buf, sizeof(buf), str, arg_list);
  va_end(arg_list);

  len = strlen(buf);
  mbstowcs(b, buf, len);

  tar->DrawText(b, len, format, D2D1::RectF(x, y, 1920, 1080), green_brush,
    D2D1_DRAW_TEXT_OPTIONS_NONE, DWRITE_MEASURING_MODE_NATURAL);
}

void FOverlay::draw_box(int x, int y, int width, int height) {
  D2D1_RECT_F box = D2D1::RectF(x, y, x+width, y+height);
  tar->DrawRectangle(&box, red_brush);
}

auto FOverlay::clear_screen() -> void {
  FOverlay::begin_scene();
  FOverlay::clear_scene();
  FOverlay::end_scene();
}