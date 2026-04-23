// Test-only file: registers FuzzIntrospectorPass with LLVM's new PM plugin API
// so the .so can be loaded via opt --load-pass-plugin.
// NOT compiled into the production LLVM tree.
//
// PassPlugin.h is absent from some LLVM binary package configurations.
// When it is missing we forward-declare the stable PassPluginLibraryInfo
// struct (unchanged since LLVM 13) to avoid a hard dependency on the header.
// PassBuilder.h must come before PassPlugin.h: PassPlugin.h only
// forward-declares PassBuilder, so accessing its members requires the full def.
#include "llvm/Passes/PassBuilder.h"
#if __has_include("llvm/Passes/PassPlugin.h")
#include "llvm/Passes/PassPlugin.h"
#else
// PassPlugin.h is absent from some LLVM binary packages.
// Forward-declare the stable PassPluginLibraryInfo struct (unchanged since LLVM 13).
// LLVM_PLUGIN_API_VERSION was bumped from 1 to 2 in LLVM 20; derive it from
// LLVM_VERSION_MAJOR (available via PassBuilder.h includes above).
#ifndef LLVM_PLUGIN_API_VERSION
#if LLVM_VERSION_MAJOR >= 20
#define LLVM_PLUGIN_API_VERSION 2
#else
#define LLVM_PLUGIN_API_VERSION 1
#endif
#endif
struct PassPluginLibraryInfo {
  uint32_t APIVersion;
  const char *PluginName;
  const char *PluginVersion;
  void (*RegisterPassBuilderCallbacks)(llvm::PassBuilder &);
};
#endif

#include "llvm/Transforms/FuzzIntrospector/FuzzIntrospector.h"

using namespace llvm;

extern "C" __attribute__((weak))
PassPluginLibraryInfo llvmGetPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "FuzzIntrospector", "0.1",
          [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, ModulePassManager &MPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "fuzz-introspector") {
                    MPM.addPass(FuzzIntrospectorPass());
                    return true;
                  }
                  return false;
                });
          }};
}
