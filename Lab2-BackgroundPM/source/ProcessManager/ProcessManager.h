#ifndef BackgroundPM
#define BackgroundPM

#include <stdio.h>

#ifdef _WIN32
  int RunWindows(const char*);
#else
  int RunPosix(const char*);
#endif

  static int Run(const char* command) {
    #ifdef _WIN32
      return RunWindows(command);
    #else
      return RunPosix(command);
    #endif
  }

  #ifdef _WIN32


    #include <windows.h>

    int RunWindows(const char* command) {
      if (!command) {
          fprintf(stderr, "Error: command is null.\n");
          return -1;
      }

      STARTUPINFO startupInfo = { sizeof(STARTUPINFO) };
      PROCESS_INFORMATION processInfo = {};

      char commandBuffer[MAX_PATH];
      strncpy(commandBuffer, command, MAX_PATH - 1);
      commandBuffer[MAX_PATH - 1] = '\0';

      if (!CreateProcess(NULL, commandBuffer, NULL, NULL, FALSE, 0, NULL, NULL, &startupInfo, &processInfo)) {
        printf("Failed to create process. Error: %\n", GetLastError());
        return -1;
      }

      WaitForSingleObject(processInfo.hProcess, INFINITE);

      DWORD exitCode;

      if (!GetExitCodeProcess(processInfo.hProcess, &exitCode)) {
        fprintf(stderr, "Failed to get exit code. Error: \n", GetLastError());
        CloseHandle(processInfo.hProcess);
        CloseHandle(processInfo.hThread);
        return -1;
      }

      CloseHandle(processInfo.hProcess);
      CloseHandle(processInfo.hThread);

      printf("Process exited with code:%\n", exitCode);
      return (int)exitCode;
    }


  #else


    #include <sys/types.h>
    #include <sys/wait.h>
    #include <unistd.h>

    int RunPosix(const char* command) {\
      if (!command) {
          perror("Error: command is NULL");
          return -1;
      }

      pid_t processId;
      int processStatus;

      if ((processId = fork()) < 0) {
          perror("Error: fork failed");
          return -1;
      } else if (processId == 0) {
          if (execl("/bin/sh", "/bin/sh", "-c", command, (char*)NULL) == -1) {
              perror("Error: execl failed");
              _exit(1);
          }
      }

      if (waitpid(processId, &processStatus, 0) == -1) {
          perror("Error: waitpid failed");
          return -1;
      }
      
      if (WIFEXITED(processStatus)) {
        int exitCode = WEXITSTATUS(processStatus);
        printf("Process exited with code:%d\n", exitCode);
        return exitCode;
      }

      perror("Error: child process terminated");
      return -1;
    }


  #endif


#endif //BackgroundPM
