#define _GNU_SOURCE
#include <errno.h>
#include <sched.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define errExit(msg)    do { logn(msg); exit(EXIT_FAILURE); } while (0)

void logn(char *msg) {
  FILE *logFile;
  logFile=fopen("main.log", "a");
  fprintf(logFile, msg);
  fprintf(logFile, "\n");
  fclose(logFile);
}

void logd(char *name, int n) {
  FILE *logFile;
  logFile=fopen("main.log", "a");
  char msg[80];
  sprintf(msg, "return code for %s=%d\n", name, n);
  fprintf(logFile, msg);
  fclose(logFile);
}

static char child_stack[1024 * 1024];

int child_main(void *arg) {
  logn("entering child_main()");

  logd("unshare", unshare(CLONE_NEWNS));
  // umount2("/proc", MNT_DETACH);
  if (mount(NULL, "/", NULL, MS_REC | MS_PRIVATE, NULL) == -1) errExit("mount-MS_PRIVATE");

  /* Pivot root */
  // mount("./rootfs", "./rootfs", "bind", MS_BIND | MS_REC, "");
  if (mount("./rootfs", "./rootfs", NULL, MS_BIND, NULL) == -1) errExit("mount-MS_BIND");

  mkdir("./rootfs/oldrootfs", 0755);

  logn("before pivot_root");
  if (syscall(SYS_pivot_root, "./rootfs", "./rootfs/oldrootfs") == -1) errExit("error-pivotroot");
  logn("after pivot_root");

  if (chdir("/") == -1) errExit("chdir");
  umount2("/oldrootfs", MNT_DETACH);
  rmdir("/oldrootfs");

  logn("before remount");

  /* Re-mount procfs */
  if (mount("proc", "/proc", "proc", 0, NULL) == -1) errExit("mount-proc");
  if (mount("sysfs", "/sys", "sysfs", 0, NULL) == -1) errExit("mount-sys");
  if (mount("none", "/tmp", "tmpfs", 0, NULL) == -1) errExit("mount-tmp");
  if (mount("none", "/dev", "tmpfs", MS_NOSUID | MS_STRICTATIME, NULL) == -1) errExit("mount-dev");

  logn("after remount");

  sethostname("example", 7);
  logn("after sethostname");

  char **argv = (char **)arg;

  logn("child execute ....");
  execvp(argv[0], argv);
  logn("child exit....");
  return 0;
}

int main(int argc, char *argv[]) {
  logn("enter main()");

  logn("before clone()");
  int flags = CLONE_NEWNS | CLONE_NEWUTS | CLONE_NEWPID | CLONE_NEWIPC | CLONE_NEWNET;
  int pid = clone(child_main, child_stack + sizeof(child_stack), flags | SIGCHLD, argv + 1);

  logn("after clone()");
  if (pid < 0) {
    fprintf(stderr, "clone failed: %d\n", errno);
    return 1;
  }

  logd("child pid", pid);
  char slirp[4096];
  snprintf(slirp, sizeof(slirp) -1, "Please run 'sudo slirp4netns -c %d tap0' in another tab", pid);
  logn(slirp);

  logn("before waitpid");
  waitpid(pid, NULL, 0);
  return 0;
}
