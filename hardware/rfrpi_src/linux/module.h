#define module_param_array_named()
struct  timespec {
    int t ;
    int tv_nsec ;
    int tv_sec ;
};
#define spinlock_t int
#define irqreturn_t int

#define printk()
#define KERN_INFO 1
#define GFP_KERNEL 1

#define ENOMEM 1
#define NULL 0
#define IRQF_SHARED 0
#define IRQF_TRIGGER_RISING 0
#define loff_t int
#define EFAULT 02
#define IRQ_HANDLED 0
#define IRQ_NONE    0
#define dev_t int
#define __init
#define __exit
#define EINVAL 0

#define MODULE_LICENSE()
#define MODULE_AUTHOR()



struct  TTHIS_MODULE {
    char * name;
};
struct TTHIS_MODULE * THIS_MODULE  ; 

struct inode {
    int i ;
};
struct timeval {
    int tv_sec ;
	 int tv_usec;
};
struct file {
    int i ;
    void * private_data ;
};

struct file_operations  {
	void * owner   ;
	void * open    ;
	void * release ;
	void *  read    ;
};

    struct cdev {
    int i ;
    void * private_data ;
};

struct timespec  timespec_sub(struct timespec  , struct timespec  );