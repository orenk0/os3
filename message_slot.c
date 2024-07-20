// Declare what kind of code we want
// from the header files. Defining __KERNEL__
// and MODULE allows us to access kernel-level
// code not usually available to userspace programs.
#undef __KERNEL__
#define __KERNEL__
#undef MODULE
#define MODULE


#include <linux/kernel.h>   /* We're doing kernel work */
#include <linux/module.h>   /* Specifically, a module */
#include <linux/fs.h>       /* for register_chrdev */
#include <linux/uaccess.h>  /* for get_user and put_user */
#include <linux/string.h>   /* for memset. NOTE - not string.h!*/
#include <linux/slab.h>     /* for 3.1.4.7 */
#include <linux/types.h>    /* for reading pointers' addresses */
#include <linux/string.h>   /* for strings */

MODULE_LICENSE("GPL");

//Our custom definitions of IOCTL operations
#include "message_slot.h"



//================== BONUS AVL TREE IMPLEMENTATION FOR EFFICIENCY :) ===========================
// AVL tree implementation based on gfg:
// AVL Tree node

/*
    remove all nodes of an AVL tree
*/
void dispose(node* t)
{
    if( t != NULL )
    {
        dispose( t->left );
        dispose( t->right );
        kfree( t );
    }
}

/*
    find a specific node's key in the tree
*/
node* find(int e, node* t )
{
    if( t == NULL )
        return NULL;
    if( e < t->channel )
        return find( e, t->left );
    else if( e > t->channel )
        return find( e, t->right );
    else
        return t;
}

/*
    find minimum node's key
*/
node* find_min( node* t )
{
    if( t == NULL )
        return NULL;
    else if( t->left == NULL )
        return t;
    else
        return find_min( t->left );
}

/*
    find maximum node's key
*/
node* find_max( node* t )
{
    if( t != NULL )
        while( t->right != NULL )
            t = t->right;

    return t;
}

/*
    get the height of a node
*/
static int height( node* n )
{
    if( n == NULL )
        return -1;
    else
        return n->height;
}

/*
    get maximum value of two integers
*/
static int max1( int l, int r)
{
    if(l>r) return l;
    return r;
}

/*
    perform a rotation between a k2 node and its left child

    note: call single_rotate_with_left only if k2 node has a left child
*/

static node* single_rotate_with_left( node* k2 )
{
    node* k1 = NULL;

    k1 = k2->left;
    k2->left = k1->right;
    k1->right = k2;

    k2->height = max1( height( k2->left ), height( k2->right ) ) + 1;
    k1->height = max1( height( k1->left ), k2->height ) + 1;
    return k1; //new root
}

/*
    perform a rotation between a node (k1) and its right child

    note: call single_rotate_with_right only if
    the k1 node has a right child
*/

static node* single_rotate_with_right( node* k1 )
{
    node* k2;

    k2 = k1->right;
    k1->right = k2->left;
    k2->left = k1;

    k1->height = max1( height( k1->left ), height( k1->right ) ) + 1;
    k2->height = max1( height( k2->right ), k1->height ) + 1;

    return k2; //new root
}

/*

    perform the left-right double rotation,

    note: call double_rotate_with_left only if k3 node has
    a left child and k3's left child has a right child
*/

static node* double_rotate_with_left( node* k3 )
{
    /* Rotate between k1 and k2 */
    k3->left = single_rotate_with_right( k3->left );

    /* Rotate between K3 and k2 */
    return single_rotate_with_left( k3 );
}

/*
    perform the right-left double rotation

   notes: call double_rotate_with_right only if k1 has a
   right child and k1's right child has a left child
*/



static node* double_rotate_with_right( node* k1 )
{
    /* rotate between K3 and k2 */
    k1->right = single_rotate_with_left( k1->right );

    /* rotate between k1 and k2 */
    return single_rotate_with_right( k1 );
}

/*
    insert a new node into the tree
*/
node* insert(int e, node* t, int length, const char* message )
{
    if( t == NULL )
    {
        /* Create and return a one-node tree */
        t = (node*)kmalloc(sizeof(node), GFP_KERNEL);
        t->channel = e;
        t->height = 0;
        t->left = t->right = NULL;
        t->length = length;
        t->message = (char*)kmalloc((length + 1) * sizeof(char), GFP_KERNEL); // Allocate memory for the message
        if (t->message != NULL) {
            strncpy(t->message, message, length);
            t->message[length] = '\0'; // Null-terminate the string
        }
    }
    else if( e < t->channel )
    {
        t->left = insert( e, t->left, length, message );
        if( height( t->left ) - height( t->right ) == 2 ){
            if( e < t->left->channel )
                t = single_rotate_with_left( t );
            else
                t = double_rotate_with_left( t );
        }
    }
    else if( e > t->channel )
    {
        t->right = insert( e, t->right, length, message );
        if( height( t->right ) - height( t->left ) == 2 ){
            if( e > t->right->channel )
                t = single_rotate_with_right( t );
            else
                t = double_rotate_with_right( t );
        }
    }
    /* Else X is in the tree already; we'll do nothing */

    t->height = max1( height( t->left ), height( t->right ) ) + 1;
    return t;
}

/*
    remove a node in the tree
*/
node* delete(int e, node* t)
{
    node* temp;
    int balance;

    if (t == NULL)
        return t;

    // Perform standard BST delete
    if (e < t->channel)
    {
        t->left = delete(e, t->left);
    }
    else if (e > t->channel)
    {
        t->right = delete(e, t->right);
    }
    else
    {
        // Node with only one child or no child
        if ((t->left == NULL) || (t->right == NULL))
        {
            temp = t->left ? t->left : t->right;

            // No child case
            if (temp == NULL)
            {
                temp = t;
                t = NULL;
            }
            else // One child case
                *t = *temp; // Copy the contents of the non-empty child

            kfree(temp);
        }
        else
        {
            // Node with two children: Get the in-order successor (smallest in the right subtree)
            temp = find_min(t->right);

            // Copy the in-order successor's channel to this node
            t->channel = temp->channel;

            // Delete the in-order successor
            t->right = delete(temp->channel, t->right);
        }
    }

    // If the tree had only one node, then return
    if (t == NULL)
        return t;

    // Update height of the current node
    t->height = max1(height(t->left), height(t->right)) + 1;

    // Get the balance factor of this node to check whether this node became unbalanced
    balance = height(t->left) - height(t->right);

    // If the node becomes unbalanced, then there are 4 cases

    // Left Left Case
    if (balance > 1 && height(t->left->left) >= height(t->left->right))
        return single_rotate_with_left(t);

    // Left Right Case
    if (balance > 1 && height(t->left->left) < height(t->left->right))
    {
        t->left = single_rotate_with_right(t->left);
        return single_rotate_with_left(t);
    }

    // Right Right Case
    if (balance < -1 && height(t->right->right) >= height(t->right->left))
        return single_rotate_with_right(t);

    // Right Left Case
    if (balance < -1 && height(t->right->right) < height(t->right->left))
    {
        t->right = single_rotate_with_left(t->right);
        return single_rotate_with_right(t);
    }

    return t;
}


/*
    channel channel of a node
*/
int get(node* n)
{
    return n->channel;
}

/*
    Recursively display AVL tree or subtree
*/
void display_avl(node* t)
{
    if (t == NULL)
        return;
    printk("%s",t->message);

    if(t->left != NULL)
        printk("(L:%s)",t->left->message);
    if(t->right != NULL)
        printk("(R:%s)",t->right->message);
    printk("\n");

    display_avl(t->left);
    display_avl(t->right);
}



//END OF AVL TREE IMPLEMENTATION.

//---------------------------------------------------------------
//for each active minor save an AVL tree with channels as keys. 
//(channel numbers are saved in file->private_data)
node* channels_msgs[260];//256 + extra O(1) space.

static int device_open( struct inode* inode,
                        struct file*  file )
{
    //int minor = iminor(file_inode(file));
    //printk("Invoking device_open(%p)\n", file);
    return SUCCESS;
}
// a process which has already opened
// the device file attempts to read from it
static ssize_t device_read( struct file* file,
                            char __user* buffer,
                            size_t       length,
                            loff_t*      offset )
{
    node* root; 
    node* msg;
    char* the_message; 
    int the_length; 
    ssize_t i;
    int minor = iminor(file_inode(file));
    uintptr_t address_as_int = (uintptr_t)(file->private_data);
    unsigned long channel = (unsigned long)address_as_int;
    //error cases:
    if(channel <= 0 || file->private_data == NULL) return -EINVAL;
    
    
    root = channels_msgs[minor];
    msg = find((int)channel, root);
    //error cases:
    if(msg==NULL) return -EWOULDBLOCK;
    the_message = msg->message;
    the_length = msg->length;
    //error cases:
    if(the_message == NULL || the_length == 0) return -EWOULDBLOCK; //unnecessary but for extra caution
    if(length < the_length) return -ENOSPC;
    for ( i = 0; i < the_length && i < BUF_LEN; ++i ){
        put_user(the_message[i], &buffer[i]);
    }

    return i;


}

//---------------------------------------------------------------
// a processs which has already opened
// the device file attempts to write to it
static ssize_t device_write( struct file*       file,
                             const char __user* buffer,
                             size_t             length,
                             loff_t*            offset)
{
    
    // According to 3.1.4.10 we shouldn't save the data to the file but only in the modules' memory.

    int minor;
    uintptr_t address_as_int;
    unsigned long channel;
    node* root;
    ssize_t i;
    char the_message[BUF_LEN];
    
    minor = iminor(file_inode(file));
    address_as_int = (uintptr_t)(file->private_data);
    channel = (unsigned long)address_as_int;
    root = channels_msgs[minor];
    //node* msg = find((int)channel, root);
    

    //error cases:
    if(channel <= 0 || file->private_data == NULL) return -EINVAL;
    if(buffer == NULL) return -EINVAL;
    if(length<=0 || length > BUF_LEN) return -EMSGSIZE;
    if(strlen(buffer)!=length) return -EINVAL;
    //use get_user to read the message
    for( i = 0; i < length && i < BUF_LEN; ++i ) {
        get_user(the_message[i], &buffer[i]);
    }
    //overwrite or new node:
    root = delete((int)channel, root);//does nothing if the node is NULL
    channels_msgs[minor] = insert((int)channel, root, (int)i, the_message);
    return i;
}

//----------------------------------------------------------------
static long device_ioctl( struct   file* file,
                          unsigned int   ioctl_command_id,
                          unsigned long  ioctl_param )
{
    //need to check errors
    int minor;
    if(ioctl_param == 0 || MSG_SLOT_CHANNEL != ioctl_command_id){
        return -EINVAL;//Returning -EINVAL signals ioctl to set errno and return -1.
    }
    file->private_data = (void*) ioctl_param;
    minor = iminor(file_inode(file));
    return SUCCESS;
}


//==================== DEVICE SETUP =============================

// This structure will hold the functions to be called
// when a process does something to the device we created
struct file_operations Fops = {
  .owner	  = THIS_MODULE, 
  .read           = device_read,
  .write          = device_write,
  .open           = device_open,
  .unlocked_ioctl = device_ioctl,
};

//---------------------------------------------------------------
// Initialize the module - Register the character device
static int __init simple_init(void)
{
    //Channels are by default 0.
    

    int rc = -1;
    // Register driver capabilities. Obtain major num
    rc = register_chrdev( MAJOR_NUM, DEVICE_RANGE_NAME, &Fops );


    // Negative values signify an error
    if( rc < 0 ) {
        printk( KERN_ERR "%s registraion failed for  %d\n",
                       DEVICE_FILE_NAME, MAJOR_NUM );
        return rc;
    }

    return SUCCESS;
}

//---------------------------------------------------------------
static void __exit simple_cleanup(void)
{
    // Unregister the device
    // Should always succeed
    int minor;
    for(minor = 0 ; minor < 260 ; minor++){
        dispose(channels_msgs[minor]);
        channels_msgs[minor] = NULL;
    }
    unregister_chrdev(MAJOR_NUM, DEVICE_RANGE_NAME);
}

//---------------------------------------------------------------
module_init(simple_init);
module_exit(simple_cleanup);

//========================= END OF FILE =========================
