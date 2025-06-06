#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

#define DEVICE_NAME "mydevice"
#define BUFFER_SIZE 1024

static int major;
static char *buffer;
static size_t buffer_len = 0;
static int operation_count = 0;
static int read_count = 0;
static int write_count = 0;

static int device_open(struct inode *inode, struct file *file) {
    operation_count++;
    printk(KERN_INFO "Dispozitivul /dev/%s a fost pornit de procesul %d\n", DEVICE_NAME, current->pid);
    return 0;
}

static int device_release(struct inode *inode, struct file *file) {
    printk(KERN_INFO "Dispozitivul /dev/%s s-a stins\n", DEVICE_NAME);
    return 0;
}

static ssize_t device_read(struct file *file, char __user *user_buffer, size_t size, loff_t *offset) {
    int to_copy = min(size, (size_t)(buffer_len - *offset));
    if (to_copy <= 0) {
        printk(KERN_WARNING "Nimic de citit la offset %lld\n", *offset);
        return 0;
    }
    if (copy_to_user(user_buffer, buffer + *offset, to_copy)) {
        printk(KERN_ERR "Eroare la citirea datelor\n");
        return -EFAULT;
    }
    *offset += to_copy;
    read_count++;
    operation_count++;
    printk(KERN_INFO "Citit %d octeți la offset %lld\n", to_copy, *offset - to_copy);
    return to_copy;
}

static ssize_t device_write(struct file *file, const char __user *user_buffer, size_t size, loff_t *offset) {
    int to_copy = min(size, (size_t)(BUFFER_SIZE - *offset));
    if (to_copy <= 0) {
        printk(KERN_WARNING "Buffer plin, nu se poate scrie la offset %lld\n", *offset);
        return -ENOSPC;
    }
    if (copy_from_user(buffer + *offset, user_buffer, to_copy)) {
        printk(KERN_ERR "Eroare la scrierea datelor\n");
        return -EFAULT;
    }

    // Terminare cu NULL
    if (*offset + to_copy < BUFFER_SIZE) {
        buffer[*offset + to_copy] = '\0';
    } else {
        buffer[BUFFER_SIZE - 1] = '\0';
    }

    if (*offset + to_copy > buffer_len) {
        buffer_len = *offset + to_copy;
    }
    *offset += to_copy;
    write_count++;
    operation_count++;

    // Afișează în dmesg aplicația activă
    printk(KERN_INFO "Aplicația activă: %s\n", buffer + *offset - to_copy);
    return to_copy;
}

static loff_t device_llseek(struct file *file, loff_t offset, int whence) {
    loff_t new_pos;
    switch (whence) {
        case SEEK_SET:
            new_pos = offset;
            break;
        case SEEK_CUR:
            new_pos = file->f_pos + offset;
            break;
        case SEEK_END:
            new_pos = buffer_len + offset;
            break;
        default:
            return -EINVAL;
    }
    if (new_pos < 0 || new_pos > BUFFER_SIZE) {
        printk(KERN_WARNING "Offset invalid: %lld\n", new_pos);
        return -EINVAL;
    }
    file->f_pos = new_pos;
    printk(KERN_INFO "Offset setat la %lld\n", new_pos);
    return new_pos;
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = device_open,
    .release = device_release,
    .read = device_read,
    .write = device_write,
    .llseek = device_llseek,
};

static int __init my_module_init(void) {
    buffer = kmalloc(BUFFER_SIZE, GFP_KERNEL);
    if (!buffer) {
        printk(KERN_ERR "Eșec la alocarea bufferului\n");
        return -ENOMEM;
    }

    major = register_chrdev(0, DEVICE_NAME, &fops);
    if (major < 0) {
        printk(KERN_ERR "Nu s-a putut înregistra dispozitivul\n");
        kfree(buffer);
        return major;
    }

    printk(KERN_INFO "Modulul a pornit! Major: %d, Buffer: %d octeți\n", major, BUFFER_SIZE);
    return 0;
}

static void __exit my_module_exit(void) {
    unregister_chrdev(major, DEVICE_NAME);
    kfree(buffer);
    printk(KERN_INFO "Modulul a fost descărcat\n");
}

module_init(my_module_init);
module_exit(my_module_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Tu");
MODULE_DESCRIPTION("Modul kernel pentru detectarea aplicației active");

