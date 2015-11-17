#include <net/sock.h>
#include <net/netlink.h>
#include <linux/skbuff.h>
#include <linux/string.h>

#define MY_GROUP    1

struct sock* socket;
struct sk_buff* socket_buff;

static void nl_receive_callback (struct sk_buff *skb)                                                     
{                                                                                                     
    nlmsg_free(skb);                                                                                  
}                                                                                                     

static void kernel_send_nl_msg(void)                                                                  
{                                                                                                     
    struct nlmsghdr *nlsk_mh;                                                                         
    char* msg = "hello kernel";

    socket = netlink_kernel_create(&init_net, NETLINK_USERSOCK, 1, nl_receive_callback, NULL, THIS_MODULE);                                                                                            

    socket_buff = nlmsg_new(256, GFP_KERNEL);                                                                                                                                                                                                                                                 
    nlsk_mh = nlmsg_put(socket_buff, 0, 0, NLMSG_DONE, strlen(msg), 0);                       
    NETLINK_CB(socket_buff).pid = 0;    // kernel pid                                                   
    NETLINK_CB(socket_buff).dst_group = MY_GROUP;                                                     
    strcpy(nlmsg_data(nlsk_mh), msg);                                                                

    nlmsg_multicast(socket, socket_buff, 0, MY_GROUP, GFP_KERNEL);                    

    return;                                                                                           
} 
