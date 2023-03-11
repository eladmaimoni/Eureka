//#include <flutter/dart_project.h>
//#include <flutter/flutter_view_controller.h>
#include <windows.h>
#include <combaseapi.h>
#include <io.h>
#include <stdio.h>
#include <windows.h>
#include <iostream>
#include <cstdio>
#include <fvkde/fvkde.h>
#include <filesystem>

//#include "flutter_window.h"
//#include "utils.h"
std::filesystem::path GetExecutableDirectory() 
{
    wchar_t buffer[MAX_PATH];
    if (GetModuleFileName(nullptr, buffer, MAX_PATH) == 0) {
        return std::filesystem::path();
    }
    std::filesystem::path executable_path(buffer);
    return executable_path.remove_filename();

}
void CreateAndAttachConsole() 
{ 
    if (::AllocConsole()) {
        FILE* unused;
        if (freopen_s(&unused, "CONOUT$", "w", stdout)) {
            _dup2(_fileno(stdout), 1);
        }
        if (freopen_s(&unused, "CONOUT$", "w", stderr)) {
            _dup2(_fileno(stdout), 2);
        }
        std::ios::sync_with_stdio();
    }
}
int APIENTRY wWinMain(
    _In_ [[maybe_unused]] HINSTANCE instance,
    _In_opt_ [[maybe_unused]] HINSTANCE prev,
    _In_ [[maybe_unused]] wchar_t *command_line,
    _In_ [[maybe_unused]] int show_command
) 
{
  // Attach to console when present (e.g., 'flutter run') or create a
  // new console when running with a debugger.
  if (!::AttachConsole(ATTACH_PARENT_PROCESS) && ::IsDebuggerPresent()) {
    CreateAndAttachConsole();
  }

  // Initialize COM, so that it is available for use in the library and/or
  // plugins.
  ::CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
  FVKDE_INIT_PARAMS params{  };

  std::cout << "WORKING DIRECTORY " << std::filesystem::current_path() << "\n";
  std::filesystem::path base  = GetExecutableDirectory();

  auto assets_path = (base / "data/flutter_assets");
  auto icudtl_path = (base / "data/icudtl.dat");
  auto assets_path_str = assets_path.string();
  auto icudtl_path_str = icudtl_path.string();
  if (!std::filesystem::exists(assets_path) || !std::filesystem::exists(icudtl_path))
  {
      std::cout << "asset path or icudtl path don't exist\n";
      std::cout << assets_path_str << '\n';
      std::cout << icudtl_path_str << '\n';

  }


  params.asset_path = assets_path_str.c_str();
  params.icudtl_path = icudtl_path_str.c_str();

  //params.asset_path = "C:/workspace/Eureka/src/apps/eureka_flutter/build/windows/runner/Debug/data/flutter_assets";
  //params.icudtl_path = "C:/workspace/Eureka/src/apps/eureka_flutter/build/windows/runner/Debug/data/icudtl.dat";

  FVKDE_HANDLE handle;
  fvkde_init(&params, &handle);
  fvkde_run(handle);
  fvkde_finalize(handle);

  ::CoUninitialize();
  return EXIT_SUCCESS;
}
