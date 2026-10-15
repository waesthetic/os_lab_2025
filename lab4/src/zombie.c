#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/wait.h>

static void ShowProcess(pid_t pid) {
  char cmd[64];
  snprintf(cmd, sizeof(cmd), "ps -o pid,ppid,stat,comm -p %d", pid);
  fflush(stdout);
  if (system(cmd) != 0) {
    printf("Process %d is not in the process table\n", pid);
  }
}

static pid_t CreateChild(void) {
  pid_t child_pid = fork();

  if (child_pid < 0) {
    perror("fork");
    exit(1);
  }

  if (child_pid == 0) {
    printf("Child %d: finished\n", getpid());
    exit(0);
  }

  return child_pid;
}

static void DemoZombie(void) {
  pid_t child_pid = CreateChild();
  printf("Parent %d: created child %d, NOT calling wait()\n", getpid(),
         child_pid);
  sleep(1);

  ShowProcess(child_pid);

  printf("Parent: sleeping 30 seconds, check `ps` in another terminal\n");
  sleep(30);
}

static void DemoWait(void) {
  pid_t child_pid = CreateChild();
  printf("Parent %d: created child %d\n", getpid(), child_pid);
  sleep(1);

  printf("\nBefore wait():\n");
  ShowProcess(child_pid);

  int status;
  waitpid(child_pid, &status, 0);
  printf("\nParent: wait() returned, child exit code = %d\n",
         WEXITSTATUS(status));

  printf("\nAfter wait():\n");
  ShowProcess(child_pid);
}

static void DemoIgnore(void) {
  signal(SIGCHLD, SIG_IGN);

  pid_t child_pid = CreateChild();
  printf("Parent %d: created child %d, SIGCHLD is ignored\n", getpid(),
         child_pid);
  sleep(1);

  ShowProcess(child_pid);

  if (waitpid(child_pid, NULL, 0) == -1) {
    printf("Parent: waitpid() failed: %s\n", strerror(errno));
  }
}

static void OnChildExit(int sig) {
  (void)sig;
  int saved_errno = errno;
  while (waitpid(-1, NULL, WNOHANG) > 0) {
  }
  errno = saved_errno;
}

static void DemoHandler(void) {
  signal(SIGCHLD, OnChildExit);

  pid_t child_pid = CreateChild();
  printf("Parent %d: created child %d, SIGCHLD handler installed\n", getpid(),
         child_pid);
  sleep(1);

  ShowProcess(child_pid);
}

static int CountZombies(pid_t parent_pid) {
  char cmd[128];
  snprintf(cmd, sizeof(cmd),
           "ps -A -o ppid=,stat= | awk '$1 == %d && $2 ~ /^Z/' | wc -l",
           parent_pid);

  FILE *output = popen(cmd, "r");
  if (output == NULL) {
    perror("popen");
    return -1;
  }

  int count = -1;
  if (fscanf(output, "%d", &count) != 1) count = -1;
  pclose(output);
  return count;
}

static void DemoMany(int n) {
  int created = 0;
  for (int i = 0; i < n; i++) {
    pid_t child_pid = fork();
    if (child_pid < 0) {
      perror("fork");
      break;
    }
    if (child_pid == 0) {
      _exit(0);
    }
    created += 1;
  }

  printf("Parent %d: created %d children, NOT calling wait()\n", getpid(),
         created);
  sleep(1);

  printf("Zombies of parent %d: %d\n", getpid(), CountZombies(getpid()));
  printf("Parent: sleeping 10 seconds, then exiting.\n");
  printf("After exit the zombies will be adopted and reaped by PID 1.\n");
  fflush(stdout);
  sleep(10);
}

int main(int argc, char **argv) {
  if (argc < 2) {
    printf("Usage: %s zombie|wait|ignore|handler|many N\n", argv[0]);
    return 1;
  }

  if (strcmp(argv[1], "many") == 0) {
    if (argc != 3) {
      printf("Usage: %s many N\n", argv[0]);
      return 1;
    }
    int n = atoi(argv[2]);
    if (n <= 0 || n > 500) {
      printf("N must be in range 1..500\n");
      return 1;
    }
    DemoMany(n);
    return 0;
  }

  if (argc != 2) {
    printf("Usage: %s zombie|wait|ignore|handler|many N\n", argv[0]);
    return 1;
  }

  if (strcmp(argv[1], "zombie") == 0) {
    DemoZombie();
  } else if (strcmp(argv[1], "wait") == 0) {
    DemoWait();
  } else if (strcmp(argv[1], "ignore") == 0) {
    DemoIgnore();
  } else if (strcmp(argv[1], "handler") == 0) {
    DemoHandler();
  } else {
    printf("Unknown mode: %s\n", argv[1]);
    return 1;
  }

  return 0;
}
