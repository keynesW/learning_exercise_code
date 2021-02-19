/*******************************************************************
 *
 *     File Name    :      vser.c
 *     Author       :      Keynes 
 *     QQ           :      1446160257  
 *     Email        :      1446160257@qq.com 
 *     Blog�� :      http://www.    .com/ 
 *     Created Time :      Thu 07 Jan 2021 07:27:36 PM CST
 *     Description  :      Hello Word in C++ , GUN-style
 *
 ******************************************************************/

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>

#include <linux/fs.h> 
#include <linux/cdev.h>
#include <linux/kfifo.h>

#define VSER_MAJOR 256
#define VSER_MINOR 0
#define VSER_DEV_CNT 2
#define VSER_DEV_NAME "vser"

static struct cdev vsdev;
static DEFINE_KFIFO(vsfifo0,char,32);
static DEFINE_KFIFO(vsfifo1,char,32);

static int vser_open(struct inode * inode, struct file * fp)
{
    switch(MINOR(inode->i_rdev)){
    default:
    case 0:
        fp->private_data = &vsfifo0;
        break;
    case 1:
        fp->private_data = &vsfifo1;
        break;
    }
    return 0;
}

static int vser_release(struct inode *inode,struct file *fp)
{
    return 0;
}
static ssize_t vser_read(struct file * fp,char __user *buf,size_t count,loff_t * pos)
{
    unsigned int copied = 0;
    struct kfifo * vsfifo = fp->private_data;
    kfifo_to_user(vsfifo,buf,count,&copied);
    return copied;
}
static ssize_t vser_write(struct file * fp,const char __user *buf,size_t count,loff_t * pos)
{
    unsigned int copied = 0;
    struct kfifo * vsfifo = fp->private_data;
    kfifo_from_user(vsfifo,buf,count,&copied);
    return copied;
}
static struct file_operations vser_ops = {
    .owner = THIS_MODULE,
    .open = vser_open,
    .release = vser_release,
    .read = vser_read,
    .write = vser_write,
};

static int __init vser_init(void)
{
    int ret;
    dev_t dev;

    dev = MKDEV(VSER_MAJOR,VSER_MINOR);
    if( register_chrdev_region(dev,VSER_DEV_CNT,VSER_DEV_NAME) )
        goto reg_err;

    cdev_init(&vsdev,&vser_ops);
    vsdev.owner = THIS_MODULE;

    if( ret = cdev_add(&vsdev,dev,VSER_DEV_CNT) )
        goto add_err;

    return 0;
add_err:
    unregister_chrdev_region(dev,VSER_DEV_CNT);
reg_err:
    return ret;
}

static void __exit vser_exit(void)
{
    dev_t dev;
    dev = MKDEV(VSER_MAJOR,VSER_MINOR);

    cdev_del(&vsdev);
    unregister_chrdev_region(dev,VSER_DEV_CNT);
}

module_init(vser_init);
module_exit(vser_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("keyens");
MODULE_DESCRIPTION("a simple char dev test");
MODULE_ALIAS("virtual-serial");