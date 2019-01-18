#include "myrcp.h"



int cpf2f(char *f1, char *f2)
{
  // printf("Copy file to file\n");
  bool file2exists = true;

  struct stat f1s = {0};
  struct stat f2s = {0};

  int fd1 = 0;
  int fd2 = 0;

  char buf[4096];
  int BLKSIZE = 4096;
  int n;



  lstat(f1, &f1s);
  if(((lstat(f2, &f2s)) < 0))
  {
    file2exists = false;
  }

  if(file2exists)
  {
    if((f1s.st_dev == f2s.st_dev) && (f1s.st_ino == f2s.st_ino))
    {
      printf("Two files are the same\n");
      exit(1);
    }
    if((f1s.st_mode & S_IFMT) == S_IFLNK)
    {
      printf("Cannot copy link to existing file\n");
      exit(1);
    }
  }
  else
  {
    if((f1s.st_mode & S_IFMT) == S_IFLNK)
    {
      return symlink(f1, f2);
    }
  }



  fd1 = open(f1, O_RDONLY);
  if(fd1 < 0)
  {
    printf("File %s could not be opened\n", f1);
    exit(1);
  }
  fd2 = open(f2, O_WRONLY | O_CREAT | O_TRUNC);
  if(fd2 < 0)
  {
    printf("File %s could not be opened", f2);
    exit(1);
  }

  chmod(f2, f1s.st_mode);
  while((n = read(fd1, buf, BLKSIZE)))
    write(fd2, buf, n);

  return 0;
}


int cpf2d(char *f1, char *f2)
{
  // printf("Copy file to directory\n");
  DIR *dest;
  struct dirent *dp;
  char *x = "";
  char *f1spare = "";

  char *new_path = "";
  struct stat new_file = {0};

  if(!(dest = opendir(f2)))
  {
    printf("ERROR: Unable to open directory %s\n", f2);
    exit(1);
  }

  f1spare = strdup(f1);
  x = basename(f1spare);

  while((dp = readdir(dest)))
  {
    if(!(strcmp(dp->d_name, ".") == 0) && !(strcmp(dp->d_name, "..") == 0))
    {
      if((strcmp(dp->d_name, x) == 0))
      {
        //is file or dir already exists
        new_path = strcat(f2, "/");
        strcat(new_path, x);

        lstat(new_path, &new_file);
        if((new_file.st_mode & S_IFMT) == S_IFDIR)
        {
          return cpf2d(f1, new_path);
        }
        else
        {
          return cpf2f(f1, new_path);
        }
      }
    }
  }

  new_path = strcat(f2, "/");
  strcat(new_path, x);
  return cpf2f(f1, new_path);

  return 0;
}


int cpd2d(char *f1, char *f2)
{
  // printf("Copy directory to directory\n");

  DIR *src;
  struct dirent *dp;
  struct stat srcst;

  char *f1spare, *f2spare, *new_path_dest, *new_path_src;

  if(strncmp(f1, f2, strlen(f1)) == 0 && ((f2[strlen(f1)] == '/') || (f2[strlen(f1) - 1] == '\0')))
  {
    printf("ERROR: Can't copy directory into itself\n");
    exit(1);
  }

  src = opendir(f1);
  if(!src)
  {
    printf("ERROR: Unable to open directory %s\n", f1);
    exit(1);
  }

  while((dp = readdir(src)))
  {
    if(!(strcmp(dp->d_name, ".") == 0) && !(strcmp(dp->d_name, "..") == 0))
    {
      f1spare = strdup(f1);
      f2spare = strdup(f2);

      new_path_src = strcat(f1spare, "/");
      strcat(new_path_src, dp->d_name);

      new_path_dest = strcat(f2spare, "/");
      strcat(new_path_dest, dp->d_name);

      lstat(new_path_src, &srcst);

      if((srcst.st_mode & S_IFMT) == S_IFDIR)
      {
        mkdir(new_path_dest, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        cpd2d(new_path_src, new_path_dest);
      } 
      else
      {
        cpf2f(new_path_src, new_path_dest);
      }
      free(f1spare);
      free(f2spare);
    }
  }
  return 0;
}

int myrcp(char *f1, char *f2)
{
  struct stat f1s;
  struct stat f2s;

  char *new_path = "";
  bool file2exists = true;

  if(lstat(f1, &f1s) < 0)
  {
    printf("ERROR: File %s does not exist\n", f1);
    exit(1);
  }
  switch(f1s.st_mode & S_IFMT)
  {
    case S_IFDIR:
    case S_IFREG:
    case S_IFLNK:
      break;
    default:
      printf("ERROR: File %s invalid file type\n", f1);
      exit(1);
  }
  if(!(lstat(f2, &f2s) < 0))
  {
    if( !((f2s.st_mode & S_IFMT) == S_IFREG) && !((f2s.st_mode & S_IFMT) == S_IFLNK) && !((f2s.st_mode & S_IFMT) == S_IFDIR))
    {
      printf("ERROR: File %s invalid file type\n", f2);
      exit(1);
    } 
  }
  else
  {
    file2exists = false;
  }

  if(((f1s.st_mode & S_IFMT) == S_IFREG) || ((f1s.st_mode & S_IFMT) == S_IFLNK))
  { 
    if(file2exists)
    {
      if(!((f2s.st_mode & S_IFMT) == S_IFDIR))
      {
        return cpf2f(f1, f2);
      }
      else
      {
        return cpf2d(f1, f2);
      }
    }
    return cpf2f(f1, f2);
  }
  if((f1s.st_mode & S_IFMT) == S_IFDIR)
  {
    if(file2exists)
    {
      if(!((f2s.st_mode & S_IFMT) == S_IFDIR))
      {
        printf("ERROR: Cannot copy directory %s into file %s\n", f1, f2);
        exit(1);
      }
      new_path = strcat(f2, "/");
      strcat(new_path, basename(f1));
      mkdir(new_path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
      return cpd2d(f1, new_path);
    }
    else
    {
      mkdir(f2, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    }
    return cpd2d(f1, f2);
  }
return 0;
}