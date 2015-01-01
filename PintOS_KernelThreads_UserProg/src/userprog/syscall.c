#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include <user/syscall.h>
#include "devices/input.h"
#include "devices/shutdown.h"
#include "filesys/file.h"
#include "filesys/filesys.h"
#include "threads/interrupt.h"
#include "threads/synch.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "userprog/pagedir.h"
#include "userprog/process.h"

/* Modified */
int arg1,arg2,arg3;
void *mapped;
struct lock inside_filesys;

static void syscall_handler (struct intr_frame *);

/*Ensuring the address is between 0x08048000 and PHYS_BASE
  based on the user virtual memory.  */
bool
is_valid_addr(void* addr){
  if(is_user_vaddr(addr) && (addr >= (void *) 0x08048000))
    return true;
  else 
    return false;
}

void check_valid_ptr (const void *vaddr)
{
  if (!is_valid_addr(vaddr))
  {
    exit(-1);
  }
}

bool
is_valid_buffer(void * buffer, unsigned size)
{
  void *b = buffer;
  unsigned i = 0;
  for( i = 0; i < size; i ++)
  {
    if(!is_valid_addr(b))
      return false;
    b++;
  }
  return true;
}

int 
get_arg_i(void *esp, int i)
{
  int * p = (int *)esp +i;
  if(is_valid_addr((void *)p))
    return *p;
  else 
    exit(-1);
}

void
syscall_init (void) 
{
  lock_init(&inside_filesys);
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  if(is_valid_addr(f->esp) && is_valid_addr(f->eip))
  {
    switch (* (int *) f->esp)
    {
      case SYS_HALT:
      {
        halt(); 
	break;
      }
      case SYS_EXIT:
      {
        arg1 = get_arg_i(f->esp, 1);
	exit(arg1);
	break;
      }
      case SYS_EXEC:
      {
        arg1 = get_arg_i(f->esp, 1);
	mapped = pagedir_get_page(thread_current()->pagedir, (void *)arg1);
	if(!mapped)
          exit(-1);
	arg1 = (int) mapped;
	f->eax = exec((const char *) arg1); 
	break;
      }
      case SYS_WAIT:
      {
        arg1 = get_arg_i(f->esp, 1);
        f->eax = wait(arg1);
	break;
      }
      case SYS_CREATE:
      {
        arg1 = get_arg_i(f->esp, 1);
	mapped = pagedir_get_page(thread_current()->pagedir, (void *)arg1);
	if(!mapped)
          exit(-1);
	arg1 = (int)(mapped);
	arg2 = get_arg_i(f->esp, 2);
	f->eax = create((const char *)arg1, (unsigned) arg2);
	break;
      }
      case SYS_REMOVE:
      {
        arg1 = get_arg_i(f->esp, 1);
        mapped = pagedir_get_page(thread_current()->pagedir, (void *)arg1);
	if(!mapped)
	  exit(-1);
	arg1 = (int)mapped;
	f->eax = remove((const char *) arg1);
	break;
      }
      case SYS_OPEN: 
     {
       arg1 = get_arg_i(f->esp, 1);        
       mapped = pagedir_get_page(thread_current()->pagedir, (void *)arg1);
       if(!mapped)
         exit(-1);
       arg1 = (int)mapped;
       f->eax = open((const char *) arg1);
       break; 		
     }
     case SYS_FILESIZE:
     {
       arg1 = get_arg_i(f->esp, 1);
       f->eax = filesize(arg1);
       break;
     }
     case SYS_READ:
     {
       arg1 = get_arg_i(f->esp, 1);
       arg2 = get_arg_i(f->esp, 2);
       arg3 = get_arg_i(f->esp, 3);
       if(!is_valid_buffer((void *) arg2, (unsigned) arg3))
       {
         exit(-1);
       }
       mapped = pagedir_get_page(thread_current()->pagedir, (void *)arg2);
       if(!mapped)
         exit(-1); 
       arg2 = (int) mapped;
       f->eax = read(arg1, (void *) arg2, (unsigned) arg3);
       break;
     }
     case SYS_WRITE:
     { 
       arg1 = get_arg_i(f->esp, 1);
       arg2 = get_arg_i(f->esp, 2);
       arg3 = get_arg_i(f->esp, 3);
       if(!is_valid_buffer((void *)arg2, (unsigned)arg3))
       {
         exit(-1);
       }
       mapped = pagedir_get_page(thread_current()->pagedir, (void *)arg2);
       if(!mapped)
         exit(-1); 
       arg2 = (int) mapped;
       f->eax = write(arg1, (const void *)arg2, (unsigned)arg3);
       break;
     }
     case SYS_SEEK:
     {
       arg1 = get_arg_i(f->esp, 1);
       arg2 = get_arg_i(f->esp, 2);
       seek(arg1, (unsigned)arg2);
       break;
     } 
     case SYS_TELL:
     { 
       arg1 = get_arg_i(f->esp, 1);
       f->eax = tell(arg1);
       break;
     }
     case SYS_CLOSE:
     { 
       arg1 = get_arg_i(f->esp,1);
       close(arg1);
       break;
     }
   }
  }
  else 
    exit(-1);
}

void halt (void)
{
  shutdown_power_off();
}

void exit(int status)
{
 
  thread_current()->exit_status= status;
  printf ("%s: exit(%d)\n", thread_name(), status);
  file_close(thread_current()->file);
  thread_exit();
}

pid_t exec (const char *cmd_line)
{
   lock_acquire(&inside_filesys);

   pid_t pid = process_execute(cmd_line);
  if(pid == -1)
  {
    lock_release(&inside_filesys);
   return -1;
  }

  struct thread* child = get_child(pid);
  sema_down(&child->load_semaphore);
  if (child->loaded == false)
    {
      lock_release(&inside_filesys);
      return -1;
    }
    lock_release(&inside_filesys);
  return pid;
}
int wait (pid_t pid)
{

  int state = process_wait(pid);

  return state ;
}

bool create (const char *file, unsigned initial_size)
{
  lock_acquire(&inside_filesys);
  bool created = filesys_create(file, initial_size);
  lock_release(&inside_filesys);
  return created;
}

bool remove (const char *file)
{
  lock_acquire(&inside_filesys);
  bool removed = filesys_remove(file);
  lock_release(&inside_filesys);
  return removed;
}

int open (const char *file)
{
  lock_acquire(&inside_filesys);
  struct file *f = filesys_open(file);
  if (f == NULL)
  {
    lock_release(&inside_filesys);
    return -1;
  }
  int file_desc = add_thread_file(f);
  lock_release(&inside_filesys);
  return file_desc;
}

int filesize (int fd)
{
  lock_acquire(&inside_filesys);
  struct file *f = get_thread_file(fd);
  if (f == NULL)
  {
    lock_release(&inside_filesys);
    return -1;
  }
  int size = file_length(f);
  lock_release(&inside_filesys);
  return size;
}

int read (int fd, void *buffer, unsigned size)
{
  lock_acquire(&inside_filesys);
  if (fd == 0) //Input from keyboard
  {
    unsigned i;
    uint8_t* local_buffer = (uint8_t *) buffer;
    for (i = 0; i < size; i++)
    {
      local_buffer[i] = input_getc(); 
    }
    lock_release(&inside_filesys);
    return size;
   }

  struct file *f = get_thread_file(fd);
  if (f == NULL)
    {
      lock_release(&inside_filesys);
      return -1;
    }
  int bytes = file_read(f, buffer, size);
  lock_release(&inside_filesys);
  return bytes;
}

int write (int fd, const void *buffer, unsigned size)
{
  lock_acquire(&inside_filesys);
  if (fd == 1)  //Output to console
    {
      putbuf(buffer, size);
      lock_release(&inside_filesys);
      return size;
    }
  struct file *f = get_thread_file(fd);
  if (f == NULL)
    {
      lock_release(&inside_filesys);
      return -1;
    }
  int bytes = file_write(f, buffer, size);
  lock_release(&inside_filesys);
  return bytes;
}

void seek (int fd, unsigned position)
{
  lock_acquire(&inside_filesys);
  struct file *f = get_thread_file(fd);
  if (f == NULL)
  {
    lock_release(&inside_filesys);
    return;
  }
  file_seek(f, position);
  lock_release(&inside_filesys);
}

unsigned tell (int fd)
{
  lock_acquire(&inside_filesys);
  struct file *f = get_thread_file(fd);
  if (f == NULL)
  {
    lock_release(&inside_filesys);
    return -1;
  }
  off_t offset = file_tell(f);
  lock_release(&inside_filesys);
  return offset;
}

void close (int fd)
{
  lock_acquire(&inside_filesys);
  close_thread_file(fd);
  lock_release(&inside_filesys);
}


