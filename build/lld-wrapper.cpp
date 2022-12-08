#include <stdio.h>
#include <string>

int main(int argc, char* argv[])
{
  std::string args;
  for (int i = 0; i < argc; ++i)
  {
    printf("%d:%s\n", i, argv[i]);
    if (strstr(argv[i], "lld-wrapper"))
    {
      args += argv[i];
      args.resize(args.size() - sizeof("lld-wrapper") + 1);
//    args += "esp-clang/bin/ld.lld";
      args += "xtensa-lx106-elf/bin/xtensa-lx106-elf-ld";
      args += " ";
      continue;
    }
    if (strcmp(argv[i], "-o") == 0)
    {
      i++;

      args += " ";
      args += "-o";
      args += " ";

      args += "\"";
      args += argv[i];
      args += "\"";
      args += " ";
      continue;
    }
    if (strcmp(argv[i], "-filelist") == 0)
    {
      i++;

      FILE* filelist = fopen(argv[i], "r");
      if (filelist)
      {
        char line[4096];
        while (fgets(line, 4096, filelist))
        {
	  args += "\"";
          args += line;
          if (args.back() == '\n')
            args.pop_back();
	  args += "\"";
          args += " ";
        }
        fclose(filelist);
      }

      continue;
    }
    if (strstr(argv[i], "-arch"))
    {
      i += 1;
      continue;
    }
    if (strstr(argv[i], "-syslibroot"))
    {
      i += 1;
      continue;
    }
    if (strstr(argv[i], "-lto_library"))
    {
      i += 1;
      continue;
    }
    if (strstr(argv[i], "-dynamic"))
    {
      continue;
    }
    if (strstr(argv[i], "-platform_version"))
    {
      i += 3;
      continue;
    }
    if (strstr(argv[i], "-no_deduplicate"))
    {
      continue;
    }
    if (strstr(argv[i], "-no_adhoc_codesign"))
    {
      continue;
    }
    if (strstr(argv[i], "libclang_rt.builtins"))
    {
      continue;
    }
    if (strstr(argv[i], "-l:libunwind.a"))
    {
      continue;
    }
    if (strncmp(argv[i], "--sysroot=", 10) == 0)
    {
      args += "--sysroot=";
      args += "\"";
      args += argv[i] + 10;
      args += "\"";
      args += " ";
      continue;
    }
    if (strncmp(argv[i], "-L", 2) == 0)
    {
      args += "-L";
      args += "\"";
      args += argv[i] + 2;
      args += "\"";
      args += " ";
      continue;
    }
    if (strncmp(argv[i], "/", 1) == 0)
    {
      args += "\"";
      args += argv[i];
      args += "\"";
      args += " ";
      continue;
    }
    args += argv[i];
    args += " ";
  }

  printf("%s\n", args.c_str());
  int status = system(args.c_str());
  return WEXITSTATUS(status);
}
