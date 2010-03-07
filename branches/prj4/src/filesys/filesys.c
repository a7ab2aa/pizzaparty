#include "filesys/filesys.h"
#include <debug.h>
#include <stdio.h>
#include <string.h>
#include "filesys/file.h"
#include "filesys/free-map.h"
#include "filesys/inode.h"
#include "filesys/directory.h"
#include "threads/thread.h"
#include "filesys/cache.h"

/* Partition that contains the file system. */
struct block *fs_device;

static void do_format (void);

/* Initializes the file system module.
   If FORMAT is true, reformats the file system. */
void
filesys_init (bool format) 
{
  fs_device = block_get_role (BLOCK_FILESYS);
  if (fs_device == NULL)
    PANIC ("No file system device found, can't initialize file system.");

  inode_init ();
  free_map_init ();

  if (format) 
    do_format ();

  free_map_open ();
}

/* Shuts down the file system module, writing any unwritten data
   to disk. */
void
filesys_done (void) 
{
  free_map_close ();
}

/* Creates a file named NAME with the given INITIAL_SIZE.
   Returns true if successful, false otherwise.
   Fails if a file named NAME already exists,
   or if internal memory allocation fails. */
bool
filesys_create (const char *name, off_t initial_size) 
{
  block_sector_t inode_sector = 0;
  struct dir *dir = dir_open_root ();
  char *file_name = malloc (sizeof (char)*(strlen(name)+1));
  strlcpy (file_name,name,strlen(name)+1);
//  bool success = (dir != NULL
//                  && free_map_allocate (1, &inode_sector));
//  printf ("free_map_allocate done\n");
//  success = success && inode_create (inode_sector, initial_size);
//  printf ("inode_create done\n");
//  success = success && dir_add (dir, name, inode_sector);
//  printf ("dir done\n");

  if (strlen(name)> NAME_MAX)
  return false;
  char *token1, *token2, *save_ptr, temp[NAME_MAX + 1];
  
  bool success = true;
  
  if (file_name != NULL)
  {
  token1 = strtok_r (file_name, "/", &save_ptr);
 // printf ("%s\n", token1);
    for (token2 = strtok_r (NULL, "/", &save_ptr); token2 != NULL; token2 = strtok_r (NULL, "/", &save_ptr))
   {
 //   printf ("%s\n", token2);
    struct inode *inode = NULL;
    strlcpy (temp, token1, sizeof temp);
    success = dir_lookup (dir, temp, &inode);
     
     if (!success)
     {
    break;
       
     }
     else 
       {   
           
           dir_close (dir);
           dir = dir_open (inode);   
       }

     token1 = token2;
    }
  }
  
  
  if (dir == NULL)
  dir = dir_open_root ();
   strlcpy (temp, token1, sizeof temp);
  if (success)
  success = (dir != NULL
                  && free_map_allocate (1, &inode_sector)
                  && inode_create (inode_sector, initial_size, 0)
                  && dir_add (dir, temp, inode_sector));




  if (!success && inode_sector != 0) 
  {
//    printf ("success = %ld, inode_sector = %ld\n", success, inode_sector);
    free_map_release (inode_sector, 1);
  }
  dir_close (dir);

  return success;
}


struct file *
filesys_open (const char *name)
{
  struct dir *dir = dir_open_root ();
  struct inode *inode = NULL;

  if (dir != NULL)
    dir_lookup (dir, name, &inode);
  dir_close (dir);

  return file_open (inode);
}


/* Opens the file with the given NAME.
   Returns the new file if successful or a null pointer
   otherwise.
   Fails if no file named NAME exists,
   or if an internal memory allocation fails. */
struct file_info * 
filesys_open_file (const char *name)
{
  struct thread *t = thread_current ();
  struct dir *dir; 
  struct inode *inode = NULL;
  struct file_info * f_info = (struct file_info *) malloc (sizeof (struct file_info));

  if (t->current_dir != NULL)
    dir = dir_reopen(t->current_dir);
  if (dir == NULL)
  {
    dir = dir_open_root ();
  }

  if (dir != NULL)
    dir_lookup (dir, name, &inode);
    dir_close (dir);
   
    if (inode == NULL)
    {
     return NULL;
    }
    
    off_t isdir = inode_isdir (inode);

    if (isdir)
    {
       f_info->p_dir = dir_open (inode);
       f_info->p_file = NULL;
    }
    else
    {
       f_info->p_file = file_open (inode);
    }
    return f_info;

}

/* Deletes the file named NAME.
   Returns true if successful, false on failure.
   Fails if no file named NAME exists,
   or if an internal memory allocation fails. */
bool
filesys_remove (const char *name) 
{
  struct thread * t = thread_current ();
  struct dir *dir = dir_reopen (t->current_dir);
  if (dir == NULL)
  {
    dir = dir_open_root ();
  }
  bool success = dir != NULL && dir_remove (dir, name);
  dir_close (dir); 

  return success;
}

/* Formats the file system. */
static void
do_format (void)
{
  printf ("Formatting file system...");
  free_map_create ();
  if (!dir_create (ROOT_DIR_SECTOR, ROOT_DIR_SECTOR, 16))
    PANIC ("root directory creation failed");
  free_map_close ();
  printf ("done.\n");
}