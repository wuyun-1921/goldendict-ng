#pragma once
#include <string>
#include <vector>

namespace Config {
  struct Preferences;
}

/// A snippet of HTML to inject into the article <head>. Fork feature.
struct Injection
{
  std::string tag;  // raw HTML: <style>...</style> or <script>...</script>
};

/// Returns all fork-added article injections for the given preferences.
/// Registers zero allocations when cfg.entryHeightLimit is false.
std::vector< Injection > articleInjections( const Config::Preferences & cfg );
