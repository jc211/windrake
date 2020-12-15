#include "drake/common/temp_directory.h"

#ifdef WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

#include <cstdlib>

#include "drake/common/drake_throw.h"
#include "drake/common/filesystem.h"
#include <filesystem>

namespace drake {

std::string temp_directory() {



  filesystem::path path;
  
  const char* test_tmpdir = std::getenv("TEST_TMPDIR");

  if (test_tmpdir == nullptr) {
    const char* tmpdir = nullptr;
    (tmpdir = std::getenv("TMPDIR")) || (tmpdir = "/tmp");

    filesystem::path path_template(tmpdir);
    path_template.append("robotlocomotion_drake_XXXXXX");

    std::string path_template_str = path_template.string();

    #ifdef WIN32
    const char* dtemp = _mktemp(&path_template_str[0]);
    #else
    const char* dtemp = ::mkdtemp(&path_template_str[0]);
    #endif

   
    
    DRAKE_THROW_UNLESS(dtemp != nullptr);

    path = dtemp;
  } else {
    path = test_tmpdir;
  }

  DRAKE_THROW_UNLESS(filesystem::is_directory(path));

  // Strip any trailing /.
  std::string result = path.string();
  if (result.back() == '/') {
    result.pop_back();
  }
  return result;
}

}  // namespace drake
