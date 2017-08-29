
/**
 * File: asgn1.c
 * Date: 13/03/2011
 * Author: Your Name 
 * Version: 0.1
 *
 * This is a module which serves as a virtual ramdisk which disk size is
 * limited by the amount of memory available and serves as the requirement for
 * COSC440 assignment 1 in 2012.
 *
 * Note: multiple devices and concurrent modules are not supported in this
 *       version.
 */
 
/* This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/list.h>
#include <asm/uaccess.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/device.h>
#include <linux/sched.h>

#define MYDEV_NAME "asgn1"
#define MYIOC_TYPE 'k'
#define SET_NPROC_OP 1
#define TEM_SET_NPROC _IOW(MYIOC_TYPE, SET_NPROC_OP, int) 

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("COSC440 asgn1");


/**
 * The node structure for the memory page linked list.
 */ 
typedef struct page_node_rec {
  struct list_head list;
  struct page *page;
} page_node;

typedef struct asgn1_dev_t {
  dev_t dev;            /* the device */
  struct cdev *cdev;
  struct list_head mem_list; 
  int num_pages;        /* number of memory pages this module currently holds */
  size_t data_size;     /* total data size in this module */
  atomic_t nprocs;      /* number of processes accessing this device */ 
  atomic_t max_nprocs;  /* max number of processes accessing this device */
  struct kmem_cache *cache;      /* cache memory */
  struct class *class;     /* the udev class */
  struct device *device;   /* the udev device node */
} asgn1_dev;

asgn1_dev asgn1_device;


int asgn1_major = 0;                      /* major number of module */  
int asgn1_minor = 0;
int asgn1_dev_count = 1;                  /* number of devices */

void free_memory_pages(void) {
  page_node *curr;
  page_node *temp;
  /* COMPLETE ME */
  /**
   * Loop through the entire page list {
   *   if (node has a page) {
   *     free the page
   *   }
   *   remove the node from the page list
   *   free the node
   * }
   * reset device data size, and num_pages
   */  	
       //loop through the list
       list_for_each_entry_safe(curr,temp,&asgn1_device.mem_list,list){
		if(curr->page != NULL){
			//free the page if there is one
			__free_page(curr->page);
		}
		//delete entry from list
		list_del(&curr->list);
		//free entry
		kfree(curr);	
	}

	/*reset device data size*/
	asgn1_device.data_size = 0;
	/*reset num_pages*/
	asgn1_device.num_pages=0;
	
}


int asgn1_open(struct inode *inode, struct file *filp) {
  /* COMPLETE ME */ 

  /**
   * Increment process count, if exceeds max_nprocs, return -EBUSY
   *
   * if opened in write-only mode, free all memory pages
   *
   */
   
	//increase nprocs.
	atomic_inc(&asgn1_device.nprocs);
	//if reachs limit, return EBUSY and reverse the change
	if(atomic_read(&asgn1_device.nprocs)>atomic_read(&asgn1_device.max_nprocs)){
		atomic_dec(&asgn1_device.nprocs);
		return -EBUSY;
	} /*             */  

		//if  in wite only mode, free all pages
		if(filp->f_mode==FMODE_WRITE)free_memory_pages();
  printk(KERN_WARNING"accessing open: current proc number: %d",atomic_read(&asgn1_device.nprocs));

  return 0; /* success */
}



/**
 * This function releases the virtual disk, but nothing needs to be done
 * in this case. 
 */
int asgn1_release (struct inode *inode, struct file *filp) {
  /* COMPLETE ME */
  /**
   * decrement process count
   */
 //decrease the nprocs. make sure this number is aloway positive. This number should never be negative so this if statement is not likely to be flase.
 if(atomic_read(&asgn1_device.nprocs)>0)atomic_dec(&asgn1_device.nprocs);
  printk(KERN_WARNING"accessing release");
		return -EINVAL;
}
/**
read here
*/
ssize_t asgn1_read(struct file *filp, char __user *buf, size_t count,
		 loff_t *f_pos) {
  size_t size_read = 0;     /* size read from virtual disk in this function */
  size_t begin_offset=*f_pos%PAGE_SIZE;      /* the offset from the beginning of a page to
			       start reading */
  int begin_page_no = *f_pos / PAGE_SIZE; /* the first page which contains
					  goto fail_device;
     the requested data */
  int curr_page_no = 0;     /* the current page number */
  size_t curr_size_read;    /* size read from the virtual disk in this round */
  size_t size_to_be_read;   /* size to be read in the current round in 
			       while loop */

  struct list_head *ptr = asgn1_device.mem_list.next;
  page_node *curr;
  int fails=0;  
  printk(KERN_WARNING"accessing read\n");
  /* COMPLETE ME */
  /**
   * check f_pos, if beyond data_size, return 0
R(asgn1_device.dev);

   * Traverse the list, once the first requested page is reached,
   *   - use copy_to_user to copy the data to the user-space buf page by page
   *   - you also need to work out the start / end offset within a page
   *   - Also needs to handle the situation where copy_to_user copy less
   *       data than requested, and
   *       copy_to_user should be called again to copy the rest of the
   *       unprocessed data, and the second and subsequent calls still
   *       need to check whether copy_to_user copies all data requested.
   *       This is best done by a while / do-while loop.
   *
   * if end of data area of ramdisk reached before copying the requested
   *   return the size copied to the user space so far
   */
 //if the file is reached it's end, return 0;
  if(*f_pos>=asgn1_device.num_pages*PAGE_SIZE)return 0;
  printk(KERN_INFO"fpos:%d vs data: %d; pageNum: %d",*f_pos,asgn1_device.num_pages*PAGE_SIZE,asgn1_device.num_pages);
  printk(KERN_INFO"begine_page: %d; ",begin_page_no);
  //loop through the list
  list_for_each(ptr,&asgn1_device.mem_list){
      //start reading when begin page reached.
      if(curr_page_no>=begin_page_no){
      //get current entry
          curr=list_entry(ptr,page_node,list);
  	  //only the begin page have offset, otherwise set offset to zero
	  if(curr_page_no!=begin_page_no)begin_offset=0;

	  //size to be read should either be a whole page or some left overs.
	  size_to_be_read=count>PAGE_SIZE-begin_offset? PAGE_SIZE-begin_offset:count;


	  //keep reading the whole data until success.
	  //this is not the most efficient way to do. should only repeat read on those data that is fail to read.
	  do{fails=copy_to_user(buf+size_read,page_address(curr->page)+begin_offset,size_to_be_read);printk(KERN_INFO"number of fails: %d",fails)    ;}while(fails>0);
	  if(fails<0)return fails;
          //reduce count, this represent the data left to read
          count-=size_to_be_read;
	  //accumulate size  read so far
	  size_read+=size_to_be_read;
	  //if all read, jump out the loop
	 
	  
	  if(count==0)break;
      }
      curr_page_no++;
  }
  printk(KERN_INFO"read: %d",size_read);
  //update f_pos
  *f_pos+=size_read;
  return size_read;
}




static loff_t asgn1_lseek (struct file *file, loff_t offset, int cmd)
{
    loff_t testpos;

    size_t buffer_size = asgn1_device.num_pages * PAGE_SIZE;

    /* COMPLETE ME */
    /**
     * set testpos according to the command
     *
     * if testpos larger than buffer_size, set testpos to buffer_size
     * 
     * if testpos smaller than 0, set testpos to 0
     *
     * set file->f_pos to testpos
     */
	switch(cmd){
	case SEEK_SET:
		testpos = offset;
		break;
	case SEEK_CUR:
		testpos = file->f_pos+offset;
		break;
	case SEEK_END:
		testpos = buffer_size+offset;
		break;
	default:
		return -EINVAL;
	}
	//do some error check
	testpos = testpos >buffer_size? buffer_size : testpos;
	testpos = testpos<0 ? 0 : testpos;
	//update f_pos
	file->f_pos = testpos;
    printk (KERN_INFO "Seeking to pos=%ld\n", (long)testpos);
    return testpos;
}

/**
 * This function writes from the user buffer to the virtual disk of this
 * module
 */
ssize_t write(struct file *filp, const char __user *buf, size_t count,
		  loff_t *f_pos) {
 
 
 /**
 
 THIS IS VERY SIMILIAR To READ
 
 */
 
 
 
 size_t orig_f_pos = *f_pos;  /* the original file position */
  size_t size_written = 0;  /* size written to virtual disk in this function */
  size_t begin_offset=*f_pos % PAGE_SIZE;      /* the offset from the beginning of a page to
			       start writing */
 
  int begin_page_no = *f_pos / PAGE_SIZE;  /* the first page this finction
					      should start writing to */
 
  int curr_page_no = 0;     /* the current page number */
  size_t curr_size_written; /* size written to virtual disk in this round */
  size_t size_to_be_written;  /* size to be read in the current round in 
				 while loop */
  
  struct list_head *ptr = asgn1_device.mem_list.next;
  page_node *curr;
  int fails=0;
  /* COMPLETE ME */
  /**
   * Traverse the list until the first page reached, and add nodes if necessary
   *
   * Then write the data page by page, remember to handle the situation
   *   when copy_from_user() writes less than the amount you requested.
   *   a while loop / do-while loop is recommended to handle this situation. 
   */
  printk(KERN_INFO"fpos:%d vs data: %d; pageNum: %d",*f_pos,asgn1_device.num_pages*PAGE_SIZE,asgn1_device.num_pages);
  //reach end 
  if(*f_pos>=asgn1_device.num_pages*PAGE_SIZE)return 0;
  printk(KERN_INFO"accessing write; ");
 
  list_for_each(ptr,&asgn1_device.mem_list){
  	
	if(curr_page_no>=begin_page_no){
	  //get entry
	  curr=list_entry(ptr,page_node,list);
	  //offset thing
	  if(curr_page_no!=begin_page_no)begin_offset=0;

	  //size to be read
	  size_to_be_written=count>PAGE_SIZE-begin_offset? PAGE_SIZE-begin_offset:count;


	  
	  
	  //loop until success
	  do{fails=copy_from_user(page_address(curr->page)+begin_offset,buf+size_written,size_to_be_written);printk(KERN_INFO"number of fails: %d",fails);}while(fails>0);	  
	  if(fails<0)return fails;
	  //change count
	  count-=size_to_be_written;
	  size_written+=size_to_be_written;

	  if(count==0)break;
	}
	curr_page_no++;
  }
  

  //update data size
  asgn1_device.data_size = max(asgn1_device.data_size,
                               orig_f_pos + size_written);
  printk(KERN_INFO"wrote: %d",size_written);
 
  *f_pos+=size_written;
  
  return size_written;
}
ssize_t asgn1_write(struct file *filp, const char __user *buf, size_t count,
                   loff_t *f_pos){
         /*
	   This method is use to write file, which envokes write();
	   if write() is not finish, this means no enough paper in the list.
	   add papers to the list and envoke write() again;
	 **/
         //get the number of writes, should be same as count if page is enough
         ssize_t wrote=write(filp,buf,count,f_pos);
         
         if(wrote==count)return wrote;
         //if not the same, add pages
	 //caculate data left to write
         size_t dataleft=count-wrote;
	 //find out how many pages we need to hold the data
         int pages = dataleft/PAGE_SIZE;
         if(dataleft%PAGE_SIZE!=0)pages++;
         printk(KERN_WARNING"<W>add pages: %d",pages);
         //make pages and add to list
	 //However, {add only one page, envoke write, check effect 
	 //to decide weather exit or start over again} can be a better desgine to make sure always 
	 //at least write something new.
	 for(;pages>0;pages--){
                 page_node *p;
                 p=kmalloc(sizeof(page_node),GFP_KERNEL);
                 if(p==NULL)break;
                 p->page=alloc_page(GFP_KERNEL);
                 list_add_tail(&p->list,&asgn1_device.mem_list);
         	 asgn1_device.num_pages+=1;
	 }
	 //write again.
         wrote+=write(filp,buf,count,f_pos);
         return wrote;
}

#define SET_NPROC_OP 1
#define GET_MAJOR 2
#define GET_DATASIZE 3
#define TEM_SET_NPROC _IOW(MYIOC_TYPE, SET_NPROC_OP, int) 

/**
 * The ioctl function, which nothing needs to be done in this case.
 */
long asgn1_ioctl (struct file *filp, unsigned cmd, unsigned long arg) {
  int nr;
  int new_nprocs=0;
  int result;

  /* COMPLETE ME */
  /** 
   * check whether cmd is for our device, if not for us, return -EINVAL 
   *
   * get command, and if command is SET_NPROC_OP, then get the data, and
     set max_nprocs accordingly, don't forget to check validity of the 
     value before setting max_nprocs
   */
  printk(KERN_WARNING"enter ioctl");
  //if command is not for this device, return
  if(_IOC_TYPE(cmd)!=MYIOC_TYPE)return -EINVAL;
  nr=_IOC_NR(cmd);
  
  switch(nr){
  	
	case SET_NPROC_OP:
		//cast the arg to pointer and read data from that
		result=copy_from_user((int*)&new_nprocs, (int *)arg, sizeof(int));
		//check vality
		if(result!=0)return -EINVAL;
		//update max_nprocs if the new value is resonable
		if(new_nprocs>1 && new_nprocs>atomic_read(&asgn1_device.nprocs)){
			atomic_set(&asgn1_device.max_nprocs,new_nprocs);
			printk(KERN_INFO"new value: %d",new_nprocs);
			return 0;
		}
	break;
	//return major number.
	case GET_MAJOR:
		result=copy_to_user((int*)arg, (int*)&asgn1_major,sizeof(int));
		if(result!=0)return -EINVAL;
		else return 0;
  	case GET_DATASIZE:
		result=copy_to_user((int*)arg, (int*)&asgn1_device.data_size, sizeof(int));
		if(result!=0)return -EINVAL;
		else return 0;
	default: return -EINVAL;
  }
  
  return -ENOTTY;
}


static int asgn1_mmap (struct file *filp, struct vm_area_struct *vma)
{
    unsigned long pfn;
    unsigned long offset = vma->vm_pgoff << PAGE_SHIFT;
    unsigned long len = vma->vm_end - vma->vm_start;
    unsigned long ramdisk_size = asgn1_device.num_pages * PAGE_SIZE;
    page_node *curr;
    unsigned long index = 0;

    /* COMPLETE ME */
    /**
     * check offset and len
     *
     * loop through the entire page list, once the first requested page
     *   reached, add each page with remap_pfn_range one by one
     *   up to the last requested page
     */
     printk(KERN_WARNING"enter mmp. offset: %ld, len: %ld, ramdisk_size: %ld",offset,len,ramdisk_size);
     printk(KERN_WARNING"vma info: vma_start: %d, vma_end: %d, vma_pgoff:%ld",vma->vm_start,vma->vm_end, vma->vm_pgoff);
    //out of range check.
    if(offset*PAGE_SIZE+len > ramdisk_size)return -1;
    list_for_each_entry(curr,&asgn1_device.mem_list,list){
    	if(index>=offset){
	//when first page reached, start operation
		printk(KERN_INFO"%d, start map",index);
		//map memory based on offset, etc
		if(remap_pfn_range(vma,vma->vm_start+PAGE_SIZE*(index-offset),page_to_pfn(curr->page),PAGE_SIZE,vma->vm_page_prot))return -EAGAIN;
		printk(KERN_INFO"%d, done map",index);
		//if map reaches length user wants, reutrn
		if((index-offset)*PAGE_SIZE>=len)return 0;
	}
	index++;
    }
    return 0;
}


struct file_operations asgn1_fops = {
  .owner = THIS_MODULE,
  .read = asgn1_read,
  .write = asgn1_write,
  .unlocked_ioctl = asgn1_ioctl,
  .open = asgn1_open,
  .mmap = asgn1_mmap,
  .release = asgn1_release,
  .llseek = asgn1_lseek
};


static void *my_seq_start(struct seq_file *s, loff_t *pos)
{
if(*pos >= 1) return NULL;
else return &asgn1_dev_count + *pos;
}
static void *my_seq_next(struct seq_file *s, void *v, loff_t *pos)
{
(*pos)++;
if(*pos >= 1) return NULL;
else return &asgn1_dev_count + *pos;
}
static void my_seq_stop(struct seq_file *s, void *v)
{
/* There's nothing to do here! */
}

int my_seq_show(struct seq_file *s, void *v) {
  /* COMPLETE ME */
  /**
   * use seq_printf to print some info to s
   */
  //print some device states.
  seq_printf(s,"Device Info:\n-Major_Number: %d\n-Num_Pages: %d\n-Data_Size: %zu\n-Nprocs: %d\n-Max_procs: %d\n",asgn1_major,asgn1_device.num_pages,asgn1_device.data_size,atomic_read(&asgn1_device.nprocs),atomic_read(&asgn1_device.max_nprocs));
  return 0;
}


static struct seq_operations my_seq_ops = {
.start = my_seq_start,
.next = my_seq_next,
.stop = my_seq_stop,
.show = my_seq_show
};

static int my_proc_open(struct inode *inode, struct file *filp)
{
    return seq_open(filp, &my_seq_ops);
}

struct file_operations asgn1_proc_ops = {
.owner = THIS_MODULE,
.open = my_proc_open,
.llseek = seq_lseek,
.read = seq_read,
.release = seq_release,
};



/**
 * Initialise the module and create the master device
 */
int __init asgn1_init_module(void){
  int result; 

  /* COMPLETE ME */
  /**
   * set nprocs and max_nprocs of the device
   *
   * allocate major number
   * allocate cdev, and set ops and owner field 
   * add cdev
   * initialize the page list
   * create proc entries
   */

   asgn1_device.cdev=NULL;
   asgn1_device.class=NULL;
   asgn1_device.device=NULL;
   printk(KERN_INFO"################################\n");
   printk(KERN_INFO"dev_t: %d\n",asgn1_device.dev);
   printk(KERN_INFO"cdev (*): %p\n",asgn1_device.cdev);
   printk(KERN_INFO"class (*): %p\n",asgn1_device.class);
   printk(KERN_INFO"device (*): %p\n",asgn1_device.device);
   printk(KERN_INFO"page_head: %d\n",asgn1_device.mem_list);
   printk(KERN_INFO"page_size: %d\n",PAGE_SIZE);
   printk(KERN_INFO"################################\n");
 
  //make dev
  result=alloc_chrdev_region(&asgn1_device.dev,0,1,"asgn1_device");
  //get major number
  asgn1_major = MAJOR(asgn1_device.dev);
  printk(KERN_WARNING "asgn1 region chrdev (dev_t)\n");
  if(result<0) {
  //if fail, return
  	printk(KERN_WARNING"asgn1 region dev_t fail");
	
	return result;
  }
  //make cdev
  asgn1_device.cdev=kmalloc(sizeof(struct cdev),GFP_KERNEL);
  if(asgn1_device.cdev==NULL){
  	printk(KERN_WARNING"no memory for cdev");
  	result=-1;
	goto fail_device;
  }
  //cdev init
  cdev_init(asgn1_device.cdev,&asgn1_fops);
  printk(KERN_WARNING"cdev init success");
  asgn1_device.cdev->owner=THIS_MODULE;
  if(cdev_add(asgn1_device.cdev,asgn1_device.dev,1)<0){
  	printk(KERN_WARNING"cdev add failed");
	result=-1;
	goto fail_device;
  }
  printk(KERN_WARNING"cdev added success");
  //make class
  asgn1_device.class = class_create(THIS_MODULE, MYDEV_NAME);
  printk(KERN_WARNING"class create\n");
  if (IS_ERR(asgn1_device.class)) {
        printk(KERN_WARNING"asgn1 class create fail");
	result=-1;
	goto fail_device;
  }
	
  //make device
  asgn1_device.device = device_create(asgn1_device.class, NULL, 
                                      asgn1_device.dev, "%s", MYDEV_NAME);
  printk(KERN_WARNING"device create");
  if (IS_ERR(asgn1_device.device)) {
    printk(KERN_WARNING "%s: can't create udev device\n", MYDEV_NAME);
    result=-1;
    goto fail_device;
  }
  
 //set device nprocs and max nprocs. I choose 5 to be the max number
 atomic_set(&asgn1_device.nprocs,0);

 atomic_set(&asgn1_device.max_nprocs,5);
 
 //create a proc. helps to debug.
 struct proc_dir_entry *file;
 file= proc_create("asgn1_device",0644,NULL,&asgn1_proc_ops);
 if(!file){
   result= -ENOMEM;
   goto fail_device;
 }

  //set data size, num pages.
  asgn1_device.num_pages=0;
  asgn1_device.data_size=0;

  printk(KERN_WARNING"fine before make page_head init;");
  //init list head.
  INIT_LIST_HEAD(&asgn1_device.mem_list);
	
 
  printk(KERN_WARNING "set up udev entry\n");
  printk(KERN_WARNING "Hello world from %s\n", MYDEV_NAME);
  return 0;
  
  /* cleanup code called when any of the initialization steps fail */
fail_device:
  /* COMPLETE ME */
  /* PLEASE PUT YOUR CLEANUP CODE HERE, IN REVERSE ORDER OF ALLOCATION */

  if(asgn1_device.device!=NULL){
    printk(KERN_INFO"try to remove device....");
    device_destroy(asgn1_device.class,asgn1_device.dev);
    printk(KERN_INFO"device removed!\n");  
  }
  if(asgn1_device.class!=NULL){
    printk(KERN_INFO"try to remove class....");
    class_destroy(asgn1_device.class);
    printk(KERN_INFO"class removed!\n");  
  }
  if(asgn1_device.cdev!=NULL){
    printk(KERN_INFO"try to remove cdev....");
    cdev_del(asgn1_device.cdev);
    printk(KERN_INFO"cdev removed!\n");
 } 
    printk("try to remvoe dev...");
    unregister_chrdev_region(asgn1_device.dev,1);
    return result;


}
/**
 * Finalise the module
 */
void __exit asgn1_exit_module(void){
  remove_proc_entry("asgn1_device",NULL);
  device_destroy(asgn1_device.class, asgn1_device.dev);
  class_destroy(asgn1_device.class);
  printk(KERN_WARNING "cleaned up udev entry\n");
  cdev_del(asgn1_device.cdev);
  printk(KERN_WARNING"deled cdev\n");
  kfree(asgn1_device.cdev);
  printk(KERN_WARNING"freed cdev\n");
  unregister_chrdev_region(asgn1_device.dev,1);
  printk(KERN_WARNING"unregisted dev\n");
  /* COMPLETE ME */
  /**
   * free all pages in the page list 
   * cleanup in reverse order
   */

  free_memory_pages();
  printk(KERN_WARNING "Good bye from %s\n", MYDEV_NAME);
}


module_init(asgn1_init_module);
module_exit(asgn1_exit_module);


