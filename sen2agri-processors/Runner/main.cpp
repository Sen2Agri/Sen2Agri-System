#include "otbWrapperCommandLineLauncher.h"
#include <vector>

int main(int argc, char* argv[])
{
  if (argc < 2)
    {
    std::cerr << "Usage : " << argv[0] << " module_name [MODULEPATH] [arguments]" << std::endl;
    return EXIT_FAILURE;
    }

  std::vector<std::string> vexp;

  std::string exp;

    // Construct the string expression
    for (int i = 1; i < argc; i++)
      {
      /*if (i != argc - 1)
        {
        exp.append(argv[i]);
        exp.append(" ");
        }
      else
        {
        exp.append(argv[i]);
        }*/
      std::string strarg(argv[i]);

      vexp.push_back(strarg);
      }
  //  std::cerr << exp << ":\n";

  typedef otb::Wrapper::CommandLineLauncher LauncherType;
  LauncherType::Pointer launcher = LauncherType::New();

  //if (launcher->Load(exp) == true)
    if (launcher->Load(vexp) == true)
    {
    if (launcher->ExecuteAndWriteOutput() == false)
      {
      return EXIT_FAILURE;
      }
    }
  else
    {
    return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}
